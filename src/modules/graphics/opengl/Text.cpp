/**
 * Copyright (c) 2006-2017 LOVE Development Team
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
#include "graphics/Graphics.h"
#include "OpenGL.h"

#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

Text::Text(love::graphics::Graphics *gfx, love::graphics::Font *font, const std::vector<Font::ColoredString> &text)
	: love::graphics::Text(gfx, font, text)
{
}

Text::~Text()
{
}

void Text::draw(Graphics *gfx, const Matrix4 &m)
{
	if (vbo == nullptr || draw_commands.empty())
		return;

	if (Shader::current)
		Shader::current->checkMainTextureType(TEXTURE_2D, false);

	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("Text object draw");

	// Re-generate the text if the Font's texture cache was invalidated.
	if (font->getTextureCacheID() != texture_cache_id)
		regenerateVertices();

	int totalverts = 0;
	for (const Font::DrawCommand &cmd : draw_commands)
		totalverts = std::max(cmd.startvertex + cmd.vertexcount, totalverts);

	if ((size_t) totalverts / 4 > quadIndices.getSize())
		quadIndices = QuadIndices(gfx, (size_t) totalverts / 4);

	vbo->unmap(); // Make sure all pending data is flushed to the GPU.

	Graphics::TempTransform transform(gfx, m);

	gl.prepareDraw();

	gl.setVertexPointers(Font::vertexFormat, vbo, 0);
	gl.useVertexAttribArrays(vertex::getFormatFlags(Font::vertexFormat));

	const GLenum gltype = OpenGL::getGLIndexDataType(quadIndices.getType());
	const size_t elemsize = quadIndices.getElementSize();

	gl.bindBuffer(BUFFER_INDEX, (GLuint) quadIndices.getBuffer()->getHandle());

	// We need a separate draw call for every section of the text which uses a
	// different texture than the previous section.
	for (const Font::DrawCommand &cmd : draw_commands)
	{
		GLsizei count = (cmd.vertexcount / 4) * 6;
		size_t offset = (cmd.startvertex / 4) * 6 * elemsize;

		// TODO: Use glDrawElementsBaseVertex when supported?
		gl.bindTextureToUnit(cmd.texture, 0, false);

		gl.drawElements(GL_TRIANGLES, count, gltype, BUFFER_OFFSET(offset));
	}
}

} // opengl
} // graphics
} // love
