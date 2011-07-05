#ifndef LOVE_GRAPHICS_FRAMEBUFFER_H
#define LOVE_GRAPHICS_FRAMEBUFFER_H

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
	class Framebuffer : public Drawable, public Volatile
	{
	public:
		Framebuffer(int width, int height);
		virtual ~Framebuffer();

		unsigned int getStatus() const { return status; }

		static Framebuffer* current;
		static void bindDefaultBuffer();

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
		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depthbuffer;
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

#endif // LOVE_GRAPHICS_FRAMEBUFFER_H
