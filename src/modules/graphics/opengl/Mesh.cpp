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

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, DrawMode drawmode, vertex::Usage usage)
	: love::graphics::Mesh(gfx, vertexformat, data, datasize, drawmode, usage)
{
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, int vertexcount, DrawMode drawmode, vertex::Usage usage)
	: love::graphics::Mesh(gfx, vertexformat, vertexcount, drawmode, usage)
{
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<Vertex> &vertices, DrawMode drawmode, vertex::Usage usage)
	: Mesh(gfx, getDefaultVertexFormat(), &vertices[0], vertices.size() * sizeof(Vertex), drawmode, usage)
{
}

Mesh::Mesh(graphics::Graphics *gfx, int vertexcount, DrawMode drawmode, vertex::Usage usage)
	: Mesh(gfx, getDefaultVertexFormat(), vertexcount, drawmode, usage)
{
}

Mesh::~Mesh()
{
}

int Mesh::bindAttributeToShaderInput(int attributeindex, const std::string &inputname)
{
	const AttribFormat &format = vertexFormat[attributeindex];

	GLint attriblocation = -1;

	// If the attribute is one of the LOVE-defined ones, use the constant
	// attribute index for it, otherwise query the index from the shader.
	VertexAttribID builtinattrib;
	if (vertex::getConstant(inputname.c_str(), builtinattrib))
		attriblocation = (GLint) builtinattrib;
	else if (Shader::current)
		attriblocation = ((Shader *) Shader::current)->getAttribLocation(inputname);

	// The active shader might not use this vertex attribute name.
	if (attriblocation < 0)
		return attriblocation;

	// Make sure the buffer isn't mapped (sends data to GPU if needed.)
	vbo->unmap();

	gl.bindBuffer(BUFFER_VERTEX, (GLuint) vbo->getHandle());

	const void *gloffset = BUFFER_OFFSET(getAttributeOffset(attributeindex));
	GLenum datatype = getGLDataType(format.type);
	GLboolean normalized = (datatype == GL_UNSIGNED_BYTE);

	glVertexAttribPointer(attriblocation, format.components, datatype, normalized, (GLsizei) vertexStride, gloffset);

	return attriblocation;
}

void Mesh::drawInternal(int start, int count, int instancecount, bool useindexbuffer, uint32 attribflags, uint32 instancedattribflags) const
{
	OpenGL::TempDebugGroup debuggroup("Mesh draw");

	gl.useVertexAttribArrays(attribflags, instancedattribflags);
	gl.bindTextureToUnit(texture, 0, false);
	gl.prepareDraw();

	GLenum gldrawmode = getGLDrawMode(drawMode);

	if (useindexbuffer)
	{
		size_t elementsize = vertex::getIndexDataSize(elementDataType);
		const void *indices = BUFFER_OFFSET(start * elementsize);
		GLenum type = OpenGL::getGLIndexDataType(elementDataType);

		gl.bindBuffer(BUFFER_INDEX, (GLuint) ibo->getHandle());
		gl.drawElements(gldrawmode, count, type, indices, instancecount);
	}
	else
	{
		gl.drawArrays(gldrawmode, start, count, instancecount);
	}
}

GLenum Mesh::getGLDrawMode(DrawMode mode)
{
	switch (mode)
	{
	case DRAWMODE_FAN:
		return GL_TRIANGLE_FAN;
	case DRAWMODE_STRIP:
		return GL_TRIANGLE_STRIP;
	case DRAWMODE_TRIANGLES:
	default:
		return GL_TRIANGLES;
	case DRAWMODE_POINTS:
		return GL_POINTS;
	}
}

GLenum Mesh::getGLDataType(DataType type)
{
	switch (type)
	{
	case DATA_BYTE:
		return GL_UNSIGNED_BYTE;
	case DATA_FLOAT:
		return GL_FLOAT;
	default:
		return 0;
	}
}

} // opengl
} // graphics
} // love
