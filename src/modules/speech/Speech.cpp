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

// LOVE
#include "Speech.h"

// eclair
#include "libraries/eclair/eclair.h"

namespace love
{
namespace speech
{

Speech::Speech()
	: Module(M_SPEECH, "love.speech")
{
	eclair_init();
}

Speech::~Speech()
{
	eclair_shutdown();
}

bool Speech::speak(const char *text, bool interrupt)
{
	eclair_error response = eclair_speak(text, interrupt);
	return response == ECLAIR_OK;
}

void Speech::stop()
{
	eclair_stop();
}

void Speech::setRoute(Route route)
{
	switch (route)
	{
	case ROUTE_OFF:
		eclair_set_route(ECLAIR_ROUTE_OFF);
		break;
	case ROUTE_SCREEN_READER_ONLY:
		eclair_set_route(ECLAIR_ROUTE_SCREEN_READER_ONLY);
		break;
	case ROUTE_PREFER_SCREEN_READER:
		eclair_set_route(ECLAIR_ROUTE_PREFER_SCREEN_READER);
		break;
	case ROUTE_SYNTHESIZER_ONLY:
		eclair_set_route(ECLAIR_ROUTE_SYNTHESIZER_ONLY);
		break;
	default:
		break;
	}
}

Speech::Output Speech::getOutput() const
{
	switch (eclair_current_output())
	{
	case ECLAIR_OUTPUT_SCREEN_READER:
		return OUTPUT_SCREEN_READER;
	case ECLAIR_OUTPUT_SYNTHESIZER:
		return OUTPUT_SYNTHESIZER;
	case ECLAIR_OUTPUT_NONE:
	default:
		return OUTPUT_NONE;
	}
}

const char *Speech::getBackendName() const
{
	return eclair_backend_name();
}

void Speech::setRate(float rate)
{
	eclair_set_rate(rate);
}

void Speech::setVolume(float volume)
{
	eclair_set_volume(volume);
}

STRINGMAP_CLASS_BEGIN(Speech, Speech::Route, Speech::ROUTE_MAX_ENUM, route)
{
	{ "off",                Speech::ROUTE_OFF                  },
	{ "screenreaderonly",   Speech::ROUTE_SCREEN_READER_ONLY   },
	{ "preferscreenreader", Speech::ROUTE_PREFER_SCREEN_READER },
	{ "synthesizeronly",    Speech::ROUTE_SYNTHESIZER_ONLY     },
}
STRINGMAP_CLASS_END(Speech, Speech::Route, Speech::ROUTE_MAX_ENUM, route)

STRINGMAP_CLASS_BEGIN(Speech, Speech::Output, Speech::OUTPUT_MAX_ENUM, output)
{
	{ "none",         Speech::OUTPUT_NONE           },
	{ "screenreader", Speech::OUTPUT_SCREEN_READER },
	{ "synthesizer",  Speech::OUTPUT_SYNTHESIZER   },
}
STRINGMAP_CLASS_END(Speech, Speech::Output, Speech::OUTPUT_MAX_ENUM, output)

} // speech
} // love
