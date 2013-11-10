/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "Image.h"

#include "libraries/utf8/utf8.h"

#include "common/math.h"
#include "common/Matrix.h"

#include <math.h>
#include <sstream>
#include <algorithm> // for max

namespace love
{
namespace graphics
{
namespace opengl
{

const int Font::TEXTURE_WIDTHS[]  = {128, 256, 256, 512, 512, 1024, 1024};
const int Font::TEXTURE_HEIGHTS[] = {128, 128, 256, 256, 512, 512,  1024};

Font::Font(love::font::Rasterizer *r, const Image::Filter &filter)
	: rasterizer(r)
	, height(r->getHeight())
	, lineHeight(1)
	, mSpacing(1)
	, filter(filter)
{
	this->filter.mipmap = Image::FILTER_NONE;

	// Try to find the best texture size match for the font size. default to the
	// largest texture size if no rough match is found.
	textureSizeIndex = NUM_TEXTURE_SIZES - 1;
	for (int i = 0; i < NUM_TEXTURE_SIZES; i++)
	{
		// Make a rough estimate of the total used texture size, based on glyph
		// height. The estimated size is likely larger than the actual total
		// size, which is good because texture switching is expensive.
		if ((height * 0.8) * height * 95 <= TEXTURE_WIDTHS[i] * TEXTURE_HEIGHTS[i])
		{
			textureSizeIndex = i;
			break;
		}
	}

	textureWidth = TEXTURE_WIDTHS[textureSizeIndex];
	textureHeight = TEXTURE_HEIGHTS[textureSizeIndex];

	love::font::GlyphData *gd = 0;

	try
	{
		gd = r->getGlyphData(32);
		type = (gd->getFormat() == love::font::GlyphData::FORMAT_LUMINANCE_ALPHA) ? FONT_TRUETYPE : FONT_IMAGE;

		loadVolatile();
	}
	catch (love::Exception &)
	{
		delete gd;
		throw;
	}

	delete gd;

	rasterizer->retain();
}

Font::~Font()
{
	rasterizer->release();
	unloadVolatile();
}

bool Font::initializeTexture(GLint format)
{
	GLint internalformat = (format == GL_LUMINANCE_ALPHA) ? GL_LUMINANCE8_ALPHA8 : GL_RGBA8;

	// clear errors before initializing
	while (glGetError() != GL_NO_ERROR);

	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 internalformat,
				 (GLsizei)textureWidth,
				 (GLsizei)textureHeight,
				 0,
				 format,
				 GL_UNSIGNED_BYTE,
				 NULL);

	return glGetError() == GL_NO_ERROR;
}

void Font::createTexture()
{
	textureX = textureY = rowHeight = TEXTURE_PADDING;

	GLuint t;
	glGenTextures(1, &t);
	textures.push_back(t);

	gl.bindTexture(t);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint format = (type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA);

	// Initialize the texture, attempting smaller sizes if initialization fails.
	bool initialized = false;
	while (textureSizeIndex >= 0)
	{
		textureWidth = TEXTURE_WIDTHS[textureSizeIndex];
		textureHeight = TEXTURE_HEIGHTS[textureSizeIndex];

		initialized = initializeTexture(format);

		if (initialized || textureSizeIndex <= 0)
			break;

		--textureSizeIndex;
	}

	if (!initialized)
	{
		// Clean up before throwing.
		gl.deleteTexture(t);
		gl.bindTexture(0);
		textures.pop_back();

		throw love::Exception("Could not create font texture!");
	}
	
	// Fill the texture with transparent black.
	std::vector<GLubyte> emptyData(textureWidth * textureHeight * (type == FONT_TRUETYPE ? 2 : 4), 0);
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					0, 0,
					(GLsizei)textureWidth,
					(GLsizei)textureHeight,
					format,
					GL_UNSIGNED_BYTE,
					&emptyData[0]);

