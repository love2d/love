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

// LOVE
#include "Mesh.h"
#include "common/Exception.h"
#include "Shader.h"
#include "graphics/Graphics.h"

// C++
#include <algorithm>
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, vertex::Usage usage)
	: love::graphics::Mesh(gfx, vertexformat, data, datasize, drawmode, usage)
{
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, int vertexcount, PrimitiveType drawmode, vertex::Usage usage)
	: love::graphics::Mesh(gfx, vertexformat, vertexcount, drawmode, usage)
{
}

Mesh::~Mesh()
{
}

void Mesh::drawInternal(int start, int count, int instancecount, bool useindexbuffer, const vertex::Attributes &attributes, const vertex::Buffers &buffers) const
{
	OpenGL::TempDebugGroup debuggroup("Mesh draw");

	gl.setVertexAttributes(attributes, buffers);
	gl.bindTextureToUnit(texture, 0, false);
	gl.prepareDraw();

	GLenum glprimitivetype = OpenGL::getGLPrimitiveType(primitiveType);

	if (useindexbuffer)
	{
		size_t elementsize = vertex::getIndexDataSize(indexDataType);
		const void *indices = BUFFER_OFFSET(start * elementsize);
		GLenum indextype = OpenGL::getGLIndexDataType(indexDataType);

		gl.bindBuffer(BUFFER_INDEX, (GLuint) ibo->getHandle());
		gl.drawElements(glprimitivetype, count, indextype, indices, instancecount);
	}
	else
	{
		gl.drawArrays(glprimitivetype, start, count, instancecount);
	}
}

} // opengl
} // graphics
} // love
