/*
 * eclair - Windows backend: NVDA, JAWS, Narrator (screen readers), SAPI (synthesizer)
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#include "eclair_backend.h"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <objbase.h> /* CoInitializeEx */
#include <oleauto.h>
#include <sapi.h>
#include <uiautomation.h>
#include <stddef.h> /* NULL */
#include <stdlib.h>
#include <string.h> /* memcpy */
#include <wchar.h> /* wcslen */
#include <math.h> /* lroundf, logf */

/* true when our CoInitializeEx call was counted and we need to CoUninitialize later */
static bool g_com_owned = false;

static float g_rate = 1.0f;
static float g_volume = 1.0f;

static wchar_t *utf8_to_wide(const char *utf8) {
	int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, NULL, 0);
	if (count <= 0)
		return NULL;

	wchar_t *wide = (wchar_t *)malloc((size_t)count * sizeof(wchar_t));
	if (wide == NULL)
		return NULL;

	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, wide, count) != count) {
		free(wide);
		return NULL;
	}

	return wide;
}

/* ---------------------------------------------------------------
 * NVDA - nvdaControllerClient.dll, resolved at runtime
 * --------------------------------------------------------------- */

/* entry points return error_status_t (unsigned long), 0 on success */
typedef unsigned long (WINAPI *PFN_nvda_test_if_running)(void);
typedef unsigned long (WINAPI *PFN_nvda_speak_text)(const wchar_t *text);
typedef unsigned long (WINAPI *PFN_nvda_cancel_speech)(void);
typedef unsigned long (WINAPI *PFN_nvda_braille_message)(const wchar_t *message);

static HMODULE g_nvda_dll = NULL;

static PFN_nvda_test_if_running g_nvda_test_if_running = NULL;
static PFN_nvda_speak_text 			g_nvda_speak_text = NULL;
static PFN_nvda_cancel_speech		g_nvda_cancel_speech = NULL;
static PFN_nvda_braille_message	g_nvda_braille_message = NULL;

static void nvda_unload(void) {
	if (g_nvda_dll != NULL) {
		FreeLibrary(g_nvda_dll);
		g_nvda_dll = NULL;
	}

	g_nvda_test_if_running = NULL;
	g_nvda_speak_text = NULL;
	g_nvda_cancel_speech = NULL;
	g_nvda_braille_message = NULL;
}

static void nvda_load(void) {
	g_nvda_dll = LoadLibraryW(L"nvdaControllerClient.dll");
	if (g_nvda_dll == NULL)
		return;

	g_nvda_test_if_running = reinterpret_cast<PFN_nvda_test_if_running>(
		GetProcAddress(g_nvda_dll, "nvdaController_testIfRunning"));
	g_nvda_speak_text = reinterpret_cast<PFN_nvda_speak_text>(
		GetProcAddress(g_nvda_dll, "nvdaController_speakText"));
	g_nvda_cancel_speech = reinterpret_cast<PFN_nvda_cancel_speech>(
		GetProcAddress(g_nvda_dll, "nvdaController_cancelSpeech"));
	g_nvda_braille_message = reinterpret_cast<PFN_nvda_braille_message>(
		GetProcAddress(g_nvda_dll, "nvdaController_brailleMessage"));
}

static bool nvda_available(void) {
	// 0 is success, anything else means NVDA is not running
	return g_nvda_test_if_running != NULL && g_nvda_test_if_running() == 0;
}

static bool nvda_speak(const char *utf8, bool interrupt) {
	wchar_t *wide = utf8_to_wide(utf8);
	if (wide == NULL)
		return false;

	// if we are interrupting, cancel the current utterance
	if (interrupt)
		g_nvda_cancel_speech();

	bool spoke = g_nvda_speak_text(wide) == 0;
	bool brailled = g_nvda_braille_message(wide) == 0;

	free(wide);
	return spoke || brailled;
}

static bool nvda_stop(void) {
	if (g_nvda_cancel_speech == NULL)
		return false;

	return g_nvda_cancel_speech() == 0;
}

/* ---------------------------------------------------------------
 * JAWS - FreedomSci.JawsApi
 * --------------------------------------------------------------- */

static IDispatch *g_jaws = NULL;
static DISPID g_jaws_say = DISPID_UNKNOWN;
static DISPID g_jaws_stop = DISPID_UNKNOWN;
static DISPID g_jaws_run = DISPID_UNKNOWN;

static void jaws_unload(void) {
	if (g_jaws != NULL) {
		g_jaws->Release();
		g_jaws = NULL;
	}

	g_jaws_say = DISPID_UNKNOWN;
	g_jaws_stop = DISPID_UNKNOWN;
	g_jaws_run = DISPID_UNKNOWN;
}

