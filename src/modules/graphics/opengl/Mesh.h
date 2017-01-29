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

#pragma once

// LOVE
#include "graphics/Mesh.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Mesh : public love::graphics::Mesh
{
public:

	Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, DrawMode drawmode, vertex::Usage usage);
	Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, int vertexcount, DrawMode drawmode, vertex::Usage usage);

	Mesh(graphics::Graphics *gfx, const std::vector<Vertex> &vertices, DrawMode drawmode, vertex::Usage usage);
	Mesh(graphics::Graphics *gfx, int vertexcount, DrawMode drawmode, vertex::Usage usage);

	virtual ~Mesh();

	int bindAttributeToShaderInput(int attributeindex, const std::string &inputname) override;
	void drawInstanced(Graphics *gfx, const Matrix4 &m, int instancecount) override;

private:

	static GLenum getGLDrawMode(DrawMode mode);
	static GLenum getGLDataType(DataType type);

}; // Mesh

} // opengl
} // graphics
} // love
