/**
* Copyright (c) 2006-2012 LOVE Development Team
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

#include "ParticleSystem.h"

#include <common/math.h>

#include "GLee.h"
#include <cmath>
#include <cstdlib>

namespace love
{
namespace graphics
{
namespace opengl
{

	namespace
	{
		Colorf colorToFloat(const Color& c) {
			return Colorf( (GLfloat)c.r/255.0f, (GLfloat)c.g/255.0f, (GLfloat)c.b/255.0f, (GLfloat)c.a/255.0f );
		}
	}

	float calculate_variation(float inner, float outer, float var)
	{
		float low = inner - (outer/2.0f)*var;
		float high = inner + (outer/2.0f)*var;
		float r = (rand() / (float(RAND_MAX)+1));
		return low*(1-r)+high*r;
	}


	ParticleSystem::ParticleSystem(Image * sprite, unsigned int buffer) : pStart(0), pLast(0), pEnd(0), active(true), emissionRate(0),
															emitCounter(0), lifetime(-1), life(0), particleLifeMin(0), particleLifeMax(0),
															direction(0), spread(0), relative(false), speedMin(0), speedMax(0), gravityMin(0),
															gravityMax(0), radialAccelerationMin(0), radialAccelerationMax(0),
															tangentialAccelerationMin(0), tangentialAccelerationMax(0),
															sizeVariation(0), rotationMin(0), rotationMax(0),
															spinStart(0), spinEnd(0), spinVariation(0), offsetX(sprite->getWidth()*0.5f),
															offsetY(sprite->getHeight()*0.5f)
	{
		this->sprite = sprite;
		sprite->retain();
		sizes.push_back(1.0f);
		colors.push_back( Colorf(1.0f, 1.0f, 1.0f, 1.0f) );
		setBufferSize(buffer);
	}

	ParticleSystem::~ParticleSystem()
	{
		if (this->sprite != 0)
			this->sprite->release();

		if (pStart != 0)
			delete [] pStart;
	}

	void ParticleSystem::add()
	{
		if (isFull()) return;

		float min,max;

		min = particleLifeMin;
		max = particleLifeMax;
		if (min == max)
			pLast->life = min;
		else
			pLast->life = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;
		pLast->lifetime = pLast->life;

		pLast->position[0] = position.getX();
		pLast->position[1] = position.getY();

		min = direction - spread/2.0f;
		max = direction + spread/2.0f;
		pLast->direction = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;

		pLast->origin = position;

		min = speedMin;
		max = speedMax;
		float speed = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;
		pLast->speed = love::Vector(cos(pLast->direction), sin(pLast->direction));
		pLast->speed *= speed;

		min = gravityMin;
		max = gravityMax;
		pLast->gravity = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;

		min = radialAccelerationMin;
		max = radialAccelerationMax;
		pLast->radialAcceleration = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;

		min = tangentialAccelerationMin;
		max = tangentialAccelerationMax;
		pLast->tangentialAcceleration = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;

		pLast->sizeOffset       = (rand() / (float(RAND_MAX)+1)) * sizeVariation; // time offset for size change
		pLast->sizeIntervalSize = (1.0f - (rand() / (float(RAND_MAX)+1)) * sizeVariation) - pLast->sizeOffset;
		pLast->size = sizes[(size_t)(pLast->sizeOffset - .5f) * (sizes.size() - 1)];

		min = rotationMin;
		max = rotationMax;
		pLast->spinStart = calculate_variation(spinStart, spinEnd, spinVariation);
		pLast->spinEnd = calculate_variation(spinEnd, spinStart, spinVariation);
		pLast->rotation = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;;

		pLast->color = colors[0];

		pLast++;
	}

	void ParticleSystem::remove(particle * p)
	{
		if (!isEmpty())
		{
			*p = *(--pLast);
		}
	}

	void ParticleSystem::setSprite(Image * image)
	{
		if (sprite != 0)
			sprite->release();

		sprite = image;
		sprite->retain();
	}

	void ParticleSystem::setBufferSize(unsigned int size)
	{
		// delete previous data
		delete [] pStart;

		pLast = pStart = new particle[size];

		pEnd = pStart + size;
	}

	void ParticleSystem::setEmissionRate(int rate)
	{
		emissionRate = rate;
	}

	void ParticleSystem::setLifetime(float life)
	{
		this->life = lifetime = life;
	}

	void ParticleSystem::setParticleLife(float min, float max)
	{
		particleLifeMin = min;
		if (max == 0)
			particleLifeMax = min;
		else
			particleLifeMax = max;
	}

	void ParticleSystem::setPosition(float x, float y)
	{
		position = love::Vector(x, y);
	}

	void ParticleSystem::setDirection(float direction)
	{
		this->direction = direction;
	}

	void ParticleSystem::setSpread(float spread)
	{
		this->spread = spread;
	}

	void ParticleSystem::setRelativeDirection(bool relative)
	{
		this->relative = relative;
	}

	void ParticleSystem::setSpeed(float speed)
	{
		speedMin = speedMax = speed;
	}

	void ParticleSystem::setSpeed(float min, float max)
	{
		speedMin = min;
		speedMax = max;
	}

	void ParticleSystem::setGravity(float gravity)
	{
		gravityMin = gravityMax = gravity;
	}

	void ParticleSystem::setGravity(float min, float max)
	{
		gravityMin = min;
		gravityMax = max;
	}

	void ParticleSystem::setRadialAcceleration(float acceleration)
	{
		radialAccelerationMin = radialAccelerationMax = acceleration;
	}

	void ParticleSystem::setRadialAcceleration(float min, float max)
	{
		radialAccelerationMin = min;
		radialAccelerationMax = max;
	}

	void ParticleSystem::setTangentialAcceleration(float acceleration)
	{
		tangentialAccelerationMin = tangentialAccelerationMax = acceleration;
	}

	void ParticleSystem::setTangentialAcceleration(float min, float max)
	{
		tangentialAccelerationMin = min;
		tangentialAccelerationMax = max;
	}

	void ParticleSystem::setSize(float size)
	{
		sizes.resize(1);
		sizes[0] = size;
	}

	void ParticleSystem::setSize(const std::vector<float>& newSizes, float variation)
	{
		sizes = newSizes;
		sizeVariation = variation;
	}

	void ParticleSystem::setSizeVariation(float variation)
	{
		sizeVariation = variation;
	}

	void ParticleSystem::setRotation(float rotation)
	{
		rotationMin = rotationMax = rotation;
	}

	void ParticleSystem::setRotation(float min, float max)
	{
		rotationMin = min;
		rotationMax = max;
	}

	void ParticleSystem::setSpin(float spin)
	{
		spinStart = spin;
		spinEnd = spin;
	}

	void ParticleSystem::setSpin(float start, float end)
	{
		spinStart = start;
		spinEnd = end;
	}

	void ParticleSystem::setSpin(float start, float end, float variation)
	{
		spinStart = start;
		spinEnd = end;
		spinVariation = variation;
	}

	void ParticleSystem::setSpinVariation(float variation)
	{
		spinVariation = variation;
	}

	void ParticleSystem::setColor(const Color& color)
	{
		colors.resize(1);
		colors[0] = colorToFloat(color);
	}

	void ParticleSystem::setColor(const std::vector<Color>& newColors)
	{
		colors.resize( newColors.size() );
		for (size_t i = 0; i < newColors.size(); ++i)
			colors[i] = colorToFloat( newColors[i] );
	}

	void ParticleSystem::setOffset(float x, float y)
	{
		offsetX = x;
		offsetY = y;
	}

	float ParticleSystem::getX() const
	{
		return position.getX();
	}

	float ParticleSystem::getY() const
	{
		return position.getY();
	}
	
	const love::Vector& ParticleSystem::getPosition() const
	{
		return position;
	}

	float ParticleSystem::getDirection() const
	{
		return direction;
	}

	float ParticleSystem::getSpread() const
	{
		return spread;
	}

	float ParticleSystem::getOffsetX() const
	{
		return offsetX;
	}

	float ParticleSystem::getOffsetY() const
	{
		return offsetY;
	}

	int ParticleSystem::count() const
	{
		return (int)(pLast - pStart);
	}

	void ParticleSystem::start()
	{
		active = true;
	}

	void ParticleSystem::stop()
	{
		active = false;
		life = lifetime;
		emitCounter = 0;
	}

	void ParticleSystem::pause()
	{
		active = false;
	}

	void ParticleSystem::reset()
	{
		pLast = pStart;
		life = lifetime;
		emitCounter = 0;
	}

	bool ParticleSystem::isActive() const
	{
		return active;
	}

	bool ParticleSystem::isEmpty() const
	{
		return pStart == pLast;
	}

	bool ParticleSystem::isFull() const
	{
		return pLast == pEnd;
	}

	void ParticleSystem::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
	{
		if (sprite == 0) return; // just in case of failure

		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);

		Matrix t;
		t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
		glMultMatrixf((const GLfloat*)t.getElements());

		particle * p = pStart;
		while (p != pLast)
		{
			glPushMatrix();

			glColor4f(p->color.r, p->color.g, p->color.b, p->color.a);
			sprite->draw(p->position[0], p->position[1], p->rotation, p->size, p->size, offsetX, offsetY, 0.0f, 0.0f);

			glPopMatrix();
			p++;
		}

		glPopAttrib();
		glPopMatrix();
	}

	void ParticleSystem::update(float dt)
	{
		// Traverse all particles and update.
		particle * p = pStart;

		// Make some more particles.
		if (active)
		{
			float rate = 1.0f / emissionRate; // the amount of time between each particle emit
			emitCounter += dt;
			while (emitCounter > rate)
			{
				add();
				emitCounter -= rate;
			}
			/*int particles = (int)(emissionRate * dt);
			for (int i = 0; i != particles; i++)
				add();*/

			life -= dt;
			if (lifetime != -1 && life < 0)
				stop();
		}

		while (p != pLast)
		{
			// Decrease lifespan.
			p->life -= dt;

			if (p->life > 0)
			{

				// Temp variables.
				love::Vector radial, tangential, gravity(0, p->gravity);
				love::Vector ppos(p->position[0], p->position[1]);

				// Get vector from particle center to particle.
				radial = ppos - p->origin;
				radial.normalize();
				tangential = radial;

				// Resize radial acceleration.
				radial *= p->radialAcceleration;

				// Calculate tangential acceleration.
				{
					float a = tangential.getX();
					tangential.setX(-tangential.getY());
					tangential.setY(a);
				}

				// Resize tangential.
				tangential *= p->tangentialAcceleration;

				// Update position.
				p->speed += (radial+tangential+gravity)*dt;

				// Modify position.
				ppos += p->speed * dt;

				p->position[0] = ppos.getX();
				p->position[1] = ppos.getY();

				const float t = 1.0f - p->life / p->lifetime;

				// Rotate.
				p->rotation += (p->spinStart * (1.0f - t) + p->spinEnd * t)*dt;

				// Change size according to given intervals:
				// i = 0       1       2      3          n-1
				//     |-------|-------|------|--- ... ---|
				// t = 0    1/(n-1)        3/(n-1)        1
				//
				// `s' is the interpolation variable scaled to the current
				// interval width, e.g. if n = 5 and t = 0.3, then the current
				// indices are 1,2 and s = 0.3 - 0.25 = 0.05
				float s = p->sizeOffset + t * p->sizeIntervalSize; // size variation
				s *= (float)(sizes.size() - 1); // 0 <= s < sizes.size()
				size_t i = (size_t)s;
				size_t k = (i == sizes.size() - 1) ? i : i + 1; // boundary check (prevents failing on t = 1.0f)
				s -= (float)i; // transpose s to be in interval [0:1]: i <= s < i + 1 ~> 0 <= s < 1
				p->size = sizes[i] * (1.0f - s) + sizes[k] * s;

				// Update color according to given intervals (as above)
				s = t * (float)(colors.size() - 1);
				i = (size_t)s;
				k = (i == colors.size() - 1) ? i : i + 1;
				s -= (float)i;                            // 0 <= s <= 1
				p->color = colors[i] * (1.0f - s) + colors[k] * s;

				// Next particle.
				p++;
			}
			else
			{
				remove(p);

				if (p >= pLast)
					return;
			} // else
		} // while
	}

} // opengl
} // graphics
} // love
