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

#ifndef LOVE_GRAPHICS_PARTICLE_SYSTEM_H
#define LOVE_GRAPHICS_PARTICLE_SYSTEM_H

// LOVE
#include "common/int.h"
#include "common/math.h"
#include "common/Vector.h"
#include "Drawable.h"
#include "Color.h"
#include "Quad.h"
#include "Texture.h"

// STL
#include <vector>

namespace love
{
namespace graphics
{

/**
 * A class for creating, moving and drawing particles.
 * A big thanks to bobthebloke.org
 **/
class ParticleSystem : public Drawable
{
public:
	/**
	 * Type of distribution new particles are drawn from: None, uniform, normal.
	 */
	enum AreaSpreadDistribution
	{
		DISTRIBUTION_NONE,
		DISTRIBUTION_UNIFORM,
		DISTRIBUTION_NORMAL,
		DISTRIBUTION_MAX_ENUM
	};

	/**
	 * Insertion modes of new particles in the list: top, bottom, random.
	 */
	enum InsertMode
	{
		INSERT_MODE_TOP,
		INSERT_MODE_BOTTOM,
		INSERT_MODE_RANDOM,
		INSERT_MODE_MAX_ENUM
	};

	/**
	 * Maximum numbers of particles in a ParticleSystem.
	 * This limit comes from the fact that a quad requires four vertices and the
	 * OpenGL API where GLsizei is a signed int.
	 **/
	static const uint32 MAX_PARTICLES = LOVE_INT32_MAX / 4;

	/**
	 * Creates a particle system with the specified buffer size and texture.
	 **/
	ParticleSystem(Texture *texture, uint32 buffer);
	ParticleSystem(const ParticleSystem &p);

	/**
	 * Deletes any allocated memory.
	 **/
	virtual ~ParticleSystem();

	/**
	 * Creates an identical copy of this ParticleSystem. The clone does not
	 * duplicate any existing particles from this ParticleSystem, just the
	 * settable parameters.
	 **/
	virtual ParticleSystem *clone() = 0;

	/**
	 * Sets the texture used in the particle system.
	 * @param texture The new texture.
	 **/
	void setTexture(Texture *texture);

	/**
	 * Returns the texture used when drawing the particle system.
	 **/
	Texture *getTexture() const;

	/**
	 * Clears the current buffer and allocates the appropriate amount of space for the buffer.
	 * @param size The new buffer size.
	 **/
	virtual void setBufferSize(uint32 size);

	/**
	 * Returns the total amount of particles this ParticleSystem can have active
	 * at any given point in time.
	 **/
	uint32 getBufferSize() const;

	/**
	 * Sets the insert mode for new particles.
	 * @param mode The new insert mode.
	 */
	void setInsertMode(InsertMode mode);

	/**
	 * Returns the current insert mode.
	 */
	InsertMode getInsertMode() const;

	/**
	 * Sets the emission rate.
	 * @param rate The amount of particles per second.
	 **/
	void setEmissionRate(float rate);

	/**
	 * Returns the number of particles created per second.
	 **/
	float getEmissionRate() const;

	/**
	 * Sets the lifetime of the particle emitter (-1 means eternal)
	 * @param life The lifetime (in seconds).
	 **/
	void setEmitterLifetime(float life);

	/**
	 * Returns the lifetime of the particle emitter.
	 **/
	float getEmitterLifetime() const;

	/**
	 * Sets the life range of the particles.
	 * @param min The minimum life.
	 * @param max The maximum life (if 0, then becomes the same as minimum life).
	 **/
	void setParticleLifetime(float min, float max = 0);

	/**
	 * Gets the lifetime of a particle.
	 * @param[out] min The minimum life.
	 * @param[out] max The maximum life.
	 **/
	void getParticleLifetime(float &min, float &max) const;

	/**
	 * Sets the position of the center of the emitter.
	 * Used to move the emitter without changing the position of already existing particles.
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 **/
	void setPosition(float x, float y);

	/**
	 * Returns the position of the emitter.
	 **/
	const love::Vector &getPosition() const;

	/**
	 * Moves the position of the center of the emitter.
	 * When update is called, newly spawned particles will appear in a line
	 * between the old emitter position and where the emitter was moved to,
	 * resulting in a smoother-feeling particle system if moveTo is called
	 * repeatedly.
	 **/
	void moveTo(float x, float y);

