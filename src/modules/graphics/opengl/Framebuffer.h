#ifndef LOVE_GRAPHICS_FRAMEBUFFER_H
#define LOVE_GRAPHICS_FRAMEBUFFER_H

#include <graphics/Drawable.h>
#include <image/Image.h>
#include <image/ImageData.h>
#include <common/math.h>
#include <map>
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{

	class Framebuffer : public Drawable
	{
	public:
		Framebuffer(int width, int height);
		virtual ~Framebuffer();

		unsigned int getStatus() const { return status; }

		static Framebuffer* current;
		static void bindDefaultBuffer();

		void startGrab();
		void stopGrab();

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;
		love::image::ImageData * getImageData(love::image::Image * image);

	private:
		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depthbuffer;
		GLuint img;

		vertex vertices[4];

		GLenum status;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FRAMEBUFFER_H
