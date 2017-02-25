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

#include "common/config.h"
#include "SpriteBatch.h"

// OpenGL
#include "OpenGL.h"

// LOVE
#include "graphics/Buffer.h"
#include "graphics/Texture.h"
#include "graphics/Graphics.h"

// C++
#include <algorithm>

// C
#include <stddef.h>

namespace love
{
namespace graphics
{
namespace opengl
{

SpriteBatch::SpriteBatch(Graphics *gfx, Texture *texture, int size, vertex::Usage usage)
	: love::graphics::SpriteBatch(gfx, texture, size, usage)
{
}

SpriteBatch::~SpriteBatch()
{
}

void SpriteBatch::draw(Graphics *gfx, const Matrix4 &m)
{
	using namespace vertex;

	if (next == 0)
		return;

	gfx->flushStreamDraws();

	if (Shader::current && texture.get())
		Shader::current->checkMainTextureType(texture->getTextureType());

	OpenGL::TempDebugGroup debuggroup("SpriteBatch draw");

	Graphics::TempTransform transform(gfx, m);

	gl.bindTextureToUnit(texture, 0, false);

	// Make sure the VBO isn't mapped when we draw (sends data to GPU if needed.)
	array_buf->unmap();

	CommonFormat format = CommonFormat::XYf_STf_RGBAub;
	if (color == nullptr)
		format = CommonFormat::XYf_STf;

	uint32 enabledattribs = getFormatFlags(format);

	gl.setVertexPointers(format, array_buf, getFormatStride(CommonFormat::XYf_STf_RGBAub), 0);

	for (const auto &it : attached_attributes)
	{
		Mesh *mesh = it.second.mesh.get();

		// We have to do this check here as wll because setBufferSize can be
		// called after attachAttribute.
		if (mesh->getVertexCount() < (size_t) next * 4)
			throw love::Exception("Mesh with attribute '%s' attached to this SpriteBatch has too few vertices", it.first.c_str());

		int location = mesh->bindAttributeToShaderInput(it.second.index, it.first);

		if (location >= 0)
			enabledattribs |= 1u << (uint32) location;
	}

	gl.useVertexAttribArrays(enabledattribs);

	gl.prepareDraw();

	int start = std::min(std::max(0, range_start), next - 1);

	int count = next;
	if (range_count > 0)
		count = std::min(count, range_count);

	count = std::min(count, next - start);

	if (count > 0)
	{
		gl.bindBuffer(BUFFER_INDEX, (GLuint) quad_indices.getBuffer()->getHandle());

		const void *indices = BUFFER_OFFSET(start * quad_indices.getElementSize());
		GLenum gltype = OpenGL::getGLIndexDataType(quad_indices.getType());

		gl.drawElements(GL_TRIANGLES, (GLsizei) quad_indices.getIndexCount(count), gltype, indices);
	}
}

} // opengl
} // graphics
} // love