	/**
	 * Sets the emission area spread parameters and distribution type. The interpretation of
	 * the parameters depends on the distribution type:
	 *
	 * * None:    Parameters are ignored. No area spread.
	 * * Uniform: Parameters denote maximal (symmetric) displacement from emitter position.
	 * * Normal:  Parameters denote the standard deviation in x and y direction. x and y are assumed to be uncorrelated.
	 * @param x First parameter. Interpretation depends on distribution type.
	 * @param y Second parameter. Interpretation depends on distribution type.
	 * @param distribution Distribution type
	 **/
	void setAreaSpread(AreaSpreadDistribution distribution, float x, float y);

	/**
	 * Returns area spread distribution type.
	 **/
	AreaSpreadDistribution getAreaSpreadDistribution() const;

	/**
	 * Returns area spread parameters.
	 **/
	const love::Vector &getAreaSpreadParameters() const;

	/**
	 * Sets the direction of the particle emitter.
	 * @param direction The direction (in degrees).
	 **/
	void setDirection(float direction);

	/**
	 * Returns the direction of the particle emitter (in radians).
	 **/
	float getDirection() const;

	/**
	 * Sets the spread of the particle emitter.
	 * @param spread The spread (in radians).
	 **/
	void setSpread(float spread);

	/**
	 * Returns the directional spread of the emitter (in radians).
	 **/
	float getSpread() const;

	/**
	 * Sets the speed of the particles.
	 * @param speed The speed.
	 **/
	void setSpeed(float speed);

	/**
	 * Sets the speed of the particles.
	 * @param min The minimum speed.
	 * @param max The maximum speed.
	 **/
	void setSpeed(float min, float max);

	/**
	 * Gets the speed of the particles.
	 * @param[out] min The minimum speed.
	 * @param[out] max The maximum speed.
	 **/
	void getSpeed(float &min, float &max) const;

	/**
	 * Sets the linear acceleration (the acceleration along the x and y axes).
	 * @param x The acceleration along the x-axis.
	 * @param y The acceleration along the y-axis.
	 **/
	void setLinearAcceleration(float x, float y);

	/**
	 * Sets the linear acceleration (the acceleration along the x and y axes).
	 * @param xmin The minimum amount of acceleration along the x-axis.
	 * @param ymin The minimum amount of acceleration along the y-axis.
	 * @param xmax The maximum amount of acceleration along the x-axis.
	 * @param ymax The maximum amount of acceleration along the y-axis.
	 **/
	void setLinearAcceleration(float xmin, float ymin, float xmax, float ymax);

	/**
	 * Gets the linear acceleration of the particles.
	 * @param[out] min The minimum acceleration.
	 * @param[out] max The maximum acceleration.
	 **/
	void getLinearAcceleration(love::Vector &min, love::Vector &max) const;

	/**
	 * Sets the radial acceleration (the acceleration towards the particle emitter).
	 * @param acceleration The amount of acceleration.
	 **/
	void setRadialAcceleration(float acceleration);

	/**
	 * Sets the radial acceleration (the acceleration towards the particle emitter).
	 * @param min The minimum acceleration.
	 * @param max The maximum acceleration.
	 **/
	void setRadialAcceleration(float min, float max);

	/**
	 * Gets the radial acceleration.
	 * @param[out] min The minimum amount of radial acceleration.
	 * @param[out] max The maximum amount of radial acceleration.
	 **/
	void getRadialAcceleration(float &min, float &max) const;

	/**
	 * Sets the tangential acceleration (the acceleration perpendicular to the particle's direction).
	 * @param acceleration The amount of acceleration.
	 **/
	void setTangentialAcceleration(float acceleration);

	/**
	 * Sets the tangential acceleration (the acceleration perpendicular to the particle's direction).
	 * @param min The minimum acceleration.
	 * @param max The maximum acceleration.
	 **/
	void setTangentialAcceleration(float min, float max);

	/**
	 * Gets the tangential acceleration.
	 * @param[out] min The minimum tangential acceleration.
	 * @param[out] max The maximum tangential acceleration.
	 **/
	void getTangentialAcceleration(float &min, float &max) const;

