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

#include "TextBatch.h"
#include "Graphics.h"

#include <algorithm>

namespace love
{
namespace graphics
{

love::Type TextBatch::type("TextBatch", &Drawable::type);

TextBatch::TextBatch(Font *font, const std::vector<love::font::ColoredString> &text)
	: font(font)
	, vertexAttributesID(font->getVertexAttributesID())
	, vertexData(nullptr)
	, modifiedVertices()
	, vertOffset(0)
	, textureCacheID(font->getTextureCacheID())
{
	set(text);
}

TextBatch::~TextBatch()
{
	if (vertexData != nullptr)
		free(vertexData);
}

void TextBatch::uploadVertices(const std::vector<Font::GlyphVertex> &vertices, size_t vertoffset)
{
	size_t offset = vertoffset * sizeof(Font::GlyphVertex);
	size_t datasize = vertices.size() * sizeof(Font::GlyphVertex);

	// If we haven't created a VBO or the vertices are too big, make a new one.
	if (datasize > 0 && (!vertexBuffer || (offset + datasize) > vertexBuffer->getSize()))
	{
		// Make it bigger than necessary to reduce potential future allocations.
		size_t newsize = size_t((offset + datasize) * 1.5);

		if (vertexBuffer != nullptr)
			newsize = std::max(size_t(vertexBuffer->getSize() * 1.5), newsize);

		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);

		Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, BUFFERDATAUSAGE_DYNAMIC);
		auto decl = Buffer::getCommonFormatDeclaration(Font::vertexFormat);

		StrongRef<Buffer> newbuffer(gfx->newBuffer(settings, decl, nullptr, newsize, 0), Acquire::NORETAIN);

		void *newdata = nullptr;
		if (vertexData != nullptr)
			newdata = realloc(vertexData, newsize);
		else
			newdata = malloc(newsize);

		if (newdata == nullptr)
			throw love::Exception("Out of memory.");
		else
			vertexData = (uint8 *) newdata;

		vertexBuffer = newbuffer;

		vertexBuffers.set(0, vertexBuffer, 0);
	}

	if (vertexData != nullptr && datasize > 0)
	{
		memcpy(vertexData + offset, &vertices[0], datasize);
		modifiedVertices.encapsulate(offset, datasize);
	}
}

void TextBatch::regenerateVertices()
{
	// If the font's texture cache was invalidated then we need to recreate the
	// text's vertices, since glyph texcoords might have changed.
	if (font->getTextureCacheID() != textureCacheID)
	{
		std::vector<TextData> textdata = textData;

		clear();

		for (const TextData &t : textdata)
			addTextData(t);

		textureCacheID = font->getTextureCacheID();
	}
}

void TextBatch::addTextData(const TextData &t)
{
	std::vector<Font::GlyphVertex> vertices;
	std::vector<Font::DrawCommand> newcommands;

	love::font::TextShaper::TextInfo textinfo;

	Colorf constantcolor = Colorf(1.0f, 1.0f, 1.0f, 1.0f);

	// We only have formatted text if the align mode is valid.
	if (t.align == Font::ALIGN_MAX_ENUM)
		newcommands = font->generateVertices(t.codepoints, Range(), constantcolor, vertices, 0.0f, Vector2(0.0f, 0.0f), &textinfo);
	else
		newcommands = font->generateVerticesFormatted(t.codepoints, constantcolor, t.wrap, t.align, vertices, &textinfo);

	size_t voffset = vertOffset;

	if (!t.appendVertices)
	{
		voffset = 0;
		vertOffset = 0;
		drawCommands.clear();
		textData.clear();
	}

	if (t.useMatrix && !vertices.empty())
		t.matrix.transformXY(vertices.data(), vertices.data(), (int) vertices.size());

	uploadVertices(vertices, voffset);

	if (!newcommands.empty())
	{
		// The start vertex should be adjusted to account for the vertex offset.
		for (Font::DrawCommand &cmd : newcommands)
			cmd.startvertex += (int) voffset;

		auto firstcmd = newcommands.begin();

		// If the first draw command in the new list has the same texture as the
		// last one in the existing list we're building and its vertices are
		// in-order, we can combine them (saving a draw call.)
		if (!drawCommands.empty())
		{
			auto prevcmd = drawCommands.back();
			if (prevcmd.texture == firstcmd->texture && (prevcmd.startvertex + prevcmd.vertexcount) == firstcmd->startvertex)
			{
				drawCommands.back().vertexcount += firstcmd->vertexcount;
				++firstcmd;
			}
		}

		// Append the new draw commands to the list we're building.
		drawCommands.insert(drawCommands.end(), firstcmd, newcommands.end());
	}

	vertOffset = voffset + vertices.size();

	textData.push_back(t);
	textData.back().textInfo = textinfo;

	// Font::generateVertices can invalidate the font's texture cache.
	if (font->getTextureCacheID() != textureCacheID)
		regenerateVertices();
}

