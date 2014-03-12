/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_TEXTURE_H
#define LOVE_GRAPHICS_OPENGL_TEXTURE_H

// LOVE
#include "graphics/Texture.h"
#include "graphics/Volatile.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Texture : public love::graphics::Texture, public Volatile
{
public:

	virtual ~Texture() {}

	/**
	 * Gets the OpenGL id for this texture.
	 **/
	virtual GLuint getGLTexture() const = 0;

	/**
	 * Any setup the texture might need to do before drawing, e.g. binding
	 * the OpenGL texture for use.
	 **/
	virtual void predraw() {}

	/**
	 * Any cleanup the texture might need to do directly after drawing.
	 **/
	virtual void postdraw() {}


}; // Texture

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_TEXTURE_H
