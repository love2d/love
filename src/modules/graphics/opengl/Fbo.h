#ifndef LOVE_GRAPHICS_FBO_H
#define LOVE_GRAPHICS_FBO_H

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

	class Fbo : public Drawable // Fbo vs. FBO?
	{
	public:
		Fbo(int width, int height);
		virtual ~Fbo();

		// for internal use (w_newFbo <-> love.graphics.newFbo) only
		GLenum statusCode() const { return status_; } //SERIOUS DISLIKE HERE
		const char* statusMessage() const;

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

		static std::map<GLenum, const char*> status_to_string;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FBO_H
