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

#pragma once

// LOVE
#include "common/math.h"
#include "graphics/Drawable.h"
#include "graphics/Volatile.h"
#include "video/VideoStream.h"
#include "audio/Source.h"

#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Video : public Drawable, public Volatile
{
public:

	Video(love::video::VideoStream *stream);
	~Video();

	// Volatile
	bool loadVolatile();
	void unloadVolatile();

	love::video::VideoStream *getStream();
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	love::audio::Source *getSource();
	void setSource(love::audio::Source *source);

	int getWidth() const;
	int getHeight() const;

	void setFilter(const Texture::Filter &f);
	const Texture::Filter &getFilter() const;

private:

	void update();

	StrongRef<love::video::VideoStream> stream;
	StrongRef<love::audio::Source> source;

	GLuint textures[3];

	Vertex vertices[4];

	Texture::Filter filter;

}; // Video

} // opengl
} // graphics
} // love
