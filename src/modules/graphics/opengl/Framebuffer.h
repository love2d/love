#ifndef LOVE_GRAPHICS_FRAMEBUFFER_H
#define LOVE_GRAPHICS_FRAMEBUFFER_H

#include <graphics/Drawable.h>
#include <graphics/Volatile.h>
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

		unsigned int getStatus() const { return status_; }

		bool grab();
		bool stop();

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;

	private:
		static bool isGrabbing;

		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depthbuffer;
		GLuint img;

		vertex vertices[4];

		GLenum status_;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FRAMEBUFFER_H
