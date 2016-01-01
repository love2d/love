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

#include "Video.h"

// LOVE
#include "Shader.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Video::Video(love::video::VideoStream *stream)
	: stream(stream)
	, filter(Texture::getDefaultFilter())
{
	filter.mipmap = Texture::FILTER_NONE;

	stream->fillBackBuffer();

	for (int i = 0; i < 4; i++)
		vertices[i].r = vertices[i].g = vertices[i].b = vertices[i].a = 255;

	// Vertices are ordered for use with triangle strips:
	// 0----2
	// |  / |
	// | /  |
	// 1----3
	vertices[0].x = 0.0f;
	vertices[0].y = 0.0f;
	vertices[1].x = 0.0f;
	vertices[1].y = (float) stream->getHeight();
	vertices[2].x = (float) stream->getWidth();
	vertices[2].y = 0.0f;
	vertices[3].x = (float) stream->getWidth();
	vertices[3].y = (float) stream->getHeight();

	vertices[0].s = 0.0f;
	vertices[0].t = 0.0f;
	vertices[1].s = 0.0f;
	vertices[1].t = 1.0f;
	vertices[2].s = 1.0f;
	vertices[2].t = 0.0f;
	vertices[3].s = 1.0f;
	vertices[3].t = 1.0f;

	loadVolatile();
}

Video::~Video()
{
	unloadVolatile();
}

bool Video::loadVolatile()
{
	glGenTextures(3, &textures[0]);

	// Create the textures using the initial frame data.
	auto frame = (const love::video::VideoStream::Frame*) stream->getFrontBuffer();

	int widths[3]  = {frame->yw, frame->cw, frame->cw};
	int heights[3] = {frame->yh, frame->ch, frame->ch};

	const unsigned char *data[3] = {frame->yplane, frame->cbplane, frame->crplane};

	Texture::Wrap wrap; // Clamp wrap mode.

	for (int i = 0; i < 3; i++)
	{
		gl.bindTexture(textures[i]);

		gl.setTextureFilter(filter);
		gl.setTextureWrap(wrap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widths[i], heights[i], 0,
		             GL_LUMINANCE, GL_UNSIGNED_BYTE, data[i]);
	}

	return true;
}

void Video::unloadVolatile()
{
	for (int i = 0; i < 3; i++)
	{
		gl.deleteTexture(textures[i]);
		textures[i] = 0;
	}
}

love::video::VideoStream *Video::getStream()
{
	return stream;
}

void Video::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	update();

	Shader *shader = Shader::current;
	bool defaultShader = (shader == Shader::defaultShader);
	if (defaultShader)
	{
		// If we're still using the default shader, substitute the video version
		Shader::defaultVideoShader->attach();
		shader = Shader::defaultVideoShader;
	}

	shader->setVideoTextures(textures[0], textures[1], textures[2]);

	OpenGL::TempTransform transform(gl);
	transform.get() *= Matrix4(x, y, angle, sx, sy, ox, oy, kx, ky);

	gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].s);

	gl.prepareDraw();
	gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// If we were using the default shader, reattach it
	if (defaultShader)
		Shader::defaultShader->attach();
}

void Video::update()
{
	bool bufferschanged = stream->swapBuffers();
	stream->fillBackBuffer();

	if (bufferschanged)
	{
		auto frame = (const love::video::VideoStream::Frame*) stream->getFrontBuffer();

		int widths[3]  = {frame->yw, frame->cw, frame->cw};
		int heights[3] = {frame->yh, frame->ch, frame->ch};

		const unsigned char *data[3] = {frame->yplane, frame->cbplane, frame->crplane};

		for (int i = 0; i < 3; i++)
		{
			gl.bindTexture(textures[i]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, widths[i], heights[i],
			                GL_LUMINANCE, GL_UNSIGNED_BYTE, data[i]);
		}
	}
}

love::audio::Source *Video::getSource()
{
	return source;
}

void Video::setSource(love::audio::Source *source)
{
	this->source = source;
}

int Video::getWidth() const
{
	return stream->getWidth();
}

int Video::getHeight() const
{
	return stream->getHeight();
}

void Video::setFilter(const Texture::Filter &f)
{
	if (!Texture::validateFilter(f, false))
		throw love::Exception("Invalid texture filter.");

	filter = f;

	for (int i = 0; i < 3; i++)
	{
		gl.bindTexture(textures[i]);
		gl.setTextureFilter(filter);
	}
}

const Texture::Filter &Video::getFilter() const
{
	return filter;
}

} // opengl
} // graphics
} // love