void TextBatch::set(const std::vector<love::font::ColoredString> &text)
{
	return set(text, -1.0f, Font::ALIGN_MAX_ENUM);
}

void TextBatch::set(const std::vector<love::font::ColoredString> &text, float wrap, Font::AlignMode align)
{
	if (text.empty() || (text.size() == 1 && text[0].str.empty()))
		return clear();

	love::font::ColoredCodepoints codepoints;
	love::font::getCodepointsFromString(text, codepoints);

	addTextData({codepoints, wrap, align, {}, false, false, Matrix4()});
}

int TextBatch::add(const std::vector<love::font::ColoredString> &text, const Matrix4 &m)
{
	return addf(text, -1.0f, Font::ALIGN_MAX_ENUM, m);
}

int TextBatch::addf(const std::vector<love::font::ColoredString> &text, float wrap, Font::AlignMode align, const Matrix4 &m)
{
	love::font::ColoredCodepoints codepoints;
	love::font::getCodepointsFromString(text, codepoints);

	addTextData({codepoints, wrap, align, {}, true, true, m});

	return (int) textData.size() - 1;
}

void TextBatch::clear()
{
	textData.clear();
	drawCommands.clear();
	textureCacheID = font->getTextureCacheID();
	vertOffset = 0;
}

void TextBatch::setFont(Font *f)
{
	font.set(f);
	
	// Invalidate the texture cache ID since the font is different. We also have
	// to re-upload all the vertices based on the new font's textures.
	textureCacheID = (uint32) -1;
	vertexAttributesID = font->getVertexAttributesID();
	regenerateVertices();
}

Font *TextBatch::getFont() const
{
	return font.get();
}

int TextBatch::getWidth(int index) const
{
	if (index < 0)
		index = std::max((int) textData.size() - 1, 0);

	if (index >= (int) textData.size())
		return 0;

	return textData[index].textInfo.width;
}

int TextBatch::getHeight(int index) const
{
	if (index < 0)
		index = std::max((int) textData.size() - 1, 0);

	if (index >= (int) textData.size())
		return 0;

	return textData[index].textInfo.height;
}

void TextBatch::draw(Graphics *gfx, const Matrix4 &m)
{
	if (vertexBuffer == nullptr || vertexData == nullptr || drawCommands.empty())
		return;

	gfx->flushBatchedDraws();

	// Re-generate the text if the Font's texture cache was invalidated.
	if (font->getTextureCacheID() != textureCacheID)
		regenerateVertices();

	if (Shader::isDefaultActive())
		Shader::attachDefault(Shader::STANDARD_DEFAULT);

	Texture *firsttex = nullptr;
	if (!drawCommands.empty())
		firsttex = drawCommands[0].texture;

	if (Shader::current)
		Shader::current->validateDrawState(PRIMITIVE_TRIANGLES, firsttex);

	int totalverts = 0;
	for (const Font::DrawCommand &cmd : drawCommands)
		totalverts = std::max(cmd.startvertex + cmd.vertexcount, totalverts);

	// Make sure all pending data is uploaded to the GPU.
	if (modifiedVertices.isValid())
	{
		size_t offset = modifiedVertices.getOffset();
		size_t size = modifiedVertices.getSize();

		if (vertexBuffer->getDataUsage() == BUFFERDATAUSAGE_STREAM)
			vertexBuffer->fill(0, vertexBuffer->getSize(), vertexData);
		else
			vertexBuffer->fill(offset, size, vertexData + offset);

		modifiedVertices.invalidate();
	}

	Graphics::TempTransform transform(gfx, m);

	for (const Font::DrawCommand &cmd : drawCommands)
	{
		Texture *tex = gfx->getTextureOrDefaultForActiveShader(cmd.texture);
		gfx->drawQuads(cmd.startvertex / 4, cmd.vertexcount / 4, vertexAttributesID, vertexBuffers, tex);
	}
}

} // graphics
} // love
