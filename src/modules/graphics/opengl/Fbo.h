#ifndef LOVE_GRAPHICS_FBO_H
#define LOVE_GRAPHICS_FBO_H

#include <graphics/Drawable.h>
#include <graphics/Volatile.h>
#include <common/math.h>
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{

	class Fbo : public Drawable // Fbo vs. FBO?
	{
	public:
		Fbo(int width, int height);
		virtual ~Fbo();

		GLenum status() const; //SERIOUS DISLIKE HERE

		void bind(); //DOUBTFUL ABOUT NAME
		void unbind(); //Maybe start/stop?
		//And what about clearing, clear() isn't entirely what I want either
		//maybe make bind/start autoclear?

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;

	private:
		GLsizei width;
		GLsizei height;
		GLuint fbo;
		GLuint depthbuffer;
		GLuint img;
		GLenum status_;

		vertex vertices[4];
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FBO_H
