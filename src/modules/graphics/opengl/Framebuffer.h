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

		// for internal use (w_newFramebuffer <-> love.graphics.newFramebuffer) only
		GLenum statusCode() const { return status_; } //SERIOUS DISLIKE HERE
		const char* statusMessage() const;

		void grab();
		void stop();

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;

	private:
		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depthbuffer;
		GLuint img;
		GLenum status_;

		vertex vertices[4];

		static std::map<GLenum, const char*> status_to_string;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FRAMEBUFFER_H
