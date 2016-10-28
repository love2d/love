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

//LOVE
#include "common/config.h"
#include "ParticleSystem.h"

#include "common/math.h"
#include "modules/math/RandomGenerator.h"

// STD
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace love
{
namespace graphics
{

namespace
{

love::math::RandomGenerator rng;

float calculate_variation(float inner, float outer, float var)
{
	float low = inner - (outer/2.0f)*var;
	float high = inner + (outer/2.0f)*var;
	float r = (float) rng.random();
	return low*(1-r)+high*r;
}

} // anonymous namespace

ParticleSystem::ParticleSystem(Texture *texture, uint32 size)
	: pMem(nullptr)
	, pFree(nullptr)
	, pHead(nullptr)
	, pTail(nullptr)
	, texture(texture)
	, active(true)
	, insertMode(INSERT_MODE_TOP)
	, maxParticles(0)
	, activeParticles(0)
	, emissionRate(0)
	, emitCounter(0)
	, areaSpreadDistribution(DISTRIBUTION_NONE)
	, lifetime(-1)
	, life(0)
	, particleLifeMin(0)
	, particleLifeMax(0)
	, direction(0)
	, spread(0)
	, speedMin(0)
	, speedMax(0)
	, linearAccelerationMin(0, 0)
	, linearAccelerationMax(0, 0)
	, radialAccelerationMin(0)
	, radialAccelerationMax(0)
	, tangentialAccelerationMin(0)
	, tangentialAccelerationMax(0)
	, linearDampingMin(0.0f)
	, linearDampingMax(0.0f)
	, sizeVariation(0)
	, rotationMin(0)
	, rotationMax(0)
	, spinStart(0)
	, spinEnd(0)
	, spinVariation(0)
	, offset(float(texture->getWidth())*0.5f, float(texture->getHeight())*0.5f)
	, defaultOffset(true)
	, relativeRotation(false)
{
	if (size == 0 || size > MAX_PARTICLES)
		throw love::Exception("Invalid ParticleSystem size.");

	sizes.push_back(1.0f);
	colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
	setBufferSize(size);
}

ParticleSystem::ParticleSystem(const ParticleSystem &p)
	: pMem(nullptr)
	, pFree(nullptr)
	, pHead(nullptr)
	, pTail(nullptr)
	, texture(p.texture)
	, active(p.active)
	, insertMode(p.insertMode)
	, maxParticles(p.maxParticles)
	, activeParticles(0)
	, emissionRate(p.emissionRate)
	, emitCounter(0.0f)
	, position(p.position)
	, prevPosition(p.prevPosition)
	, areaSpreadDistribution(p.areaSpreadDistribution)
	, areaSpread(p.areaSpread)
	, lifetime(p.lifetime)
	, life(p.lifetime) // Initialize with the maximum life time.
	, particleLifeMin(p.particleLifeMin)
	, particleLifeMax(p.particleLifeMax)
	, direction(p.direction)
	, spread(p.spread)
	, speedMin(p.speedMin)
	, speedMax(p.speedMax)
	, linearAccelerationMin(p.linearAccelerationMin)
	, linearAccelerationMax(p.linearAccelerationMax)
	, radialAccelerationMin(p.radialAccelerationMin)
	, radialAccelerationMax(p.radialAccelerationMax)
	, tangentialAccelerationMin(p.tangentialAccelerationMin)
	, tangentialAccelerationMax(p.tangentialAccelerationMax)
	, linearDampingMin(p.linearDampingMin)
	, linearDampingMax(p.linearDampingMax)
	, sizes(p.sizes)
	, sizeVariation(p.sizeVariation)
	, rotationMin(p.rotationMin)
	, rotationMax(p.rotationMax)
	, spinStart(p.spinStart)
	, spinEnd(p.spinEnd)
	, spinVariation(p.spinVariation)
	, offset(p.offset)
	, defaultOffset(p.defaultOffset)
	, colors(p.colors)
	, quads(p.quads)
	, relativeRotation(p.relativeRotation)
{
	setBufferSize(maxParticles);
}

ParticleSystem::~ParticleSystem()
{
	deleteBuffers();
}

void ParticleSystem::resetOffset()
{
	if (quads.empty())
		offset = love::Vector(float(texture->getWidth())*0.5f, float(texture->getHeight())*0.5f);
	else
	{
		Quad::Viewport v = quads[0]->getViewport();
		offset = love::Vector(v.x*0.5f, v.y*0.5f);
	}
}

void ParticleSystem::createBuffers(size_t size)
{
	try
	{
		pFree = pMem = new Particle[size];
		maxParticles = (uint32) size;
	}
	catch (std::bad_alloc &)
	{
		deleteBuffers();
		throw love::Exception("Out of memory");
	}
}

void ParticleSystem::deleteBuffers()
{
	// Clean up for great gracefulness!
	delete[] pMem;

	pMem = nullptr;
	maxParticles = 0;
	activeParticles = 0;
}

void ParticleSystem::setBufferSize(uint32 size)
{
	if (size == 0 || size > MAX_PARTICLES)
		throw love::Exception("Invalid buffer size");
	deleteBuffers();
	createBuffers(size);
	reset();
}

uint32 ParticleSystem::getBufferSize() const
{
	return maxParticles;
}

void ParticleSystem::addParticle(float t)
{
	if (isFull())
		return;

	// Gets a free particle and updates the allocation pointer.
	Particle *p = pFree++;
	initParticle(p, t);

	switch (insertMode)
	{
	default:
	case INSERT_MODE_TOP:
		insertTop(p);
		break;
	case INSERT_MODE_BOTTOM:
		insertBottom(p);
		break;
	case INSERT_MODE_RANDOM:
		insertRandom(p);
		break;
	}

	activeParticles++;
}

void ParticleSystem::initParticle(Particle *p, float t)
{
	float min,max;

	// Linearly interpolate between the previous and current emitter position.
	love::Vector pos = prevPosition + (position - prevPosition) * t;

	min = particleLifeMin;
	max = particleLifeMax;
	if (min == max)
		p->life = min;
	else
		p->life = (float) rng.random(min, max);
	p->lifetime = p->life;

	p->position = pos;

	float rand_x, rand_y;
	switch (areaSpreadDistribution)
	{
	case DISTRIBUTION_UNIFORM:
		p->position.x += (float) rng.random(-areaSpread.getX(), areaSpread.getX());
		p->position.y += (float) rng.random(-areaSpread.getY(), areaSpread.getY());
		break;
	case DISTRIBUTION_NORMAL:
		p->position.x += (float) rng.randomNormal(areaSpread.getX());
		p->position.y += (float) rng.randomNormal(areaSpread.getY());
		break;
	case DISTRIBUTION_ELLIPSE:
		rand_x = (float) rng.random(-1, 1);
		rand_y = (float) rng.random(-1, 1);
		p->position.x += areaSpread.getX() * (rand_x * sqrt(1 - 0.5f*pow(rand_y, 2)));
		p->position.y += areaSpread.getY() * (rand_y * sqrt(1 - 0.5f*pow(rand_x, 2)));
		break;
	case DISTRIBUTION_NONE:
	default:
		break;
	}

	p->origin = pos;

	min = speedMin;
	max = speedMax;
	float speed = (float) rng.random(min, max);

	min = direction - spread/2.0f;
	max = direction + spread/2.0f;
	float dir = (float) rng.random(min, max);

	p->velocity = love::Vector(cosf(dir), sinf(dir)) * speed;

	p->linearAcceleration.x = (float) rng.random(linearAccelerationMin.x, linearAccelerationMax.x);
	p->linearAcceleration.y = (float) rng.random(linearAccelerationMin.y, linearAccelerationMax.y);

	min = radialAccelerationMin;
	max = radialAccelerationMax;
	p->radialAcceleration = (float) rng.random(min, max);

	min = tangentialAccelerationMin;
	max = tangentialAccelerationMax;
	p->tangentialAcceleration = (float) rng.random(min, max);

	min = linearDampingMin;
	max = linearDampingMax;
	p->linearDamping = (float) rng.random(min, max);

	p->sizeOffset       = (float) rng.random(sizeVariation); // time offset for size change
	p->sizeIntervalSize = (1.0f - (float) rng.random(sizeVariation)) - p->sizeOffset;
	p->size = sizes[(size_t)(p->sizeOffset - .5f) * (sizes.size() - 1)];

	min = rotationMin;
	max = rotationMax;
	p->spinStart = calculate_variation(spinStart, spinEnd, spinVariation);
	p->spinEnd = calculate_variation(spinEnd, spinStart, spinVariation);
	p->rotation = (float) rng.random(min, max);

	p->angle = p->rotation;
	if (relativeRotation)
		p->angle += atan2f(p->velocity.y, p->velocity.x);

	p->color = colors[0];

	p->quadIndex = 0;
}

void ParticleSystem::insertTop(Particle *p)
{
	if (pHead == nullptr)
	{
		pHead = p;
		p->prev = nullptr;
	}
	else
	{
		pTail->next = p;
		p->prev = pTail;
	}
	p->next = nullptr;
	pTail = p;
}

void ParticleSystem::insertBottom(Particle *p)
{
	if (pTail == nullptr)
	{
		pTail = p;
		p->next = nullptr;
	}
	else
	{
		pHead->prev = p;
		p->next = pHead;
	}
	p->prev = nullptr;
	pHead = p;
}

void ParticleSystem::insertRandom(Particle *p)
{
	// Nonuniform, but 64-bit is so large nobody will notice. Hopefully.
	uint64 pos = rng.rand() % ((int64) activeParticles + 1);

	// Special case where the particle gets inserted before the head.
	if (pos == activeParticles)
	{
		Particle *pA = pHead;
		if (pA)
			pA->prev = p;
		p->prev = nullptr;
		p->next = pA;
		pHead = p;
		return;
	}

	// Inserts the particle after the randomly selected particle.
	Particle *pA = pMem + pos;
	Particle *pB = pA->next;
	pA->next = p;
	if (pB)
		pB->prev = p;
	else
		pTail = p;
	p->prev = pA;
	p->next = pB;
}

ParticleSystem::Particle *ParticleSystem::removeParticle(Particle *p)
{
	// The linked list is updated in this function and old pointers may be
	// invalidated. The returned pointer will inform the caller of the new
	// pointer to the next particle.
	Particle *pNext = nullptr;

	// Removes the particle from the linked list.
	if (p->prev)
		p->prev->next = p->next;
	else
		pHead = p->next;

	if (p->next)
	{
		p->next->prev = p->prev;
		pNext = p->next;
	}
	else
		pTail = p->prev;

	// The (in memory) last particle can now be moved into the free slot.
	// It will skip the moving if it happens to be the removed particle.
	pFree--;
	if (p != pFree)
	{
		*p = *pFree;
		if (pNext == pFree)
			pNext = p;

		if (p->prev)
			p->prev->next = p;
		else
			pHead = p;

		if (p->next)
			p->next->prev = p;
		else
			pTail = p;
	}

	activeParticles--;
	return pNext;
}

void ParticleSystem::setTexture(Texture *tex)
{
	texture.set(tex);

	if (defaultOffset)
		resetOffset();
}

Texture *ParticleSystem::getTexture() const
{
	return texture.get();
}

void ParticleSystem::setInsertMode(InsertMode mode)
{
	insertMode = mode;
}

ParticleSystem::InsertMode ParticleSystem::getInsertMode() const
{
	return insertMode;
}

void ParticleSystem::setEmissionRate(float rate)
{
	if (rate < 0.0f)
		throw love::Exception("Invalid emission rate");
	emissionRate = rate;

	// Prevent an explosion when dramatically increasing the rate
	emitCounter = std::min(emitCounter, 1.0f/rate);
}

float ParticleSystem::getEmissionRate() const
{
	return emissionRate;
}

void ParticleSystem::setEmitterLifetime(float life)
{
	this->life = lifetime = life;
}

float ParticleSystem::getEmitterLifetime() const
{
	return lifetime;
}

void ParticleSystem::setParticleLifetime(float min, float max)
{
	particleLifeMin = min;
	if (max == 0)
		particleLifeMax = min;
	else
		particleLifeMax = max;
}

void ParticleSystem::getParticleLifetime(float &min, float &max) const
{
	min = particleLifeMin;
	max = particleLifeMax;
}

void ParticleSystem::setPosition(float x, float y)
{
	position = love::Vector(x, y);
	prevPosition = position;
}

const love::Vector &ParticleSystem::getPosition() const
{
	return position;
}

void ParticleSystem::moveTo(float x, float y)
{
	position = love::Vector(x, y);
}

void ParticleSystem::setAreaSpread(AreaSpreadDistribution distribution, float x, float y)
{
	areaSpread = love::Vector(x, y);
	areaSpreadDistribution = distribution;
}

ParticleSystem::AreaSpreadDistribution ParticleSystem::getAreaSpreadDistribution() const
{
	return areaSpreadDistribution;
}

const love::Vector &ParticleSystem::getAreaSpreadParameters() const
{
	return areaSpread;
}

void ParticleSystem::setDirection(float direction)
{
	this->direction = direction;
}

float ParticleSystem::getDirection() const
{
	return direction;
}

void ParticleSystem::setSpread(float spread)
{
	this->spread = spread;
}

float ParticleSystem::getSpread() const
{
	return spread;
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

void ParticleSystem::getSpeed(float &min, float &max) const
{
	min = speedMin;
	max = speedMax;
}

void ParticleSystem::setLinearAcceleration(float x, float y)
{
	linearAccelerationMin.x = linearAccelerationMax.x = x;
	linearAccelerationMin.y = linearAccelerationMax.y = y;
}

void ParticleSystem::setLinearAcceleration(float xmin, float ymin, float xmax, float ymax)
{
	linearAccelerationMin = love::Vector(xmin, ymin);
	linearAccelerationMax = love::Vector(xmax, ymax);
}

void ParticleSystem::getLinearAcceleration(love::Vector &min, love::Vector &max) const
{
	min = linearAccelerationMin;
	max = linearAccelerationMax;
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

void ParticleSystem::getRadialAcceleration(float &min, float &max) const
{
	min = radialAccelerationMin;
	max = radialAccelerationMax;
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

void ParticleSystem::getTangentialAcceleration(float &min, float &max) const
{
	min = tangentialAccelerationMin;
	max = tangentialAccelerationMax;
}

void ParticleSystem::setLinearDamping(float min, float max)
{
	linearDampingMin = min;
	linearDampingMax = max;
}

void ParticleSystem::getLinearDamping(float &min, float &max) const
{
	min = linearDampingMin;
	max = linearDampingMax;
}

void ParticleSystem::setSize(float size)
{
	sizes.resize(1);
	sizes[0] = size;
}

void ParticleSystem::setSizes(const std::vector<float> &newSizes)
{
	sizes = newSizes;
}

const std::vector<float> &ParticleSystem::getSizes() const
{
	return sizes;
}

void ParticleSystem::setSizeVariation(float variation)
{
	sizeVariation = variation;
}

float ParticleSystem::getSizeVariation() const
{
	return sizeVariation;
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

void ParticleSystem::getRotation(float &min, float &max) const
{
	min = rotationMin;
	max = rotationMax;
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

void ParticleSystem::getSpin(float &start, float &end) const
{
	start = spinStart;
	end = spinEnd;
}

void ParticleSystem::setSpinVariation(float variation)
{
	spinVariation = variation;
}

float ParticleSystem::getSpinVariation() const
{
	return spinVariation;
}

void ParticleSystem::setOffset(float x, float y)
{
	offset = love::Vector(x, y);
	defaultOffset = false;
}

love::Vector ParticleSystem::getOffset() const
{
	return offset;
}

void ParticleSystem::setColor(const std::vector<Colorf> &newColors)
{
	colors = newColors;

	for (Colorf &c : colors)
	{
		// We want to store the colors as [0, 1], rather than [0, 255].
		c.r /= 255.0f;
		c.g /= 255.0f;
		c.b /= 255.0f;
		c.a /= 255.0f;
	}
}

std::vector<Colorf> ParticleSystem::getColor() const
{
	// The particle system stores colors in the range of [0, 1]...
	std::vector<Colorf> ncolors(colors);

	for (Colorf &c : ncolors)
	{
		c.r *= 255.0f;
		c.g *= 255.0f;
		c.b *= 255.0f;
		c.a *= 255.0f;
	}

	return ncolors;
}

void ParticleSystem::setQuads(const std::vector<Quad *> &newQuads)
{
	std::vector<StrongRef<Quad>> quadlist;
	quadlist.reserve(newQuads.size());

	for (Quad *q : newQuads)
		quadlist.push_back(q);

	quads = quadlist;

	if (defaultOffset)
		resetOffset();
}

void ParticleSystem::setQuads()
{
	quads.clear();
}

std::vector<Quad *> ParticleSystem::getQuads() const
{
	std::vector<Quad *> quadlist;
	quadlist.reserve(quads.size());

	for (const StrongRef<Quad> &q : quads)
		quadlist.push_back(q.get());

	return quadlist;
}

void ParticleSystem::setRelativeRotation(bool enable)
{
	relativeRotation = enable;
}

bool ParticleSystem::hasRelativeRotation() const
{
	return relativeRotation;
}

uint32 ParticleSystem::getCount() const
{
	return activeParticles;
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
	if (pMem == nullptr)
		return;

	pFree = pMem;
	pHead = nullptr;
	pTail = nullptr;
	activeParticles = 0;
	life = lifetime;
	emitCounter = 0;
}

void ParticleSystem::emit(uint32 num)
{
	if (!active)
		return;

	num = std::min(num, maxParticles - activeParticles);

	while (num--)
		addParticle(1.0f);
}

bool ParticleSystem::isActive() const
{
	return active;
}

bool ParticleSystem::isPaused() const
{
	return !active && life < lifetime;
}

bool ParticleSystem::isStopped() const
{
	return !active && life >= lifetime;
}

bool ParticleSystem::isEmpty() const
{
	return activeParticles == 0;
}

bool ParticleSystem::isFull() const
{
	return activeParticles == maxParticles;
}

void ParticleSystem::update(float dt)
{
	if (pMem == nullptr || dt == 0.0f)
		return;

	// Traverse all particles and update.
	Particle *p = pHead;

	while (p)
	{
		// Decrease lifespan.
		p->life -= dt;

		if (p->life <= 0)
			p = removeParticle(p);
		else
		{
			// Temp variables.
			love::Vector radial, tangential;
			love::Vector ppos = p->position;

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

			// Update velocity.
			p->velocity += (radial + tangential + p->linearAcceleration) * dt;

			// Apply damping.
			p->velocity *= 1.0f / (1.0f + p->linearDamping * dt);

			// Modify position.
			ppos += p->velocity * dt;

			p->position = ppos;

			const float t = 1.0f - p->life / p->lifetime;

			// Rotate.
			p->rotation += (p->spinStart * (1.0f - t) + p->spinEnd * t) * dt;

			p->angle = p->rotation;

			if (relativeRotation)
				p->angle += atan2f(p->velocity.y, p->velocity.x);

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

			// Update the quad index.
			k = quads.size();
			if (k > 0)
			{
				s = t * (float) k; // [0:numquads-1] (clamped below)
				i = (s > 0.0f) ? (size_t) s : 0;
				p->quadIndex = (int) ((i < k) ? i : k - 1);
			}

			// Next particle.
			p = p->next;
		}
	}

	// Make some more particles.
	if (active)
	{
		float rate = 1.0f / emissionRate; // the amount of time between each particle emit
		emitCounter += dt;
		float total = emitCounter - rate;
		while (emitCounter > rate)
		{
			addParticle(1.0f - (emitCounter - rate) / total);
			emitCounter -= rate;
		}

		life -= dt;
		if (lifetime != -1 && life < 0)
			stop();
	}

	prevPosition = position;
}

bool ParticleSystem::getConstant(const char *in, AreaSpreadDistribution &out)
{
	return distributions.find(in, out);
}

bool ParticleSystem::getConstant(AreaSpreadDistribution in, const char *&out)
{
	return distributions.find(in, out);
}

bool ParticleSystem::getConstant(const char *in, InsertMode &out)
{
	return insertModes.find(in, out);
}

bool ParticleSystem::getConstant(InsertMode in, const char *&out)
{
	return insertModes.find(in, out);
}

StringMap<ParticleSystem::AreaSpreadDistribution, ParticleSystem::DISTRIBUTION_MAX_ENUM>::Entry ParticleSystem::distributionsEntries[] =
{
	{ "none",    DISTRIBUTION_NONE },
	{ "uniform", DISTRIBUTION_UNIFORM },
	{ "normal",  DISTRIBUTION_NORMAL },
	{ "ellipse",  DISTRIBUTION_ELLIPSE },
};

StringMap<ParticleSystem::AreaSpreadDistribution, ParticleSystem::DISTRIBUTION_MAX_ENUM> ParticleSystem::distributions(ParticleSystem::distributionsEntries, sizeof(ParticleSystem::distributionsEntries));

StringMap<ParticleSystem::InsertMode, ParticleSystem::INSERT_MODE_MAX_ENUM>::Entry ParticleSystem::insertModesEntries[] =
{
	{ "top",    INSERT_MODE_TOP },
	{ "bottom", INSERT_MODE_BOTTOM },
	{ "random", INSERT_MODE_RANDOM },
};

StringMap<ParticleSystem::InsertMode, ParticleSystem::INSERT_MODE_MAX_ENUM> ParticleSystem::insertModes(ParticleSystem::insertModesEntries, sizeof(ParticleSystem::insertModesEntries));

} // graphics
} // love
