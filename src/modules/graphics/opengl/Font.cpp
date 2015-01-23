/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "libraries/utf8/utf8.h"

#include "common/math.h"
#include "common/Matrix.h"

#include <math.h>
#include <sstream>
#include <algorithm> // for max
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

int Font::fontCount = 0;

Font::Font(love::font::Rasterizer *r, const Texture::Filter &filter)
	: rasterizer(r)
	, height(r->getHeight())
	, lineHeight(1)
	, textureWidth(128)
	, textureHeight(128)
	, filter(filter)
	, useSpacesAsTab(false)
	, indexBuffer(20) // We make this bigger at draw-time, if needed.
	, textureCacheID(0)
	, textureMemorySize(0)
{
	this->filter.mipmap = Texture::FILTER_NONE;

	// Try to find the best texture size match for the font size. default to the
	// largest texture size if no rough match is found.
	while (true)
	{
		if ((height * 0.8) * height * 30 <= textureWidth * textureHeight)
			break;

		TextureSize nextsize = getNextTextureSize();

		if (nextsize.width <= textureWidth && nextsize.height <= textureHeight)
			break;

		textureWidth = nextsize.width;
		textureHeight = nextsize.height;
	}

	love::font::GlyphData *gd = r->getGlyphData(32); // Space character.
	type = (gd->getFormat() == font::GlyphData::FORMAT_LUMINANCE_ALPHA) ? FONT_TRUETYPE : FONT_IMAGE;
	gd->release();

	if (!r->hasGlyph(9)) // No tab character in the Rasterizer.
		useSpacesAsTab = true;

	loadVolatile();

	++fontCount;
}

Font::~Font()
{
	unloadVolatile();

	--fontCount;
}

Font::TextureSize Font::getNextTextureSize() const
{
	TextureSize size = {textureWidth, textureHeight};

	int maxsize = std::min(4096, gl.getMaxTextureSize());

	if (size.width * 2 <= maxsize || size.height * 2 <= maxsize)
	{
		// {128, 128} -> {256, 128} -> {256, 256} -> {512, 256} -> etc.
		if (size.width == size.height)
			size.width *= 2;
		else
			size.height *= 2;
	}

	return size;
}

