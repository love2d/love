/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

// C
#include <cstring>

// C++
#include <unordered_map>

// LOVE
#include "common/math.h"
#include "common/Matrix.h"
#include "common/Color.h"
#include "Drawable.h"
#include "Mesh.h"
#include "vertex.h"

namespace love
{
namespace graphics
{

// Forward declarations.
class Graphics;
class Texture;
class Quad;
class Buffer;

class SpriteBatch : public Drawable
{
public:

	static love::Type type;

	SpriteBatch(Graphics *gfx, Texture *texture, int size, vertex::Usage usage);
	virtual ~SpriteBatch();

	int add(const Matrix4 &m, int index = -1);
	int add(Quad *quad, const Matrix4 &m, int index = -1);
	int addLayer(int layer, const Matrix4 &m, int index = -1);
	int addLayer(int layer, Quad *quad, const Matrix4 &m, int index = -1);

	void clear();

	void flush();

	void setTexture(Texture *newtexture);
	Texture *getTexture() const;

	/**
	 * Set the current color for this SpriteBatch. The sprites added
	 * after this call will use this color. Note that global color
	 * will not longer apply to the SpriteBatch if this is used.
	 *
	 * @param color The color to use for the following sprites.
	 */
	void setColor(const Colorf &color);

	/**
	 * Disable per-sprite colors for this SpriteBatch. The next call to
	 * draw will use the global color for all sprites.
	 */
	void setColor();

	/**
	 * Get the current color for this SpriteBatch.
	 **/
	Colorf getColor(bool &active) const;

	/**
	 * Get the number of sprites currently in this SpriteBatch.
	 **/
	int getCount() const;

	/**
	 * Get the total number of sprites this SpriteBatch can currently hold.
	 **/
	int getBufferSize() const;

	/**
	 * Attaches a specific vertex attribute from a Mesh to this SpriteBatch.
	 * The vertex attribute will be used when drawing the SpriteBatch.
	 **/
	void attachAttribute(const std::string &name, Mesh *mesh);

	void setDrawRange(int start, int count);
	void setDrawRange();
	bool getDrawRange(int &start, int &count) const;

	// Implements Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

private:

	struct AttachedAttribute
	{
		StrongRef<Mesh> mesh;
		int index;
	};

	/**
	 * Sets the total number of sprites this SpriteBatch can hold.
	 * Leaves existing sprite data intact when possible.
	 **/
	void setBufferSize(int newsize);

	StrongRef<Texture> texture;

	// Max number of sprites in the batch.
	int size;

	// The next free element.
	int next;

	// Current color. This color, if present, will be applied to the next
	// added sprite.
	Color32 color;
	bool color_active;

	vertex::CommonFormat vertex_format;
	size_t vertex_stride;
	
	love::graphics::Buffer *array_buf;

	std::unordered_map<std::string, AttachedAttribute> attached_attributes;
	
	int range_start;
	int range_count;
	
}; // SpriteBatch

} // graphics
} // love
