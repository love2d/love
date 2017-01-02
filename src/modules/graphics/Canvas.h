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

#include "image/Image.h"
#include "image/ImageData.h"
#include "Texture.h"

namespace love
{
namespace graphics
{

class Graphics;

class Canvas : public Texture
{
public:

	static love::Type type;

	struct Settings
	{
		PixelFormat format = PIXELFORMAT_NORMAL;
		float pixeldensity = 1.0f;
		int msaa = 0;
	};

	Canvas();
	virtual ~Canvas();

	virtual love::image::ImageData *newImageData(love::image::Image *module, int x, int y, int w, int h) = 0;

	virtual int getMSAA() const = 0;
	virtual int getRequestedMSAA() const = 0;
	virtual ptrdiff_t getMSAAHandle() const = 0;

	static int canvasCount;

private:

	void drawv(Graphics *gfx, const Matrix4 &t, const Vertex *v) override;
	
}; // Canvas

} // graphics
} // love
