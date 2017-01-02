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

#pragma once

// LOVE
#include "graphics/Font.h"
#include "graphics/Volatile.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Font final : public love::graphics::Font, public Volatile
{
public:

	Font(love::font::Rasterizer *r, const Texture::Filter &filter);
	virtual ~Font();

	void setFilter(const Texture::Filter &f) override;

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

private:

	void createTexture() override;
	void uploadGlyphToTexture(font::GlyphData *data, Glyph &glyph) override;

	// vector of packed textures
	std::vector<GLuint> textures;

	size_t textureMemorySize;

}; // Font

} // opengl
} // graphics
} // love
