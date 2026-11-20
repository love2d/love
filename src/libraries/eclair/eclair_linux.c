/*
 * eclair - Linux backend: Orca (screen reader), Speech Dispatcher (synthesizer)
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#include "eclair_backend.h"

#if defined(__linux__)

#include <dlfcn.h>
#include <stddef.h> /* NULL */
#include <math.h> /* lroundf */

#define ECLAIR_DLSYM(lib, fn, name) (*(void **)(&(fn)) = dlsym((lib), (name)))

static float g_rate = 1.0f;
static float g_volume = 1.0f;

/* ---------------------------------------------------------------
 * Speech Dispatcher - libspeechd.so.2, resolved at runtime
 * --------------------------------------------------------------- */

typedef struct SPDConnection SPDConnection;

// SPDConnectionMode - SINGLE means libspeechd spawns no reader thread
#define ECLAIR_SPD_MODE_SINGLE 0

// SPDPriority - TEXT is superseded by the next message of any priority
#define ECLAIR_SPD_TEXT 3

typedef SPDConnection *(*PFN_spd_open)(const char *client_name, const char *connection_name, const char *user_name, int mode);

typedef void (*PFN_spd_close)(SPDConnection *conn);
typedef int (*PFN_spd_say)(SPDConnection *conn, int priority, const char *text);
typedef int (*PFN_spd_cancel)(SPDConnection *conn);
typedef int (*PFN_spd_set_voice_rate)(SPDConnection *conn, int rate);
typedef int (*PFN_spd_set_volume)(SPDConnection *conn, int volume);

static void *g_speechd_lib = NULL;
static SPDConnection *g_spd = NULL;

static PFN_spd_open g_spd_open = NULL;
static PFN_spd_close g_spd_close = NULL;
static PFN_spd_say g_spd_say = NULL;
static PFN_spd_cancel g_spd_cancel = NULL;
static PFN_spd_set_voice_rate g_spd_set_voice_rate = NULL;
static PFN_spd_set_volume g_spd_set_volume = NULL;

static void speechd_unload(void) {
	if (g_spd != NULL) {
		g_spd_close(g_spd);
		g_spd = NULL;
	}

	if (g_speechd_lib != NULL) {
		dlclose(g_speechd_lib);
		g_speechd_lib = NULL;
	}

	g_spd_open = NULL;
	g_spd_close = NULL;
	g_spd_say = NULL;
	g_spd_cancel = NULL;
	g_spd_set_voice_rate = NULL;
	g_spd_set_volume = NULL;
}

static void speechd_load(void) {
	// point to versioned soname
	g_speechd_lib = dlopen("libspeechd.so.2", RTLD_LAZY | RTLD_LOCAL);
	if (g_speechd_lib == NULL)
		return;

	ECLAIR_DLSYM(g_speechd_lib, g_spd_open, "spd_open");
	ECLAIR_DLSYM(g_speechd_lib, g_spd_close, "spd_close");
	ECLAIR_DLSYM(g_speechd_lib, g_spd_say, "spd_say");
	ECLAIR_DLSYM(g_speechd_lib, g_spd_cancel, "spd_cancel");
	ECLAIR_DLSYM(g_speechd_lib, g_spd_set_voice_rate, "spd_set_voice_rate");
	ECLAIR_DLSYM(g_speechd_lib, g_spd_set_volume, "spd_set_volume");

	// connect to speech-dispatcher (spawns daemon if it is not running)
	g_spd = g_spd_open("eclair", "eclair", NULL, ECLAIR_SPD_MODE_SINGLE);
}

static bool speechd_available(void) {
	return g_spd != NULL;
}

static bool speechd_speak(const char *utf8, bool interrupt) {
	if (g_spd == NULL)
		return false;

	// cancel clears the queue and current utterance
	if (interrupt)
		g_spd_cancel(g_spd);

	// if the string is empty, simply return
	// (this otherwises causes the host application to crash)
	if (utf8[0] == '\0')
		return true;

	return g_spd_say(g_spd, ECLAIR_SPD_TEXT, utf8) != -1;
}