static bool jaws_dispid(const wchar_t *name, DISPID *out) {
	OLECHAR *n = (OLECHAR *)name;
	return SUCCEEDED(g_jaws->GetIDsOfNames(IID_NULL, &n, 1, LOCALE_USER_DEFAULT, out));
}

static void jaws_load(void) {
	CLSID clsid;
	if (FAILED(CLSIDFromProgID(L"FreedomSci.JawsApi", &clsid)))
		return; // jaws not installed

	// check if jaws is installed and running
	if (FAILED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
															IID_IDispatch, (void **)&g_jaws))) {
		g_jaws = NULL;
		return;
	}

	jaws_dispid(L"SayString", &g_jaws_say);
	jaws_dispid(L"StopSpeech", &g_jaws_stop);
	jaws_dispid(L"RunFunction", &g_jaws_run);
}

static bool jaws_available(void) {
	// use window to detect JAWS is running
	return g_jaws != NULL && FindWindowW(L"JFWUI2", NULL) != NULL;
}

static bool jaws_invoke(DISPID id, const wchar_t *text, const VARIANT_BOOL *flush) {
	BSTR bstr = SysAllocString(text);
	if (bstr == NULL)
		return false;

	VARIANT args[2];
	VariantInit(&args[0]);
	VariantInit(&args[1]);

	UINT count;
	if (flush != NULL) {
		// DISPPARAMS args run backwards - rgvarg[0] is the last parameter
		args[0].vt = VT_BOOL;
		args[0].boolVal = *flush;

		args[1].vt = VT_BSTR;
		args[1].bstrVal = bstr;

		count = 2;
	} else {
		args[0].vt = VT_BSTR;
		args[0].bstrVal = bstr;

		count = 1;
	}

	DISPPARAMS params = { args, NULL, count, 0 };
	VARIANT result;
	VariantInit(&result);

	HRESULT hr = g_jaws->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT,
															DISPATCH_METHOD, &params, &result, NULL, NULL);

	bool ok = SUCCEEDED(hr) && result.vt == VT_BOOL && result.boolVal != VARIANT_FALSE;

	VariantClear(&result);
	SysFreeString(bstr);
	return ok;
}

/* using `BrailleString("...")` script expression to send text to braille devices */
static wchar_t *jaws_braille_expression(const wchar_t *text) {
	static const wchar_t prefix[] = L"BrailleString(\"";
	static const wchar_t suffix[] = L"\")";

	const size_t plen = sizeof(prefix) / sizeof(wchar_t) - 1;
	const size_t slen = sizeof(suffix) / sizeof(wchar_t) - 1;
	const size_t tlen = wcslen(text);

	wchar_t *expr = (wchar_t *)malloc((plen + tlen + slen + 1) * sizeof(wchar_t));
	if (expr == NULL)
		return NULL;

	memcpy(expr, prefix, plen * sizeof(wchar_t));
	memcpy(expr + plen, text, tlen * sizeof(wchar_t));
	memcpy(expr + plen + tlen, suffix, slen * sizeof(wchar_t));
	expr[plen + tlen + slen] = L'\0';

	for (size_t i = plen; i < plen + tlen; i++) {
		if (expr[i] == L'"')
			expr[i] = L'\'';
		else if (expr[i] == L'\r' || expr[i] == L'\n')
			expr[i] = L' ';
	}

	return expr;
}

static bool jaws_speak(const char *utf8, bool interrupt) {
	wchar_t *wide = utf8_to_wide(utf8);
	if (wide == NULL)
		return false;

	// bFlush to interrupt
	VARIANT_BOOL flush = interrupt ? VARIANT_TRUE : VARIANT_FALSE;
	bool spoke = jaws_invoke(g_jaws_say, wide, &flush);

	bool brailled = false;
	wchar_t *expr = jaws_braille_expression(wide);
	if (expr != NULL) {
		brailled = jaws_invoke(g_jaws_run, expr, NULL);
		free(expr);
	}

	free(wide);
	return spoke || brailled;
}

static bool jaws_stop(void) {
	DISPPARAMS none = { NULL, NULL, 0, 0 };
	return SUCCEEDED(g_jaws->Invoke(g_jaws_stop, IID_NULL, LOCALE_USER_DEFAULT,
																	DISPATCH_METHOD, &none, NULL, NULL, NULL));
}



/* ---------------------------------------------------------------
 * UIA - screen reader api for Narrator
 * (included on all Windows machines)
 * --------------------------------------------------------------- */

// note - we test for narrator specifically because
// detecting UIA is not possible trivially (and is used for other engines like JAWS / NVDA)
static bool narrator_available(void) {
	HANDLE narrator = OpenMutexW(SYNCHRONIZE, FALSE, L"NarratorRunning");
	if (narrator == NULL)
		return false;
	CloseHandle(narrator);

	return true;
}

