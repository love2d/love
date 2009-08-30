/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include <SDL_opengl.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace love
{
namespace graphics
{
namespace opengl
{

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
															sizeStart(1), sizeEnd(1), sizeVariation(0), rotationMin(0), rotationMax(0),
															spinStart(0), spinEnd(0), spinVariation(0), offsetX(sprite->getWidth()*0.5f),
															offsetY(sprite->getHeight()*0.5f)
	{	
		this->sprite = sprite;
		sprite->retain();
		memset(colorStart, 255, 4);
		memset(colorEnd, 255, 4);
		setBufferSize(buffer);
	}

	ParticleSystem::~ParticleSystem()
	{
		if(this->sprite != 0)
		{
			this->sprite->release();
			this->sprite = 0;
		}

		if(pStart != 0)
			delete [] pStart;
	}

	void ParticleSystem::add()
	{
		if(isFull()) return;
		
		float min,max;

		min = particleLifeMin;
		max = particleLifeMax;
		if(min == max)
			pLast->life = min;
		else
			pLast->life = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;
		pLast->lifetime = pLast->life;

		pLast->position[0] = position.getX();
		pLast->position[1] = position.getY();

		min = direction - spread/2.0f;
		max = direction + spread/2.0f;
		pLast->direction = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;

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

		pLast->sizeStart = calculate_variation(sizeStart, sizeEnd, sizeVariation);
		pLast->sizeEnd = calculate_variation(sizeEnd, sizeStart, sizeVariation);
		pLast->size = pLast->sizeStart;

		min = rotationMin;
		max = rotationMax;
		pLast->spinStart = calculate_variation(spinStart, spinEnd, spinVariation);
		pLast->spinEnd = calculate_variation(spinEnd, spinStart, spinVariation);
		pLast->rotation = (rand() / (float(RAND_MAX)+1)) * (max - min) + min;;

		pLast->color[0] = (float)colorStart[0] / 255;
		pLast->color[1] = (float)colorStart[1] / 255;
		pLast->color[2] = (float)colorStart[2] / 255;
		pLast->color[3] = (float)colorStart[3] / 255;
			
		pLast++;
	}

	void ParticleSystem::remove(particle * p)
	{
		if(!isEmpty())
		{
			*p = *(--pLast);
		}
	}

	void ParticleSystem::setSprite(Image * image)
	{
		if(this->sprite != 0)
		{
			this->sprite->release();
			this->sprite = 0;
		}

		this->sprite = image;
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
		if(max == 0)
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
		this->direction = direction * LOVE_M_TORAD;
	}

	void ParticleSystem::setSpread(float spread)
	{
		this->spread = spread * LOVE_M_TORAD;
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
		sizeStart = size;
		sizeEnd = size;
	}

	void ParticleSystem::setSize(float start, float end)
	{
		sizeStart = start;
		sizeEnd = end;
	}

	void ParticleSystem::setSize(float start, float end, float variation)
	{
		sizeStart = start;
		sizeEnd = end;
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
		spinStart = spin * LOVE_M_TORAD;
		spinEnd = spin * LOVE_M_TORAD;
	}

	void ParticleSystem::setSpin(float start, float end)
	{
		spinStart = start * LOVE_M_TORAD;
		spinEnd = end * LOVE_M_TORAD;
	}

	void ParticleSystem::setSpin(float start, float end, float variation)
	{
		spinStart = start * LOVE_M_TORAD;
		spinEnd = end * LOVE_M_TORAD;
		spinVariation = variation * LOVE_M_TORAD;
	}

	void ParticleSystem::setSpinVariation(float variation)
	{
		spinVariation = variation * LOVE_M_TORAD;
	}

	void ParticleSystem::setColor(unsigned char * color)
	{
		memcpy(colorStart, color, 4);
		memcpy(colorEnd, color, 4);
	}

	void ParticleSystem::setColor(unsigned char * start, unsigned char * end)
	{
		memcpy(colorStart, start, 4);
		memcpy(colorEnd, end, 4);
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

	float ParticleSystem::getDirection() const
	{
		return direction * LOVE_M_TODEG;
	}

	float ParticleSystem::getSpread() const
	{
		return spread * LOVE_M_TODEG;
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

	void ParticleSystem::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		if(sprite == 0) return; // just in case of failure

		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);

		glTranslatef(x, y, 0);
		glRotatef(angle, 0, 0, 1.0f);
		glScalef(sx, sy, 1.0f);
		glTranslatef( ox, oy, 0);

		particle * p = pStart;
		while(p != pLast)
		{
			glPushMatrix();

			glColor4f(p->color[0],p->color[1],p->color[2],p->color[3]);
			glTranslatef(p->position[0],p->position[1],0.0f);
			glRotatef(p->rotation * 57.29578f, 0.0f, 0.0f, 1.0f); // rad * (180 / pi)
			glScalef(p->size,p->size,1.0f);
			glTranslatef(-offsetX,-offsetY,0.0f);
			sprite->draw(0,0, 0, 1, 1, 0, 0);

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
		if(active)
		{
			float rate = 1.0f / emissionRate; // the amount of time between each particle emit
			emitCounter += dt;
			while(emitCounter > rate)
			{
				add();
				emitCounter -= rate;
			}
			/*int particles = (int)(emissionRate * dt);
			for(int i = 0; i != particles; i++)
				add();*/

			life -= dt;
			if(lifetime != -1 && life < 0)
				stop();
		}

		while(p != pLast)
		{
			// Decrease lifespan.
			p->life -= dt;

			if(p->life > 0)
			{

				// Temp variables.
				love::Vector radial, tangential, gravity(0, p->gravity);
				love::Vector ppos(p->position[0], p->position[1]);

				// Get vector from particle center to particle.
				radial = ppos - position;
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

				const float t = p->life / p->lifetime;

				// Change size.
				p->size = p->sizeEnd - ((p->sizeEnd - p->sizeStart) * t);

				// Rotate.
				p->rotation += (p->spinStart*(1-t) + p->spinEnd*t)*dt;

				// Update color.
				p->color[0] = (float)(colorEnd[0]*(1.0f-t) + colorStart[0] * t)/255.0f;
				p->color[1] = (float)(colorEnd[1]*(1.0f-t) + colorStart[1] * t)/255.0f;
				p->color[2] = (float)(colorEnd[2]*(1.0f-t) + colorStart[2] * t)/255.0f;
				p->color[3] = (float)(colorEnd[3]*(1.0f-t) + colorStart[3] * t)/255.0f;
	
				// Next particle.
				p++;
			}
			else
			{
				remove(p);

				if(p >= pLast)
					return;
			} // else
		} // while
	}

} // opengl
} // graphics
} // love
