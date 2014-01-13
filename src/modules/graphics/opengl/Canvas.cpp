/**
 * Copyright (c) 2006-2014 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Canvas.h"
#include "Image.h"
#include "Graphics.h"
#include "common/Matrix.h"
#include "common/config.h"

#include <cstring> // For memcpy
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

// strategy for fbo creation, interchangable at runtime:
// none, opengl >= 3.0, extensions
struct FramebufferStrategy
{
	virtual ~FramebufferStrategy() {}

	/// create a new framebuffer and texture
	/**
	 * @param[out] framebuffer Framebuffer name
	 * @param[in]  texture     Texture name
	 * @return Creation status
	 */
	virtual GLenum createFBO(GLuint &, GLuint)
	{
		return GL_FRAMEBUFFER_UNSUPPORTED;
	}

	/// Create a stencil buffer and attach it to the active framebuffer object
	/**
	 * @param[in]  width   Width of the stencil buffer
	 * @param[in]  height  Height of the stencil buffer
	 * @param[out] stencil Name for stencil buffer
	 * @return Whether the stencil buffer was successfully created
	 **/
	virtual bool createStencil(int, int, GLuint &)
	{
		return false;
	}

	/// remove objects
	/**
	 * @param[in] framebuffer   Framebuffer name
	 * @param[in] depth_stencil Name for packed depth and stencil buffer
	 */
	virtual void deleteFBO(GLuint, GLuint) {}
	virtual void bindFBO(GLuint) {}

	/// attach additional canvases to the active framebuffer for rendering
	/**
	 * @param[in] canvases List of canvases to attach
	 **/
	virtual void setAttachments(const std::vector<Canvas *> &) {}

	/// stop using all additional attached canvases
	virtual void setAttachments() {}
};

struct FramebufferStrategyGL3 : public FramebufferStrategy
{
	virtual GLenum createFBO(GLuint &framebuffer, GLuint texture)
	{
		// get currently bound fbo to reset to it later
		GLint current_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);

		// create framebuffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, texture, 0);

		// check status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint) current_fbo);
		return status;
	}

	virtual bool createStencil(int width, int height, GLuint &stencil)
	{
		// create combined depth/stencil buffer
		glDeleteRenderbuffers(1, &stencil);
		glGenRenderbuffers(1, &stencil);
		glBindRenderbuffer(GL_RENDERBUFFER, stencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
								  GL_RENDERBUFFER, stencil);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// check status
		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	virtual void deleteFBO(GLuint framebuffer, GLuint depth_stencil)
	{
		if (depth_stencil != 0)
			glDeleteRenderbuffers(1, &depth_stencil);
		if (framebuffer != 0)
			glDeleteFramebuffers(1, &framebuffer);
	}

	virtual void bindFBO(GLuint framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	}

	virtual void setAttachments()
	{
		// set a single render target
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

	virtual void setAttachments(const std::vector<Canvas *> &canvases)
	{
		if (canvases.size() == 0)
		{
			setAttachments();
			return;
		}

		std::vector<GLenum> drawbuffers;
		drawbuffers.push_back(GL_COLOR_ATTACHMENT0);

		// Attach the canvas textures to the currently bound framebuffer.
		for (size_t i = 0; i < canvases.size(); i++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + i,
				GL_TEXTURE_2D, canvases[i]->getGLTexture(), 0);
			drawbuffers.push_back(GL_COLOR_ATTACHMENT1 + i);
		}

		// set up multiple render targets
		if (GLEE_VERSION_2_0)
			glDrawBuffers(drawbuffers.size(), &drawbuffers[0]);
		else if (GLEE_ARB_draw_buffers)
			glDrawBuffersARB(drawbuffers.size(), &drawbuffers[0]);
	}
};

struct FramebufferStrategyPackedEXT : public FramebufferStrategy
{
	virtual GLenum createFBO(GLuint &framebuffer, GLuint texture)
	{
		GLint current_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &current_fbo);

