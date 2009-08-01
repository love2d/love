/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include "Source.h"

namespace love
{
namespace audio
{
	Source::Source()
		: audible(0), looping(false)
	{
	}

	Source::~Source()
	{
		if(audible != 0)
		{
			audible->stop(this);
			audible->release();
		}
	}

	void Source::setAudible(Audible * audible)
	{
		// If this source already has an audible, remove it.
		if(this->audible != 0)
		{
			this->audible->stop(this);
			this->audible->release();
		}

		this->audible = audible;
		audible->retain();
	}

	Audible * Source::getAudible() const
	{
		return audible;
	}

	void Source::setLooping(bool looping)
	{
		this->looping = looping;
	}

	bool Source::isLooping() const
	{
		return looping;
	}

} // audio
} // love
