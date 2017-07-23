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

void SpriteBatch::drawInternal(vertex::CommonFormat format, size_t indexbytestart, size_t indexcount)
{
	OpenGL::TempDebugGroup debuggroup("SpriteBatch draw");

	uint32 enabledattribs = getFormatFlags(format);

	// We want attached attributes to override local attributes, so we should
	// call this before binding attached attributes.
	gl.setVertexPointers(format, array_buf, vertex_stride, 0);

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
	gl.bindTextureToUnit(texture, 0, false);

	gl.prepareDraw();

	gl.bindBuffer(BUFFER_INDEX, (GLuint) quad_indices.getBuffer()->getHandle());

	const void *indices = BUFFER_OFFSET(indexbytestart);
	GLenum gltype = OpenGL::getGLIndexDataType(quad_indices.getType());

	gl.drawElements(GL_TRIANGLES, (GLsizei) indexcount, gltype, indices);
}

} // opengl
} // graphics
} // love
