/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "common/math.h"
#include "modules/math/RandomGenerator.h"

#include "OpenGL.h"
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

love::math::RandomGenerator rng;

Colorf colorToFloat(const Color &c)
{
	return Colorf((GLfloat)c.r/255.0f, (GLfloat)c.g/255.0f, (GLfloat)c.b/255.0f, (GLfloat)c.a/255.0f);
}

float calculate_variation(float inner, float outer, float var)
{
	float low = inner - (outer/2.0f)*var;
	float high = inner + (outer/2.0f)*var;
	float r = rng.random();
	return low*(1-r)+high*r;
}

} // anonymous namespace

StringMap<ParticleSystem::AreaSpreadDistribution, ParticleSystem::DISTRIBUTION_MAX_ENUM>::Entry ParticleSystem::distributionsEntries[] = {
	{ "none",     ParticleSystem::DISTRIBUTION_NONE },
	{ "uniform",  ParticleSystem::DISTRIBUTION_UNIFORM },
	{ "normal",   ParticleSystem::DISTRIBUTION_NORMAL },
};

StringMap<ParticleSystem::AreaSpreadDistribution, ParticleSystem::DISTRIBUTION_MAX_ENUM> ParticleSystem::distributions(ParticleSystem::distributionsEntries, sizeof(ParticleSystem::distributionsEntries));


ParticleSystem::ParticleSystem(Image *image, unsigned int buffer)
	: pStart(0)
	, pLast(0)
	, pEnd(0)
	, particleVerts(0)
	, image(image)
	, active(true)
	, emissionRate(0)
	, emitCounter(0)
	, areaSpreadDistribution(DISTRIBUTION_NONE)
	, lifetime(-1)
	, life(0)
	, particleLifeMin(0)
	, particleLifeMax(0)
	, direction(0)
	, spread(0)
	, relative(false)
	, speedMin(0)
	, speedMax(0)
	, radialAccelerationMin(0)
	, radialAccelerationMax(0)
	, tangentialAccelerationMin(0)
	, tangentialAccelerationMax(0)
	, sizeVariation(0)
	, rotationMin(0)
	, rotationMax(0)
	, spinStart(0)
	, spinEnd(0)
	, spinVariation(0)
	, offsetX(image->getWidth()*0.5f)
	, offsetY(image->getHeight()*0.5f)
{
	sizes.push_back(1.0f);
	colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
	setBufferSize(buffer);
	image->retain();
}

ParticleSystem::~ParticleSystem()
{
	if (this->image != 0)
		this->image->release();

	if (pStart != 0)
		delete [] pStart;

	if (particleVerts != 0)
		delete [] particleVerts;
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
		pLast->life = rng.random(min, max);
	pLast->lifetime = pLast->life;

	pLast->position[0] = position.getX();
	pLast->position[1] = position.getY();

	switch (areaSpreadDistribution)
	{
	case DISTRIBUTION_UNIFORM:
		pLast->position[0] += rng.random(-areaSpread.getX(), areaSpread.getX());
		pLast->position[1] += rng.random(-areaSpread.getY(), areaSpread.getY());
		break;
	case DISTRIBUTION_NORMAL:
		pLast->position[0] += rng.randomnormal(areaSpread.getX());
		pLast->position[1] += rng.randomnormal(areaSpread.getY());
		break;
	case DISTRIBUTION_NONE:
	default:
		break;
	}

	min = direction - spread/2.0f;
	max = direction + spread/2.0f;
	pLast->direction = rng.random(min, max);

	pLast->origin = position;

	min = speedMin;
	max = speedMax;
	float speed = rng.random(min, max);
	pLast->speed = love::Vector(cos(pLast->direction), sin(pLast->direction));
	pLast->speed *= speed;

	pLast->linearAcceleration.x = rng.random(linearAccelerationMin.x, linearAccelerationMax.x);
	pLast->linearAcceleration.y = rng.random(linearAccelerationMin.y, linearAccelerationMax.y);

	min = radialAccelerationMin;
	max = radialAccelerationMax;
	pLast->radialAcceleration = rng.random(min, max);

	min = tangentialAccelerationMin;
	max = tangentialAccelerationMax;
	pLast->tangentialAcceleration = rng.random(min, max);

	pLast->sizeOffset       = rng.random(sizeVariation); // time offset for size change
	pLast->sizeIntervalSize = (1.0f - rng.random(sizeVariation)) - pLast->sizeOffset;
	pLast->size = sizes[(size_t)(pLast->sizeOffset - .5f) * (sizes.size() - 1)];

	min = rotationMin;
	max = rotationMax;
	pLast->spinStart = calculate_variation(spinStart, spinEnd, spinVariation);
	pLast->spinEnd = calculate_variation(spinEnd, spinStart, spinVariation);
	pLast->rotation = rng.random(min, max);

	pLast->color = colors[0];

	pLast++;
}

void ParticleSystem::remove(particle *p)
{
	if (!isEmpty())
	{
		 *p = *(--pLast);
	}
}

void ParticleSystem::setImage(Image *image)
{
	if (this->image != 0)
		this->image->release();

	this->image = image;
	this->image->retain();
}

Image *ParticleSystem::getImage() const
{
	return image;
}

void ParticleSystem::setBufferSize(unsigned int size)
{
	// delete previous data
	if (pStart != 0)
		delete [] pStart;

	pLast = pStart = new particle[size]();

	pEnd = pStart + size;

	if (particleVerts != 0)
		delete [] particleVerts;

	// each particle has 4 vertices
	particleVerts = new vertex[size*4];
}