	/**
	 * Sets the amount of linear damping. Damping reduces the velocity of
	 * particles over time. A value of 0 corresponds to no damping.
	 **/
	void setLinearDamping(float min, float max);

	/**
	 * Gets the current amount of linear damping.
	 **/
	void getLinearDamping(float &min, float &max) const;

	/**
	 * Sets the size of the sprite (1.0 being the default size).
	 * @param size The size of the sprite.
	 **/
	void setSize(float size);

	/**
	 * Sets the sizes of the sprite upon creation and upon death (1.0 being the default size).
	 * @param newSizes Array of sizes
	 **/
	void setSizes(const std::vector<float> &newSizes);

	/**
	 * Returns the sizes of the particle sprites.
	 **/
	const std::vector<float> &getSizes() const;

	/**
	 * Sets the amount of variation to the sprite's beginning size (0 being no variation and 1.0 a random size between start and end).
	 * @param variation The amount of variation.
	 **/
	void setSizeVariation(float variation);

	/**
	 * Returns the amount of initial size variation between particles.
	 **/
	float getSizeVariation() const;

	/**
	 * Sets the amount of rotation a sprite starts out with.
	 * @param rotation The amount of rotation.
	 **/
	void setRotation(float rotation);

	/**
	 * Sets the amount of rotation a sprite starts out with (a random value between min and max).
	 * @param min The minimum amount of rotation.
	 * @param max The maximum amount of rotation.
	 **/
	void setRotation(float min, float max);

	/**
	 * Gets the initial amount of rotation of a particle, in radians.
	 * @param[out] min The minimum initial rotation.
	 * @param[out] max The maximum initial rotation.
	 **/
	void getRotation(float &min, float &max) const;

	/**
	 * Sets the spin of the sprite.
	 * @param spin The spin of the sprite (in degrees).
	 **/
	void setSpin(float spin);

	/**
	 * Sets the spin of the sprite upon particle creation and death.
	 * @param start The spin of the sprite upon creation (in radians / second).
	 * @param end The spin of the sprite upon death (in radians / second).
	 **/
	void setSpin(float start, float end);

	/**
	 * Gets the amount of spin of a particle during its lifetime.
	 * @param[out] start The initial spin, in radians / s.
	 * @param[out] end The final spin, in radians / s.
	 **/
	void getSpin(float &start, float &end) const;

	/**
	 * Sets the variation of the start spin (0 being no variation and 1 being a random spin between start and end).
	 * @param variation The variation.
	 **/
	void setSpinVariation(float variation);

	/**
	 * Returns the amount of variation of the start spin of a particle.
	 **/
	float getSpinVariation() const;

	/**
	 * Sets the particles' offsets for rotation.
	 * @param x The x offset.
	 * @param y The y offset.
	 **/
	void setOffset(float x, float y);

	/**
	 * Returns of the particle offset.
	 **/
	love::Vector getOffset() const;

	/**
	 * Sets the color of the particles.
	 * @param newColors Array of colors
	 **/
	void setColor(const std::vector<Colorf> &newColors);

	/**
	 * Returns the color of the particles.
	 **/
	std::vector<Colorf> getColor() const;

	/**
	 * Sets a list of Quads to use for particles over their lifetime.
	 **/
	void setQuads(const std::vector<Quad *> &newQuads);
	void setQuads();

	/**
	 * Gets the Quads used when drawing the particles.
	 **/
	std::vector<Quad *> getQuads() const;

	/**
	 * sets whether particle angles & rotations are relative to their velocities.
	 **/
	void setRelativeRotation(bool enable);
	bool hasRelativeRotation() const;

	/**
	 * Returns the amount of particles that are currently active in the system.
	 **/
	uint32 getCount() const;

	/**
	 * Starts/resumes the particle emitter.
	 **/
	void start();

	/**
	 * Stops the particle emitter and resets.
	 **/
	void stop();

	/**
	 * Pauses the particle emitter.
	 **/
	void pause();

	/**
	 * Resets the particle emitter.
	 **/
	void reset();

	/**
	 * Instantly emits a number of particles.
	 * @param num The number of particles to emit.
	 **/
	void emit(uint32 num);

	/**
	 * Returns whether the particle emitter is active.
	 **/
	bool isActive() const;

	/**
	 * Returns whether the particle emitter is paused.
	 **/
	bool isPaused() const;

