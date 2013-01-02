/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

static GLuint boundTexture = 0;

void resetBoundTexture()
{
	// OpenGL might not be initialized yet, so we can't do a real reset
	boundTexture = 0;
}

void bindTexture(GLuint texture, bool override)
{
	if (texture != boundTexture || texture == 0 || override)
	{
		boundTexture = texture;
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

void deleteTexture(GLuint texture)
{
	if (texture == boundTexture)
		boundTexture = 0;

	glDeleteTextures(1, &texture);
}

void setTextureFilter(const graphics::Image::Filter &f)
{
	GLint gmin, gmag;
	
	if (f.mipmap == Image::FILTER_NONE)
	{
		if (f.min == Image::FILTER_NEAREST)
			gmin = GL_NEAREST;
		else // f.min == Image::FILTER_LINEAR
			gmin = GL_LINEAR;
	}
	else
	{
		if (f.min == f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_NEAREST_MIPMAP_NEAREST;
		else if (f.min == Image::FILTER_NEAREST && f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_NEAREST_MIPMAP_LINEAR;
		else if (f.min == Image::FILTER_LINEAR && f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_LINEAR_MIPMAP_NEAREST;
		else if (f.min == f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_LINEAR_MIPMAP_LINEAR;
		else
			gmin = GL_LINEAR;
	}
	
	
	switch (f.mag)
	{
	case Image::FILTER_NEAREST:
		gmag = GL_NEAREST;
		break;
	case Image::FILTER_LINEAR:
	default:
		gmag = GL_LINEAR;
		break;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);
}

graphics::Image::Filter getTextureFilter()
{
	GLint gmin, gmag;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &gmin);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &gmag);
	
	Image::Filter f;
	
	switch (gmin)
	{
	case GL_NEAREST:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_NONE;
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
		f.min = f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		f.min = f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR:
	default:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NONE;
		break;
	}
	
	switch (gmag)
	{
	case GL_NEAREST:
		f.mag = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR:
	default:
		f.mag = Image::FILTER_LINEAR;
		break;
	}
	
	return f;
}

void setTextureWrap(const graphics::Image::Wrap &w)
{
	GLint gs, gt;
	
	switch (w.s)
	{
	case Image::WRAP_CLAMP:
		gs = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gs = GL_REPEAT;
		break;
	}
	
	switch (w.t)
	{
	case Image::WRAP_CLAMP:
		gt = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gt = GL_REPEAT;
		break;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gt);
}

graphics::Image::Wrap getTextureWrap()
{
	GLint gs, gt;
	
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &gs);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &gt);
	
	Image::Wrap w;
	
	switch (gs)
	{
	case GL_CLAMP_TO_EDGE:
		w.s = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.s = Image::WRAP_REPEAT;
		break;
	}
	
	switch (gt)
	{
	case GL_CLAMP_TO_EDGE:
		w.t = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.t = Image::WRAP_REPEAT;
		break;
	}
	
	return w;
}

} // opengl
} // graphics
} // love
