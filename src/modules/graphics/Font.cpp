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
#include "common/config.h"
#include "Font.h"
#include "font/GlyphData.h"

#include "common/math.h"
#include "common/Matrix.h"
#include "Graphics.h"

#include <math.h>
#include <sstream>
#include <algorithm> // for max
#include <limits>

namespace love
{
namespace graphics
{

static inline uint16 normToUint16(double n)
{
	return (uint16) (n * LOVE_UINT16_MAX);
}

static inline uint64 packGlyphIndex(love::font::TextShaper::GlyphIndex glyphindex)
{
	return ((uint64)glyphindex.rasterizerIndex << 32) | (uint64)glyphindex.index;
}

static inline love::font::TextShaper::GlyphIndex unpackGlyphIndex(uint64 packedindex)
{
	return {(int) (packedindex & 0xFFFFFFFF), (int) (packedindex >> 32)};
}

love::Type Font::type("Font", &Object::type);
int Font::fontCount = 0;

const CommonFormat Font::vertexFormat = CommonFormat::XYf_STus_RGBAub;

Font::Font(love::font::Rasterizer *r, const SamplerState &s)
	: shaper(r->newTextShaper(), Acquire::NORETAIN)
	, textureWidth(128)
	, textureHeight(128)
	, samplerState()
	, dpiScale(r->getDPIScale())
	, textureCacheID(0)
{
	samplerState.minFilter = s.minFilter;
	samplerState.magFilter = s.magFilter;
	samplerState.maxAnisotropy = s.maxAnisotropy;

	// Try to find the best texture size match for the font size. default to the
	// largest texture size if no rough match is found.
	while (true)
	{
		float dpiscale = r->getDPIScale();
		if ((shaper->getHeight() * 0.8 * dpiscale) * shaper->getHeight() * 30 * dpiscale <= textureWidth * textureHeight)
			break;

		TextureSize nextsize = getNextTextureSize();

		if (nextsize.width <= textureWidth && nextsize.height <= textureHeight)
			break;

		textureWidth = nextsize.width;
		textureHeight = nextsize.height;
	}

	love::font::GlyphData *gd = r->getGlyphData(32); // Space character.
	pixelFormat = gd->getFormat();
	gd->release();

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (pixelFormat == PIXELFORMAT_LA8_UNORM && !gfx->isPixelFormatSupported(pixelFormat, PIXELFORMATUSAGEFLAGS_SAMPLE))
		pixelFormat = PIXELFORMAT_RGBA8_UNORM;

	vertexAttributesID = gfx->registerVertexAttributes(VertexAttributes(vertexFormat, 0));

	loadVolatile();
	++fontCount;
}

Font::~Font()
{
	--fontCount;
}

Font::TextureSize Font::getNextTextureSize() const
{
	TextureSize size = {textureWidth, textureHeight};

	int maxsize = 2048;
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
	{
		const auto &caps = gfx->getCapabilities();
		maxsize = (int) caps.limits[Graphics::LIMIT_TEXTURE_SIZE];
	}

	int maxwidth  = std::min(8192, maxsize);
	int maxheight = std::min(4096, maxsize);

	if (size.width * 2 <= maxwidth || size.height * 2 <= maxheight)
	{
		// {128, 128} -> {256, 128} -> {256, 256} -> {512, 256} -> etc.
		if (size.width == size.height)
			size.width *= 2;
		else
			size.height *= 2;
	}

	return size;
}

bool Font::loadVolatile()
{
	textureCacheID++;
	glyphs.clear();
	textures.clear();
	createTexture();
	return true;
}

void Font::createTexture()
{
	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	gfx->flushBatchedDraws();

	Texture *texture = nullptr;
	TextureSize size = {textureWidth, textureHeight};
	TextureSize nextsize = getNextTextureSize();
	bool recreatetexture = false;

	// If we have an existing texture already, we'll try replacing it with a
	// larger-sized one rather than creating a second one. Having a single
	// texture reduces texture switches and draw calls when rendering.
	if ((nextsize.width > size.width || nextsize.height > size.height) && !textures.empty())
	{
		recreatetexture = true;
		size = nextsize;
		textures.pop_back();
	}

	Texture::Settings settings;
	settings.format = pixelFormat;
	settings.width = size.width;
	settings.height = size.height;
	texture = gfx->newTexture(settings, nullptr);
	texture->setSamplerState(samplerState);

	{
		size_t datasize = getPixelFormatSliceSize(pixelFormat, size.width, size.height);
		size_t pixelcount = size.width * size.height;

		// Initialize the texture with transparent white for truetype fonts
		// (since we keep luminance constant and vary alpha in those glyphs),
		// and transparent black otherwise.
		std::vector<uint8> emptydata(datasize, 0);

		if (shaper->getRasterizers()[0]->getDataType() == font::Rasterizer::DATA_TRUETYPE)
		{
			if (pixelFormat == PIXELFORMAT_LA8_UNORM)
			{
				for (size_t i = 0; i < pixelcount; i++)
					emptydata[i * 2 + 0] = 255;
			}
			else if (pixelFormat == PIXELFORMAT_RGBA8_UNORM)
			{
				for (size_t i = 0; i < pixelcount; i++)
				{
					emptydata[i * 4 + 0] = 255;
					emptydata[i * 4 + 1] = 255;
					emptydata[i * 4 + 2] = 255;
				}
			}
		}

		Rect rect = {0, 0, size.width, size.height};
		texture->replacePixels(emptydata.data(), emptydata.size(), 0, 0, rect, false);
	}

	textures.emplace_back(texture, Acquire::NORETAIN);

	textureWidth  = size.width;
	textureHeight = size.height;

	rowHeight = textureX = textureY = TEXTURE_PADDING;

	// Re-add the old glyphs if we re-created the existing texture object.
	if (recreatetexture)
	{
		textureCacheID++;

		std::vector<love::font::TextShaper::GlyphIndex> glyphstoadd;

		for (const auto &glyphpair : glyphs)
			glyphstoadd.push_back(unpackGlyphIndex(glyphpair.first));

		glyphs.clear();
		
		for (auto glyphindex : glyphstoadd)
			addGlyph(glyphindex);
	}
}

void Font::unloadVolatile()
{
	glyphs.clear();
	textures.clear();
}

love::font::GlyphData *Font::getRasterizerGlyphData(love::font::TextShaper::GlyphIndex glyphindex, float &dpiscale)
{
	const auto &r = shaper->getRasterizers()[glyphindex.rasterizerIndex];
	dpiscale = r->getDPIScale();
	return r->getGlyphDataForIndex(glyphindex.index);
}

const Font::Glyph &Font::addGlyph(love::font::TextShaper::GlyphIndex glyphindex)
{
	float glyphdpiscale = getDPIScale();
	StrongRef<love::font::GlyphData> gd(getRasterizerGlyphData(glyphindex, glyphdpiscale), Acquire::NORETAIN);

	int w = gd->getWidth();
	int h = gd->getHeight();

	if (w + TEXTURE_PADDING * 2 < textureWidth && h + TEXTURE_PADDING * 2 < textureHeight)
	{
		if (textureX + w + TEXTURE_PADDING > textureWidth)
		{
			// Out of space - new row!
			textureX = TEXTURE_PADDING;
			textureY += rowHeight;
			rowHeight = TEXTURE_PADDING;
		}

		if (textureY + h + TEXTURE_PADDING > textureHeight)
		{
			// Totally out of space - new texture!
			createTexture();

			// Makes sure the above code for checking if the glyph can fit at
			// the current position in the texture is run again for this glyph.
			return addGlyph(glyphindex);
		}
	}

	Glyph g;

	g.texture = nullptr;
	memset(g.vertices, 0, sizeof(GlyphVertex) * 4);

	// Don't waste space for empty glyphs.
	if (w > 0 && h > 0)
	{
		Texture *texture = textures.back();
		g.texture = texture;

		Rect rect = {textureX, textureY, gd->getWidth(), gd->getHeight()};

		if (pixelFormat != gd->getFormat())
		{
			if (!(pixelFormat == PIXELFORMAT_RGBA8_UNORM && gd->getFormat() == PIXELFORMAT_LA8_UNORM))
				throw love::Exception("Cannot upload font glyphs to texture atlas: unexpected format conversion.");

			const uint8 *src = (const uint8 *) gd->getData();

			size_t dstsize = getPixelFormatSliceSize(pixelFormat, w, h);
			std::vector<uint8> dst(dstsize, 0);
			uint8 *dstdata = dst.data();

			for (int pixel = 0; pixel < w * h; pixel++)
			{
				dstdata[pixel * 4 + 0] = src[pixel * 2 + 0];
				dstdata[pixel * 4 + 1] = src[pixel * 2 + 0];
				dstdata[pixel * 4 + 2] = src[pixel * 2 + 0];
				dstdata[pixel * 4 + 3] = src[pixel * 2 + 1];
			}

			texture->replacePixels(dstdata, dstsize, 0, 0, rect, false);
		}
		else
		{
			texture->replacePixels(gd->getData(), gd->getSize(), 0, 0, rect, false);
		}

		double tX     = (double) textureX,     tY      = (double) textureY;
		double tWidth = (double) textureWidth, tHeight = (double) textureHeight;

		Color32 c(255, 255, 255, 255);

		// Extrude the quad borders by 1 pixel. We have an extra pixel of
		// transparent padding in the texture atlas, so the quad extrusion will
		// add some antialiasing at the edges of the quad.
		float o = 1;

		// 0---2
		// | / |
		// 1---3
		const GlyphVertex verts[4] =
		{
			{ -o,  -o, normToUint16((tX-o)/tWidth),   normToUint16((tY-o)/tHeight),   c},
			{ -o, h+o, normToUint16((tX-o)/tWidth),   normToUint16((tY+h+o)/tHeight), c},
			{w+o,  -o, normToUint16((tX+w+o)/tWidth), normToUint16((tY-o)/tHeight),   c},
			{w+o, h+o, normToUint16((tX+w+o)/tWidth), normToUint16((tY+h+o)/tHeight), c}
		};

		// Copy vertex data to the glyph and set proper bearing.
		for (int i = 0; i < 4; i++)
		{
			g.vertices[i] = verts[i];
			g.vertices[i].x += gd->getBearingX();
			g.vertices[i].y -= gd->getBearingY();
			g.vertices[i].x /= glyphdpiscale;
			g.vertices[i].y /= glyphdpiscale;
		}

		textureX += w + TEXTURE_PADDING;
		rowHeight = std::max(rowHeight, h + TEXTURE_PADDING);
	}

	uint64 packedindex = packGlyphIndex(glyphindex);
	glyphs[packedindex] = g;
	return glyphs[packedindex];
}

const Font::Glyph &Font::findGlyph(love::font::TextShaper::GlyphIndex glyphindex)
{
	uint64 packedindex = packGlyphIndex(glyphindex);
	const auto it = glyphs.find(packedindex);

	if (it != glyphs.end())
		return it->second;

	return addGlyph(glyphindex);
}

float Font::getKerning(uint32 leftglyph, uint32 rightglyph)
{
	return shaper->getKerning(leftglyph, rightglyph);
}

float Font::getKerning(const std::string &leftchar, const std::string &rightchar)
{
	return shaper->getKerning(leftchar, rightchar);
}

float Font::getHeight() const
{
	return shaper->getHeight();
}

std::vector<Font::DrawCommand> Font::generateVertices(const love::font::ColoredCodepoints &codepoints, Range range, const Colorf &constantcolor, std::vector<GlyphVertex> &vertices, float extra_spacing, Vector2 offset, love::font::TextShaper::TextInfo *info)
{
	std::vector<love::font::TextShaper::GlyphPosition> glyphpositions;
	std::vector<love::font::IndexedColor> colors;
	shaper->computeGlyphPositions(codepoints, range, offset, extra_spacing, &glyphpositions, &colors, info);

	size_t vertstartsize = vertices.size();
	vertices.reserve(vertstartsize + glyphpositions.size() * 4);

	Colorf linearconstantcolor = gammaCorrectColor(constantcolor);
	Color32 curcolor = toColor32(constantcolor);

	int curcolori = 0;
	int ncolors = (int)colors.size();

	// Keeps track of when we need to switch textures in our vertex array.
	std::vector<DrawCommand> commands;

	for (int i = 0; i < (int) glyphpositions.size(); i++)
	{
		const auto &info = glyphpositions[i];

		uint32 cacheid = textureCacheID;

		const Glyph &glyph = findGlyph(info.glyphIndex);

		// If findGlyph invalidates the texture cache, restart the loop.
		if (cacheid != textureCacheID)
		{
			i = -1; // The next iteration will increment this to 0.
			commands.clear();
			vertices.resize(vertstartsize);
			curcolori = 0;
			curcolor = toColor32(constantcolor);
			continue;
		}

		if (curcolori < ncolors && colors[curcolori].index == i)
		{
			Colorf c = colors[curcolori].color;

			c.r = std::min(std::max(c.r, 0.0f), 1.0f);
			c.g = std::min(std::max(c.g, 0.0f), 1.0f);
			c.b = std::min(std::max(c.b, 0.0f), 1.0f);
			c.a = std::min(std::max(c.a, 0.0f), 1.0f);

			gammaCorrectColor(c);
			c *= linearconstantcolor;
			unGammaCorrectColor(c);

			curcolor = toColor32(c);
			curcolori++;
		}

		if (glyph.texture != nullptr)
		{
			// Copy the vertices and set their colors and relative positions.
			for (int j = 0; j < 4; j++)
			{
				vertices.push_back(glyph.vertices[j]);
				vertices.back().x += info.position.x;
				vertices.back().y += info.position.y;
				vertices.back().color = curcolor;
			}

			// Check if glyph texture has changed since the last iteration.
			if (commands.empty() || commands.back().texture != glyph.texture)
			{
				// Add a new draw command if the texture has changed.
				DrawCommand cmd;
				cmd.startvertex = (int)vertices.size() - 4;
				cmd.vertexcount = 0;
				cmd.texture = glyph.texture;
				commands.push_back(cmd);
			}

			commands.back().vertexcount += 4;
		}
	}

	const auto drawsort = [](const DrawCommand &a, const DrawCommand &b) -> bool
	{
		// Texture binds are expensive, so we should sort by that first.
		if (a.texture != b.texture)
			return a.texture < b.texture;
		else
			return a.startvertex < b.startvertex;
	};

	std::sort(commands.begin(), commands.end(), drawsort);

	return commands;
}

std::vector<Font::DrawCommand> Font::generateVerticesFormatted(const love::font::ColoredCodepoints &text, const Colorf &constantcolor, float wrap, AlignMode align, std::vector<GlyphVertex> &vertices, love::font::TextShaper::TextInfo *info)
{
	wrap = std::max(wrap, 0.0f);

	uint32 cacheid = textureCacheID;

	std::vector<DrawCommand> drawcommands;
	vertices.reserve(text.cps.size() * 4);

	std::vector<Range> ranges;
	std::vector<float> widths;
	shaper->getWrap(text, wrap, ranges, &widths);

	float y = 0.0f;
	float maxwidth = 0.0f;

	for (int i = 0; i < (int)ranges.size(); i++)
	{
		const auto &range = ranges[i];

		if (!range.isValid())
		{
			y += shaper->getCombinedHeight();
			continue;
		}

		float width = widths[i];
		love::Vector2 offset(0.0f, floorf(y));
		float extraspacing = 0.0f;

		maxwidth = std::max(width, maxwidth);

		switch (align)
		{
			case ALIGN_RIGHT:
				offset.x = floorf(wrap - width);
				break;
			case ALIGN_CENTER:
				offset.x = floorf((wrap - width) / 2.0f);
				break;
			case ALIGN_JUSTIFY:
			{
				auto start = text.cps.begin() + range.getOffset();
				auto end = start + range.getSize();
				float numspaces = std::count(start, end, ' ');

				if (text.cps[range.last] == ' ')
					--numspaces;

				if (width < wrap && numspaces >= 1)
					extraspacing = (wrap - width) / numspaces;
				else
					extraspacing = 0.0f;
				break;
			}
			case ALIGN_LEFT:
			default:
				break;
		}

		std::vector<DrawCommand> newcommands = generateVertices(text, range, constantcolor, vertices, extraspacing, offset);

		if (!newcommands.empty())
		{
			auto firstcmd = newcommands.begin();

			// If the first draw command in the new list has the same texture
			// as the last one in the existing list we're building and its
			// vertices are in-order, we can combine them (saving a draw call.)
			if (!drawcommands.empty())
			{
				auto prevcmd = drawcommands.back();
				if (prevcmd.texture == firstcmd->texture && (prevcmd.startvertex + prevcmd.vertexcount) == firstcmd->startvertex)
				{
					drawcommands.back().vertexcount += firstcmd->vertexcount;
					++firstcmd;
				}
			}

			// Append the new draw commands to the list we're building.
			drawcommands.insert(drawcommands.end(), firstcmd, newcommands.end());
		}

		y += shaper->getCombinedHeight();
	}

	if (info != nullptr)
	{
		info->width = (int) maxwidth;
		info->height = (int) y;
	}

	if (cacheid != textureCacheID)
	{
		vertices.clear();
		drawcommands = generateVerticesFormatted(text, constantcolor, wrap, align, vertices);
	}

	return drawcommands;
}

void Font::printv(graphics::Graphics *gfx, const Matrix4 &t, const std::vector<DrawCommand> &drawcommands, const std::vector<GlyphVertex> &vertices)
{
	if (vertices.empty() || drawcommands.empty())
		return;

	Matrix4 m(gfx->getTransform(), t);

	for (const DrawCommand &cmd : drawcommands)
	{
		Graphics::BatchedDrawCommand streamcmd;
		streamcmd.formats[0] = vertexFormat;
		streamcmd.indexMode = TRIANGLEINDEX_QUADS;
		streamcmd.vertexCount = cmd.vertexcount;
		streamcmd.texture = cmd.texture;

		Graphics::BatchedVertexData data = gfx->requestBatchedDraw(streamcmd);
		GlyphVertex *vertexdata = (GlyphVertex *) data.stream[0];

		memcpy(vertexdata, &vertices[cmd.startvertex], sizeof(GlyphVertex) * cmd.vertexcount);
		m.transformXY(vertexdata, &vertices[cmd.startvertex], cmd.vertexcount);
	}
}

void Font::print(graphics::Graphics *gfx, const std::vector<love::font::ColoredString> &text, const Matrix4 &m, const Colorf &constantcolor)
{
	love::font::ColoredCodepoints codepoints;
	love::font::getCodepointsFromString(text, codepoints);

	std::vector<GlyphVertex> vertices;
	std::vector<DrawCommand> drawcommands = generateVertices(codepoints, Range(), constantcolor, vertices);

	printv(gfx, m, drawcommands, vertices);
}

void Font::printf(graphics::Graphics *gfx, const std::vector<love::font::ColoredString> &text, float wrap, AlignMode align, const Matrix4 &m, const Colorf &constantcolor)
{
	love::font::ColoredCodepoints codepoints;
	love::font::getCodepointsFromString(text, codepoints);

	std::vector<GlyphVertex> vertices;
	std::vector<DrawCommand> drawcommands = generateVerticesFormatted(codepoints, constantcolor, wrap, align, vertices);

	printv(gfx, m, drawcommands, vertices);
}

int Font::getWidth(const std::string &str)
{
	return shaper->getWidth(str);
}

int Font::getWidth(uint32 glyph)
{
	return shaper->getGlyphAdvance(glyph);
}

void Font::getWrap(const love::font::ColoredCodepoints &codepoints, float wraplimit, std::vector<Range> &ranges, std::vector<float> *linewidths)
{
	shaper->getWrap(codepoints, wraplimit, ranges, linewidths);
}

void Font::getWrap(const std::vector<love::font::ColoredString> &text, float wraplimit, std::vector<std::string> &lines, std::vector<float> *linewidths)
{
	shaper->getWrap(text, wraplimit, lines, linewidths);
}

void Font::setLineHeight(float height)
{
	shaper->setLineHeight(height);
}

float Font::getLineHeight() const
{
	return shaper->getLineHeight();
}

void Font::setSamplerState(const SamplerState &s)
{
	samplerState.minFilter = s.minFilter;
	samplerState.magFilter = s.magFilter;
	samplerState.maxAnisotropy = s.maxAnisotropy;

	for (const auto &texture : textures)
		texture->setSamplerState(samplerState);
}

const SamplerState &Font::getSamplerState() const
{
	return samplerState;
}

int Font::getAscent() const
{
	return shaper->getAscent();
}

int Font::getDescent() const
{
	return shaper->getDescent();
}

float Font::getBaseline() const
{
	return shaper->getBaseline();
}

bool Font::hasGlyph(uint32 glyph) const
{
	return shaper->hasGlyph(glyph);
}

bool Font::hasGlyphs(const std::string &text) const
{
	return shaper->hasGlyphs(text);
}

void Font::setFallbacks(const std::vector<Font *> &fallbacks)
{
	std::vector<love::font::Rasterizer*> rasterizerfallbacks;
	for (const Font* f : fallbacks)
		rasterizerfallbacks.push_back(f->shaper->getRasterizers()[0]);

	shaper->setFallbacks(rasterizerfallbacks);

	// Invalidate existing textures.
	textureCacheID++;
	glyphs.clear();
	while (textures.size() > 1)
		textures.pop_back();

	rowHeight = textureX = textureY = TEXTURE_PADDING;
}

float Font::getDPIScale() const
{
	return dpiScale;
}

uint32 Font::getTextureCacheID() const
{
	return textureCacheID;
}

bool Font::getConstant(const char *in, AlignMode &out)
{
	return alignModes.find(in, out);
}

bool Font::getConstant(AlignMode in, const char  *&out)
{
	return alignModes.find(in, out);
}

std::vector<std::string> Font::getConstants(AlignMode)
{
	return alignModes.getNames();
}

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM>::Entry Font::alignModeEntries[] =
{
	{ "left", ALIGN_LEFT },
	{ "right", ALIGN_RIGHT },
	{ "center", ALIGN_CENTER },
	{ "justify", ALIGN_JUSTIFY },
};

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM> Font::alignModes(Font::alignModeEntries, sizeof(Font::alignModeEntries));

} // graphics
} // love