// provider for UIA to tap into when the user is running Narrator
class eclair_uia_provider : public IRawElementProviderSimple {
	public:
		eclair_uia_provider(void) : m_hwnd(NULL) {}

		void set_window(HWND hwnd) {
			m_hwnd = hwnd;
		}

		/* IUnknown */

		// we are never freed, so the counts are normal - COM only
		// requires that they stay non-zero as long as the object is alive
		IFACEMETHODIMP_(ULONG) AddRef(void) {
			return 2;
		}

		IFACEMETHODIMP_(ULONG) Release(void) {
			return 1;
		}

		IFACEMETHODIMP QueryInterface(REFIID riid, void **object) {
			if (IsEqualIID(riid, IID_IUnknown) ||
					IsEqualIID(riid, IID_IRawElementProviderSimple)) {
				*object = static_cast<IRawElementProviderSimple *>(this);
			} else {
				*object = NULL;
				return E_NOINTERFACE;
			}

			AddRef();
			return S_OK;
		}

		/* IRawElementProviderSimple */

		IFACEMETHODIMP get_ProviderOptions(ProviderOptions *options) {
			*options = ProviderOptions_ServerSideProvider;
			return S_OK;
		}

		IFACEMETHODIMP GetPatternProvider(PATTERNID, IUnknown **pattern) {
			// no pattern support, we only raise events
			*pattern = NULL;
			return S_OK;
		}

		IFACEMETHODIMP GetPropertyValue(PROPERTYID id, VARIANT *value) {
			if (id == UIA_ControlTypePropertyId) {
				value->vt = VT_I4;
				value->lVal = UIA_WindowControlTypeId;
			} else if (id == UIA_NamePropertyId) {
				value->vt = VT_BSTR;
				value->bstrVal = SysAllocString(L"eclair");
			} else {
				// "empty" indicates that we should ask the host window
				value->vt = VT_EMPTY;
			}

			return S_OK;
		}

		IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple **host) {
			return UiaHostProviderFromHwnd(m_hwnd, host);
		}

	private:
		HWND m_hwnd;
};

static eclair_uia_provider g_uia_provider;

// the window we should announce speech through
static HWND uia_window(void) {
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return NULL;

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	return pid == GetCurrentProcessId() ? hwnd : NULL;
}

static bool uia_speak(const char *utf8, bool interrupt) {
	HWND hwnd = uia_window();
	if (hwnd == NULL)
		return false;

	wchar_t *wide = utf8_to_wide(utf8);
	if (wide == NULL)
		return false;

	BSTR text = SysAllocString(wide);
	free (wide);
	if (text == NULL)
		return false;

	g_uia_provider.set_window(hwnd);

	// group id for our utterances
	BSTR activity = SysAllocString(L"eclair");

	// ImportantMostRecent supersedes what is queued
	NotificationProcessing processing = interrupt
		? NotificationProcessing_ImportantMostRecent
		: NotificationProcessing_ImportantAll;

	HRESULT hr = UiaRaiseNotificationEvent(&g_uia_provider, NotificationKind_Other,
																				 processing, text, activity);

	SysFreeString(activity);
	SysFreeString(text);
	return SUCCEEDED(hr);
}

static bool uia_stop(void) {
	// UIA exposes no cancel
	return true;
}

// screen reader backends supported by windows;
// we will pick the first available one - we only expect one to be active at a time

typedef struct {
	const char *name;
	bool (*available)(void);
	bool (*speak)(const char *utf8, bool interrupt);
	bool (*stop)(void);
} eclair_sr_backend;

static const eclair_sr_backend g_sr_backends[] = {
	{ "NVDA", nvda_available, nvda_speak, nvda_stop },
	{ "JAWS", jaws_available, jaws_speak, jaws_stop },
	{ "Narrator",  narrator_available,  uia_speak,  uia_stop  },
};

static const eclair_sr_backend *sr_active(void) {
	size_t count = sizeof(g_sr_backends) / sizeof(g_sr_backends[0]);

	// as soon as we find an available screen reader, return it
	for (size_t i = 0; i < count; i++) {
		if (g_sr_backends[i].available()) {
			return &g_sr_backends[i];
		}
	}

	// could not find an available screen reader
	return NULL;
}

/* ---------------------------------------------------------------
 * SAPI - windows synthesizer that is always present
 * --------------------------------------------------------------- */

static ISpVoice *g_voice = NULL;

static void sapi_unload(void) {
	if (g_voice != NULL) {
		g_voice->Release();
		g_voice = NULL;
	}
}

