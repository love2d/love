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

#ifndef LOVE_GRAPHICS_OPENGL_TEXT_H
#define LOVE_GRAPHICS_OPENGL_TEXT_H

// LOVE
#include "common/config.h"
#include "graphics/Drawable.h"
#include "Font.h"
#include "GLBuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Text : public Drawable
{
public:

	Text(Font *font, const std::vector<Font::ColoredString> &text = {});
	virtual ~Text();

	void set(const std::vector<Font::ColoredString> &text);
	void set(const std::vector<Font::ColoredString> &text, float wrap, Font::AlignMode align);
	void set();

	int add(const std::vector<Font::ColoredString> &text, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);
	int addf(const std::vector<Font::ColoredString> &text, float wrap, Font::AlignMode align, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);
	void clear();

	// Implements Drawable.
	virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

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

private:

	struct TextData
	{
		Font::ColoredCodepoints codepoints;
		float wrap;
		Font::AlignMode align;
		Font::TextInfo text_info;
		bool use_matrix;
		bool append_vertices;
		Matrix3 matrix;
	};

	void uploadVertices(const std::vector<Font::GlyphVertex> &vertices, size_t vertoffset);
	void regenerateVertices();
	void addTextData(const TextData &s);

	StrongRef<Font> font;
	GLBuffer *vbo;

	std::vector<Font::DrawCommand> draw_commands;

	std::vector<TextData> text_data;

	size_t vert_offset;

	// Used so we know when the font's texture cache is invalidated.
	uint32 texture_cache_id;

}; // Text

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_TEXT_H