		// create framebuffer
		glGenFramebuffersEXT(1, &framebuffer);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, texture, 0);

		// check status
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

		// unbind framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint) current_fbo);
		return status;
	}

	virtual bool createStencil(int width, int height, GLuint &stencil)
	{
		// create combined depth/stencil buffer
		glDeleteRenderbuffers(1, &stencil);
		glGenRenderbuffersEXT(1, &stencil);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencil);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT,
								 width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
									 GL_RENDERBUFFER_EXT, stencil);

		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		// check status
		return glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
	}

	virtual void deleteFBO(GLuint framebuffer, GLuint depth_stencil)
	{
		if (depth_stencil != 0)
			glDeleteRenderbuffersEXT(1, &depth_stencil);
		if (framebuffer != 0)
			glDeleteFramebuffersEXT(1, &framebuffer);
	}

	virtual void bindFBO(GLuint framebuffer)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
	}

	virtual void setAttachments()
	{
		// set a single render target
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	}

	virtual void setAttachments(const std::vector<Canvas *> &canvases)
	{
		if (canvases.size() == 0)
		{
			setAttachments();
			return;
		}

		std::vector<GLenum> drawbuffers;
		drawbuffers.push_back(GL_COLOR_ATTACHMENT0_EXT);

		// Attach the canvas textures to the currently bound framebuffer.
		for (size_t i = 0; i < canvases.size(); i++)
		{
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT + i,
								   GL_TEXTURE_2D, canvases[i]->getGLTexture(), 0);
			drawbuffers.push_back(GL_COLOR_ATTACHMENT1_EXT + i);
		}

		// set up multiple render targets
		if (GLEE_VERSION_2_0)
			glDrawBuffers(drawbuffers.size(), &drawbuffers[0]);
		else if (GLEE_ARB_draw_buffers)
			glDrawBuffersARB(drawbuffers.size(), &drawbuffers[0]);
	}
};

struct FramebufferStrategyEXT : public FramebufferStrategyPackedEXT
{
	virtual bool createStencil(int width, int height, GLuint &stencil)
	{
		// create stencil buffer
		glDeleteRenderbuffers(1, &stencil);
		glGenRenderbuffersEXT(1, &stencil);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencil);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX,
								 width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
									 GL_RENDERBUFFER_EXT, stencil);

		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		// check status
		return glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
	}

	bool isSupported()
	{
		GLuint fb = 0, stencil = 0;
		GLenum status = createFBO(fb, 0);
		deleteFBO(fb, stencil);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}
};

FramebufferStrategy *strategy = NULL;

FramebufferStrategy strategyNone;

FramebufferStrategyGL3 strategyGL3;

FramebufferStrategyPackedEXT strategyPackedEXT;

FramebufferStrategyEXT strategyEXT;

Canvas *Canvas::current = NULL;
OpenGL::Viewport Canvas::systemViewport = OpenGL::Viewport();

static void getStrategy()
{
	if (!strategy)
	{
		if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object)
			strategy = &strategyGL3;
		else if (GLEE_EXT_framebuffer_object && GLEE_EXT_packed_depth_stencil)
			strategy = &strategyPackedEXT;
		else if (GLEE_EXT_framebuffer_object && strategyEXT.isSupported())
			strategy = &strategyEXT;
		else
			strategy = &strategyNone;
	}
}

static int maxFBOColorAttachments = 0;
static int maxDrawBuffers = 0;

Canvas::Canvas(int width, int height, TextureType texture_type)
	: fbo(0)
	, texture(0)
	, depth_stencil(0)
	, texture_type(texture_type)
{
	this->width = width;
	this->height = height;

	float w = static_cast<float>(width);
	float h = static_cast<float>(height);

	// world coordinates
	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = h;
	vertices[2].x = w;
	vertices[2].y = h;
	vertices[3].x = w;
	vertices[3].y = 0;

	// texture coordinates
	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 1;
	vertices[3].s = 1;
	vertices[3].t = 0;

	getStrategy();

	loadVolatile();
}

Canvas::~Canvas()
{
	// reset framebuffer if still using this one
	if (current == this)
		stopGrab();

	unloadVolatile();
}

