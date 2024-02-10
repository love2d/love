/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include "Pool.h"

#include "event/Event.h"
#include "Source.h"

namespace love
{
namespace audio
{
namespace openal
{

static Variant::SharedTable *putSourcesAsSharedTable(std::vector<audio::Source *> &sources)
{
	Variant::SharedTable *table = new Variant::SharedTable();

	for (int i = 0; i < (int) sources.size(); i++)
		table->pairs.emplace_back((double) (i + 1), Variant(&Source::type, sources[i]));

	return table;
}

Pool::Pool(ALCdevice *device)
	: device(device)
	, sources()
	, disconnectNotified(false)
	, totalSources(0)
{
	// Clear errors.
	alGetError();

	// Generate sources.
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		alGenSources(1, &sources[i]);

		// We might hit an implementation-dependent limit on the total number
		// of sources before reaching MAX_SOURCES.
		if (alGetError() != AL_NO_ERROR)
			break;

		totalSources++;
	}

	if (totalSources < 4)
		throw love::Exception("Could not generate sources.");

#ifdef AL_SOFT_direct_channels
	ALboolean hasext = alIsExtensionPresent("AL_SOFT_direct_channels");
#endif

	// Make all sources available initially.
	for (int i = 0; i < totalSources; i++)
	{
#ifdef AL_SOFT_direct_channels
		if (hasext)
		{
			// Bypass virtualization of speakers for multi-channel sources in OpenAL Soft.
			alSourcei(sources[i], AL_DIRECT_CHANNELS_SOFT, AL_TRUE);
		}
#endif

		available.push(sources[i]);
	}
}

Pool::~Pool()
{
	Source::stop(this);

	// Free all sources.
	alDeleteSources(totalSources, sources);
}

bool Pool::isAvailable() const
{
	bool has = false;
	{
		thread::Lock lock(mutex);
		has = !available.empty();
	}
	return has;
}

bool Pool::isPlaying(Source *s)
{
	bool p = false;
	{
		thread::Lock lock(mutex);
		p = (playing.find(s) != playing.end());
	}
	return p;
}

void Pool::update()
{
#ifndef ALC_CONNECTED
	constexpr ALCenum ALC_CONNECTED = 0x313;
#endif

	thread::Lock lock(mutex);

	static bool disconnectExtSupported = alcIsExtensionPresent(device, "ALC_EXT_Disconnect") == ALC_TRUE;

	// Device disconnection event
	if (disconnectExtSupported)
	{
		auto eventModule = Module::getInstance<event::Event>(Module::M_EVENT);
		if (eventModule)
		{
			ALCint connected;
			alcGetIntegerv(device, ALC_CONNECTED, 1, &connected);

			if (connected)
				disconnectNotified = false;
			else if (!disconnectNotified)
			{
				// Get all sources in this Pool then stop it
				// since they're all internally stopped.
				std::vector<audio::Source *> sources = getPlayingSources();
				Source::stop(sources);

				std::vector<Variant> vargs;
				vargs.emplace_back(putSourcesAsSharedTable(sources));

				StrongRef<event::Message> msg(new event::Message("audiodisconnected", vargs), Acquire::NORETAIN);
				eventModule->push(msg);

				disconnectNotified = true;
			}
		}
	}

	std::vector<Source *> torelease;

	for (const auto &i : playing)
	{
		if (!i.first->update())
			torelease.push_back(i.first);
	}

	for (Source *s : torelease)
		releaseSource(s);
}

int Pool::getActiveSourceCount() const
{
	return (int) playing.size();
}

int Pool::getMaxSources() const
{
	return totalSources;
}

bool Pool::assignSource(Source *source, ALuint &out, char &wasPlaying)
{
	out = 0;

	if (findSource(source, out))
		return wasPlaying = true;

	wasPlaying = false;

	if (available.empty())
		return false;

	out = available.front();
	available.pop();

	playing.insert(std::make_pair(source, out));
	source->retain();
	return true;
}

bool Pool::releaseSource(Source *source, bool stop)
{
	ALuint s;

	if (findSource(source, s))
	{
		if (stop)
			source->stopAtomic();
		source->release();
		available.push(s);
		playing.erase(source);
		return true;
	}

	return false;
}

bool Pool::findSource(Source *source, ALuint &out)
{
	std::map<Source *, ALuint>::const_iterator i = playing.find(source);

	if (i == playing.end())
		return false;

	out = i->second;
	return true;
}

thread::Lock Pool::lock()
{
	return thread::Lock(mutex);
}

std::vector<love::audio::Source*> Pool::getPlayingSources()
{
	std::vector<love::audio::Source*> sources;
	sources.reserve(playing.size());
	for (auto &i : playing)
		sources.push_back(i.first);
	return sources;
}

} // openal
} // audio
} // love
