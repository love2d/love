/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "Decoder.h"

#include "common/Exception.h"

namespace love
{
namespace sound
{

love::Type Decoder::type("Decoder", &Object::type);

Decoder::Decoder(Data *data, int bufferSize)
	: data(data)
	, bufferSize(bufferSize)
	, sampleRate(DEFAULT_SAMPLE_RATE)
	, buffer(0)
	, eof(false)
{
	buffer = new char[bufferSize];
}

Decoder::~Decoder()
{
	if (buffer != 0)
		delete [](char *) buffer;
}

void *Decoder::getBuffer() const
{
	return buffer;
}

int Decoder::getSize() const
{
	return bufferSize;
}

int Decoder::getSampleRate() const
{
	return sampleRate;
}

bool Decoder::isFinished()
{
	return eof;
}

} // sound
} // love
