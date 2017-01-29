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

#pragma once

// LOVE
#include "common/math.h"
#include "Drawable.h"
#include "Texture.h"
#include "vertex.h"
#include "video/VideoStream.h"
#include "audio/Source.h"

namespace love
{
namespace graphics
{

class Video : public Drawable
{
public:

	static love::Type type;

	Video(love::video::VideoStream *stream, float pixeldensity = 1.0f);
	virtual ~Video();

	// Drawable
	void draw(Graphics *gfx, const Matrix4 &m) override;

	love::video::VideoStream *getStream();

	love::audio::Source *getSource();
	void setSource(love::audio::Source *source);

	int getWidth() const;
	int getHeight() const;

	int getPixelWidth() const;
	int getPixelHeight() const;

	virtual void setFilter(const Texture::Filter &f) = 0;
	const Texture::Filter &getFilter() const;

protected:

	virtual void uploadFrame(const love::video::VideoStream::Frame *frame) = 0;

	StrongRef<love::video::VideoStream> stream;

	int width;
	int height;

	Texture::Filter filter;

	Vertex vertices[4];

	ptrdiff_t textureHandles[3];

private:

	void update();
	StrongRef<love::audio::Source> source;
	
}; // Video

} // graphics
} // love
