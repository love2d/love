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
#include "System.h"
#include "data/ByteData.h"
#include "window/Window.h"

// SDL
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_locale.h>
#include <SDL3/SDL_misc.h>

namespace love
{
namespace system
{
namespace sdl
{

System::System()
	: love::system::System("love.system.sdl")
{
}

int System::getProcessorCount() const
{
	return SDL_GetNumLogicalCPUCores();
}

int System::getMemorySize() const
{
	return SDL_GetSystemRAM();
}

bool System::isWindowOpen() const
{
	auto window = Module::getInstance<window::Window>(M_WINDOW);
	return window != nullptr && window->isOpen();
}

void System::setClipboardText(const std::string &text) const
{
	// SDL requires the video subsystem to be initialized and a window to be
	// opened in order for clipboard text to work, on at least some platforms.
	if (!isWindowOpen())
		throw love::Exception("A window must be created in order for setClipboardText to function properly.");

	SDL_SetClipboardText(text.c_str());
}

std::string System::getClipboardText() const
{
	if (!isWindowOpen())
		throw love::Exception("A window must be created in order for getClipboardText to function properly.");

	std::string text("");

	char *ctext = SDL_GetClipboardText();
	if (ctext)
	{
		text = std::string(ctext);
		SDL_free(ctext);
	}

	return text;
}

void System::setClipboardData(const love::Data *data, const std::vector<std::string> &mimetypes) const
{
	// SDL requires the video subsystem to be initialized and a window to be
	// opened in order for clipboard text to work, on at least some platforms.
	if (!isWindowOpen())
		throw love::Exception("A window must be created in order for setClipboardData to function properly.");

	std::vector<const char*> mimetypes_cstr;

	// Using c_str() is probably fine in this circumstance, the mimetypes paramter is not very likely going to messed with on purpose.
	// The std::vector is created in a wrapper function, the array shouldn't get modfied until it isn't referenced anymore by C++.
	for (const std::string& str: mimetypes)
		mimetypes_cstr.push_back(str.c_str());

	bool success = SDL_SetClipboardData(
		[] (void *userdata, const char *mime_type, size_t *size) -> const void* {
			const love::Data *data = (const love::Data *)userdata;

			*size = data->getSize();
			return data->getData();
		},

		[] (void *userdata) {}, // unused cleanup function (cleanup will be handled by C++ and/or Lua)
		(void *)data,

		mimetypes_cstr.data(), mimetypes_cstr.size()
	);

	if (!success)
		throw love::Exception("Could not set clipboard data: %s", SDL_GetError());
}

ByteData *System::getClipboardData(const std::string &mimetype) const
{
	size_t size = 0;

	if (!isWindowOpen())
		throw love::Exception("A window must be created in order for getClipboardData to function properly.");


	void *data_ptr = SDL_GetClipboardData(mimetype.c_str(), &size);
	ByteData *data = nullptr;
	if (data_ptr)
	{
		data = new ByteData(data_ptr, size, false);
		SDL_free(data_ptr);
	}

	return data;
}

std::vector<std::string> System::getClipboardTypes() const
{
	std::vector<std::string> result;
	size_t count = 0;

	// SDL requires the video subsystem to be initialized and a window to be
	// opened in order for clipboard text to work, on at least some platforms.
	if (!isWindowOpen())
		throw love::Exception("A window must be created in order for getClipboardTypes to function properly.");

	char **mimetypes = SDL_GetClipboardMimeTypes(&count);
	if (!mimetypes)
		throw love::Exception("Could not get clipboard types: %s", SDL_GetError());

	for (size_t i = 0; i < count; i++)
	{
		const char *mimetype = mimetypes[i];
		result.push_back(mimetype);
	}

	SDL_free(mimetypes);

	return result;
}


love::system::System::PowerState System::getPowerInfo(int &seconds, int &percent) const
{
	SDL_PowerState sdlstate = SDL_GetPowerInfo(&seconds, &percent);

	PowerState state = POWER_UNKNOWN;
	powerStates.find(sdlstate, state);

	return state;
}

bool System::openURL(const std::string &url) const
{
	return SDL_OpenURL(url.c_str());
}

std::vector<std::string> System::getPreferredLocales() const
{
	std::vector<std::string> result;

	int count = 0;
	SDL_Locale **locales = SDL_GetPreferredLocales(&count);
	for (int i = 0; i < count; i++)
	{
		SDL_Locale *locale = locales[i];
		if (locale->country)
			result.push_back(std::string(locale->language) + "_" + std::string(locale->country));
		else
			result.push_back(locale->language);
	}
	SDL_free(locales);

	return result;
}

EnumMap<System::PowerState, SDL_PowerState, System::POWER_MAX_ENUM>::Entry System::powerEntries[] =
{
	{System::POWER_UNKNOWN, SDL_POWERSTATE_UNKNOWN},
	{System::POWER_BATTERY, SDL_POWERSTATE_ON_BATTERY},
	{System::POWER_NO_BATTERY, SDL_POWERSTATE_NO_BATTERY},
	{System::POWER_CHARGING, SDL_POWERSTATE_CHARGING},
	{System::POWER_CHARGED, SDL_POWERSTATE_CHARGED},
};

EnumMap<System::PowerState, SDL_PowerState, System::POWER_MAX_ENUM> System::powerStates(System::powerEntries, sizeof(System::powerEntries));

} // sdl
} // system
} // love