void Font::createTexture()
{
	GLenum format = type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA;
	size_t bpp = format == GL_LUMINANCE_ALPHA ? 2 : 4;

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

	gl.bindTexture(t);

	gl.setTextureFilter(filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLenum internalformat = type == FONT_TRUETYPE ? GL_LUMINANCE8_ALPHA8 : GL_RGBA8;

	// in GLES2, the internalformat and format params of TexImage have to match.
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
		internalformat = format;

	// Initialize the texture with transparent black.
	std::vector<GLubyte> emptydata(size.width * size.height * bpp, 0);

	// Clear errors before initializing.
	while (glGetError() != GL_NO_ERROR);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, size.width, size.height, 0,
	             format, GL_UNSIGNED_BYTE, &emptydata[0]);

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

love::font::GlyphData *Font::getRasterizerGlyphData(uint32 glyph)
{
	// Use spaces for the tab 'glyph'.
	if (glyph == 9 && useSpacesAsTab)
	{
		love::font::GlyphData *spacegd = rasterizer->getGlyphData(32);
		love::font::GlyphData::Format fmt = spacegd->getFormat();

		love::font::GlyphMetrics gm = {};
		gm.advance = spacegd->getAdvance() * SPACES_PER_TAB;
		gm.bearingX = spacegd->getBearingX();
		gm.bearingY = spacegd->getBearingY();

		spacegd->release();

		return new love::font::GlyphData(glyph, gm, fmt);
	}

	return rasterizer->getGlyphData(glyph);
}

const Font::Glyph &Font::addGlyph(uint32 glyph)
{
	love::font::GlyphData *gd = getRasterizerGlyphData(glyph);

	int w = gd->getWidth();
	int h = gd->getHeight();

	if (textureX + w + TEXTURE_PADDING > textureWidth)
	{
		// out of space - new row!
		textureX = TEXTURE_PADDING;
		textureY += rowHeight;
		rowHeight = TEXTURE_PADDING;
	}
	if (textureY + h + TEXTURE_PADDING > textureHeight)
	{
		// totally out of space - new texture!
		try
		{
			createTexture();
		}
		catch (love::Exception &)
		{
			gd->release();
			throw;
		}
	}

	Glyph g;

	g.texture = 0;
	g.spacing = gd->getAdvance();

	memset(g.vertices, 0, sizeof(GlyphVertex) * 4);

	// don't waste space for empty glyphs. also fixes a divide by zero bug with ATI drivers
	if (w > 0 && h > 0)
	{
		const GLuint t = textures.back();

		gl.bindTexture(t);
		glTexSubImage2D(GL_TEXTURE_2D, 0, textureX, textureY, w, h,
		                (type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA),
		                GL_UNSIGNED_BYTE, gd->getData());

		g.texture = t;

		float tX     = (float) textureX,     tY      = (float) textureY;
		float tWidth = (float) textureWidth, tHeight = (float) textureHeight;

		const GlyphVertex verts[4] = {
			{    0.0f,     0.0f,     tX/tWidth,     tY/tHeight},
			{    0.0f, float(h),     tX/tWidth, (tY+h)/tHeight},
			{float(w), float(h), (tX+w)/tWidth, (tY+h)/tHeight},
			{float(w),     0.0f, (tX+w)/tWidth,     tY/tHeight}
		};

		// Copy vertex data to the glyph and set proper bearing.
		for (int i = 0; i < 4; i++)
		{
			g.vertices[i] = verts[i];
			g.vertices[i].x += gd->getBearingX();
			g.vertices[i].y -= gd->getBearingY();
		}
	}

	if (w > 0)
		textureX += (w + TEXTURE_PADDING);

	if (h > 0)
		rowHeight = std::max(rowHeight, h + TEXTURE_PADDING);

	gd->release();

	const auto p = glyphs.insert(std::make_pair(glyph, g));

	return p.first->second;
}

const Font::Glyph &Font::findGlyph(uint32 glyph)
{
	const auto it = glyphs.find(glyph);

	if (it != glyphs.end())
		return it->second;

	return addGlyph(glyph);
}

float Font::getHeight() const
{
	return (float) height;
}

std::vector<Font::DrawCommand> Font::generateVertices(const std::string &text, std::vector<GlyphVertex> &vertices, float extra_spacing, Vector offset, TextInfo *info)
{
	// Spacing counter and newline handling.
	float dx = offset.x;
	float dy = offset.y;

	float lineheight = getBaseline();
	int maxwidth = 0;

	// Keeps track of when we need to switch textures in our vertex array.
	std::vector<DrawCommand> drawcommands;

	// Pre-allocate space for the maximum possible number of vertices.
	size_t vertstartsize = vertices.size();
	vertices.reserve(vertstartsize + text.length() * 4);

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
		{
			uint32 g = *i++;

			if (g == '\n')
			{
				if (dx > maxwidth)
					maxwidth = (int) dx;

				// Wrap newline, but do not print it.
				dy += floorf(getHeight() * getLineHeight() + 0.5f);
				dx = offset.x;
				continue;
			}

			uint32 cacheid = textureCacheID;

			const Glyph &glyph = findGlyph(g);

			// If findGlyph invalidates the texture cache, re-start the loop.
			if (cacheid != textureCacheID)
			{
				i = utf8::iterator<std::string::const_iterator>(text.begin(), text.begin(), text.end());
				maxwidth = 0;
				dx = offset.x;
				dy = offset.y;
				drawcommands.clear();
				vertices.resize(vertstartsize);
				continue;
			}

			if (glyph.texture != 0)
			{
				// Copy the vertices and set their proper relative positions.
				for (int j = 0; j < 4; j++)
				{
					vertices.push_back(glyph.vertices[j]);
					vertices.back().x += dx;
					vertices.back().y += dy + lineheight;
				}

				// Check if glyph texture has changed since the last iteration.
				if (drawcommands.empty() || drawcommands.back().texture != glyph.texture)
				{
					// Add a new draw command if the texture has changed.
					DrawCommand cmd;
					cmd.startvertex = (int) vertices.size() - 4;
					cmd.vertexcount = 0;
					cmd.texture = glyph.texture;
					drawcommands.push_back(cmd);
				}

				drawcommands.back().vertexcount += 4;
			}

			// Advance the x position for the next glyph.
			dx += glyph.spacing;

			// Account for extra spacing given to space characters.
			if (g == ' ' && extra_spacing != 0.0f)
				dx = floorf(dx + extra_spacing);
		}
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	// Sort draw commands by texture first, and quad position in memory second
	// (using the struct's < operator).
	std::sort(drawcommands.begin(), drawcommands.end());

	if (dx > maxwidth)
		maxwidth = (int) dx;

	if (info != nullptr)
	{
		info->width = maxwidth - offset.x;;
		info->height = (int) dy + (dx > 0.0f ? floorf(getHeight() * getLineHeight() + 0.5f) : 0) - offset.y;
	}

	return drawcommands;
}

std::vector<Font::DrawCommand> Font::generateVerticesFormatted(const std::string &text, float wrap, AlignMode align, std::vector<GlyphVertex> &vertices, TextInfo *info)
{
	if (wrap < 0.0f)
		wrap = std::numeric_limits<float>::max();

	uint32 cacheid = textureCacheID;

	std::vector<DrawCommand> drawcommands;
	vertices.reserve(text.length() * 4);

	// wrappedlines indicates which lines were automatically wrapped. It
	// has the same number of elements as lines_to_draw.
	std::vector<bool> wrappedlines;
	std::vector<int> widths;
	std::vector<std::string> lines;

	// We only need the list of wrapped lines in 'justify' mode.
	getWrap(text, wrap, lines, &widths, align == ALIGN_JUSTIFY ? &wrappedlines : nullptr);

	float extraspacing = 0.0f;
	int numspaces = 0;
	int i = 0;
	float y = 0.0f;
	float maxwidth = 0;

	for (const std::string &line : lines)
	{
		extraspacing = 0.0f;
		float width = (float) widths[i];
		love::Vector offset(0.0f, floorf(y));

		if (width > maxwidth)
			maxwidth = width;

		switch (align)
		{
		case ALIGN_RIGHT:
			offset.x = floorf(wrap - width);
			break;
		case ALIGN_CENTER:
			offset.x = floorf((wrap - width) / 2.0f);
			break;
		case ALIGN_JUSTIFY:
			numspaces = std::count(line.begin(), line.end(), ' ');
			if (wrappedlines[i] && numspaces >= 1)
				extraspacing = (wrap - width) / float(numspaces);
			else
				extraspacing = 0.0f;
			break;
		case ALIGN_LEFT:
		default:
			break;
		}

		std::vector<DrawCommand> commands = generateVertices(line, vertices, extraspacing, offset);

		if (!commands.empty())
		{
			// If the first draw command in the new list has the same texture
			// as the last one in the existing list we're building and its
			// vertices are in-order, we can combine them (saving a draw call.)
			auto firstcmd = commands.begin();
			auto prevcmd = drawcommands.back();
			if (!drawcommands.empty() && prevcmd.texture == firstcmd->texture
				&& (prevcmd.startvertex + prevcmd.vertexcount) == firstcmd->startvertex)
			{
				drawcommands.back().vertexcount += firstcmd->vertexcount;
				++firstcmd;
			}

			// Append the new draw commands to the list we're building.
			drawcommands.insert(drawcommands.end(), firstcmd, commands.end());
		}

		y += getHeight() * getLineHeight();
		i++;
	}

	if (info != nullptr)
	{
		info->width = (int) maxwidth;
		info->height = (int) y;
	}

	if (cacheid != textureCacheID)
	{
		vertices.clear();
		drawcommands = generateVerticesFormatted(text, wrap, align, vertices);
	}

	return drawcommands;
}

void Font::drawVertices(const std::vector<DrawCommand> &drawcommands)
{
	// Vertex attribute pointers need to be set before calling this function.
	// This assumes that the attribute pointers are constant for all vertices.

	int totalverts = 0;
	for (const DrawCommand &cmd : drawcommands)
		totalverts = std::max(cmd.startvertex + cmd.vertexcount, totalverts);

	if ((size_t) totalverts / 4 > indexBuffer.getSize())
		indexBuffer = VertexIndex((size_t) totalverts / 4);

	gl.prepareDraw();

	const GLenum gltype = indexBuffer.getType();
	const size_t elemsize = indexBuffer.getElementSize();

	VertexBuffer::Bind bind(*indexBuffer.getVertexBuffer());

	// We need a separate draw call for every section of the text which uses a
	// different texture than the previous section.
	for (const DrawCommand &cmd : drawcommands)
	{
		GLsizei count = (cmd.vertexcount / 4) * 6;
		size_t offset = (cmd.startvertex / 4) * 6 * elemsize;

		// TODO: Use glDrawElementsBaseVertex when supported?
		gl.bindTexture(cmd.texture);
		gl.drawElements(GL_TRIANGLES, count, gltype, indexBuffer.getPointer(offset));
	}
}

void Font::printv(const Matrix &t, const std::vector<DrawCommand> &drawcommands, const std::vector<GlyphVertex> &vertices)
{
	if (vertices.empty() || drawcommands.empty())
		return;

	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	glEnableVertexAttribArray(ATTRIB_POS);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), &vertices[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), &vertices[0].s);

	try
	{
		drawVertices(drawcommands);
	}
	catch (love::Exception &)
	{
		glDisableVertexAttribArray(ATTRIB_TEXCOORD);
		glDisableVertexAttribArray(ATTRIB_POS);
		throw;
	}

	glDisableVertexAttribArray(ATTRIB_TEXCOORD);
	glDisableVertexAttribArray(ATTRIB_POS);
}

