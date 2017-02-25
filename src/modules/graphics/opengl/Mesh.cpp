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

void Mesh::drawInstanced(love::graphics::Graphics *gfx, const love::Matrix4 &m, int instancecount)
{
	if (vertexCount <= 0 || instancecount <= 0)
		return;

	if (instancecount > 1 && !gl.isInstancingSupported())
		throw love::Exception("Instancing is not supported on this system.");

	if (Shader::current && texture.get())
		Shader::current->checkMainTextureType(texture->getTextureType());

	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("Mesh draw");

	uint32 enabledattribs = 0;
	uint32 instancedattribs = 0;

	for (const auto &attrib : attachedAttributes)
	{
		if (!attrib.second.enabled)
			continue;

		love::graphics::Mesh *mesh = attrib.second.mesh;
		int location = mesh->bindAttributeToShaderInput(attrib.second.index, attrib.first);

		if (location >= 0)
		{
			uint32 bit = 1u << (uint32) location;

			enabledattribs |= bit;

			if (attrib.second.step == STEP_PER_INSTANCE)
				instancedattribs |= bit;
		}
	}

	// Not supported on all platforms or GL versions, I believe.
	if (!(enabledattribs & ATTRIBFLAG_POS))
		throw love::Exception("Mesh must have an enabled VertexPosition attribute to be drawn.");

	gl.useVertexAttribArrays(enabledattribs, instancedattribs);

	gl.bindTextureToUnit(texture, 0, false);

	Graphics::TempTransform transform(gfx, m);

	gl.prepareDraw();

	if (useIndexBuffer && ibo && elementCount > 0)
	{
		// Use the custom vertex map (index buffer) to draw the vertices.
		gl.bindBuffer(BUFFER_INDEX, (GLuint) ibo->getHandle());

		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		ibo->unmap();

		int start = std::min(std::max(0, rangeStart), (int) elementCount - 1);

		int count = (int) elementCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) elementCount - start);

		size_t elementsize = vertex::getIndexDataSize(elementDataType);
		const void *indices = BUFFER_OFFSET(start * elementsize);
		GLenum type = OpenGL::getGLIndexDataType(elementDataType);

		if (count > 0)
			gl.drawElements(getGLDrawMode(drawMode), count, type, indices, instancecount);
	}
	else
	{
		int start = std::min(std::max(0, rangeStart), (int) vertexCount - 1);

		int count = (int) vertexCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) vertexCount - start);

		// Normal non-indexed drawing (no custom vertex map.)
		if (count > 0)
			gl.drawArrays(getGLDrawMode(drawMode), start, count, instancecount);
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
