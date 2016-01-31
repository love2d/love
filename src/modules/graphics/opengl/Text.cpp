/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "Text.h"
#include "common/Matrix.h"

#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

Text::Text(Font *font, const std::vector<Font::ColoredString> &text)
	: font(font)
	, vbo(nullptr)
	, vert_offset(0)
	, texture_cache_id((uint32) -1)
{
	set(text);
}

Text::~Text()
{
	delete vbo;
}

void Text::uploadVertices(const std::vector<Font::GlyphVertex> &vertices, size_t vertoffset)
{
	size_t offset = vertoffset * sizeof(Font::GlyphVertex);
	size_t datasize = vertices.size() * sizeof(Font::GlyphVertex);
	uint8 *vbodata = nullptr;

	// If we haven't created a VBO or the vertices are too big, make a new one.
	if (datasize > 0 && (!vbo || (offset + datasize) > vbo->getSize()))
	{
		// Make it bigger than necessary to reduce potential future allocations.
		size_t newsize = size_t((offset + datasize) * 1.5);
		if (vbo != nullptr)
			newsize = std::max(size_t(vbo->getSize() * 1.5), newsize);

		GLBuffer *new_vbo = new GLBuffer(newsize, nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

		if (vbo != nullptr)
		{
			try
			{
				GLBuffer::Bind bind(*vbo);
				vbodata = (uint8 *) vbo->map();
			}
			catch (love::Exception &)
			{
				delete new_vbo;
				throw;
			}

			GLBuffer::Bind bind(*new_vbo);
			new_vbo->fill(0, vbo->getSize(), vbodata);
		}

		delete vbo;
		vbo = new_vbo;
	}

	if (vbo != nullptr && datasize > 0)
	{
		GLBuffer::Bind bind(*vbo);
		vbodata = (uint8 *) vbo->map();
		memcpy(vbodata + offset, &vertices[0], datasize);
		// We unmap when we draw, to avoid unnecessary full map()/unmap() calls.
	}
}

void Text::regenerateVertices()
{
	// If the font's texture cache was invalidated then we need to recreate the
	// text's vertices, since glyph texcoords might have changed.
	if (font->getTextureCacheID() != texture_cache_id)
	{
		std::vector<TextData> textdata = text_data;

		clear();

		for (const TextData &t : textdata)
			addTextData(t);

		texture_cache_id = font->getTextureCacheID();
	}
}

void Text::addTextData(const TextData &t)
{
	std::vector<Font::GlyphVertex> vertices;
	std::vector<Font::DrawCommand> new_commands;

	Font::TextInfo text_info;

	// We only have formatted text if the align mode is valid.
	if (t.align == Font::ALIGN_MAX_ENUM)
		new_commands = font->generateVertices(t.codepoints, vertices, 0.0f, Vector(0.0f, 0.0f), &text_info);
	else
		new_commands = font->generateVerticesFormatted(t.codepoints, t.wrap, t.align, vertices, &text_info);

	if (t.use_matrix)
		t.matrix.transform(&vertices[0], &vertices[0], (int) vertices.size());

	size_t voffset = vert_offset;

	if (!t.append_vertices)
	{
		voffset = 0;
		draw_commands.clear();
	}

	uploadVertices(vertices, voffset);

	if (!new_commands.empty())
	{
		// The start vertex should be adjusted to account for the vertex offset.
		for (Font::DrawCommand &cmd : new_commands)
			cmd.startvertex += (int) voffset;

		auto firstcmd = new_commands.begin();

		// If the first draw command in the new list has the same texture as the
		// last one in the existing list we're building and its vertices are
		// in-order, we can combine them (saving a draw call.)
		if (!draw_commands.empty())
		{
			auto prevcmd = draw_commands.back();
			if (prevcmd.texture == firstcmd->texture && (prevcmd.startvertex + prevcmd.vertexcount) == firstcmd->startvertex)
			{
				draw_commands.back().vertexcount += firstcmd->vertexcount;
				++firstcmd;
			}
		}

		// Append the new draw commands to the list we're building.
		draw_commands.insert(draw_commands.end(), firstcmd, new_commands.end());
	}

	vert_offset = voffset + vertices.size();

	text_data.push_back(t);
	text_data.back().text_info = text_info;

	// Font::generateVertices can invalidate the font's texture cache.
	if (font->getTextureCacheID() != texture_cache_id)
		regenerateVertices();
}

void Text::set(const std::vector<Font::ColoredString> &text)
{
	return set(text, -1.0f, Font::ALIGN_MAX_ENUM);
}

void Text::set(const std::vector<Font::ColoredString> &text, float wrap, Font::AlignMode align)
{
	if (text.empty() || (text.size() == 1 && text[0].str.empty()))
		return set();

	Font::ColoredCodepoints codepoints;
	Font::getCodepointsFromString(text, codepoints);

	addTextData({codepoints, wrap, align, {}, false, false, Matrix3()});
}

void Text::set()
{
	clear();
}

int Text::add(const std::vector<Font::ColoredString> &text, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	return addf(text, -1.0f, Font::ALIGN_MAX_ENUM, x, y, angle, sx, sy, ox, oy, kx, ky);
}

int Text::addf(const std::vector<Font::ColoredString> &text, float wrap, Font::AlignMode align, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	Font::ColoredCodepoints codepoints;
	Font::getCodepointsFromString(text, codepoints);

	Matrix3 m(x, y, angle, sx, sy, ox, oy, kx, ky);

	addTextData({codepoints, wrap, align, {}, true, true, m});

	return (int) text_data.size() - 1;
}

void Text::clear()
{
	text_data.clear();
	draw_commands.clear();
	texture_cache_id = font->getTextureCacheID();
	vert_offset = 0;
}

void Text::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	if (vbo == nullptr || draw_commands.empty())
		return;

	OpenGL::TempDebugGroup debuggroup("Text object draw");

	// Re-generate the text if the Font's texture cache was invalidated.
	if (font->getTextureCacheID() != texture_cache_id)
		regenerateVertices();

	const size_t pos_offset   = offsetof(Font::GlyphVertex, x);
	const size_t tex_offset   = offsetof(Font::GlyphVertex, s);
	const size_t color_offset = offsetof(Font::GlyphVertex, color.r);
	const size_t stride = sizeof(Font::GlyphVertex);

	OpenGL::TempTransform transform(gl);
	transform.get() *= Matrix4(x, y, angle, sx, sy, ox, oy, kx, ky);

	{
		GLBuffer::Bind bind(*vbo);
		vbo->unmap(); // Make sure all pending data is flushed to the GPU.

		// Font::drawVertices expects AttribPointer calls to be done already.
		glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride, vbo->getPointer(pos_offset));
		glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, vbo->getPointer(tex_offset));
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, vbo->getPointer(color_offset));
	}

	gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR);

	font->drawVertices(draw_commands, true);
}

void Text::setFont(Font *f)
{
	font.set(f);

	// Invalidate the texture cache ID since the font is different. We also have
	// to re-upload all the vertices based on the new font's textures.
	texture_cache_id = (uint32) -1;
	regenerateVertices();
}

Font *Text::getFont() const
{
	return font.get();
}

int Text::getWidth(int index) const
{
	if (index < 0)
		index = std::max((int) text_data.size() - 1, 0);

	if (index >= (int) text_data.size())
		return 0;

	return text_data[index].text_info.width;
}

int Text::getHeight(int index) const
{
	if (index < 0)
		index = std::max((int) text_data.size() - 1, 0);

	if (index >= (int) text_data.size())
		return 0;

	return text_data[index].text_info.height;
}

} // opengl
} // graphics
} // love
