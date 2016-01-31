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

#ifndef LOVE_GRAPHICS_OPENGL_PARTICLE_SYSTEM_H
#define LOVE_GRAPHICS_OPENGL_PARTICLE_SYSTEM_H

// LOVE
#include "graphics/ParticleSystem.h"
#include "GLBuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class ParticleSystem : public love::graphics::ParticleSystem
{
public:

	ParticleSystem(Texture *texture, uint32 buffer);
	ParticleSystem(const ParticleSystem &p);

	virtual ~ParticleSystem();

	ParticleSystem *clone() override;
	void setBufferSize(uint32 size) override;
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) override;

private:

	void createVertices(size_t numparticles);

	// array of transformed vertex data for all particles, for drawing
	Vertex *particleVerts;

	// Vertex index buffer.
	QuadIndices quadIndices;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_PARTICLE_SYSTEM_H