void Font::print(const std::string &text, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	std::vector<GlyphVertex> vertices;
	std::vector<DrawCommand> drawcommands = generateVertices(text, vertices);

	Matrix t;
	t.setTransformation(ceilf(x), ceilf(y), angle, sx, sy, ox, oy, kx, ky);

	printv(t, drawcommands, vertices);
}

void Font::printf(const std::string &text, float x, float y, float wrap, AlignMode align, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	std::vector<GlyphVertex> vertices;
	std::vector<DrawCommand> drawcommands = generateVerticesFormatted(text, wrap, align, vertices);

	Matrix t;
	t.setTransformation(ceilf(x), ceilf(y), angle, sx, sy, ox, oy, kx, ky);

	printv(t, drawcommands, vertices);
}

int Font::getWidth(const std::string &str)
{
	if (str.size() == 0) return 0;

	std::istringstream iss(str);
	std::string line;
	int max_width = 0;

	while (getline(iss, line, '\n'))
	{
		int width = 0;
		try
		{
			utf8::iterator<std::string::const_iterator> i(line.begin(), line.begin(), line.end());
			utf8::iterator<std::string::const_iterator> end(line.end(), line.begin(), line.end());
			while (i != end)
			{
				uint32 c = *i++;
				const Glyph &g = findGlyph(c);
				width += g.spacing;
			}
		}
		catch(utf8::exception &e)
		{
			throw love::Exception("UTF-8 decoding error: %s", e.what());
		}

		if (width > max_width)
			max_width = width;
	}

	return max_width;
}