	bool isStopped() const;

	/**
	 * Returns whether the particle system is empty of particles or not.
	 **/
	bool isEmpty() const;

	/**
	 * Returns whether the amount of particles has reached the buffer limit or not.
	 **/
	bool isFull() const;

	/**
	 * Updates the particle system.
	 * @param dt Time since last update.
	 **/
	void update(float dt);

	static bool getConstant(const char *in, AreaSpreadDistribution &out);
	static bool getConstant(AreaSpreadDistribution in, const char *&out);

	static bool getConstant(const char *in, InsertMode &out);
	static bool getConstant(InsertMode in, const char *&out);

protected:

	// Represents a single particle.
	struct Particle
	{
		Particle *prev;
		Particle *next;

		float lifetime;
		float life;

		love::Vector position;

		// Particles gravitate towards this point.
		love::Vector origin;

		love::Vector velocity;
		love::Vector linearAcceleration;
		float radialAcceleration;
		float tangentialAcceleration;

		float linearDamping;

		float size;
		float sizeOffset;
		float sizeIntervalSize;

		float rotation; // Amount of rotation applied to the final angle.
		float angle;
		float spinStart;
		float spinEnd;

		Colorf color;

		int quadIndex;
	};

	// Pointer to the beginning of the allocated memory.
	Particle *pMem;

	// Pointer to a free particle.
	Particle *pFree;

	// Pointer to the start of the linked list.
	Particle *pHead;

	// Pointer to the end of the linked list.
	Particle *pTail;

	// The texture to be drawn.
	StrongRef<Texture> texture;

	// Whether the particle emitter is active.
	bool active;

	// Insert mode of new particles.
	InsertMode insertMode;

	// The maximum number of particles.
	uint32 maxParticles;

	// The number of active particles.
	uint32 activeParticles;

	// The emission rate (particles/sec).
	float emissionRate;

	// Used to determine when a particle should be emitted.
	float emitCounter;

	// The relative position of the particle emitter.
	love::Vector position;
	love::Vector prevPosition;

	// Emission area spread.
	AreaSpreadDistribution areaSpreadDistribution;
	love::Vector areaSpread;

	// The lifetime of the particle emitter (-1 means infinite) and the life it has left.
	float lifetime;
	float life;

	// The particle life.
	float particleLifeMin;
	float particleLifeMax;

	// The direction (and spread) the particles will be emitted in. Measured in radians.
	float direction;
	float spread;

	// The speed.
	float speedMin;
	float speedMax;

	// Acceleration along the x and y axes.
	love::Vector linearAccelerationMin;
	love::Vector linearAccelerationMax;

	// Acceleration towards the emitter's center
	float radialAccelerationMin;
	float radialAccelerationMax;

	// Acceleration perpendicular to the particle's direction.
	float tangentialAccelerationMin;
	float tangentialAccelerationMax;

	float linearDampingMin;
	float linearDampingMax;

	// Size.
	std::vector<float> sizes;
	float sizeVariation;

	// Rotation
	float rotationMin;
	float rotationMax;

	// Spin.
	float spinStart;
	float spinEnd;
	float spinVariation;

	// Offsets
	love::Vector offset;

	// Is the ParticleSystem using a default offset?
	bool defaultOffset;

	// Color.
	std::vector<Colorf> colors;

	// Quads.
	std::vector<StrongRef<Quad>> quads;

	bool relativeRotation;

private:

	void resetOffset();

	void createBuffers(size_t size);
	void deleteBuffers();

	void addParticle(float t);
	Particle *removeParticle(Particle *p);

	// Called by addParticle.
	void initParticle(Particle *p, float t);
	void insertTop(Particle *p);
	void insertBottom(Particle *p);
	void insertRandom(Particle *p);

	static StringMap<AreaSpreadDistribution, DISTRIBUTION_MAX_ENUM>::Entry distributionsEntries[];
	static StringMap<AreaSpreadDistribution, DISTRIBUTION_MAX_ENUM> distributions;

	static StringMap<InsertMode, INSERT_MODE_MAX_ENUM>::Entry insertModesEntries[];
	static StringMap<InsertMode, INSERT_MODE_MAX_ENUM> insertModes;
};

} // graphics
} // love

#endif // LOVE_GRAPHICS_PARTICLE_SYSTEM_H
