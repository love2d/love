/**
 * Copyright (c) 2006-2026 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_SPEECH_SPEECH_H
#define LOVE_SPEECH_SPEECH_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"

namespace love
{
namespace speech
{

class Speech : public Module
{
public:

	enum Route
	{
		ROUTE_OFF,
		ROUTE_SCREEN_READER_ONLY,
		ROUTE_PREFER_SCREEN_READER,
		ROUTE_SYNTHESIZER_ONLY,
		ROUTE_MAX_ENUM
	};

	enum Output
	{
		OUTPUT_NONE,
		OUTPUT_SCREEN_READER,
		OUTPUT_SYNTHESIZER,
		OUTPUT_MAX_ENUM
	};

	Speech();
	virtual ~Speech();

	/**
	 * Sends text to a Screen Reader or Text to Speech Synthesizer to be read aloud.
	 * @param text the text to read out (or display on accessible hardware).
	 * @param interrupt whether we should attempt to stop a previously sent message (default false).
	 * @return whether we were able to successfully send the message to the connected backend.
	 **/
	bool speak(const char *text, bool interrupt);

	/**
	 * Stop the current message being read aloud (not available on all platforms).
	 **/
	void stop();

	/**
	 * Specify whether text should be sent to only screen readers or synthesizers.
	 * Defaults to "preferscreenreader", which connects to screen readers when
	 * enabled, and synthesizer as a fallback.
	 * @param route which devices we should try to route to.
	 **/
	void setRoute(Route route);

	/**
	 * Gets which backend type we would actively send speech to (Screen reader or
	 * synthesizer).
	 * @return The backend type.
	 **/
	Output getOutput() const;

	/**
	 * Gets the name of the backend that we would actively route to.
	 * @return The backend name.
	 **/
	const char *getBackendName() const;

	/**
	 * Set how fast speech should be read by text to speech synthesizers.
	 * Does not apply to screen readers. Defaults to 1.0 (default speed).
	 * 0.5 roughly corresponds to half speed, 2.0 is twice as fast.
	 * Exact rates vary by platform and voices. Some platforms start with a
	 * default that is not 1.0 (namely Android).
	 * @param rate A value between 0.5 and 4.0.
	 **/
	void setRate(float rate);

	/**
	 * Set how loud speech should be when read by a text to speech synthesizer.
	 * Does not apply to screen readers. Defaults to 1.0 (max volume).
	 * @param volume A value between 0.0 and 1.0
	 **/
	void setVolume(float volume);

	STRINGMAP_CLASS_DECLARE(Route);
	STRINGMAP_CLASS_DECLARE(Output);

}; // Speech

} // speech
} // love

#endif // LOVE_SPEECH_SPEECH_H
