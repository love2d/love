/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "Graphics.h"

#include <algorithm>

namespace love
{
namespace graphics
{

love::Type Text::type("Text", &Drawable::type);

Text::Text(Font *font, const std::vector<Font::ColoredString> &text)
	: font(font)
	, vertexAttributes(Font::vertexFormat, 0)
	, vertex_buffer(nullptr)
	, vert_offset(0)
	, texture_cache_id((uint32) -1)
{
	set(text);
}

Text::~Text()
{
	delete vertex_buffer;
}

void Text::uploadVertices(const std::vector<Font::GlyphVertex> &vertices, size_t vertoffset)
{
	size_t offset = vertoffset * sizeof(Font::GlyphVertex);
	size_t datasize = vertices.size() * sizeof(Font::GlyphVertex);

	// If we haven't created a VBO or the vertices are too big, make a new one.
	if (datasize > 0 && (!vertex_buffer || (offset + datasize) > vertex_buffer->getSize()))
	{
		// Make it bigger than necessary to reduce potential future allocations.
		size_t newsize = size_t((offset + datasize) * 1.5);

		if (vertex_buffer != nullptr)
			newsize = std::max(size_t(vertex_buffer->getSize() * 1.5), newsize);

		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		Buffer *new_buffer = gfx->newBuffer(newsize, nullptr, BUFFER_VERTEX, vertex::USAGE_DYNAMIC, 0);

		if (vertex_buffer != nullptr)
			vertex_buffer->copyTo(0, vertex_buffer->getSize(), new_buffer, 0);

		delete vertex_buffer;
		vertex_buffer = new_buffer;

		vertexBuffers.set(0, vertex_buffer, 0);
	}

	if (vertex_buffer != nullptr && datasize > 0)
	{
		uint8 *bufferdata = (uint8 *) vertex_buffer->map();
		memcpy(bufferdata + offset, &vertices[0], datasize);
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

	Colorf constantcolor = Colorf(1.0f, 1.0f, 1.0f, 1.0f);

	// We only have formatted text if the align mode is valid.
	if (t.align == Font::ALIGN_MAX_ENUM)
		new_commands = font->generateVertices(t.codepoints, constantcolor, vertices, 0.0f, Vector2(0.0f, 0.0f), &text_info);
	else
		new_commands = font->generateVerticesFormatted(t.codepoints, constantcolor, t.wrap, t.align, vertices, &text_info);

	size_t voffset = vert_offset;

	if (!t.append_vertices)
	{
		voffset = 0;
		vert_offset = 0;
		draw_commands.clear();
		text_data.clear();
	}

	if (t.use_matrix && !vertices.empty())
		t.matrix.transformXY(vertices.data(), vertices.data(), (int) vertices.size());

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
		return clear();

	Font::ColoredCodepoints codepoints;
	Font::getCodepointsFromString(text, codepoints);

	addTextData({codepoints, wrap, align, {}, false, false, Matrix4()});
}

int Text::add(const std::vector<Font::ColoredString> &text, const Matrix4 &m)
{
	return addf(text, -1.0f, Font::ALIGN_MAX_ENUM, m);
}

int Text::addf(const std::vector<Font::ColoredString> &text, float wrap, Font::AlignMode align, const Matrix4 &m)
{
	Font::ColoredCodepoints codepoints;
	Font::getCodepointsFromString(text, codepoints);

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

void Text::draw(Graphics *gfx, const Matrix4 &m)
{
	if (vertex_buffer == nullptr || draw_commands.empty())
		return;

	gfx->flushStreamDraws();

	if (Shader::isDefaultActive())
		Shader::attachDefault(Shader::STANDARD_DEFAULT);

	if (Shader::current)
		Shader::current->checkMainTextureType(TEXTURE_2D, false);

	// Re-generate the text if the Font's texture cache was invalidated.
	if (font->getTextureCacheID() != texture_cache_id)
		regenerateVertices();

	int totalverts = 0;
	for (const Font::DrawCommand &cmd : draw_commands)
		totalverts = std::max(cmd.startvertex + cmd.vertexcount, totalverts);

	vertex_buffer->unmap(); // Make sure all pending data is flushed to the GPU.

	Graphics::TempTransform transform(gfx, m);

	for (const Font::DrawCommand &cmd : draw_commands)
		gfx->drawQuads(cmd.startvertex / 4, cmd.vertexcount / 4, vertexAttributes, vertexBuffers, cmd.texture);
}

} // graphics
} // love