bool Canvas::loadVolatile()
{
	fbo = depth_stencil = texture = 0;

	// glTexImage2D is guaranteed to error in this case.
	if (width > gl.getMaxTextureSize() || height > gl.getMaxTextureSize())
	{
		status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
		return false;
	}

	glGenTextures(1, &texture);
	gl.bindTexture(texture);

	setFilter(filter);
	setWrap(wrap);

	GLint internalformat;
	GLenum textype;
	switch (texture_type)
	{
	case TYPE_HDR:
		internalformat = GL_RGBA16F;
		textype = GL_FLOAT;
		break;
	case TYPE_NORMAL:
	default:
		internalformat = GL_RGBA8;
		textype = GL_UNSIGNED_BYTE;
	}

	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             internalformat,
	             width, height,
	             0,
	             GL_RGBA,
	             textype,
	             nullptr);

	if (glGetError() != GL_NO_ERROR)
	{
		status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
		return false;
	}

	status = strategy->createFBO(fbo, texture);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		return false;

	clear(Color(0, 0, 0, 0));
	return true;
}

void Canvas::unloadVolatile()
{
	strategy->deleteFBO(fbo, depth_stencil);

	gl.deleteTexture(texture);

	fbo = depth_stencil = texture = 0;

	for (size_t i = 0; i < attachedCanvases.size(); i++)
		attachedCanvases[i]->release();

	attachedCanvases.clear();
}

void Canvas::drawv(const Matrix &t, const Vertex *v) const
{
	glPushMatrix();

	glMultMatrixf((const GLfloat *)t.getElements());

	gl.bindTexture(texture);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid *)&v[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid *)&v[0].s);

	gl.prepareDraw();
	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glPopMatrix();
}

void Canvas::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, vertices);
}

void Canvas::drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	const Vertex *v = quad->getVertices();
	drawv(t, v);
}

void Canvas::setFilter(const Texture::Filter &f)
{
	filter = f;
	gl.bindTexture(texture);
	gl.setTextureFilter(filter);
}

void Canvas::setWrap(const Texture::Wrap &w)
{
	wrap = w;
	gl.bindTexture(texture);
	gl.setTextureWrap(wrap);
}

GLuint Canvas::getGLTexture() const
{
	return texture;
}

void Canvas::predraw() const
{
	gl.bindTexture(texture);
}

void Canvas::setupGrab()
{
	// already grabbing
	if (current == this)
		return;

	// cleanup after previous fbo
	if (current != NULL)
		current->stopGrab();

	// bind the framebuffer object.
	systemViewport = gl.getViewport();
	strategy->bindFBO(fbo);
	gl.setViewport(OpenGL::Viewport(0, 0, width, height));

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// Set up orthographic view (no depth)
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

	// Switch back to modelview matrix
	glMatrixMode(GL_MODELVIEW);

	// indicate we are using this fbo
	current = this;
}

void Canvas::startGrab(const std::vector<Canvas *> &canvases)
{
	// Whether the new canvas list is different from the old one.
	// A more thorough check is done below.
	bool canvaseschanged = canvases.size() != attachedCanvases.size();

	if (canvases.size() > 0)
	{
		if (!isMultiCanvasSupported())
			throw love::Exception("Multi-canvas rendering is not supported on this system.");

		if (canvases.size()+1 > size_t(maxDrawBuffers) || canvases.size()+1 > size_t(maxFBOColorAttachments))
			throw love::Exception("This system can't simultaniously render to %d canvases.", canvases.size()+1);
	}

	for (size_t i = 0; i < canvases.size(); i++)
	{
		if (canvases[i]->getWidth() != width || canvases[i]->getHeight() != height)
			throw love::Exception("All canvas arguments must have the same dimensions.");

		if (canvases[i]->getTextureType() != texture_type)
			throw love::Exception("All canvas arguments must have the same texture type.");

		if (!canvaseschanged && canvases[i] != attachedCanvases[i])
			canvaseschanged = true;
	}

	setupGrab();

	// Don't attach anything if there's nothing to change.
	if (!canvaseschanged)
		return;

	// Attach the canvas textures to the active FBO and set up MRTs.
	strategy->setAttachments(canvases);

	for (size_t i = 0; i < canvases.size(); i++)
		canvases[i]->retain();

	for (size_t i = 0; i < attachedCanvases.size(); i++)
		attachedCanvases[i]->release();

	attachedCanvases = canvases;
}

void Canvas::startGrab()
{
	setupGrab();

	if (attachedCanvases.size() == 0)
		return;

	// make sure the FBO is only using a single canvas
	strategy->setAttachments();

	// release any previously attached canvases
	for (size_t i = 0; i < attachedCanvases.size(); i++)
		attachedCanvases[i]->release();

	attachedCanvases.clear();
}

