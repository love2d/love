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

#ifndef LOVE_COMMON_OPENGL_H
#define LOVE_COMMON_OPENGL_H

#include "GLee.h"
#include "graphics/Image.h"

namespace love
{
namespace graphics
{
namespace opengl
{

// resets the stored bound texture id
void resetBoundTexture();

/**
 * Helper for binding an OpenGL texture.
 * Makes sure we aren't redundantly binding textures.
 * @param texture The texture to bind.
 * @param override Overrides the checks to guarantee texture bind
 **/
void bindTexture(GLuint texture, bool override = false);

/**
 * Helper for deleting an OpenGL texture.
 * Cleans up if the texture is currently bound.
 * @param texture The texture to delete.
 **/
void deleteTexture(GLuint texture);

/**
 * Sets the image filter mode for the currently bound texture
 * @param f The image filter to set
 */
void setTextureFilter(const graphics::Image::Filter &f);

/**
 * Returns the image filter mode for the currently bound texture
 */
graphics::Image::Filter getTextureFilter();

/**
 * Sets the image wrap mode for the currently bound texture
 * @param w The wrap mode to set
 */
void setTextureWrap(const graphics::Image::Wrap &w);

/**
 * Returns the image wrap mode for the currently bound texture
 */
graphics::Image::Wrap getTextureWrap();

} // opengl
} // graphics
} // love

#endif
