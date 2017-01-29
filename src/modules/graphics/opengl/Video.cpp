/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#include "Video.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Video::Video(love::video::VideoStream *stream, float pixeldensity)
	: love::graphics::Video(stream, pixeldensity)
{
	loadVolatile();
}

Video::~Video()
{
	unloadVolatile();
}

bool Video::loadVolatile()
{
	GLuint textures[3];
	glGenTextures(3, textures);

	for (int i = 0; i < 3; i++)
		textureHandles[i] = textures[i];

	// Create the textures using the initial frame data.
	auto frame = (const love::video::VideoStream::Frame*) stream->getFrontBuffer();

	int widths[3]  = {frame->yw, frame->cw, frame->cw};
	int heights[3] = {frame->yh, frame->ch, frame->ch};

	const unsigned char *data[3] = {frame->yplane, frame->cbplane, frame->crplane};

	Texture::Wrap wrap; // Clamp wrap mode.

	bool srgb = false;
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(PIXELFORMAT_R8, false, srgb);

	for (int i = 0; i < 3; i++)
	{
		gl.bindTextureToUnit(textures[i], 0, false);

		gl.setTextureFilter(filter);
		gl.setTextureWrap(wrap);

		glTexImage2D(GL_TEXTURE_2D, 0, fmt.internalformat, widths[i], heights[i],
		             0, fmt.externalformat, fmt.type, data[i]);
	}

	return true;
}

void Video::unloadVolatile()
{
	for (int i = 0; i < 3; i++)
	{
		gl.deleteTexture((GLuint) textureHandles[i]);
		textureHandles[i] = 0;
	}
}

void Video::uploadFrame(const love::video::VideoStream::Frame *frame)
{
	int widths[3]  = {frame->yw, frame->cw, frame->cw};
	int heights[3] = {frame->yh, frame->ch, frame->ch};

	const unsigned char *data[3] = {frame->yplane, frame->cbplane, frame->crplane};

	bool srgb = false;
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(PIXELFORMAT_R8, false, srgb);

	for (int i = 0; i < 3; i++)
	{
		gl.bindTextureToUnit((GLuint) textureHandles[i], 0, false);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, widths[i], heights[i],
		                fmt.externalformat, fmt.type, data[i]);
	}
}

void Video::setFilter(const Texture::Filter &f)
{
	if (!Texture::validateFilter(f, false))
		throw love::Exception("Invalid texture filter.");

	filter = f;

	for (int i = 0; i < 3; i++)
	{
		gl.bindTextureToUnit((GLuint) textureHandles[i], 0, false);
		gl.setTextureFilter(filter);
	}
}

} // opengl
} // graphics
} // love