static void sapi_load(void) {
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER,
																IID_ISpVoice, (void **)&g_voice);

	if (FAILED(hr))
		g_voice = NULL;
}

static bool sapi_available(void) {
	// SAPI ships with windows, so this is true on any machine where
	// CoCreateInstance succeeded
	return g_voice != NULL;
}

static bool sapi_speak(const char *utf8, bool interrupt) {
	wchar_t *wide = utf8_to_wide(utf8);
	if (wide == NULL)
		return false;

	/* ASYNC, otherwise Speak() will block until utterance finishes;
	 * NOT_XML, otherwise SAPI parses text with "<" as XML
	 */
	DWORD flags = SPF_ASYNC | SPF_IS_NOT_XML;
	if (interrupt)
		flags |= SPF_PURGEBEFORESPEAK;

	HRESULT hr = g_voice->Speak(wide, flags, NULL);

	free(wide);
	return SUCCEEDED(hr);
}

static bool sapi_stop(void) {
	if (g_voice == NULL)
		return false;

	// NULL is documented as a way to stop current utterance
	HRESULT hr = g_voice->Speak(NULL, SPF_PURGEBEFORESPEAK, NULL);
	return SUCCEEDED(hr);
}

#define SAPI_MIN_RATE_VALUE (-10)
#define SAPI_MAX_RATE_VALUE 10
#define SAPI_MAX_RATE_SPEED 3.0f /* measured speed at +10 */

/* map eclair's multiplication-based range (0.5 to 4.0) to SAPI's logarithmic range (-10 to +10) */
static long eclair_to_sapi_rate(float rate) {
	long sapi_rate = lroundf(SAPI_MAX_RATE_VALUE * logf(rate) / logf(SAPI_MAX_RATE_SPEED));

	if (sapi_rate < SAPI_MIN_RATE_VALUE) {
		return SAPI_MIN_RATE_VALUE;
	}

	if (sapi_rate > SAPI_MAX_RATE_VALUE) {
		return SAPI_MAX_RATE_VALUE;
	}

	return sapi_rate;
}

static void sapi_set_rate(float rate) {
	g_rate = rate;
	if (g_voice != NULL) {
		long sapi_rate = eclair_to_sapi_rate(rate);
		g_voice->SetRate(sapi_rate);
	}
}

static void sapi_set_volume(float volume) {
	g_volume = volume;
	if (g_voice != NULL) {
		USHORT sapi_volume = (USHORT) lroundf(volume * 100.0f);
		g_voice->SetVolume(sapi_volume);
	}
}

static const char *sapi_name(void) {
	return "SAPI";
}

// lifecycle

bool eclair_platform_init(void) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	/* S_OK - we initialized it (we'll need to uninitialize),
	 * S_FALSE - already initialized (still need to uninitialize),
	 * RPC_E_CHANGED_MODE - host chose the other apartment (no need to uninitialize)
	 */
	if (hr == RPC_E_CHANGED_MODE) {
		g_com_owned = false;
	} else if (SUCCEEDED(hr)) {
		g_com_owned = true;
	} else {
		// resource failure
		return false;
	}

	sapi_load();
	nvda_load();
	jaws_load();
	return true;
}

void eclair_platform_shutdown(void) {
	nvda_unload();
	jaws_unload();
	sapi_unload();

	if (g_com_owned) {
		CoUninitialize();
		g_com_owned = false;
	}
}

// screen readers lifecycle functions (NVDA, JAWS, Narrator)

bool eclair_sr_available(void) {
	return sr_active() != NULL;
}

bool eclair_sr_speak(const char *utf8, bool interrupt) {
	const eclair_sr_backend *sr = sr_active();
	return sr != NULL && sr->speak(utf8, interrupt);
}

bool eclair_sr_stop(void) {
	const eclair_sr_backend *sr = sr_active();
	return sr != NULL && sr->stop();
}

const char *eclair_sr_name(void) {
	const eclair_sr_backend *sr = sr_active();
	return sr != NULL ? sr->name : NULL;
}

// synthesizer lifecycle functions (SAPI)

bool eclair_synth_available(void) {
	return sapi_available();
}

bool eclair_synth_speak(const char *utf8, bool interrupt) {
	return sapi_speak(utf8, interrupt);
}

bool eclair_synth_stop(void) {
	return sapi_stop();
}

void eclair_synth_set_rate(float rate) {
	sapi_set_rate(rate);
}

void eclair_synth_set_volume(float volume) {
	sapi_set_volume(volume);
}

const char *eclair_synth_name(void) {
	return sapi_available() ? sapi_name() : NULL;
}


#else
// Not a windows platform
typedef int eclair_windows_unused_translation_unit;
#endif /* _WIN32 */
