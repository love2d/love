#include "Fbo.h"
#include <common/Matrix.h>

namespace love
{
namespace graphics
{
namespace opengl
{

	Fbo::Fbo(int width, int height) :
		width(width), height(height)
	{
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

	Fbo::~Fbo()
	{
		glDeleteTextures(1, &fbo);
		glDeleteRenderbuffers(1, &depthbuffer);
		glDeleteFramebuffers(1, &img);
	}

	GLenum Fbo::status() const
	{
		return status_;
	}

	void Fbo::bind()
	{
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);
	}

	void Fbo::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
	}

	void Fbo::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
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
