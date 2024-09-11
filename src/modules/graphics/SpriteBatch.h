/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "common/Range.h"
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

	SpriteBatch(Graphics *gfx, Texture *texture, int size, BufferDataUsage usage);
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
	 * Set the current color for this SpriteBatch. The sprites added after this
	 * call will use this color.
	 *
	 * @param color The color to use for the following sprites.
	 */
	void setColor(const Colorf &color);

	/**
	 * Get the current color for this SpriteBatch.
	 **/
	Colorf getColor() const;

	/**
	 * Get the number of sprites currently in this SpriteBatch.
	 **/
	int getCount() const;

	/**
	 * Get the total number of sprites this SpriteBatch can currently hold.
	 **/
	int getBufferSize() const;

	/**
	 * Attaches a specific vertex attribute from a Buffer to this SpriteBatch.
	 * The vertex attribute will be used when drawing the SpriteBatch.
	 * If the attribute comes from a Mesh, it should be given as an argument as
	 * well, to make sure the SpriteBatch flushes its data to its Buffer when
	 * the SpriteBatch is drawn.
	 **/
	void attachAttribute(const std::string &name, Buffer *buffer, Mesh *mesh);

	void setDrawRange(int start, int count);
	void setDrawRange();
	bool getDrawRange(int &start, int &count) const;

	// Implements Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

private:

	void updateVertexAttributes(Graphics *gfx);

	struct AttachedAttribute
	{
		StrongRef<Buffer> buffer;
		StrongRef<Mesh> mesh;
		int index;
		int bindingIndex;
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

	// Current color. This color will be applied to the next added sprite.
	Color32 color;
	Colorf colorf;

	CommonFormat vertex_format;
	size_t vertex_stride;

	VertexAttributesID attributesID;
	BufferBindings bufferBindings;

	StrongRef<love::graphics::Buffer> array_buf;
	uint8 *vertex_data;

	Range modified_sprites;

	std::unordered_map<std::string, AttachedAttribute> attached_attributes;
	
	int range_start;
	int range_count;
	
}; // SpriteBatch

} // graphics
} // love
