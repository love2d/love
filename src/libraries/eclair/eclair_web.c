/*
 * eclair - Web backend: ariaNotify (screen reader), Web Speech (synthesizer)
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#include "eclair_backend.h"

#if defined(__EMSCRIPTEN__)

// functions we want exposed and available in eclair_web.js
extern bool eclair_js_sr_speak(const char *utf8, bool interrupt);
extern bool eclair_js_synth_available(void);
extern bool eclair_js_synth_speak(const char *utf8, bool interrupt);
extern bool eclair_js_synth_stop(void);
extern void eclair_js_synth_set_rate(float rate);
extern void eclair_js_synth_set_volume(float volume);

/* ---------------------------------------------------------------
 * lifecycle
 * --------------------------------------------------------------- */

bool eclair_platform_init(void) {
	// nothing to do here, always return true
	return true;
}

void eclair_platform_shutdown(void) {
	// nothing to do here
}

/* ---------------------------------------------------------------
 * screen reader route - ariaNotify
 * --------------------------------------------------------------- */

bool eclair_sr_available(void) {
	// we can't determine from the browser if a screen reader is running, so by
	// default we assume one could be running.
	return true;
}

bool eclair_sr_speak(const char *utf8, bool interrupt) {
	return eclair_js_sr_speak(utf8, interrupt);
}

bool eclair_sr_stop(void) {
	// no-op
	return true;
}

const char *eclair_sr_name(void) {
	return "ariaNotify";
}

/* ---------------------------------------------------------------
 * synthesizer route - Web Speech
 * --------------------------------------------------------------- */

bool eclair_synth_available(void) {
	return eclair_js_synth_available();
}

bool eclair_synth_speak(const char *utf8, bool interrupt) {
	return eclair_js_synth_speak(utf8, interrupt);
}

bool eclair_synth_stop(void) {
	return eclair_js_synth_stop();
}

/* SpeechSynthesisUtterance.rate is already a speed multiplier (0.1 to 10, 1.0 is normal),
 * so eclair's range passes straight through (note: measured speeds vary by voice and browser) */
void eclair_synth_set_rate(float rate) {
	eclair_js_synth_set_rate(rate);
}

void eclair_synth_set_volume(float volume) {
	eclair_js_synth_set_volume(volume);
}

const char *eclair_synth_name(void) {
	return "Web Speech";
}

#else
// Not an emscripten platform
#endif /* __EMSCRIPTEN__ */
