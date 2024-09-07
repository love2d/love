/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "font/TextShaper.h"
#include "Texture.h"
#include "vertex.h"
#include "Volatile.h"

namespace love
{
namespace graphics
{

class Graphics;

class Font : public Object, public Volatile
{
public:

	static love::Type type;

	typedef std::vector<uint32> Codepoints;
	typedef XYf_STus_RGBAub GlyphVertex;

	static const CommonFormat vertexFormat;

	enum AlignMode
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_JUSTIFY,
		ALIGN_MAX_ENUM
	};

	// Used to determine when to change textures in the generated vertex array.
	struct DrawCommand
	{
		Texture *texture;
		int startvertex;
		int vertexcount;
	};

	Font(love::font::Rasterizer *r, const SamplerState &samplerState);

	virtual ~Font();

	std::vector<DrawCommand> generateVertices(const love::font::ColoredCodepoints &codepoints, Range range, const Colorf &constantColor, std::vector<GlyphVertex> &vertices,
	                                          float extra_spacing = 0.0f, Vector2 offset = {}, love::font::TextShaper::TextInfo *info = nullptr);

	std::vector<DrawCommand> generateVerticesFormatted(const love::font::ColoredCodepoints &text, const Colorf &constantColor, float wrap, AlignMode align,
	                                                   std::vector<GlyphVertex> &vertices, love::font::TextShaper::TextInfo *info = nullptr);

	/**
	 * Draws the specified text.
	 **/
	void print(graphics::Graphics *gfx, const std::vector<love::font::ColoredString> &text, const Matrix4 &m, const Colorf &constantColor);
	void printf(graphics::Graphics *gfx, const std::vector<love::font::ColoredString> &text, float wrap, AlignMode align, const Matrix4 &m, const Colorf &constantColor);

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
	 * Returns the width of the passed glyph.
	 **/
	int getWidth(uint32 glyph);

	/**
	 * Returns the maximal width of a wrapped string
	 * and optionally the number of lines
	 *
	 * @param text The input text
	 * @param wraplimit The number of pixels to wrap at
	 * @param max_width Optional output of the maximum width
	 * Returns a vector with the lines.
	 **/
	void getWrap(const std::vector<love::font::ColoredString> &text, float wraplimit, std::vector<std::string> &lines, std::vector<float> *line_widths = nullptr);
	void getWrap(const love::font::ColoredCodepoints &codepoints, float wraplimit, std::vector<Range> &ranges, std::vector<float> *line_widths = nullptr);

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

	void setSamplerState(const SamplerState &s);
	const SamplerState &getSamplerState() const;

	// Extra font metrics
	int getAscent() const;
	int getDescent() const;
	float getBaseline() const;

	bool hasGlyph(uint32 glyph) const;
	bool hasGlyphs(const std::string &text) const;

	float getKerning(uint32 leftglyph, uint32 rightglyph);
	float getKerning(const std::string &leftchar, const std::string &rightchar);

	void setFallbacks(const std::vector<Font *> &fallbacks);

	float getDPIScale() const;

	uint32 getTextureCacheID() const;

	VertexAttributesID getVertexAttributesID() const { return vertexAttributesID; }

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	static bool getConstant(const char *in, AlignMode &out);
	static bool getConstant(AlignMode in, const char *&out);
	static std::vector<std::string> getConstants(AlignMode);

	static int fontCount;

private:

	struct Glyph
	{
		Texture *texture;
		GlyphVertex vertices[4];
	};

	struct TextureSize
	{
		int width;
		int height;
	};

	void createTexture();

	TextureSize getNextTextureSize() const;
	love::font::GlyphData *getRasterizerGlyphData(love::font::TextShaper::GlyphIndex glyphindex, float &dpiscale);
	const Glyph &addGlyph(love::font::TextShaper::GlyphIndex glyphindex);
	const Glyph &findGlyph(love::font::TextShaper::GlyphIndex glyphindex);
	void printv(Graphics *gfx, const Matrix4 &t, const std::vector<DrawCommand> &drawcommands, const std::vector<GlyphVertex> &vertices);

	StrongRef<love::font::TextShaper> shaper;

	int textureWidth;
	int textureHeight;

	std::vector<StrongRef<Texture>> textures;

	// maps packed glyph index values to glyph texture information
	std::unordered_map<uint64, Glyph> glyphs;

	PixelFormat pixelFormat;

	SamplerState samplerState;

	float dpiScale;

	int textureX, textureY;
	int rowHeight;

	// ID which is incremented when the texture cache is invalidated.
	uint32 textureCacheID;

	VertexAttributesID vertexAttributesID;

	// 1 pixel of transparent padding between glyphs (so quads won't pick up
	// other glyphs), plus one pixel of transparent padding that the quads will
	// use, for edge antialiasing.
	static const int TEXTURE_PADDING = 2;

	static StringMap<AlignMode, ALIGN_MAX_ENUM>::Entry alignModeEntries[];
	static StringMap<AlignMode, ALIGN_MAX_ENUM> alignModes;
	
}; // Font

} // graphics
} // love