static bool speechd_stop(void) {
	if (g_spd == NULL)
		return false;

	return g_spd_cancel(g_spd) == 0;
}

#define SPEECHD_MIN_RATE_VALUE (-100)
#define SPEECHD_MAX_RATE_VALUE 100
#define SPEECHD_MIN_RATE_SPEED 0.5f /* measured at -100 */
#define SPEECHD_MAX_RATE_SPEED 2.55f /* measured at +100 */

/* map eclair's multiplication-based range (0.5 to 4.0) to Speech Dispatcher's -100 to +100;
 * espeak-ng maps -100 / 0 / +100 linearly onto 80 / 170 / 449 wpm, so mostly a straight line
 * before and after 1.0 */
static int eclair_to_speechd_rate(float rate) {
	float speechd_rate;
	if (rate < 1.0f) {
		speechd_rate = SPEECHD_MAX_RATE_VALUE * (rate - 1.0f) / (1.0f - SPEECHD_MIN_RATE_SPEED);
	} else {
		speechd_rate = SPEECHD_MAX_RATE_VALUE * (rate - 1.0f) / (SPEECHD_MAX_RATE_SPEED - 1.0f);
	}

	if (speechd_rate < SPEECHD_MIN_RATE_VALUE) {
		return SPEECHD_MIN_RATE_VALUE;
	}

	if (speechd_rate > SPEECHD_MAX_RATE_VALUE) {
		return SPEECHD_MAX_RATE_VALUE;
	}

	return (int) lroundf(speechd_rate);
}

static void speechd_set_rate(float rate) {
	g_rate = rate;
	if (g_spd != NULL) {
		int speechd_rate = eclair_to_speechd_rate(rate);
		g_spd_set_voice_rate(g_spd, speechd_rate);
	}
}

static void speechd_set_volume(float volume) {
	g_volume = volume;
	if (g_spd != NULL) {
		// range is -100 to +100, +100 is the default volume
		int spd_volume = (int) lroundf(volume * 200.0f - 100.0f);
		g_spd_set_volume(g_spd, spd_volume);
	}
}

static const char *speechd_name(void) {
	return "Speech Dispatcher";
}

/* ---------------------------------------------------------------
 * Orca - org.gnome.Orca.Service over libdbus-1
 * --------------------------------------------------------------- */

typedef struct DBusConnection DBusConnection;
typedef struct DBusMessage DBusMessage;
typedef struct DBusError DBusError;
typedef unsigned int dbus_bool_t;
typedef unsigned int dbus_uint32_t;

// DBusBusType
#define ECLAIR_DBUS_BUS_SESSION 0

// d-bus type codes are ascii characters
#define ECLAIR_DBUS_TYPE_INVALID ((int)'\0')
#define ECLAIR_DBUS_TYPE_BOOLEAN ((int)'b')
#define ECLAIR_DBUS_TYPE_STRING ((int)'s')

// Orca service objects
#define ECLAIR_ORCA_NAME "org.gnome.Orca.Service"
#define ECLAIR_ORCA_PATH "/org/gnome/Orca/Service"
#define ECLAIR_ORCA_SPEECH_PATH "/org/gnome/Orca/Service/SpeechAndVerbosityManager"
#define ECLAIR_ORCA_MODULE "org.gnome.Orca.Module"

