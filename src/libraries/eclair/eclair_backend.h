/*
 * eclair - internal backend contract
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 *
 * platform files should implement the functions below
 */

#ifndef ECLAIR_BACKEND_H
#define ECLAIR_BACKEND_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* called from eclair_init() / eclair_shutdown() */
bool eclair_platform_init(void);
void eclair_platform_shutdown(void);

/* screen reader specific functions */
bool eclair_sr_available(void);
bool eclair_sr_speak(const char *utf8, bool interrupt);
bool eclair_sr_stop(void);
const char *eclair_sr_name(void);

/* synthesizer specific functions */
bool eclair_synth_available(void);
bool eclair_synth_speak(const char *utf8, bool interrupt);
bool eclair_synth_stop(void);
void eclair_synth_set_rate(float rate);
void eclair_synth_set_volume(float volume);
const char *eclair_synth_name(void);

#ifdef __cplusplus
}
#endif

#endif /* ECLAIR_BACKEND_H */
