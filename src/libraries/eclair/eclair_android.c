/*
 * eclair - Android backend: AccessibilityManager (screen reader), TextToSpeech (synthesizer)
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#include "eclair_backend.h"

#if defined(__ANDROID__)

#include <jni.h>
#include <stddef.h> /* NULL, size_t */
#include <string.h> /* strlen */

#define ECLAIR_JAVA_CLASS "org/eclair/Eclair"

static JavaVM *g_vm = NULL;
static jclass g_eclair = NULL;

static jmethodID g_init = NULL;
static jmethodID g_shutdown = NULL;
static jmethodID g_sr_available = NULL;
static jmethodID g_sr_speak = NULL;
static jmethodID g_sr_stop = NULL;
static jmethodID g_synth_available = NULL;
static jmethodID g_synth_speak = NULL;
static jmethodID g_synth_stop = NULL;
static jmethodID g_synth_set_rate = NULL;
static jmethodID g_synth_set_volume = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv *env = NULL;
	jclass local = NULL;
	size_t i;

	static const struct {
		jmethodID *id;
		const char *name;
		const char *signature;
	} methods[] = {
		{ &g_init, "init", "()Z" },
		{ &g_shutdown, "shutdown", "()V" },
		{ &g_sr_available, "srAvailable", "()Z" },
		{ &g_sr_speak, "srSpeak", "([BZ)Z" },
		{ &g_sr_stop, "srStop", "()Z" },
		{ &g_synth_available, "synthAvailable", "()Z" },
		{ &g_synth_speak, "synthSpeak", "([BZ)Z"},
		{ &g_synth_stop, "synthStop", "()Z" },
		{ &g_synth_set_rate, "synthSetRate", "(F)V" },
		{ &g_synth_set_volume, "synthSetVolume", "(F)V" }
	};

	(void) reserved;

	if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	}

	// confirm that app's loader was able hit our calling context
	local = (*env)->FindClass(env, ECLAIR_JAVA_CLASS);
	if (local == NULL) {
		(*env)->ExceptionClear(env);
		return JNI_ERR;
	}

	// move eclair from a local ref to a global one
	g_eclair = (*env)->NewGlobalRef(env, local);
	(*env)->DeleteLocalRef(env, local);

	for (i = 0; i < sizeof(methods) / sizeof(methods[0]); i++) {
		*methods[i].id = (*env)->GetStaticMethodID(
			env, g_eclair, methods[i].name, methods[i].signature
		);

		if (*methods[i].id == NULL) {
			(*env)->ExceptionClear(env);
			(*env)->DeleteGlobalRef(env, g_eclair);
			g_eclair = NULL;
			return JNI_ERR;
		}
	}

	g_vm = vm;
	return JNI_VERSION_1_6;
}

static JNIEnv *eclair_env(void) {
	JNIEnv *env = NULL;

	if (g_vm == NULL) {
		return NULL;
	}

	if ((*g_vm)->GetEnv(g_vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		return NULL;
	}

	return env;
}

static bool eclair_failed(JNIEnv *env) {
	if (!(*env)->ExceptionCheck(env)) {
		return false;
	}

	(*env)->ExceptionClear(env);
	return true;
}

static bool eclair_call_bool(jmethodID method) {
	JNIEnv *env = eclair_env();
	jboolean ok;

	if (env == NULL || method == NULL) {
		return false;
	}

	ok = (*env)->CallStaticBooleanMethod(env, g_eclair, method);

	if (eclair_failed(env)) {
		return false;
	}

	return ok == JNI_TRUE;
}

static void eclair_call_float(jmethodID method, float value) {
	JNIEnv *env =  eclair_env();

	if (env == NULL || method == NULL) {
		return;
	}

	(*env)->CallStaticVoidMethod(env, g_eclair, method, (jfloat) value);
	eclair_failed(env);
}

static bool eclair_speak_via(jmethodID method, const char *utf8, bool interrupt) {
	JNIEnv *env = eclair_env();
	jbyteArray bytes;
	jboolean ok;
	jsize len;

	if (env == NULL || method == NULL) {
		return false;
	}

	len = (jsize) strlen(utf8);

	bytes = (*env)->NewByteArray(env, len);

	(*env)->SetByteArrayRegion(env, bytes, 0, len, (const jbyte *) utf8);

	ok = (*env)->CallStaticBooleanMethod(
		env, g_eclair, method, bytes,
		interrupt ? JNI_TRUE : JNI_FALSE
	);

	if (eclair_failed(env)) {
		ok = JNI_FALSE;
	}

	// ensure this reference is cleared
	(*env)->DeleteLocalRef(env, bytes);

	return ok == JNI_TRUE;
}

/* ---------------------------------------------------------------
 * lifecycle
 * --------------------------------------------------------------- */

bool eclair_platform_init(void) {
	if (eclair_env() == NULL) {
		return false;
	}

	return eclair_call_bool(g_init);
}

void eclair_platform_shutdown(void) {
	JNIEnv *env = eclair_env();

	if (env == NULL || g_shutdown == NULL) {
		return;
	}

	(*env)->CallStaticVoidMethod(env, g_eclair, g_shutdown);
	eclair_failed(env);
}

/* ---------------------------------------------------------------
 * screen reader route - AccessibilityManager
 * --------------------------------------------------------------- */

bool eclair_sr_available(void) {
	return eclair_call_bool(g_sr_available);
}

bool eclair_sr_speak(const char *utf8, bool interrupt) {
	return eclair_speak_via(g_sr_speak, utf8, interrupt);
}

bool eclair_sr_stop(void) {
	return eclair_call_bool(g_sr_stop);
}

const char *eclair_sr_name(void) {
	return "AccessibilityManager";
}

/* ---------------------------------------------------------------
 * synthesizer route - TextToSpeech
 * --------------------------------------------------------------- */

bool eclair_synth_available(void) {
	return eclair_call_bool(g_synth_available);
}

bool eclair_synth_speak(const char *utf8, bool interrupt) {
	return eclair_speak_via(g_synth_speak, utf8, interrupt);
}

bool eclair_synth_stop(void) {
	return eclair_call_bool(g_synth_stop);
}

#define ANDROID_MIN_RATE_VALUE 0.25f
#define ANDROID_MIN_RATE_SPEED 0.53f /* measured at 0.25 */

/* map eclair's multiplication-based range (0.5 to 4.0) to setSpeechRate's multiplier;
 * Google's engine honours rates above 1.0 exactly, but compresses rates below 1.0 */
static float eclair_to_android_rate(float rate) {
	if (rate >= 1.0f) {
		return rate;
	}

	float android_rate = 1.0f + (1.0f - ANDROID_MIN_RATE_VALUE) * (rate - 1.0f) / (1.0f - ANDROID_MIN_RATE_SPEED);

	if (android_rate < ANDROID_MIN_RATE_VALUE) {
		return ANDROID_MIN_RATE_VALUE;
	}

	return android_rate;
}

void eclair_synth_set_rate(float rate) {
	eclair_call_float(g_synth_set_rate, eclair_to_android_rate(rate));
}

void eclair_synth_set_volume(float volume) {
	eclair_call_float(g_synth_set_volume, volume);
}

const char *eclair_synth_name(void) {
	return "TextToSpeech";
}

#else
// Not an android platform
#endif /* __ANDROID__ */
