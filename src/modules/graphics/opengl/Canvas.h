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

#ifndef LOVE_GRAPHICS_OPENGL_CANVAS_H
#define LOVE_GRAPHICS_OPENGL_CANVAS_H

#include <graphics/DrawQable.h>
#include <graphics/Volatile.h>
#include <graphics/Image.h>
#include <graphics/Color.h>
#include <image/Image.h>
#include <image/ImageData.h>
#include <common/math.h>
#include <common/Matrix.h>
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	class Canvas : public DrawQable, public Volatile
	{
	public:
		Canvas(int width, int height);
		virtual ~Canvas();

		static bool isSupported();

		unsigned int getStatus() const { return status; }

		static Canvas* current;
		static void bindDefaultCanvas();

		void startGrab();
		void stopGrab();

		void clear(const Color& c);

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

		/**
		* @copydoc DrawQable::drawq()
		**/
		void drawq(love::graphics::Quad * quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

		love::image::ImageData * getImageData(love::image::Image * image);

		void setFilter(const Image::Filter &f);
		Image::Filter getFilter() const;

		void setWrap(const Image::Wrap &w);
		Image::Wrap getWrap() const;

		int getWidth();
		int getHeight();

		bool loadVolatile();
		void unloadVolatile();

	private:
		friend class PixelEffect;
		GLuint getTextureName() const { return img; }

		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depth_stencil;
		GLuint img;

		vertex vertices[4];

		GLenum status;

		struct {
			Image::Filter filter;
			Image::Wrap wrap;
		} settings;

		void drawv(const Matrix & t, const vertex * v) const;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
