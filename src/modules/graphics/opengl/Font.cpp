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

// LOVE
#include "Font.h"
#include "graphics/Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Font::Font(love::font::Rasterizer *r, const Texture::Filter &f)
	: love::graphics::Font(r, f)
	, textureMemorySize(0)
{
	loadVolatile();
}

Font::~Font()
{
	unloadVolatile();
}

void Font::createTexture()
{
	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("Font create texture");

	size_t bpp = getPixelFormatSize(pixelFormat);

	size_t prevmemsize = textureMemorySize;
	if (prevmemsize > 0)
	{
		textureMemorySize -= (textureWidth * textureHeight * bpp);
		gl.updateTextureMemorySize(prevmemsize, textureMemorySize);
	}

	GLuint t = 0;
	TextureSize size = {textureWidth, textureHeight};
	TextureSize nextsize = getNextTextureSize();
	bool recreatetexture = false;

	// If we have an existing texture already, we'll try replacing it with a
	// larger-sized one rather than creating a second one. Having a single
	// texture reduces texture switches and draw calls when rendering.
	if ((nextsize.width > size.width || nextsize.height > size.height)
		&& !textures.empty())
	{
		recreatetexture = true;
		size = nextsize;
		t = textures.back();
	}
	else
		glGenTextures(1, &t);

	gl.bindTextureToUnit(t, 0, false);

	gl.setTextureFilter(filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	bool sRGB = isGammaCorrect();
	OpenGL::TextureFormat fmt = gl.convertPixelFormat(pixelFormat, false, sRGB);

	// Initialize the texture with transparent black.
	std::vector<GLubyte> emptydata(size.width * size.height * bpp, 0);

	// Clear errors before initializing.
	while (glGetError() != GL_NO_ERROR);

	glTexImage2D(GL_TEXTURE_2D, 0, fmt.internalformat, size.width, size.height,
	             0, fmt.externalformat, fmt.type, &emptydata[0]);

	if (glGetError() != GL_NO_ERROR)
	{
		if (!recreatetexture)
			gl.deleteTexture(t);
		throw love::Exception("Could not create font texture!");
	}

	textureWidth  = size.width;
	textureHeight = size.height;

	rowHeight = textureX = textureY = TEXTURE_PADDING;

	prevmemsize = textureMemorySize;
	textureMemorySize += emptydata.size();
	gl.updateTextureMemorySize(prevmemsize, textureMemorySize);

	// Re-add the old glyphs if we re-created the existing texture object.
	if (recreatetexture)
	{
		textureCacheID++;

		std::vector<uint32> glyphstoadd;

		for (const auto &glyphpair : glyphs)
			glyphstoadd.push_back(glyphpair.first);

		glyphs.clear();

		for (uint32 g : glyphstoadd)
			addGlyph(g);
	}
	else
		textures.push_back(t);
}

void Font::uploadGlyphToTexture(font::GlyphData *gd, Glyph &glyph)
{
	bool isSRGB = isGammaCorrect();
	OpenGL::TextureFormat fmt = gl.convertPixelFormat(pixelFormat, false, isSRGB);

	glyph.texture = textures.back();

	gl.bindTextureToUnit(glyph.texture, 0, false);
	glTexSubImage2D(GL_TEXTURE_2D, 0, textureX, textureY, gd->getWidth(), gd->getHeight(),
	                fmt.externalformat, fmt.type, gd->getData());
}

void Font::setFilter(const Texture::Filter &f)
{
	if (!Texture::validateFilter(f, false))
		throw love::Exception("Invalid texture filter.");

	filter = f;

	for (GLuint texture : textures)
	{
		gl.bindTextureToUnit(texture, 0, false);
		gl.setTextureFilter(filter);
	}
}

bool Font::loadVolatile()
{
	createTexture();
	textureCacheID++;
	return true;
}

void Font::unloadVolatile()
{
	// nuke everything from orbit

	glyphs.clear();

	for (GLuint texture : textures)
		gl.deleteTexture(texture);

	textures.clear();

	gl.updateTextureMemorySize(textureMemorySize, 0);
	textureMemorySize = 0;
}

} // opengl
} // graphics
} // love