typedef DBusConnection *(*PFN_dbus_bus_get_private)(int type, DBusError *error);
typedef void (*PFN_dbus_connection_set_exit_on_disconnect)(DBusConnection *conn, dbus_bool_t exit_on_disconnect);
typedef void (*PFN_dbus_connection_close)(DBusConnection *conn);
typedef void (*PFN_dbus_connection_unref)(DBusConnection *conn);
typedef dbus_bool_t (*PFN_dbus_bus_name_has_owner)(DBusConnection *conn, const char *name, DBusError *error);
typedef DBusMessage *(*PFN_dbus_message_new_method_call)(const char *destination, const char *path, const char *iface, const char *method);
typedef dbus_bool_t (*PFN_dbus_message_append_args)(DBusMessage *msg, int first_arg_type, ...);
typedef void (*PFN_dbus_message_set_no_reply)(DBusMessage *msg, dbus_bool_t no_reply);
typedef dbus_bool_t (*PFN_dbus_connection_send)(DBusConnection *conn, DBusMessage *msg, dbus_uint32_t *serial);
typedef void (*PFN_dbus_connection_flush)(DBusConnection *conn);
typedef void (*PFN_dbus_message_unref)(DBusMessage *msg);
typedef dbus_bool_t (*PFN_dbus_validate_utf8)(const char *utf8, DBusError *error);

static void *g_dbus_lib = NULL;
static DBusConnection *g_dbus = NULL;

static PFN_dbus_bus_get_private g_dbus_bus_get_private = NULL;
static PFN_dbus_connection_set_exit_on_disconnect g_dbus_set_exit_on_disconnect = NULL;
static PFN_dbus_connection_close g_dbus_connection_close = NULL;
static PFN_dbus_connection_unref g_dbus_connection_unref = NULL;
static PFN_dbus_bus_name_has_owner g_dbus_bus_name_has_owner = NULL;
static PFN_dbus_message_new_method_call g_dbus_message_new_method_call = NULL;
static PFN_dbus_message_append_args g_dbus_message_append_args = NULL;
static PFN_dbus_message_set_no_reply g_dbus_message_set_no_reply = NULL;
static PFN_dbus_connection_send g_dbus_connection_send = NULL;
static PFN_dbus_connection_flush g_dbus_connection_flush = NULL;
static PFN_dbus_message_unref g_dbus_message_unref = NULL;
static PFN_dbus_validate_utf8 g_dbus_validate_utf8 = NULL;

// libdbus aborts on a non UFT-8 string, so check it is valid befor speaking
static bool utf8_valid(const char *utf8) {
	return g_dbus_validate_utf8(utf8, NULL) != 0;
}

static void orca_unload(void) {
	if (g_dbus != NULL) {
		g_dbus_connection_close(g_dbus);
		g_dbus_connection_unref(g_dbus);
		g_dbus = NULL;
	}

	if (g_dbus_lib != NULL) {
		dlclose(g_dbus_lib);
		g_dbus_lib = NULL;
	}

	g_dbus_bus_get_private = NULL;
	g_dbus_set_exit_on_disconnect = NULL;
	g_dbus_connection_close = NULL;
	g_dbus_connection_unref = NULL;
	g_dbus_bus_name_has_owner = NULL;
	g_dbus_message_new_method_call = NULL;
	g_dbus_message_append_args = NULL;
	g_dbus_message_set_no_reply = NULL;
	g_dbus_connection_send = NULL;
	g_dbus_connection_flush = NULL;
	g_dbus_message_unref = NULL;
	g_dbus_validate_utf8 = NULL;
}

static void orca_load(void) {
	g_dbus_lib = dlopen("libdbus-1.so.3", RTLD_LAZY | RTLD_LOCAL);
	if (g_dbus_lib == NULL)
		return;

	ECLAIR_DLSYM(g_dbus_lib, g_dbus_bus_get_private, "dbus_bus_get_private");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_set_exit_on_disconnect, "dbus_connection_set_exit_on_disconnect");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_connection_close, "dbus_connection_close");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_connection_unref, "dbus_connection_unref");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_bus_name_has_owner, "dbus_bus_name_has_owner");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_message_new_method_call, "dbus_message_new_method_call");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_message_append_args, "dbus_message_append_args");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_message_set_no_reply, "dbus_message_set_no_reply");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_connection_send, "dbus_connection_send");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_connection_flush, "dbus_connection_flush");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_message_unref, "dbus_message_unref");
	ECLAIR_DLSYM(g_dbus_lib, g_dbus_validate_utf8, "dbus_validate_utf8");

	if (g_dbus_bus_get_private == NULL || g_dbus_validate_utf8 == NULL) {
		orca_unload();
		return;
	}

	g_dbus = g_dbus_bus_get_private(ECLAIR_DBUS_BUS_SESSION, NULL);
	if (g_dbus == NULL)
		return;

	// do not kill host program when killed
	g_dbus_set_exit_on_disconnect(g_dbus, 0);
}

