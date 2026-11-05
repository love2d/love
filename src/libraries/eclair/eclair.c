/*
 * eclair - Embedded Cross-platform Library for Assistive Interface Routing
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#include "eclair.h"
#include "eclair_backend.h"

#include <stddef.h> /* NULL */

static bool g_initialized = false;
static eclair_route g_route = ECLAIR_ROUTE_PREFER_SCREEN_READER;

/* single source of truth for routing; eclair_speak(), eclair_backend_name(),
 * and eclair_current_output() all resolve through here
 */
static eclair_output resolve_output(void) {
	if (!g_initialized)
		return ECLAIR_OUTPUT_NONE;

	switch (g_route) {
		case ECLAIR_ROUTE_OFF:
			return ECLAIR_OUTPUT_NONE;

		case ECLAIR_ROUTE_SCREEN_READER_ONLY:
			if (eclair_sr_available())
				return ECLAIR_OUTPUT_SCREEN_READER;
			return ECLAIR_OUTPUT_NONE;

		case ECLAIR_ROUTE_SYNTHESIZER_ONLY:
			if (eclair_synth_available())
				return ECLAIR_OUTPUT_SYNTHESIZER;
			return ECLAIR_OUTPUT_NONE;

		case ECLAIR_ROUTE_PREFER_SCREEN_READER:
			if (eclair_sr_available())
				return ECLAIR_OUTPUT_SCREEN_READER;
			if (eclair_synth_available())
				return ECLAIR_OUTPUT_SYNTHESIZER;
			return ECLAIR_OUTPUT_NONE;
	}

	return ECLAIR_OUTPUT_NONE;
}

/* eclair_set_rate's range */
#define ECLAIR_RATE_MIN 0.5f
#define ECLAIR_RATE_MAX 4.0f

/* clamp function for rate and volume setters */
static float clamp(float value, float min, float max) {
	// use !(value > min) to handle NaN
	if (!(value > min)) return min;
	if (value > max) return max;
	return value;
}

// lifecycle functions

eclair_error eclair_init(void) {
	if (g_initialized)
		return ECLAIR_OK;

	if (!eclair_platform_init())
		return ECLAIR_ERR_BACKEND_FAILED;

	g_initialized = true;

	return ECLAIR_OK;
}

void eclair_shutdown(void) {
	if (!g_initialized)
		return;

	eclair_stop();
	eclair_platform_shutdown();

	g_initialized = false;
}

// output functions

eclair_error eclair_speak(const char *utf8, bool interrupt) {
	if (!g_initialized)
		return ECLAIR_ERR_NOT_INITIALIZED;

	if (utf8 == NULL)
		return ECLAIR_ERR_INVALID_ARG;

	switch (resolve_output()) {
		case ECLAIR_OUTPUT_SCREEN_READER: {
			bool success = eclair_sr_speak(utf8, interrupt);
			return success ? ECLAIR_OK : ECLAIR_ERR_BACKEND_FAILED;
		}
		case ECLAIR_OUTPUT_SYNTHESIZER: {
			bool success = eclair_synth_speak(utf8, interrupt);
			return success ? ECLAIR_OK : ECLAIR_ERR_BACKEND_FAILED;
		}
		case ECLAIR_OUTPUT_NONE:
			break;
	}

	if (g_route == ECLAIR_ROUTE_OFF)
		return ECLAIR_OK;

	return ECLAIR_ERR_NO_BACKEND;
}

eclair_error eclair_stop(void) {
	if (!g_initialized)
		return ECLAIR_ERR_NOT_INITIALIZED;

	// attempt to stop (best effort)
	eclair_sr_stop();
	eclair_synth_stop();
	return ECLAIR_OK;
}

// settings and introspection

void eclair_set_route(eclair_route route) {
	g_route = route;
}

void eclair_set_rate(float rate) {
	float clamped_rate = clamp(rate, ECLAIR_RATE_MIN, ECLAIR_RATE_MAX);
	if (g_initialized)
		eclair_synth_set_rate(clamped_rate);
}

void eclair_set_volume(float volume) {
	float clamped_volume = clamp(volume, 0.0f, 1.0f);
	if (g_initialized)
		eclair_synth_set_volume(clamped_volume);
}

eclair_output eclair_current_output(void) {
	return resolve_output();
}

const char *eclair_backend_name(void) {
	switch (resolve_output()) {
		case ECLAIR_OUTPUT_SCREEN_READER: return eclair_sr_name();
		case ECLAIR_OUTPUT_SYNTHESIZER: return eclair_synth_name();
		case ECLAIR_OUTPUT_NONE: break;
	}

	return NULL;
}

const char *eclair_error_string(eclair_error err) {
	switch (err) {
		case ECLAIR_OK: return "ok";
		case ECLAIR_ERR_NOT_INITIALIZED: return "eclair is not initialized";
		case ECLAIR_ERR_NO_BACKEND: return "no screen reader or synthesizer is available";
		case ECLAIR_ERR_INVALID_ARG: return "invalid argument";
		case ECLAIR_ERR_BACKEND_FAILED: return "the backend refused the call";
	}

	return "unknown error";
}
