#include "Framebuffer.h"
#include <common/Matrix.h>

namespace love
{
namespace graphics
{
namespace opengl
{

	std::map<GLenum, const char*> Framebuffer::status_to_string;

	Framebuffer::Framebuffer(int width, int height) :
		width(width), height(height)
	{
		// maybe create status code messages
		if (status_to_string.empty()) {
			status_to_string[GL_FRAMEBUFFER_UNSUPPORTED] // the most important one
				= "your opengl implementation does not support framebuffer objects";

			status_to_string[GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT]
				= "framebuffer has incomplete attachments";
			status_to_string[GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER]
				= "incomplete draw buffer";

			// the ones that should never, ever happen:
			status_to_string[GL_FRAMEBUFFER_UNDEFINED]
				= "default framebuffer does not exist";
			status_to_string[GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT]
				= "framebuffer needs at least one image attached";
			status_to_string[GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER]
				= "incomplete read buffer";
			status_to_string[GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE]
				= "number of samples mismatch in attached buffers";
			// "Additionally, if an error occurs, zero is returned." and
			// "GL_INVALID_ENUM is generated if target is not GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER."
			status_to_string[0] = "Framebuffer hijacked by aliens";
		}

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
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, img, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER, depthbuffer);
		status_ = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// unbind buffers and texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteTextures(1, &fbo);
		glDeleteRenderbuffers(1, &depthbuffer);
		glDeleteFramebuffers(1, &img);
	}

	const char* Framebuffer::statusMessage() const
	{
		status_to_string[statusCode()];
	}

	void Framebuffer::bind()
	{
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(.0f, .0f, .0f, .0f);
		glViewport(0, 0, width, height);
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
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

} // opengl
} // graphics
} // love