static bool orca_available(void) {
	return g_dbus != NULL && g_dbus_bus_name_has_owner(g_dbus, ECLAIR_ORCA_NAME, NULL) != 0;
}

static bool orca_send(DBusMessage *msg) {
	bool sent;

	g_dbus_message_set_no_reply(msg, 1);
	sent = g_dbus_connection_send(g_dbus, msg, NULL) != 0;
	g_dbus_connection_flush(g_dbus);
	g_dbus_message_unref(msg);

	return sent;
}

static bool orca_interrupt(void) {
	const char *command = "InterruptSpeech";
	dbus_bool_t notify_user = 0;

	DBusMessage *msg = g_dbus_message_new_method_call(ECLAIR_ORCA_NAME, ECLAIR_ORCA_SPEECH_PATH, ECLAIR_ORCA_MODULE, "ExecuteCommand");
	if (msg == NULL)
		return false;

	if (!g_dbus_message_append_args(msg, ECLAIR_DBUS_TYPE_STRING, &command, ECLAIR_DBUS_TYPE_BOOLEAN, &notify_user, ECLAIR_DBUS_TYPE_INVALID)) {
		g_dbus_message_unref(msg);
		return false;
	}

	return orca_send(msg);
}

static bool orca_speak(const char *utf8, bool interrupt) {
	DBusMessage *msg;

	if (g_dbus == NULL)
		return false;

	if (!utf8_valid(utf8))
		return false;

	if (interrupt)
		orca_interrupt();

	msg = g_dbus_message_new_method_call(ECLAIR_ORCA_NAME, ECLAIR_ORCA_PATH, ECLAIR_ORCA_NAME, "PresentMessage");
	if (msg == NULL)
		return false;

	if (!g_dbus_message_append_args(msg, ECLAIR_DBUS_TYPE_STRING, &utf8, ECLAIR_DBUS_TYPE_INVALID)) {
		g_dbus_message_unref(msg);
		return false;
	}

	return orca_send(msg);
}

static bool orca_stop(void) {
	if (g_dbus == NULL)
		return false;

	return orca_interrupt();
}

static const char *orca_name(void) {
	return "Orca";
}

// lifecycle

bool eclair_platform_init(void) {
	speechd_load();
	orca_load();

	return true;
}

void eclair_platform_shutdown(void) {
	orca_unload();
	speechd_unload();
}

// screen reader lifecycle functions (Orca)

bool eclair_sr_available(void) {
	return orca_available();
}

bool eclair_sr_speak(const char *utf8, bool interrupt) {
	return orca_speak(utf8, interrupt);
}

bool eclair_sr_stop(void) {
	return orca_stop();
}

const char *eclair_sr_name(void) {
	return orca_available() ? orca_name() : NULL;
}

// synthesizer lifecycle functions (Speech Dispatcher)

bool eclair_synth_available(void) {
	return speechd_available();
}

bool eclair_synth_speak(const char *utf8, bool interrupt) {
	return speechd_speak(utf8, interrupt);
}

bool eclair_synth_stop(void) {
	return speechd_stop();
}

void eclair_synth_set_rate(float rate) {
	speechd_set_rate(rate);
}

void eclair_synth_set_volume(float volume) {
	speechd_set_volume(volume);
}

const char *eclair_synth_name(void) {
	return speechd_available() ? speechd_name() : NULL;
}

#else
// not a linux platform
typedef int eclair_linux_unused_translation_unit;
#endif /* __linux__ */
