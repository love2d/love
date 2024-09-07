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

// LOVE
#include "common/config.h"
#include "common/Range.h"
#include "Drawable.h"
#include "Font.h"
#include "Buffer.h"

namespace love
{
namespace graphics
{

class Graphics;

class TextBatch : public Drawable
{
public:

	static love::Type type;

	TextBatch(Font *font, const std::vector<love::font::ColoredString> &text = {});
	virtual ~TextBatch();

	void set(const std::vector<love::font::ColoredString> &text);
	void set(const std::vector<love::font::ColoredString> &text, float wrap, Font::AlignMode align);

	int add(const std::vector<love::font::ColoredString> &text, const Matrix4 &m);
	int addf(const std::vector<love::font::ColoredString> &text, float wrap, Font::AlignMode align, const Matrix4 &m);

	void clear();

	void setFont(Font *f);
	Font *getFont() const;

	/**
	 * Gets the width of the currently set text.
	 **/
	int getWidth(int index = 0) const;

	/**
	 * Gets the height of the currently set text.
	 **/
	int getHeight(int index = 0) const;

	// Implements Drawable.
	void draw(love::graphics::Graphics *gfx, const Matrix4 &m) override;

private:

	struct TextData
	{
		love::font::ColoredCodepoints codepoints;
		float wrap;
		Font::AlignMode align;
		love::font::TextShaper::TextInfo textInfo;
		bool useMatrix;
		bool appendVertices;
		Matrix4 matrix;
	};

	void uploadVertices(const std::vector<Font::GlyphVertex> &vertices, size_t vertoffset);
	void regenerateVertices();
	void addTextData(const TextData &s);

	StrongRef<Font> font;

	VertexAttributesID vertexAttributesID;
	BufferBindings vertexBuffers;

	StrongRef<Buffer> vertexBuffer;
	uint8 *vertexData;
	Range modifiedVertices;

	std::vector<Font::DrawCommand> drawCommands;

	std::vector<TextData> textData;

	size_t vertOffset;
	
	// Used so we know when the font's texture cache is invalidated.
	uint32 textureCacheID;
	
}; // Text

} // graphics
} // love
