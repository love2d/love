#include "Framebuffer.h"
#include <common/Matrix.h>

#include <cstring> // For memcpy

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
	
	FramebufferStrategy* strategy;
	
	FramebufferStrategy strategyNone;
	
	FramebufferStrategyGL3 strategyGL3;
	
	FramebufferStrategyEXT strategyEXT;
	
	Framebuffer* Framebuffer::current = NULL;

	Framebuffer::Framebuffer(int width, int height) :
		width(width), height(height)
	{
		strategy = NULL;

		// world coordinates
		vertices[0].x = 0;     vertices[0].y = 0;
		vertices[1].x = 0;     vertices[1].y = height;
		vertices[2].x = width; vertices[2].y = height;
		vertices[3].x = width; vertices[3].y = 0;

		// texture coordinates
		vertices[0].s = 0;     vertices[0].t = 1;
		vertices[1].s = 0;     vertices[1].t = 0;
		vertices[2].s = 1;     vertices[2].t = 0;
		vertices[3].s = 1;     vertices[3].t = 1;

		if (!strategy) {
			if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object)
				strategy = &strategyGL3;
			else if (GLEE_EXT_framebuffer_object)
				strategy = &strategyEXT;
			else
				strategy = &strategyNone;
		}

		loadVolatile();
	}

	Framebuffer::~Framebuffer()
	{
		// reset framebuffer if still using this one
		if (current == this)
			stopGrab();

		unloadVolatile();
	}

	void Framebuffer::bindDefaultBuffer()
	{
		if (current != NULL)
			current->stopGrab();
	}

	void Framebuffer::startGrab()
	{
		// already grabbing
		if (current == this)
			return;

		// cleanup after previous fbo
		if (current != NULL)
			glPopAttrib();

		// bind buffer and clear screen
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		strategy->bindFBO(fbo);
		glClearColor(.0f, .0f, .0f, .0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);

		// indicate we are using this fbo
		current = this;
	}

	void Framebuffer::stopGrab()
	{
		// i am not grabbing. leave me alone
		if (current != this)
			return;

		// bind default
		strategy->bindFBO( 0 );
		glPopAttrib();
		current = NULL;
	}

	void Framebuffer::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		static Matrix t;
		t.setTransformation(x, y, angle, sx, sy, ox, oy);

		glPushMatrix();
		glMultMatrixf((const GLfloat*)t.getElements());

		glBindTexture(GL_TEXTURE_2D, img);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].s);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
	}

	love::image::ImageData * Framebuffer::getImageData(love::image::Image * image)
	{
		int row = 4 * width;
		int size = row * height;

		// see Graphics::newScreenshot. OpenGL reads from lower-left,
		// but we need the pixels from upper-left.
		GLubyte* pixels = new GLubyte[size];
		GLubyte* screenshot = new GLubyte[size];

		strategy->bindFBO( fbo );
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		if (current)
			strategy->bindFBO( current->fbo );
		else
			strategy->bindFBO( 0 );

		GLubyte* src = pixels - row; // second line of buffer
		GLubyte* dst = screenshot + size; // end of buffer

		for (int i = 0; i < height; ++i)
			memcpy(dst -= row, src += row, row);

		love::image::ImageData * img = image->newImageData(width, height, (void*)screenshot);

		delete[] screenshot;
		delete[] pixels;

		return img;
	}
	
	bool Framebuffer::loadVolatile()
	{
		status = strategy->createFBO(fbo, depthbuffer, img, width, height);
		return (status == GL_FRAMEBUFFER_COMPLETE);
	}
	
	void Framebuffer::unloadVolatile()
	{
		strategy->deleteFBO(fbo, depthbuffer, img);
	}

} // opengl
} // graphics
} // love