int ParticleSystem::getBufferSize() const
{
	return pEnd - pStart;
}

void ParticleSystem::setEmissionRate(int rate)
{
	emissionRate = rate;
}

int ParticleSystem::getEmissionRate() const
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

void ParticleSystem::getParticleLifetime(float *min, float *max) const
{
	if (min)
		*min = particleLifeMin;
	if (max)
		*max = particleLifeMax;
}

void ParticleSystem::setPosition(float x, float y)
{
	position = love::Vector(x, y);
}

const love::Vector &ParticleSystem::getPosition() const
{
	return position;
}

float ParticleSystem::getX() const
{
	return position.getX();
}

float ParticleSystem::getY() const
{
	return position.getY();
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

void ParticleSystem::setRelativeDirection(bool relative)
{
	this->relative = relative;
}

bool ParticleSystem::isRelativeDirection() const
{
	return relative;
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

void ParticleSystem::getSpeed(float *min, float *max) const
{
	if (min)
		*min = speedMin;
	if (max)
		*max = speedMax;
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

void ParticleSystem::getLinearAcceleration(love::Vector *min, love::Vector *max) const
{
	if (min)
		*min = linearAccelerationMin;
	if (max)
		*max = linearAccelerationMax;
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

void ParticleSystem::getRadialAcceleration(float *min, float *max) const
{
	if (min)
		*min = radialAccelerationMin;
	if (max)
		*max = radialAccelerationMax;
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

void ParticleSystem::getTangentialAcceleration(float *min, float *max) const
{
	if (min)
		*min = tangentialAccelerationMin;
	if (max)
		*max = tangentialAccelerationMax;
}

void ParticleSystem::setSize(float size)
{
	sizes.resize(1);
	sizes[0] = size;
}

void ParticleSystem::setSize(const std::vector<float> &newSizes, float variation)
{
	sizes = newSizes;
	sizeVariation = variation;
}

const std::vector<float> &ParticleSystem::getSize() const
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

void ParticleSystem::getRotation(float *min, float *max) const
{
	if (min)
		*min = rotationMin;
	if (max)
		*max = rotationMax;
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

void ParticleSystem::getSpin(float *start, float *end) const
{
	if (start)
		*start = spinStart;
	if (end)
		*end = spinEnd;
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
	offsetX = x;
	offsetY = y;
}

love::Vector ParticleSystem::getOffset() const
{
	return love::Vector(offsetX, offsetY);
}

void ParticleSystem::setColor(const Color &color)
{
	colors.resize(1);
	colors[0] = colorToFloat(color);
}

void ParticleSystem::setColor(const std::vector<Color> &newColors)
{
	colors.resize(newColors.size());
	for (size_t i = 0; i < newColors.size(); ++i)
		colors[i] = colorToFloat(newColors[i]);
}

std::vector<Color> ParticleSystem::getColor() const
{
	// We store colors as floats...
	std::vector<Color> ncolors(colors.size());

	for (size_t i = 0; i < colors.size(); ++i)
		ncolors[i] = Color(colors[i].r, colors[i].g, colors[i].b, colors[i].a);

	return ncolors;
}

int ParticleSystem::getCount() const
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

void ParticleSystem::emit(int num)
{
	if (!active)
		return;

	for (int i = 0; i < num; i++)
	{
		if (isFull())
			return;

		add();
	}
}

bool ParticleSystem::isActive() const
{
	return active;
}

bool ParticleSystem::isPaused() const
{
	return !active && life < lifetime;
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
	if (image == 0) return;  // just in case of failure

	int num = getCount();
	if (num == 0) return; // don't bother if there's nothing to do

	Color curcolor = gl.getColor();

	glPushMatrix();

	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	const vertex *imageVerts = image->getVertices();
	const vertex *tVerts;

	// set the vertex data for each particle (transformation, texcoords, color)
	for (int i = 0; i < num; i++)
	{
		particle *p = pStart + i;

		tVerts = imageVerts;

		// particle vertices are image vertices transformed by particle information
		t.setTransformation(p->position[0], p->position[1], p->rotation, p->size, p->size, offsetX, offsetY, 0.0f, 0.0f);
		t.transform(&particleVerts[i*4], &tVerts[0], 4);

		// set the texture coordinate and color data for particle vertices
		for (int v = 0; v < 4; v++)
		{
			int vi = (i * 4) + v; // current vertex index for particle

			particleVerts[vi].s = tVerts[v].s;
			particleVerts[vi].t = tVerts[v].t;

			// particle colors are stored as floats (0-1) but vertex colors are stored as unsigned bytes (0-255)
			particleVerts[vi].r = p->color.r*255;
			particleVerts[vi].g = p->color.g*255;
			particleVerts[vi].b = p->color.b*255;
			particleVerts[vi].a = p->color.a*255;
		}
	}

	image->bind();

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (GLvoid *)&particleVerts[0].r);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&particleVerts[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&particleVerts[0].s);

	glDrawArrays(GL_QUADS, 0, num*4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();

	gl.setColor(curcolor);
}

void ParticleSystem::update(float dt)
{
	// Traverse all particles and update.
	particle *p = pStart;

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
			love::Vector radial, tangential;
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
			p->speed += (radial+tangential+p->linearAcceleration)*dt;

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

bool ParticleSystem::getConstant(const char *in, AreaSpreadDistribution &out)
{
	return distributions.find(in, out);
}

bool ParticleSystem::getConstant(AreaSpreadDistribution in, const char *&out)
{
	return distributions.find(in, out);
}

} // opengl
} // graphics
} // love