	setFilter(filter);
}

Font::Glyph *Font::addGlyph(uint32 glyph)
{
	love::font::GlyphData *gd = rasterizer->getGlyphData(glyph);
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
		createTexture();
	}

	Glyph *g = new Glyph;

	g->texture = 0;
	g->spacing = gd->getAdvance();

	memset(g->vertices, 0, sizeof(GlyphVertex) * 4);

	// don't waste space for empty glyphs. also fixes a division by zero bug with ati drivers
	if (w > 0 && h > 0)
	{
		const GLuint t = textures.back();

		gl.bindTexture(t);
		glTexSubImage2D(GL_TEXTURE_2D,
						0,
						textureX,
						textureY,
						w, h,
						(type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA),
						GL_UNSIGNED_BYTE,
						gd->getData());

		g->texture = t;

		const GlyphVertex verts[4] = {
			{    0.0f,     0.0f, float(textureX)/float(textureWidth),   float(textureY)/float(textureHeight)},
			{    0.0f, float(h), float(textureX)/float(textureWidth),   float(textureY+h)/float(textureHeight)},
			{float(w), float(h), float(textureX+w)/float(textureWidth), float(textureY+h)/float(textureHeight)},
			{float(w),     0.0f, float(textureX+w)/float(textureWidth), float(textureY)/float(textureHeight)},
		};

		// copy vertex data to the glyph and set proper bearing
		for (int i = 0; i < 4; i++)
		{
			g->vertices[i] = verts[i];
			g->vertices[i].x += gd->getBearingX();
			g->vertices[i].y -= gd->getBearingY();
		}
	}

	if (w > 0)
		textureX += (w + TEXTURE_PADDING);
	if (h > 0)
		rowHeight = std::max(rowHeight, h + TEXTURE_PADDING);

	delete gd;

	glyphs[glyph] = g;

	return g;
}

Font::Glyph *Font::findGlyph(uint32 glyph)
{
	auto it = glyphs.find(glyph);

	if (it != glyphs.end())
		return it->second;
	else
		return addGlyph(glyph);
}

float Font::getHeight() const
{
	return static_cast<float>(height);
}

void Font::print(const std::string &text, float x, float y, float extra_spacing, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	// Spacing counter and newline handling.
	float dx = 0.0f;
	float dy = 0.0f;

	float lineheight = getBaseline();

	// Keeps track of when we need to switch textures in our vertex array.
	std::vector<GlyphArrayDrawInfo> glyphinfolist;

	// Pre-allocate space for the maximum possible number of vertices.
	std::vector<GlyphVertex> glyphverts;
	glyphverts.reserve(text.length() * 4);

	int vertexcount = 0;

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
		{
			uint32 g = *i++;

			if (g == '\n')
			{
				// Wrap newline, but do not print it.
				dy += floorf(getHeight() * getLineHeight() + 0.5f);
				dx = 0.0f;
				continue;
			}

			Glyph *glyph = findGlyph(g);

			if (glyph->texture != 0)
			{
				// Copy the vertices and set their proper relative positions.
				for (int j = 0; j < 4; j++)
				{
					glyphverts.push_back(glyph->vertices[j]);
					glyphverts.back().x += dx;
					glyphverts.back().y += dy + lineheight;
				}

				// Check if glyph texture has changed since the last iteration.
				if (glyphinfolist.size() == 0 || glyphinfolist.back().texture != glyph->texture)
				{
					// keep track of each sub-section of the string whose glyphs use different textures than the previous section
					GlyphArrayDrawInfo gdrawinfo;
					gdrawinfo.startvertex = vertexcount;
					gdrawinfo.vertexcount = 0;
					gdrawinfo.texture = glyph->texture;
					glyphinfolist.push_back(gdrawinfo);
				}

				vertexcount += 4;
				glyphinfolist.back().vertexcount += 4;
			}

			// Advance the x position for the next glyph.
			dx += glyph->spacing;

			// Account for extra spacing given to space characters.
			if (g == ' ' && extra_spacing != 0.0f)
				dx = floorf(dx + extra_spacing);
		}
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("Decoding error: %s", e.what());
	}

	if (vertexcount <= 0 || glyphinfolist.size() == 0)
		return;

	// Sort glyph draw info list by texture first, and quad position in memory
	// second (using the struct's < operator).
	std::sort(glyphinfolist.begin(), glyphinfolist.end());

	glPushMatrix();

	Matrix t;
	t.setTransformation(ceilf(x), ceilf(y), angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(GlyphVertex), (GLvoid *)&glyphverts[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(GlyphVertex), (GLvoid *)&glyphverts[0].s);

	// We need to draw a new vertex array for every section of the string which
	// uses a different texture than the previous section.
	std::vector<GlyphArrayDrawInfo>::const_iterator it;
	for (it = glyphinfolist.begin(); it != glyphinfolist.end(); ++it)
	{
		gl.bindTexture(it->texture);
		glDrawArrays(GL_QUADS, it->startvertex, it->vertexcount);
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

int Font::getWidth(const std::string &str)
{
	if (str.size() == 0) return 0;

	std::istringstream iss(str);
	std::string line;
	Glyph *g;
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
				g = findGlyph(c);
				width += static_cast<int>(g->spacing * mSpacing);
			}
		}
		catch(utf8::exception &e)
		{
			throw love::Exception("Decoding error: %s", e.what());
		}

		if (width > max_width)
			max_width = width;
	}

	return max_width;
}

