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

// STD
#include <unordered_map>
#include <string>
#include <vector>
#include <stddef.h>

// LOVE
#include "common/config.h"
#include "common/Object.h"
#include "common/Matrix.h"
#include "common/Vector.h"

#include "font/Rasterizer.h"
#include "Texture.h"
#include "vertex.h"

namespace love
{
namespace graphics
{

class Graphics;

class Font : public Object
{
public:

	static love::Type type;

	typedef std::vector<uint32> Codepoints;
	typedef vertex::XYf_STus_RGBAub GlyphVertex;

	enum AlignMode
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_JUSTIFY,
		ALIGN_MAX_ENUM
	};

	struct ColoredString
	{
		std::string str;
		Colorf color;
	};

	struct IndexedColor
	{
		Colorf color;
		int index;
	};

	struct ColoredCodepoints
	{
		std::vector<uint32> cps;
		std::vector<IndexedColor> colors;
	};

	struct TextInfo
	{
		int width;
		int height;
	};

	// Used to determine when to change textures in the generated vertex array.
	struct DrawCommand
	{
		ptrdiff_t texture;
		int startvertex;
		int vertexcount;
	};

	Font(love::font::Rasterizer *r, const Texture::Filter &filter);

	virtual ~Font();

	std::vector<DrawCommand> generateVertices(const ColoredCodepoints &codepoints, const Colorf &constantColor, std::vector<GlyphVertex> &vertices,
	                                          float extra_spacing = 0.0f, Vector offset = {}, TextInfo *info = nullptr);

	std::vector<DrawCommand> generateVerticesFormatted(const ColoredCodepoints &text, const Colorf &constantColor, float wrap, AlignMode align,
	                                                   std::vector<GlyphVertex> &vertices, TextInfo *info = nullptr);

	static void getCodepointsFromString(const std::string &str, Codepoints &codepoints);
	static void getCodepointsFromString(const std::vector<ColoredString> &strs, ColoredCodepoints &codepoints);

	/**
	 * Draws the specified text.
	 **/
	void print(graphics::Graphics *gfx, const std::vector<ColoredString> &text, const Matrix4 &m, const Colorf &constantColor);
	void printf(graphics::Graphics *gfx, const std::vector<ColoredString> &text, float wrap, AlignMode align, const Matrix4 &m, const Colorf &constantColor);

	/**
	 * Returns the height of the font.
	 **/
	float getHeight() const;

	/**
	 * Returns the width of the passed string.
	 *
	 * @param str A string of text.
	 **/
	int getWidth(const std::string &str);

	/**
	 * Returns the width of the passed character.
	 *
	 * @param character A character.
	 **/
	int getWidth(char character);

	/**
	 * Returns the maximal width of a wrapped string
	 * and optionally the number of lines
	 *
	 * @param text The input text
	 * @param wraplimit The number of pixels to wrap at
	 * @param max_width Optional output of the maximum width
	 * Returns a vector with the lines.
	 **/
	void getWrap(const std::vector<ColoredString> &text, float wraplimit, std::vector<std::string> &lines, std::vector<int> *line_widths = nullptr);
	void getWrap(const ColoredCodepoints &codepoints, float wraplimit, std::vector<ColoredCodepoints> &lines, std::vector<int> *line_widths = nullptr);

	/**
	 * Sets the line height (which should be a number to multiply the font size by,
	 * example: line height = 1.2 and size = 12 means that rendered line height = 12*1.2)
	 * @param height The new line height.
	 **/
	void setLineHeight(float height);

	/**
	 * Returns the line height.
	 **/
	float getLineHeight() const;

	virtual void setFilter(const Texture::Filter &f) = 0;
	const Texture::Filter &getFilter() const;

	// Extra font metrics
	int getAscent() const;
	int getDescent() const;
	float getBaseline() const;

	bool hasGlyph(uint32 glyph) const;
	bool hasGlyphs(const std::string &text) const;

	void setFallbacks(const std::vector<Font *> &fallbacks);

	float getPixelDensity() const;

	uint32 getTextureCacheID() const;

	static bool getConstant(const char *in, AlignMode &out);
	static bool getConstant(AlignMode in, const char *&out);

	static int fontCount;

protected:

	struct Glyph
	{
		ptrdiff_t texture;
		int spacing;
		GlyphVertex vertices[4];
	};

	struct TextureSize
	{
		int width;
		int height;
	};

	virtual void createTexture() = 0;
	virtual void uploadGlyphToTexture(font::GlyphData *data, Glyph &glyph) = 0;

	TextureSize getNextTextureSize() const;
	love::font::GlyphData *getRasterizerGlyphData(uint32 glyph);
	const Glyph &addGlyph(uint32 glyph);
	const Glyph &findGlyph(uint32 glyph);
	float getKerning(uint32 leftglyph, uint32 rightglyph);
	void printv(Graphics *gfx, const Matrix4 &t, const std::vector<DrawCommand> &drawcommands, const std::vector<GlyphVertex> &vertices);

	std::vector<StrongRef<love::font::Rasterizer>> rasterizers;

	int height;
	float lineHeight;

	int textureWidth;
	int textureHeight;

	// maps glyphs to glyph texture information
	std::unordered_map<uint32, Glyph> glyphs;

	// map of left/right glyph pairs to horizontal kerning.
	std::unordered_map<uint64, float> kerning;

	PixelFormat pixelFormat;

	Texture::Filter filter;

	float pixelDensity;

	int textureX, textureY;
	int rowHeight;

	bool useSpacesAsTab;
	
	// ID which is incremented when the texture cache is invalidated.
	uint32 textureCacheID;
	
	static const int TEXTURE_PADDING = 1;
	
	// This will be used if the Rasterizer doesn't have a tab character itself.
	static const int SPACES_PER_TAB = 4;
	
	static StringMap<AlignMode, ALIGN_MAX_ENUM>::Entry alignModeEntries[];
	static StringMap<AlignMode, ALIGN_MAX_ENUM> alignModes;
	
}; // Font

} // graphics
} // love