int Font::getWidth(char character)
{
	const Glyph &g = findGlyph(character);
	return g.spacing;
}

void Font::getWrap(const std::string &text, float wrap, std::vector<std::string> &lines, std::vector<int> *linewidths, std::vector<bool> *wrappedlines)
{
	using namespace std;
	const float width_space = (float) getWidth(' ');

	//split text at newlines
	istringstream iss(text);
	string line;
	ostringstream string_builder;
	while (getline(iss, line, '\n'))
	{
		// split line into words
		vector<string> words;
		istringstream word_iss(line);
		copy(istream_iterator<string>(word_iss), istream_iterator<string>(),
			 back_inserter< vector<string> >(words));

		// put words back together until a wrap occurs
		float width = 0.0f;
		float oldwidth = 0.0f;
		string_builder.str("");
		vector<string>::const_iterator word_iter, wend = words.end();
		for (word_iter = words.begin(); word_iter != wend; ++word_iter)
		{
			const string &word = *word_iter;
			width += getWidth(word);

			// on wordwrap, push line to line buffer and clear string builder
			if (width > wrap && oldwidth > 0)
			{
				int realw = (int) width;

				// remove trailing space
				string tmp = string_builder.str();
				lines.push_back(tmp.substr(0,tmp.size()-1));
				string_builder.str("");
				width = static_cast<float>(getWidth(word));
				realw -= (int) width;

				if (linewidths)
					linewidths->push_back(realw);

				// Indicate that this line was automatically wrapped.
				if (wrappedlines)
					wrappedlines->push_back(true);
			}
			string_builder << word << " ";
			width += width_space;
			oldwidth = width;
		}
		// push last line
		if (linewidths)
			linewidths->push_back(width);

		string tmp = string_builder.str();
		lines.push_back(tmp.substr(0,tmp.size()-1));

		// Indicate that this line was not automatically wrapped.
		if (wrappedlines)
			wrappedlines->push_back(false);
	}
}

void Font::setLineHeight(float height)
{
	lineHeight = height;
}

float Font::getLineHeight() const
{
	return lineHeight;
}

void Font::setFilter(const Texture::Filter &f)
{
	if (!Texture::validateFilter(f, false))
		throw love::Exception("Invalid texture filter.");

	filter = f;

	for (GLuint texture : textures)
	{
		gl.bindTexture(texture);
		gl.setTextureFilter(filter);
	}
}

const Texture::Filter &Font::getFilter()
{
	return filter;
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

int Font::getAscent() const
{
	return rasterizer->getAscent();
}

int Font::getDescent() const
{
	return rasterizer->getDescent();
}

float Font::getBaseline() const
{
	// 1.25 is magic line height for true type fonts
	return (type == FONT_TRUETYPE) ? floorf(getHeight() / 1.25f + 0.5f) : 0.0f;
}

bool Font::hasGlyph(uint32 glyph) const
{
	return rasterizer->hasGlyph(glyph);
}

bool Font::hasGlyphs(const std::string &text) const
{
	return rasterizer->hasGlyphs(text);
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

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM>::Entry Font::alignModeEntries[] =
{
	{ "left", Font::ALIGN_LEFT },
	{ "right", Font::ALIGN_RIGHT },
	{ "center", Font::ALIGN_CENTER },
	{ "justify", Font::ALIGN_JUSTIFY },
};

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM> Font::alignModes(Font::alignModeEntries, sizeof(Font::alignModeEntries));

} // opengl
} // graphics
} // love