void Canvas::stopGrab()
{
	// i am not grabbing. leave me alone
	if (current != this)
		return;

	// bind default
	strategy->bindFBO(0);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	current = nullptr;

	gl.setViewport(systemViewport);
}

void Canvas::clear(Color c)
{
	if (strategy == &strategyNone)
		return;

	GLuint previous = 0;

	if (current != this)
	{
		if (current != nullptr)
			previous = current->fbo;

		strategy->bindFBO(fbo);
	}

	GLfloat glcolor[] = {c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f};

	// We don't need to worry about multiple FBO attachments or global clear
	// color state when OpenGL 3.0+ is supported.
	if (GLEE_VERSION_3_0)
	{
		glClearBufferfv(GL_COLOR, 0, glcolor);

		if (depth_stencil != 0)
		{
			GLint stencilvalue = 0;
			glClearBufferiv(GL_STENCIL, 0, &stencilvalue);
		}
	}
	else
	{
		// glClear will clear all active draw buffers, so we need to temporarily
		// detach any other canvases (when MRT is being used.)
		if (attachedCanvases.size() > 0)
			strategy->setAttachments();

		// Don't use the state-shadowed gl.setClearColor because we want to save
		// the previous clear color.
		glClearColor(glcolor[0], glcolor[1], glcolor[2], glcolor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if (attachedCanvases.size() > 0)
			strategy->setAttachments(attachedCanvases);

		// Restore the global clear color.
		gl.setClearColor(gl.getClearColor());
	}

	if (current != this)
		strategy->bindFBO(previous);
}

bool Canvas::checkCreateStencil()
{
	// Do nothing if we've already created the stencil buffer.
	if (depth_stencil != 0)
		return true;

	if (current != this)
		strategy->bindFBO(fbo);

	bool success = strategy->createStencil(width, height, depth_stencil);

	if (current && current != this)
		strategy->bindFBO(current->fbo);
	else if (!current)
		strategy->bindFBO(0);

	return success;
}

love::image::ImageData *Canvas::getImageData(love::image::Image *image)
{
	int row = 4 * width;
	int size = row * height;
	GLubyte *pixels  = new GLubyte[size];

	strategy->bindFBO(fbo);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	if (current)
		strategy->bindFBO(current->fbo);
	else
		strategy->bindFBO(0);

	// The new ImageData now owns the pixel data, so we don't delete it here.
	love::image::ImageData *img = image->newImageData(width, height, (void *)pixels, true);
	return img;
}

void Canvas::getPixel(unsigned char* pixel_rgba, int x, int y)
{
	if (current != this)
		strategy->bindFBO(fbo);

	glReadPixels(x, height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel_rgba);

	if (current && current != this)
		strategy->bindFBO(current->fbo);
	else if (!current)
		strategy->bindFBO(0);
}

bool Canvas::isSupported()
{
	getStrategy();
	return (strategy != &strategyNone);
}

bool Canvas::isHDRSupported()
{
	return GLEE_VERSION_3_0 || (isSupported() && GLEE_ARB_texture_float);
}

bool Canvas::isMultiCanvasSupported()
{
	if (!(isSupported() && (GLEE_VERSION_2_0 || GLEE_ARB_draw_buffers)))
		return false;

	if (maxFBOColorAttachments == 0 || maxDrawBuffers == 0)
	{
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxFBOColorAttachments);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	}

	// system must support at least 4 simultanious active canvases
	return maxFBOColorAttachments >= 4 && maxDrawBuffers >= 4;
}

void Canvas::bindDefaultCanvas()
{
	if (current != nullptr)
		current->stopGrab();
}

bool Canvas::getConstant(const char *in, Canvas::TextureType &out)
{
	return textureTypes.find(in, out);
}

bool Canvas::getConstant(Canvas::TextureType in, const char *&out)
{
	return textureTypes.find(in, out);
}

StringMap<Canvas::TextureType, Canvas::TYPE_MAX_ENUM>::Entry Canvas::textureTypeEntries[] =
{
	{"normal", Canvas::TYPE_NORMAL},
	{"hdr",    Canvas::TYPE_HDR},
};
StringMap<Canvas::TextureType, Canvas::TYPE_MAX_ENUM> Canvas::textureTypes(Canvas::textureTypeEntries, sizeof(Canvas::textureTypeEntries));

} // opengl
} // graphics
} // love
