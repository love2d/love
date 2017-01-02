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
#include "common/config.h"
#include "ParticleSystem.h"
#include "graphics/Graphics.h"

#include "OpenGL.h"

// STD
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace love
{
namespace graphics
{
namespace opengl
{

ParticleSystem::ParticleSystem(Texture *texture, uint32 size)
	: love::graphics::ParticleSystem(texture, size)
	, particleVerts(nullptr)
	, quadIndices(size)
{
	createVertices(size);
}

ParticleSystem::ParticleSystem(const ParticleSystem &p)
	: love::graphics::ParticleSystem(p)
	, particleVerts(nullptr)
	, quadIndices(p.quadIndices)
{
	createVertices(maxParticles);
}

ParticleSystem::~ParticleSystem()
{
	delete[] particleVerts;
}

void ParticleSystem::createVertices(size_t numparticles)
{
	try
	{
		Vertex *pverts = new Vertex[numparticles * 4];
		delete[] particleVerts;
		particleVerts = pverts;
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}
}

ParticleSystem *ParticleSystem::clone()
{
	return new ParticleSystem(*this);
}

void ParticleSystem::setBufferSize(uint32 size)
{
	love::graphics::ParticleSystem::setBufferSize(size);

	quadIndices = QuadIndices(size);
	createVertices(size);
}

void ParticleSystem::draw(Graphics *gfx, const Matrix4 &m)
{
	uint32 pCount = getCount();

	if (pCount == 0 || texture.get() == nullptr || pMem == nullptr || particleVerts == nullptr)
		return;

	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("ParticleSystem draw");

	Graphics::TempTransform transform(gfx, m);

	const Vertex *textureVerts = texture->getVertices();
	Vertex *pVerts = particleVerts;
	Particle *p = pHead;

	bool useQuads = !quads.empty();

	Matrix3 t;

	// set the vertex data for each particle (transformation, texcoords, color)
	while (p)
	{
		if (useQuads)
			textureVerts = quads[p->quadIndex]->getVertices();

		// particle vertices are image vertices transformed by particle info
		t.setTransformation(p->position.x, p->position.y, p->angle, p->size, p->size, offset.x, offset.y, 0.0f, 0.0f);
		t.transform(pVerts, textureVerts, 4);

		// Particle colors are stored as floats (0-1) but vertex colors are
		// unsigned bytes (0-255).
		Color c = toColor(p->color);

		// set the texture coordinate and color data for particle vertices
		for (int v = 0; v < 4; v++)
		{
			pVerts[v].s = textureVerts[v].s;
			pVerts[v].t = textureVerts[v].t;
			pVerts[v].color = c;
		}

		pVerts += 4;
		p = p->next;
	}

	gl.bindTextureToUnit(texture, 0, false);
	gl.prepareDraw();

	gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR);

	gl.bindBuffer(BUFFER_VERTEX, 0);
	glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), &particleVerts[0].color.r);
	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &particleVerts[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &particleVerts[0].s);

	GLsizei count = (GLsizei) quadIndices.getIndexCount(pCount);
	GLenum gltype = quadIndices.getType();

	// We use a client-side index array instead of an Index Buffers, because
	// at least one graphics driver (the one for Kepler nvidia GPUs in OS X
	// 10.11) fails to render geometry if an index buffer is used with
	// client-side vertex arrays.
	gl.bindBuffer(BUFFER_INDEX, 0);
	gl.drawElements(GL_TRIANGLES, count, gltype, quadIndices.getIndices(0));
}

} // opengl
} // graphics
} // love
