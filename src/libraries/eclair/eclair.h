/*
 * eclair - Embedded Cross-platform Library for Assistive Interface Routing
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

#ifndef ECLAIR_H
#define ECLAIR_H

#define ECLAIR_VERSION_MAJOR 0
#define ECLAIR_VERSION_MINOR 3
#define ECLAIR_VERSION_PATCH 1

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// export public functions
#if defined(_WIN32)
	#define ECLAIR_API __declspec(dllexport)
#else
	#define ECLAIR_API __attribute__((visibility("default")))
#endif

typedef enum {
	// no error - everything good!
	ECLAIR_OK = 0,

	// a call was made before eclair_init(), or after eclair_shutdown()
	ECLAIR_ERR_NOT_INITIALIZED,

	// nothing on this system can speak - no screen reader or TTS
	ECLAIR_ERR_NO_BACKEND,

	// a NULL pointer where one was not allowed
	ECLAIR_ERR_INVALID_ARG,

	// a backend (screen reader or tts) failed to respond
	ECLAIR_ERR_BACKEND_FAILED
} eclair_error;

typedef enum {
	// don't send text to any backend
	ECLAIR_ROUTE_OFF,

	// screen readers and assistive technology only;
	// usually installed by the user: NVDA, VoiceOver, Orca, JAWS, etc...
	ECLAIR_ROUTE_SCREEN_READER_ONLY,

	// screen readers first, and synthesizers as fallback;
	// good default that prefers the user installed application,
	// but will speak through the built-in TTS otherwise
	ECLAIR_ROUTE_PREFER_SCREEN_READER,

	// use built-in Text-to-Speech, ignore Screen Reader even if it could be present;
	// (this is required in scenarios when we need to route to TTS, but aren't
	// sure if a screen reader is present)
	ECLAIR_ROUTE_SYNTHESIZER_ONLY
} eclair_route;

typedef enum {
	// no backend routable
	ECLAIR_OUTPUT_NONE,

	// assistive technology routable
	ECLAIR_OUTPUT_SCREEN_READER,

	// synthesizer routable (built-in Text-to-Speech);
	// this usually means options like rate and volume can be set
	ECLAIR_OUTPUT_SYNTHESIZER
} eclair_output;

/* start eclair. idempotent; returns ECLAIR_OK on success;
 * ECLAIR_ERR_BACKEND_FAILED if the platform could not start
 */
ECLAIR_API eclair_error eclair_init(void);

/* stop eclair and release everything; silences any speech in-progress */
ECLAIR_API void eclair_shutdown(void);

/* speak text, and send it to a routable screen reader or synthesizer;
 *
 * 'interrupt' - ask to cut off whatever is currently being spoken (best-effort)
 *
 * ECLAIR_ERR_INVALID_ARG if utf8 is NULL.
 * ECLAIR_ERR_NO_BACKEND if no routable backend is available (and one was intended)
 * ECLAIR_OK if everything is good
 */
ECLAIR_API eclair_error eclair_speak(const char *utf8, bool interrupt);

/* stop speech in progress (best effort, not always available) */
ECLAIR_API eclair_error eclair_stop(void);

/* choose where text may go;
 * defaults to ECLAIR_ROUTE_PREFER_SCREEN_READER
 */
ECLAIR_API void eclair_set_route(eclair_route route);

/* set speech rate from 0.5 (roughly half speed) to 4.0 (roughly four times speed);
 * actual speeds vary by platform and voice; the default is 1.0;
 * only applies to synthesizers (screen readers have their own rate);
 * some platforms (Android) start with a platform defined rate instead of the 1.0 default
 */
ECLAIR_API void eclair_set_rate(float rate);

/* set speech volume, 0.0 silent, 1.0 full, default 1.0;
 * like rate only applies to synthesizers, not screen readers
 */
ECLAIR_API void eclair_set_volume(float volume);

/* return what kind of device eclair will send text to,
 * either synthesizer or screen reader; mostly important for
 * exposing whether controls like set_volume and set_rate should
 * be made available
 */
ECLAIR_API eclair_output eclair_current_output(void);

/* name of the active backend; NULL when there is none;
 * not intended for branching (use eclair_current_output instead)
 */
ECLAIR_API const char *eclair_backend_name(void);

/* short english description of an error code */
ECLAIR_API const char *eclair_error_string(eclair_error err);

#ifdef __cplusplus
}
#endif

#endif /* ECLAIR_H */
