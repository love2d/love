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
#include "common/config.h"
#include "Font.h"
#include "font/GlyphData.h"
#include "Quad.h"
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
	, mipmapsharpness(0.0f)
{
	love::font::GlyphData *gd = r->getGlyphData(32);
	type = (gd->getFormat() == love::font::GlyphData::FORMAT_LUMINANCE_ALPHA ? FONT_TRUETYPE : FONT_IMAGE);
	delete gd;

	// try to find the best texture size match for the font size
	// default to the largest texture size if no rough match is found
	texture_size_index = NUM_TEXTURE_SIZES - 1;
	for (int i = 0; i < NUM_TEXTURE_SIZES; i++)
	{
		// base our chosen texture width/height on a very rough guess of the total size taken up by the font's used glyphs
		// the estimate is likely larger than the actual total size taken up, which is good since texture changes are expensive
		if ((height * 0.8) * height * 95 <= TEXTURE_WIDTHS[i] * TEXTURE_HEIGHTS[i])
		{
			texture_size_index = i;
			break;
		}
	}

	texture_width = TEXTURE_WIDTHS[texture_size_index];
	texture_height = TEXTURE_HEIGHTS[texture_size_index];

	loadVolatile();

	r->retain();
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
				 (GLsizei)texture_width,
				 (GLsizei)texture_height,
				 0,
				 format,
				 GL_UNSIGNED_BYTE,
				 NULL);

	return glGetError() == GL_NO_ERROR;
}

void Font::createTexture()
{
	texture_x = texture_y = rowHeight = TEXTURE_PADDING;

	GLuint t;
	glGenTextures(1, &t);
	textures.push_back(t);

	bindTexture(t);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint format = (type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA);

	// try to initialize the texture, attempting smaller sizes if initialization fails
	bool initialized = false;
	while (texture_size_index >= 0)
	{
		texture_width = TEXTURE_WIDTHS[texture_size_index];
		texture_height = TEXTURE_HEIGHTS[texture_size_index];

		initialized = initializeTexture(format);

		if (initialized || texture_size_index <= 0)
			break;

		--texture_size_index;
	}

	if (!initialized)
	{
		// cleanup before throwing
		deleteTexture(t);
		bindTexture(0);
		textures.pop_back();

		throw love::Exception("Could not create font texture!");
	}
	
	// Fill the texture with transparent black
	std::vector<GLubyte> emptyData(texture_width * texture_height * (type == FONT_TRUETYPE ? 2 : 4), 0);
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					0, 0,
					(GLsizei)texture_width,
					(GLsizei)texture_height,
					format,
					GL_UNSIGNED_BYTE,
					&emptyData[0]);

	setFilter(filter);
	setMipmapSharpness(mipmapsharpness);
}

Font::Glyph *Font::addGlyph(const int glyph)
{
	love::font::GlyphData *gd = rasterizer->getGlyphData(glyph);
	int w = gd->getWidth();
	int h = gd->getHeight();

	if (texture_x + w + TEXTURE_PADDING > texture_width)
	{
		// out of space - new row!
		texture_x = TEXTURE_PADDING;
		texture_y += rowHeight;
		rowHeight = TEXTURE_PADDING;
	}
	if (texture_y + h + TEXTURE_PADDING > texture_height)
	{
		// totally out of space - new texture!
		createTexture();
	}

	Glyph *g = new Glyph;

	g->texture = 0;
	g->spacing = gd->getAdvance();

	memset(&g->quad, 0, sizeof(GlyphQuad));

	// don't waste space for empty glyphs. also fixes a division by zero bug with ati drivers
	if (w > 0 && h > 0)
	{
		const GLuint t = textures.back();

		bindTexture(t);
		glTexSubImage2D(GL_TEXTURE_2D,
						0,
						texture_x,
						texture_y,
						w, h,
						(type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA),
						GL_UNSIGNED_BYTE,
						gd->getData());

		g->texture = t;

		Quad::Viewport v;
		v.x = (float) texture_x;
		v.y = (float) texture_y;
		v.w = (float) w;
		v.h = (float) h;

		Quad q = Quad(v, (const float) texture_width, (const float) texture_height);
		const vertex *verts = q.getVertices();

		// copy vertex data to the glyph and set proper bearing
		for (int i = 0; i < 4; i++)
		{
			g->quad.vertices[i] = verts[i];
			g->quad.vertices[i].x += gd->getBearingX();
			g->quad.vertices[i].y -= gd->getBearingY();
		}
	}

	if (w > 0)
		texture_x += (w + TEXTURE_PADDING);
	if (h > 0)
		rowHeight = std::max(rowHeight, h + TEXTURE_PADDING);

	delete gd;

	glyphs[glyph] = g;

	return g;
}

Font::Glyph *Font::findGlyph(const int glyph)
{
	Glyph *g = glyphs[glyph];
	if (!g)
		g = addGlyph(glyph);

	return g;
}

float Font::getHeight() const
{
	return static_cast<float>(height);
}

