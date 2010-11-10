#ifndef LOVE_GRAPHICS_FRAMEBUFFER_H
#define LOVE_GRAPHICS_FRAMEBUFFER_H

#include <graphics/Drawable.h>
#include <graphics/Volatile.h>
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
		
	// strategy for fbo creation, interchangable at runtime:
	// none, opengl >= 3.0, extensions
	struct FramebufferStrategy {
		/// create a new framebuffer, depthbuffer and texture
		/**
		 * @param[out] framebuffer Framebuffer name
		 * @param[out] depthbuffer Depthbuffer name
		 * @param[out] img         Texture name
		 * @param[in]  width       Width of framebuffer
		 * @param[in]  height      Height of framebuffer
		 * @return Creation status
		 */
		virtual GLenum createFBO(GLuint&, GLuint&, GLuint&, int, int)
		{ return GL_FRAMEBUFFER_UNSUPPORTED; }
		/// remove objects
		/**
		 * @param[in] framebuffer Framebuffer name
		 * @param[in] depthbuffer Depthbuffer name
		 * @param[in] img         Texture name
		 */
		virtual void deleteFBO(GLuint, GLuint, GLuint) {}
		virtual void bindFBO(GLuint) {}
	};

	struct FramebufferStrategyGL3 : public FramebufferStrategy {
		virtual GLenum createFBO(GLuint& framebuffer, GLuint& depthbuffer, GLuint& img, int width, int height)
		{
			// generate depth buffer
			glGenRenderbuffers(1, &depthbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			// generate texture save target
			glGenTextures(1, &img);
			glBindTexture(GL_TEXTURE_2D, img);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);

			// create framebuffer
			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_2D, img, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					GL_RENDERBUFFER, depthbuffer);
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			// unbind buffers and texture
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return status;
		}
		virtual void deleteFBO(GLuint framebuffer, GLuint depthbuffer, GLuint img)
		{
			glDeleteTextures(1, &img);
			glDeleteRenderbuffers(1, &depthbuffer);
			glDeleteFramebuffers(1, &framebuffer);
		}

		virtual void bindFBO(GLuint framebuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		}
	};

	struct FramebufferStrategyEXT : public FramebufferStrategy {

		virtual GLenum createFBO(GLuint& framebuffer, GLuint& depthbuffer, GLuint& img, int width, int height)
		{
			// generate depth buffer
			glGenRenderbuffersEXT(1, &depthbuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

			// generate texture save target
			glGenTextures(1, &img);
			glBindTexture(GL_TEXTURE_2D, img);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);

			// create framebuffer
			glGenFramebuffersEXT(1, &framebuffer);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
					GL_TEXTURE_2D, img, 0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
					GL_RENDERBUFFER_EXT, depthbuffer);
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

			// unbind buffers and texture
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			return status;
		}

		virtual void deleteFBO(GLuint framebuffer, GLuint depthbuffer, GLuint img)
		{
			glDeleteTextures(1, &img);
			glDeleteRenderbuffersEXT(1, &depthbuffer);
			glDeleteFramebuffersEXT(1, &framebuffer);
		}

		virtual void bindFBO(GLuint framebuffer)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
		}
	};

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

		virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;
		love::image::ImageData * getImageData(love::image::Image * image);
		
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
		
		FramebufferStrategy* strategy;
		
		FramebufferStrategy strategyNone;
		
		FramebufferStrategyGL3 strategyGL3;
		
		FramebufferStrategyEXT strategyEXT;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_FRAMEBUFFER_H