int Font::getWidth(char character)
{
	Glyph *g = findGlyph(character);
	return g->spacing;
}

std::vector<std::string> Font::getWrap(const std::string &text, float wrap, int *max_width, std::vector<bool> *wrappedlines)
{
	using namespace std;
	const float width_space = static_cast<float>(getWidth(' '));
	vector<string> lines_to_draw;
	int maxw = 0;

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
				lines_to_draw.push_back(tmp.substr(0,tmp.size()-1));
				string_builder.str("");
				width = static_cast<float>(getWidth(word));
				realw -= (int) width;
				if (realw > maxw)
					maxw = realw;

				// Indicate that this line was automatically wrapped.
				if (wrappedlines)
					wrappedlines->push_back(true);
			}
			string_builder << word << " ";
			width += width_space;
			oldwidth = width;
		}
		// push last line
		if (width > maxw)
			maxw = (int) width;
		string tmp = string_builder.str();
		lines_to_draw.push_back(tmp.substr(0,tmp.size()-1));

		// Indicate that this line was not automatically wrapped.
		if (wrappedlines)
			wrappedlines->push_back(false);
	}

	if (max_width)
		 *max_width = maxw;

	return lines_to_draw;
}

void Font::setLineHeight(float height)
{
	this->lineHeight = height;
}

float Font::getLineHeight() const
{
	return lineHeight;
}

void Font::setSpacing(float amount)
{
	mSpacing = amount;
}

float Font::getSpacing() const
{
	return mSpacing;
}

void Font::setFilter(const Image::Filter &f)
{
	filter = f;

	for (auto it = textures.begin(); it != textures.end(); ++it)
	{
		gl.bindTexture(*it);
		filter.anisotropy = gl.setTextureFilter(f);
	}
}

const Image::Filter &Font::getFilter()
{
	return filter;
}

bool Font::loadVolatile()
{
	createTexture();
	return true;
}

void Font::unloadVolatile()
{
	// nuke everything from orbit
	std::map<uint32, Glyph *>::iterator it = glyphs.begin();
	Glyph *g;
	while (it != glyphs.end())
	{
		g = it->second;
		delete g;
		glyphs.erase(it++);
	}
	std::vector<GLuint>::iterator iter = textures.begin();
	while (iter != textures.end())
	{
		gl.deleteTexture(*iter);
		iter++;
	}
	textures.clear();
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

} // opengl
} // graphics
} // love