void Font::print(const std::string &text, float x, float y, float letter_spacing, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	float dx = 0.0f; // spacing counter for newline handling
	float dy = 0.0f;

	// keeps track of when we need to switch textures in our vertex array
	std::vector<GlyphArrayDrawInfo> glyphinfolist;

	std::vector<GlyphQuad> glyphquads;
	glyphquads.reserve(text.size()); // pre-allocate space for the maximum possible number of quads

	int quadindex = 0;

	glPushMatrix();

	Matrix t;
	t.setTransformation(ceil(x), ceil(y), angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
		{
			int g = *i++;

			if (g == '\n')
			{
				// wrap newline, but do not print it
				dy += floor(getHeight() * getLineHeight() + 0.5f);
				dx = 0.0f;
				continue;
			}

			Glyph *glyph = findGlyph(g);

			// we only care about the vertices of glyphs which have a texture
			if (glyph->texture != 0)
			{
				// copy glyphquad (4 vertices) from original glyph to our current quad list
				glyphquads.push_back(glyph->quad);

				float lineheight = getBaseline();

				// set proper relative position
				for (int i = 0; i < 4; i++)
				{
					glyphquads[quadindex].vertices[i].x += dx;
					glyphquads[quadindex].vertices[i].y += dy + lineheight;
				}

				size_t listsize = glyphinfolist.size();

				// check if current glyph texture has changed since the previous iteration
				if (listsize == 0 || glyphinfolist[listsize-1].texture != glyph->texture)
				{
					// keep track of each sub-section of the string whose glyphs use different textures than the previous section
					GlyphArrayDrawInfo glyphdrawinfo;
					glyphdrawinfo.startquad = quadindex;
					glyphdrawinfo.numquads = 0;
					glyphdrawinfo.texture = glyph->texture;
					glyphinfolist.push_back(glyphdrawinfo);
				}

				++quadindex;
				++glyphinfolist[glyphinfolist.size()-1].numquads;
			}

			// advance the x position for the next glyph
			dx += glyph->spacing + letter_spacing;
		}
	}
	catch (love::Exception &)
	{
		glPopMatrix();
		throw;
	}
	catch (utf8::exception &e)
	{
		glPopMatrix();
		throw love::Exception("%s", e.what());
	}
	
	if (quadindex > 0 && glyphinfolist.size() > 0)
	{
		// sort glyph draw info list by texture first, and quad position in memory second (using the struct's < operator)
		std::sort(glyphinfolist.begin(), glyphinfolist.end());

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&glyphquads[0].vertices[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&glyphquads[0].vertices[0].s);

		// we need to draw a new vertex array for every section of the string that uses a different texture than the previous section
		std::vector<GlyphArrayDrawInfo>::const_iterator it;
		for (it = glyphinfolist.begin(); it != glyphinfolist.end(); ++it)
		{
			bindTexture(it->texture);

			int startvertex = it->startquad * 4;
			int numvertices = it->numquads * 4;

			glDrawArrays(GL_QUADS, startvertex, numvertices);
		}

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

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
				int c = *i++;
				g = findGlyph(c);
				width += static_cast<int>(g->spacing * mSpacing);
			}
		}
		catch(utf8::exception &e)
		{
			throw love::Exception("%s", e.what());
		}

		if (width > max_width)
			max_width = width;
	}

	return max_width;
}

int Font::getWidth(const char *str)
{
	return this->getWidth(std::string(str));
}

int Font::getWidth(const char character)
{
	Glyph *g = findGlyph(character);
	return g->spacing;
}

std::vector<std::string> Font::getWrap(const std::string &text, float wrap, int *max_width)
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
			if (width >= wrap && oldwidth > 0)
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

void Font::checkMipmapsCreated() const
{
	if (filter.mipmap != Image::FILTER_NEAREST && filter.mipmap != Image::FILTER_LINEAR)
		return;

	if (!Image::hasMipmapSupport())
		throw love::Exception("Mipmap filtering is not supported on this system!");

	GLint mipmapscreated;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, &mipmapscreated);

	// generate mipmaps for this image if we haven't already
	if (!mipmapscreated)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

		if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object)
			glGenerateMipmap(GL_TEXTURE_2D);
		else if (GLEE_EXT_framebuffer_object)
			glGenerateMipmapEXT(GL_TEXTURE_2D);
		else
		{
			// modify single texel to trigger mipmap chain generation
			std::vector<GLubyte> emptydata(type == FONT_TRUETYPE ? 2 : 4);
			glTexSubImage2D(GL_TEXTURE_2D,
							0,
							0, 0,
							1, 1,
							type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA,
							GL_UNSIGNED_BYTE,
							&emptydata[0]);
		}
	}
}

void Font::setFilter(const Image::Filter &f)
{
	filter = f;

	std::vector<GLuint>::const_iterator it;
	for (it = textures.begin(); it != textures.end(); ++it)
	{
		bindTexture(*it);
		checkMipmapsCreated();
		setTextureFilter(f);
	}
}

const Image::Filter &Font::getFilter()
{
	return filter;
}

void Font::setMipmapSharpness(float sharpness)
{
	if (!Image::hasMipmapSharpnessSupport())
		return;

	// LOD bias has the range (-maxbias, maxbias)
	mipmapsharpness = std::min(std::max(sharpness, -maxmipmapsharpness + 0.01f), maxmipmapsharpness - 0.01f);

	std::vector<GLuint>::const_iterator it;
	for (it = textures.begin(); it != textures.end(); ++it)
	{
		bindTexture(*it);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -mipmapsharpness); // negative bias is sharper
	}
}

float Font::getMipmapSharpness() const
{
	return mipmapsharpness;
}

bool Font::loadVolatile()
{
	if (Image::hasMipmapSharpnessSupport())
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxmipmapsharpness);

	createTexture();
	return true;
}

void Font::unloadVolatile()
{
	// nuke everything from orbit
	std::map<int, Glyph *>::iterator it = glyphs.begin();
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
		deleteTexture(*iter);
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
	return (type == FONT_TRUETYPE) ? floor(getHeight() / 1.25f + 0.5f) : 0.0f;
}

} // opengl
} // graphics
} // love
