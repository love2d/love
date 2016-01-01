/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "Source.h"

namespace love
{
namespace audio
{
namespace openal
{

Pool::Pool()
	: sources()
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
	stop();

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
	thread::Lock lock(mutex);

	std::map<Source *, ALuint>::iterator i = playing.begin();

	while (i != playing.end())
	{
		if (!i->first->update())
		{
			i->first->stopAtomic();
			i->first->rewindAtomic();
			i->first->release();
			available.push(i->second);
			playing.erase(i++);
		}
		else
			i++;
	}
}

int Pool::getSourceCount() const
{
	return (int) playing.size();
}

int Pool::getMaxSources() const
{
	return totalSources;
}

bool Pool::play(Source *source, ALuint &out)
{
	thread::Lock lock(mutex);

	bool ok = true;
	out = 0;

	bool alreadyPlaying = findSource(source, out);

	if (!alreadyPlaying)
	{
		// Try to play.
		if (!available.empty())
		{
			// Get the first available source.
			out = available.front();

			// Remove it.
			available.pop();

			// Insert into map of playing sources.
			playing.insert(std::pair<Source *, ALuint>(source, out));

			source->retain();

			ok = source->playAtomic();
		}
		else
		{
			ok = false;
		}
	}
	else
	{
		ok = true;
	}

	return ok;
}

void Pool::stop()
{
	thread::Lock lock(mutex);
	for (const auto &i : playing)
	{
		i.first->stopAtomic();
		i.first->rewindAtomic();
		i.first->release();
		available.push(i.second);
	}

	playing.clear();
}

void Pool::stop(Source *source)
{
	thread::Lock lock(mutex);
	removeSource(source);
}

void Pool::pause()
{
	thread::Lock lock(mutex);
	for (const auto &i : playing)
		i.first->pauseAtomic();
}

void Pool::pause(Source *source)
{
	thread::Lock lock(mutex);
	ALuint out;
	if (findSource(source, out))
		source->pauseAtomic();
}

void Pool::resume()
{
	thread::Lock lock(mutex);
	for (const auto &i : playing)
		i.first->resumeAtomic();
}

void Pool::resume(Source *source)
{
	thread::Lock lock(mutex);
	ALuint out;
	if (findSource(source, out))
		source->resumeAtomic();
}

void Pool::rewind()
{
	thread::Lock lock(mutex);
	for (const auto &i : playing)
		i.first->rewindAtomic();
}

// For those times we don't need it backed.
void Pool::softRewind(Source *source)
{
	thread::Lock lock(mutex);
	source->rewindAtomic();
}

void Pool::rewind(Source *source)
{
	thread::Lock lock(mutex);
	source->rewindAtomic();
}

void Pool::release(Source *source)
{
	ALuint s = findi(source);

	if (s != 0)
	{
		available.push(s);
		playing.erase(source);
	}
}

void Pool::seek(Source *source, float offset, void *unit)
{
	thread::Lock lock(mutex);
	return source->seekAtomic(offset, unit);
}

float Pool::tell(Source *source, void *unit)
{
	thread::Lock lock(mutex);
	return source->tellAtomic(unit);
}

double Pool::getDuration(Source *source, void *unit)
{
	thread::Lock lock(mutex);
	return source->getDurationAtomic(unit);
}

ALuint Pool::findi(const Source *source) const
{
	std::map<Source *, ALuint>::const_iterator i = playing.find((Source *)source);

	if (i != playing.end())
		return i->second;

	return 0;
}

bool Pool::findSource(Source *source, ALuint &out)
{
	std::map<Source *, ALuint>::const_iterator i = playing.find((Source *)source);

	bool found = i != playing.end();

	if (found)
		out = i->second;

	return found;
}

bool Pool::removeSource(Source *source)
{
	std::map<Source *, ALuint>::iterator i = playing.find((Source *)source);

	if (i != playing.end())
	{
		source->stopAtomic();
		available.push(i->second);
		playing.erase(i++);
		source->release();
		return true;
	}

	return false;
}

} // openal
} // audio
} // love
