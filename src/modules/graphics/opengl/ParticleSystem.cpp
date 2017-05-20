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

//LOVE
#include "ParticleSystem.h"
#include "graphics/Graphics.h"

#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

ParticleSystem::ParticleSystem(Graphics *gfx, Texture *texture, uint32 size)
	: love::graphics::ParticleSystem(gfx, texture, size)
{
}

ParticleSystem::ParticleSystem(const ParticleSystem &p)
	: love::graphics::ParticleSystem(p)
{
}

ParticleSystem::~ParticleSystem()
{
}

ParticleSystem *ParticleSystem::clone()
{
	return new ParticleSystem(*this);
}

void ParticleSystem::draw(Graphics *gfx, const Matrix4 &m)
{
	using namespace vertex;

	if (!prepareDraw(gfx))
		return;

	Graphics::TempTransform transform(gfx, m);

	if (Shader::isDefaultActive())
		Shader::attachDefault(Shader::STANDARD_DEFAULT);

	if (Shader::current && texture.get())
		Shader::current->checkMainTexture(texture);

	OpenGL::TempDebugGroup debuggroup("ParticleSystem draw");

	gl.bindTextureToUnit(texture, 0, false);
	gl.prepareDraw();

	gl.useVertexAttribArrays(getFormatFlags(CommonFormat::XYf_STf_RGBAub));
	gl.setVertexPointers(CommonFormat::XYf_STf_RGBAub, buffer, 0);

	GLsizei count = (GLsizei) quadIndices.getIndexCount(getCount());
	GLenum gltype = OpenGL::getGLIndexDataType(quadIndices.getType());

	gl.bindBuffer(BUFFER_INDEX, (GLuint) quadIndices.getBuffer()->getHandle());
	gl.drawElements(GL_TRIANGLES, count, gltype, BUFFER_OFFSET(0));
}

} // opengl
} // graphics
} // love
