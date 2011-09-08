#ifndef LOVE_GRAPHICS_CANVAS_H
#define LOVE_GRAPHICS_CANVAS_H

#include <graphics/Drawable.h>
#include <graphics/Volatile.h>
#include <graphics/Image.h>
#include <graphics/Color.h>
#include <image/Image.h>
#include <image/ImageData.h>
#include <common/math.h>
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	class Canvas : public Drawable, public Volatile
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
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_CANVAS_H
