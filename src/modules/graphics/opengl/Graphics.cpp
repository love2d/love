/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

// LOVE
#include "common/config.h"
#include "common/math.h"
#include "common/Vector.h"

#include "Graphics.h"
#include "font/Font.h"
#include "Font.h"
#include "graphics/Polyline.h"
#include "math/MathModule.h"
#include "window/Window.h"

#include "libraries/xxHash/xxhash.h"

// C++
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

// C
#include <cmath>
#include <cstdio>

#ifdef LOVE_IOS
#include <SDL_syswm.h>
#endif

namespace love
{
namespace graphics
{
namespace opengl
{

Graphics::Graphics()
	: quadIndices(nullptr)
{
	gl = OpenGL();

	states.reserve(10);
	states.push_back(DisplayState());

	auto window = getInstance<love::window::Window>(M_WINDOW);

	if (window != nullptr)
	{
		window->setGraphics(this);

		if (window->isOpen())
		{
			double w = window->getWidth();
			double h = window->getHeight();
			window->windowToDPICoords(&w, &h);

			setMode((int) w, (int) h, window->getPixelWidth(), window->getPixelHeight());
		}
	}
}

Graphics::~Graphics()
{
	if (quadIndices)
		delete quadIndices;
}

const char *Graphics::getName() const
{
	return "love.graphics.opengl";
}

Image *Graphics::newImage(const std::vector<love::image::ImageData *> &data, const Image::Settings &settings)
{
	return new Image(data, settings);
}

Image *Graphics::newImage(const std::vector<love::image::CompressedImageData *> &cdata, const Image::Settings &settings)
{
	return new Image(cdata, settings);
}

graphics::Font *Graphics::newFont(love::font::Rasterizer *r, const Texture::Filter &filter)
{
	return new Font(r, filter);
}

SpriteBatch *Graphics::newSpriteBatch(Texture *texture, int size, vertex::Usage usage)
{
	return new SpriteBatch(texture, size, usage);
}

ParticleSystem *Graphics::newParticleSystem(Texture *texture, int size)
{
	return new ParticleSystem(texture, size);
}

love::graphics::Canvas *Graphics::newCanvas(int width, int height, const Canvas::Settings &settings)
{
	if (!Canvas::isSupported())
		throw love::Exception("Canvases are not supported by your OpenGL drivers!");

	if (!Canvas::isFormatSupported(settings.format))
	{
		const char *fstr = "rgba8";
		love::getConstant(Canvas::getSizedFormat(settings.format), fstr);
		throw love::Exception("The %s canvas format is not supported by your OpenGL drivers.", fstr);
	}

	if (width > gl.getMaxTextureSize())
		throw Exception("Cannot create canvas: width of %d pixels is too large for this system.", width);
	else if (height > gl.getMaxTextureSize())
		throw Exception("Cannot create canvas: height of %d pixels is too large for this system.", height);

	Canvas *canvas = new Canvas(width, height, settings);
	GLenum err = canvas->getStatus();

	// everything ok, return canvas (early out)
	if (err == GL_FRAMEBUFFER_COMPLETE)
		return canvas;

	canvas->release();
	throw love::Exception("Cannot create Canvas: %s", OpenGL::framebufferStatusString(err));
	return nullptr; // never reached
}

love::graphics::Shader *Graphics::newShader(const Shader::ShaderSource &source)
{
	return new Shader(source);
}

Mesh *Graphics::newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(vertices, drawmode, usage);
}

Mesh *Graphics::newMesh(int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(vertexcount, drawmode, usage);
}

Mesh *Graphics::newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(vertexformat, vertexcount, drawmode, usage);
}

Mesh *Graphics::newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, const void *data, size_t datasize, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(vertexformat, data, datasize, drawmode, usage);
}

Text *Graphics::newText(graphics::Font *font, const std::vector<Font::ColoredString> &text)
{
	return new Text(font, text);
}

Video *Graphics::newVideo(love::video::VideoStream *stream, float pixeldensity)
{
	return new Video(stream, pixeldensity);
}

void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight)
{
	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	if (states.back().canvases.empty())
	{
		// Set the viewport to top-left corner.
		gl.setViewport({0, 0, pixelwidth, pixelheight});

		// Re-apply the scissor if it was active, since the rectangle passed to
		// glScissor is affected by the viewport dimensions.
		if (states.back().scissor)
			setScissor(states.back().scissorRect);

		// Set up the projection matrix
		projectionMatrix = Matrix4::ortho(0.0, (float) width, (float) height, 0.0);
	}
}

bool Graphics::setMode(int width, int height, int pixelwidth, int pixelheight)
{
	this->width = width;
	this->height = height;

	// Okay, setup OpenGL.
	gl.initContext();
	gl.setupContext();

	created = true;

	setViewportSize(width, height, pixelwidth, pixelheight);

	// Enable blending
	glEnable(GL_BLEND);

	// Auto-generated mipmaps should be the best quality possible
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	if (!GLAD_ES_VERSION_2_0)
	{
		// Make sure antialiasing works when set elsewhere
		glEnable(GL_MULTISAMPLE);

		// Enable texturing
		glEnable(GL_TEXTURE_2D);
	}

	gl.setTextureUnit(0);

	// Set pixel row alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set whether drawing converts input from linear -> sRGB colorspace.
	if (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB
		|| GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB)
	{
		if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
			gl.setFramebufferSRGB(isGammaCorrect());
	}
	else
		setGammaCorrect(false);

	bool enabledebug = false;

	if (GLAD_VERSION_3_0)
	{
		// Enable OpenGL's debug output if a debug context has been created.
		GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		enabledebug = (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;
	}

	setDebug(enabledebug);

	if (streamBufferState.vb[0] == nullptr)
	{
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		streamBufferState.vb[0] = new StreamBuffer(StreamBuffer::MODE_VERTEX, 1024 * 1024 * 1);
		streamBufferState.vb[1] = new StreamBuffer(StreamBuffer::MODE_VERTEX, 256  * 1024 * 1);
		streamBufferState.indexBuffer = new StreamBuffer(StreamBuffer::MODE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
	}

	// Reload all volatile objects.
	if (!Volatile::loadAll())
		::printf("Could not reload all volatile objects.\n");

	// Create a quad indices object owned by love.graphics, so at least one
	// QuadIndices object is alive at all times while love.graphics is alive.
	// This makes sure there aren't too many expensive destruction/creations of
	// index buffer objects, since the shared index buffer used by QuadIndices
	// objects is destroyed when the last object is destroyed.
	if (quadIndices == nullptr)
		quadIndices = new QuadIndices(20);

	// Restore the graphics state.
	restoreState(states.back());

	pixelScaleStack.clear();
	pixelScaleStack.reserve(5);
	pixelScaleStack.push_back(1);

	int gammacorrect = isGammaCorrect() ? 1 : 0;

	// We always need a default shader.
	if (!Shader::defaultShader)
	{
		Renderer renderer = GLAD_ES_VERSION_2_0 ? RENDERER_OPENGLES : RENDERER_OPENGL;
		Shader::defaultShader = newShader(defaultShaderCode[renderer][gammacorrect]);
	}

	// and a default video shader.
	if (!Shader::defaultVideoShader)
	{
		Renderer renderer = GLAD_ES_VERSION_2_0 ? RENDERER_OPENGLES : RENDERER_OPENGL;
		Shader::defaultVideoShader = newShader(defaultVideoShaderCode[renderer][gammacorrect]);
	}

	// A shader should always be active, but the default shader shouldn't be
	// returned by getShader(), so we don't do setShader(defaultShader).
	if (!Shader::current)
		Shader::defaultShader->attach();

	return true;
}

void Graphics::unSetMode()
{
	if (!isCreated())
		return;

	flushStreamDraws();

	// Unload all volatile objects. These must be reloaded after the display
	// mode change.
	Volatile::unloadAll();

	for (const auto &pair : framebufferObjects)
		gl.deleteFramebuffer(pair.second);

	for (const CachedRenderbuffer &rb : stencilBuffers)
		glDeleteRenderbuffers(1, &rb.renderbuffer);

	framebufferObjects.clear();
	stencilBuffers.clear();

	gl.deInitContext();

	created = false;
}

void Graphics::setActive(bool enable)
{
	flushStreamDraws();

	// Make sure all pending OpenGL commands have fully executed before
	// returning, when going from active to inactive. This is required on iOS.
	if (isCreated() && this->active && !enable)
		glFinish();

	active = enable;
}

void Graphics::flushStreamDraws()
{
	using namespace vertex;

	const auto &sbstate = streamBufferState;

	if (sbstate.vertexCount == 0 && sbstate.indexCount == 0)
		return;

	OpenGL::TempDebugGroup debuggroup("Stream vertices flush and draw");

	uint32 attribs = 0;

	for (int i = 0; i < 2; i++)
	{
		if (sbstate.formats[i] == CommonFormat::NONE)
			continue;

		StreamBuffer *buffer = sbstate.vb[i];

		buffer->resetOffset();
		ptrdiff_t offset = (ptrdiff_t) buffer->getData();
		GLsizei stride = (GLsizei) getFormatStride(sbstate.formats[i]);

		gl.bindBuffer(BUFFER_VERTEX, 0);

		switch (sbstate.formats[i])
		{
		case CommonFormat::NONE:
			break;
		case CommonFormat::XYf:
			attribs |= ATTRIBFLAG_POS;
			glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset));
			break;
		case CommonFormat::RGBAub:
			attribs |= ATTRIBFLAG_COLOR;
			glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, BUFFER_OFFSET(offset));
			break;
		case CommonFormat::XYf_STf:
			attribs |= ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD;
			glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STf, x)));
			glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STf, s)));
			break;
		case CommonFormat::XYf_STf_RGBAub:
			attribs |= ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR;
			glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STf_RGBAub, x)));
			glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STf_RGBAub, s)));
			glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STf_RGBAub, color.r)));
			break;
		case CommonFormat::XYf_STus_RGBAub:
			attribs |= ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR;
			glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STus_RGBAub, x)));
			glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STus_RGBAub, s)));
			glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, BUFFER_OFFSET(offset + offsetof(XYf_STus_RGBAub, color.r)));
			break;
		}
	}

	if (attribs == 0)
		return;

	GLenum glmode = GL_ZERO;

	switch (sbstate.primitiveMode)
	{
	case PrimitiveMode::TRIANGLES:
		glmode = GL_TRIANGLES;
		break;
	case PrimitiveMode::POINTS:
		glmode = GL_POINTS;
		break;
	}

	Colorf nc = gl.getConstantColor();
	if (attribs & ATTRIBFLAG_COLOR)
		gl.setConstantColor(Colorf(1.0f, 1.0f, 1.0f, 1.0f));

	pushIdentityTransform();

	gl.prepareDraw();

	if (sbstate.textureHandle != 0)
		gl.bindTextureToUnit((GLuint) sbstate.textureHandle, 0, false);
	else
		gl.bindTextureToUnit(sbstate.texture, 0, false);

	gl.useVertexAttribArrays(attribs);

	if (sbstate.indexCount > 0)
	{
		sbstate.indexBuffer->resetOffset();
		ptrdiff_t offset = (ptrdiff_t) sbstate.indexBuffer->getData();

		gl.bindBuffer(BUFFER_INDEX, 0);
		gl.drawElements(glmode, sbstate.indexCount, GL_UNSIGNED_SHORT, BUFFER_OFFSET(offset));
	}
	else
		gl.drawArrays(glmode, 0, sbstate.vertexCount);

	popTransform();

	if (attribs & ATTRIB_CONSTANTCOLOR)
		gl.setConstantColor(nc);

	streamBufferState.vertexCount = 0;
	streamBufferState.indexCount = 0;
}

static void APIENTRY debugCB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*len*/, const GLchar *msg, const GLvoid* /*usr*/)
{
	// Human-readable strings for the debug info.
	const char *sourceStr = OpenGL::debugSourceString(source);
	const char *typeStr = OpenGL::debugTypeString(type);
	const char *severityStr = OpenGL::debugSeverityString(severity);

	const char *fmt = "OpenGL: %s [source=%s, type=%s, severity=%s, id=%d]\n";
	printf(fmt, msg, sourceStr, typeStr, severityStr, id);
}

void Graphics::setDebug(bool enable)
{
	// Make sure debug output is supported. The AMD ext. is a bit different
	// so we don't make use of it, since AMD drivers now support KHR_debug.
	if (!(GLAD_VERSION_4_3 || GLAD_KHR_debug || GLAD_ARB_debug_output))
		return;

	// TODO: We don't support GL_KHR_debug in GLES yet.
	if (GLAD_ES_VERSION_2_0)
		return;

	// Ugly hack to reduce code duplication.
	if (GLAD_ARB_debug_output && !(GLAD_VERSION_4_3 || GLAD_KHR_debug))
	{
		fp_glDebugMessageCallback = (pfn_glDebugMessageCallback) fp_glDebugMessageCallbackARB;
		fp_glDebugMessageControl = (pfn_glDebugMessageControl) fp_glDebugMessageControlARB;
	}

	if (!enable)
	{
		// Disable the debug callback function.
		glDebugMessageCallback(nullptr, nullptr);

		// We can disable debug output entirely with KHR_debug.
		if (GLAD_VERSION_4_3 || GLAD_KHR_debug)
			glDisable(GL_DEBUG_OUTPUT);

		return;
	}

	// We don't want asynchronous debug output.
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback(debugCB, nullptr);

	// Initially, enable everything.
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);

	// Disable messages about deprecated OpenGL functionality.
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_FALSE);
	glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_FALSE);

	if (GLAD_VERSION_4_3 || GLAD_KHR_debug)
		glEnable(GL_DEBUG_OUTPUT);

	::printf("OpenGL debug output enabled (LOVE_GRAPHICS_DEBUG=1)\n");
}

void Graphics::setCanvas(const std::vector<love::graphics::Canvas *> &canvases)
{
	DisplayState &state = states.back();
	int ncanvases = (int) canvases.size();

	if (ncanvases == 0)
		return setCanvas();

	if (ncanvases == (int) state.canvases.size())
	{
		bool modified = false;

		for (int i = 0; i < ncanvases; i++)
		{
			if (canvases[i] != state.canvases[i].get())
			{
				modified = true;
				break;
			}
		}

		if (!modified)
			return;
	}

	if (ncanvases > gl.getMaxRenderTargets())
		throw love::Exception("This system can't simultaneously render to %d canvases.", ncanvases);

	love::graphics::Canvas *firstcanvas = canvases[0];

	bool multiformatsupported = Canvas::isMultiFormatMultiCanvasSupported();
	PixelFormat firstformat = firstcanvas->getPixelFormat();

	bool hasSRGBcanvas = firstformat == PIXELFORMAT_sRGBA8;
	int pixelwidth = firstcanvas->getPixelWidth();
	int pixelheight = firstcanvas->getPixelHeight();

	for (int i = 1; i < ncanvases; i++)
	{
		love::graphics::Canvas *c = canvases[i];

		if (c->getPixelWidth() != pixelwidth || c->getPixelHeight() != pixelheight)
			throw love::Exception("All canvases in must have the same pixel dimensions.");

		if (!multiformatsupported && c->getPixelFormat() != firstformat)
			throw love::Exception("This system doesn't support multi-canvas rendering with different canvas formats.");

		if (c->getRequestedMSAA() != firstcanvas->getRequestedMSAA())
			throw love::Exception("All Canvases in must have the same requested MSAA value.");

		if (c->getPixelFormat() == PIXELFORMAT_sRGBA8)
			hasSRGBcanvas = true;
	}

	OpenGL::TempDebugGroup debuggroup("setCanvas(...)");

	endPass();

	bindCachedFBO(canvases);

	gl.setViewport({0, 0, pixelwidth, pixelheight});

	// Re-apply the scissor if it was active, since the rectangle passed to
	// glScissor is affected by the viewport dimensions.
	if (state.scissor)
		setScissor(state.scissorRect);

	int w = firstcanvas->getWidth();
	int h = firstcanvas->getHeight();
	projectionMatrix = Matrix4::ortho(0.0, (float) w, 0.0, (float) h);

	// Make sure the correct sRGB setting is used when drawing to the canvases.
	if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
	{
		if (hasSRGBcanvas && !gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(true);
		else if (!hasSRGBcanvas && gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(false);
	}

	std::vector<StrongRef<love::graphics::Canvas>> canvasrefs;
	canvasrefs.reserve(canvases.size());

	for (love::graphics::Canvas *c : canvases)
		canvasrefs.push_back(c);

	std::swap(state.canvases, canvasrefs);

	canvasSwitchCount++;
}

void Graphics::setCanvas()
{
	DisplayState &state = states.back();

	if (state.canvases.empty())
		return;

	OpenGL::TempDebugGroup debuggroup("setCanvas()");

	endPass();

	state.canvases.clear();

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, gl.getDefaultFBO());

	gl.setViewport({0, 0, pixelWidth, pixelHeight});

	// Re-apply the scissor if it was active, since the rectangle passed to
	// glScissor is affected by the viewport dimensions.
	if (state.scissor)
		setScissor(state.scissorRect);

	// The projection matrix is flipped compared to rendering to a canvas, due
	// to OpenGL considering (0,0) bottom-left instead of top-left.
	projectionMatrix = Matrix4::ortho(0.0, (float) width, (float) height, 0.0);

	if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
	{
		if (isGammaCorrect() && !gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(true);
		else if (!isGammaCorrect() && gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(false);
	}

	canvasSwitchCount++;
}

void Graphics::endPass()
{
	flushStreamDraws();

	// Discard the stencil buffer.
	discard({}, true);

	auto &canvases = states.back().canvases;

	// Resolve MSAA buffers.
	if (canvases.size() > 0 && canvases[0]->getMSAA() > 1)
	{
		int w = canvases[0]->getPixelWidth();
		int h = canvases[0]->getPixelHeight();

		for (int i = 0; i < (int) canvases.size(); i++)
		{
			Canvas *c = (Canvas *) canvases[i].get();

			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

			gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, c->getFBO());

			if (GLAD_APPLE_framebuffer_multisample)
				glResolveMultisampleFramebufferAPPLE();
			else
				glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}
}

void Graphics::clear(Colorf c)
{
	flushStreamDraws();

	gammaCorrectColor(c);
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
	{
		// This seems to be enough to fix the bug for me. Other methods I've
		// tried (e.g. dummy draws) don't work in all cases.
		gl.useProgram(0);
		gl.useProgram(((Shader *)Shader::current)->getProgram());
	}
}

void Graphics::clear(const std::vector<OptionalColorf> &colors)
{
	if (colors.size() == 0)
		return;

	int ncanvases = (int) states.back().canvases.size();
	int ncolors = std::min((int) colors.size(), ncanvases);

	if (ncolors <= 1 && ncanvases <= 1)
	{
		if (colors[0].enabled)
			clear(colors[0].c);

		return;
	}

	flushStreamDraws();

	bool drawbuffersmodified = false;

	for (int i = 0; i < ncolors; i++)
	{
		if (!colors[i].enabled)
			continue;

		Colorf c = colors[i].c;
		gammaCorrectColor(c);

		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0)
		{
			const GLfloat carray[] = {c.r, c.g, c.b, c.a};
			glClearBufferfv(GL_COLOR, i, carray);
		}
		else
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
			glClearColor(c.r, c.g, c.b, c.a);
			glClear(GL_COLOR_BUFFER_BIT);

			drawbuffersmodified = true;
		}
	}

	// Revert to the expected draw buffers once we're done, if glClearBuffer
	// wasn't supported.
	if (drawbuffersmodified)
	{
		GLenum bufs[MAX_COLOR_RENDER_TARGETS];

		for (int i = 0; i < ncanvases; i++)
			bufs[i] = GL_COLOR_ATTACHMENT0 + i;

		glDrawBuffers(ncanvases, bufs);
	}

	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
	{
		// This seems to be enough to fix the bug for me. Other methods I've
		// tried (e.g. dummy draws) don't work in all cases.
		gl.useProgram(0);
		gl.useProgram(((Shader *)Shader::current)->getProgram());
	}
}

void Graphics::discard(const std::vector<bool> &colorbuffers, bool depthstencil)
{
	flushStreamDraws();
	discard(OpenGL::FRAMEBUFFER_ALL, colorbuffers, depthstencil);
}

void Graphics::discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil)
{
	if (!(GLAD_VERSION_4_3 || GLAD_ARB_invalidate_subdata || GLAD_ES_VERSION_3_0 || GLAD_EXT_discard_framebuffer))
		return;

	GLenum gltarget = GL_FRAMEBUFFER;
	if (target == OpenGL::FRAMEBUFFER_READ)
		gltarget = GL_READ_FRAMEBUFFER;
	else if (target == OpenGL::FRAMEBUFFER_DRAW)
		gltarget = GL_DRAW_FRAMEBUFFER;

	std::vector<GLenum> attachments;
	attachments.reserve(colorbuffers.size());

	// glDiscardFramebuffer uses different attachment enums for the default FBO.
	if (states.back().canvases.empty() && gl.getDefaultFBO() == 0)
	{
		if (colorbuffers.size() > 0 && colorbuffers[0])
			attachments.push_back(GL_COLOR);

		if (depthstencil)
		{
			attachments.push_back(GL_STENCIL);
			attachments.push_back(GL_DEPTH);
		}
	}
	else
	{
		int rendertargetcount = std::max((int) states.back().canvases.size(), 1);

		for (int i = 0; i < (int) colorbuffers.size(); i++)
		{
			if (colorbuffers[i] && i < rendertargetcount)
				attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		if (depthstencil)
		{
			attachments.push_back(GL_STENCIL_ATTACHMENT);
			attachments.push_back(GL_DEPTH_ATTACHMENT);
		}
	}

	// Hint for the driver that it doesn't need to save these buffers.
	if (GLAD_VERSION_4_3 || GLAD_ARB_invalidate_subdata || GLAD_ES_VERSION_3_0)
		glInvalidateFramebuffer(gltarget, (GLint) attachments.size(), &attachments[0]);
	else if (GLAD_EXT_discard_framebuffer)
		glDiscardFramebufferEXT(gltarget, (GLint) attachments.size(), &attachments[0]);
}

void Graphics::bindCachedFBO(const std::vector<love::graphics::Canvas *> &canvases)
{
	int ncanvases = (int) canvases.size();

	uint32 hash = XXH32(&canvases[0], sizeof(love::graphics::Canvas *) * ncanvases, 0);

	GLuint fbo = framebufferObjects[hash];

	if (fbo != 0)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);
	}
	else
	{
		int w = canvases[0]->getPixelWidth();
		int h = canvases[0]->getPixelHeight();
		int msaa = std::max(canvases[0]->getMSAA(), 1);

		glGenFramebuffers(1, &fbo);
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

		GLenum drawbuffers[MAX_COLOR_RENDER_TARGETS];

		for (int i = 0; i < ncanvases; i++)
		{
			drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;

			if (msaa > 1)
			{
				GLuint rbo = (GLuint) canvases[i]->getMSAAHandle();
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, drawbuffers[i], GL_RENDERBUFFER, rbo);
			}
			else
			{
				GLuint tex = (GLuint) canvases[i]->getHandle();
				glFramebufferTexture2D(GL_FRAMEBUFFER, drawbuffers[i], GL_TEXTURE_2D, tex, 0);
			}
		}

		if (ncanvases > 1)
			glDrawBuffers(ncanvases, drawbuffers);

		GLuint stencil = attachCachedStencilBuffer(w, h, canvases[0]->getRequestedMSAA());

		if (stencil == 0)
		{
			gl.deleteFramebuffer(fbo);
			gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, gl.getDefaultFBO());
			throw love::Exception("Could not create stencil buffer!");
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			gl.deleteFramebuffer(fbo);
			const char *sstr = OpenGL::framebufferStatusString(status);
			throw love::Exception("Could not create Framebuffer Object! %s", sstr);
		}

		framebufferObjects[hash] = fbo;
	}
}

GLuint Graphics::attachCachedStencilBuffer(int w, int h, int samples)
{
	samples = samples == 1 ? 0 : samples;

	for (const CachedRenderbuffer &rb : stencilBuffers)
	{
		if (rb.w == w && rb.h == h && rb.samples == samples)
		{
			// Attach the buffer to the framebuffer object.
			for (GLenum attachment : rb.attachments)
			{
				if (attachment != GL_NONE)
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb.renderbuffer);
			}

			return rb.renderbuffer;
		}
	}

	OpenGL::TempDebugGroup debuggroup("Create cached stencil buffer");

	CachedRenderbuffer rb;
	rb.w = w;
	rb.h = h;
	rb.samples = samples;

	rb.attachments[0] = GL_STENCIL_ATTACHMENT;
	rb.attachments[1] = GL_NONE;

	GLenum format = GL_STENCIL_INDEX8;

	// Prefer a combined depth/stencil buffer.
	if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object)
	{
		format = GL_DEPTH24_STENCIL8;
		rb.attachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
	}
	else if (GLAD_EXT_packed_depth_stencil || GLAD_OES_packed_depth_stencil)
	{
		format = GL_DEPTH24_STENCIL8;
		rb.attachments[0] = GL_DEPTH_ATTACHMENT;
		rb.attachments[1] = GL_STENCIL_ATTACHMENT;
	}

	glGenRenderbuffers(1, &rb.renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, rb.renderbuffer);

	if (rb.samples > 1)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, rb.samples, format, rb.w, rb.h);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, format, rb.w, rb.h);

	// Attach the buffer to the framebuffer object.
	for (GLenum attachment : rb.attachments)
	{
		if (attachment != GL_NONE)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb.renderbuffer);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glDeleteRenderbuffers(1, &rb.renderbuffer);
		rb.renderbuffer = 0;
	}

	if (rb.renderbuffer != 0)
	{
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		stencilBuffers.push_back(rb);
	}

	return rb.renderbuffer;
}

void Graphics::present(void *screenshotCallbackData)
{
	if (!isActive())
		return;

	if (!states.back().canvases.empty())
		throw love::Exception("present cannot be called while a Canvas is active.");

	endPass();

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, gl.getDefaultFBO());

	if (!pendingScreenshotCallbacks.empty())
	{
		int w = getPixelWidth();
		int h = getPixelHeight();

		size_t row = 4 * w;
		size_t size = row * h;

		GLubyte *pixels = nullptr;
		GLubyte *screenshot = nullptr;

		try
		{
			pixels = new GLubyte[size];
			screenshot = new GLubyte[size];
		}
		catch (std::exception &)
		{
			delete[] pixels;
			delete[] screenshot;
			throw love::Exception("Out of memory.");
		}

#ifdef LOVE_IOS
		SDL_SysWMinfo info = {};
		SDL_VERSION(&info.version);
		SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);

		if (info.info.uikit.resolveFramebuffer != 0)
		{
			gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, info.info.uikit.resolveFramebuffer);

			// We need to do an explicit MSAA resolve on iOS, because it uses GLES
			// FBOs rather than a system framebuffer.
			if (GLAD_ES_VERSION_3_0)
				glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			else if (GLAD_APPLE_framebuffer_multisample)
				glResolveMultisampleFramebufferAPPLE();

			gl.bindFramebuffer(OpenGL::FRAMEBUFFER_READ, info.info.uikit.resolveFramebuffer);
		}
#endif

		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Replace alpha values with full opacity.
		for (size_t i = 3; i < size; i += 4)
			pixels[i] = 255;

		// OpenGL sucks and reads pixels from the lower-left. Let's fix that.
		GLubyte *src = pixels - row;
		GLubyte *dst = screenshot + size;

		for (int i = 0; i < h; ++i)
			memcpy(dst-=row, src+=row, row);

		delete[] pixels;

		auto imagemodule = Module::getInstance<love::image::Image>(M_IMAGE);

		for (int i = 0; i < (int) pendingScreenshotCallbacks.size(); i++)
		{
			const auto &info = pendingScreenshotCallbacks[i];
			image::ImageData *img = nullptr;

			try
			{
				img = imagemodule->newImageData(w, h, PIXELFORMAT_RGBA8, screenshot);
			}
			catch (love::Exception &)
			{
				delete[] screenshot;
				info.callback(nullptr, info.ref, nullptr);
				for (int j = i + 1; j < (int) pendingScreenshotCallbacks.size(); j++)
				{
					const auto &ninfo = pendingScreenshotCallbacks[j];
					ninfo.callback(nullptr, ninfo.ref, nullptr);
				}
				pendingScreenshotCallbacks.clear();
				throw;
			}

			info.callback(img, info.ref, screenshotCallbackData);
			img->release();
		}

		delete[] screenshot;
		pendingScreenshotCallbacks.clear();
	}

#ifdef LOVE_IOS
	// Hack: SDL's color renderbuffer must be bound when swapBuffers is called.
	SDL_SysWMinfo info = {};
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);
	glBindRenderbuffer(GL_RENDERBUFFER, info.info.uikit.colorbuffer);
#endif

	auto window = getInstance<love::window::Window>(M_WINDOW);
	if (window != nullptr)
		window->swapBuffers();

	// Reset the per-frame stat counts.
	gl.stats.drawCalls = 0;
	gl.stats.shaderSwitches = 0;
	canvasSwitchCount = 0;
}

void Graphics::setScissor(const Rect &rect)
{
	flushStreamDraws();

	DisplayState &state = states.back();

	glEnable(GL_SCISSOR_TEST);

	double density = getCurrentPixelDensity();

	Rect glrect;
	glrect.x = (int) (rect.x * density);
	glrect.y = (int) (rect.y * density);
	glrect.w = (int) (rect.w * density);
	glrect.h = (int) (rect.h * density);

	// OpenGL's reversed y-coordinate is compensated for in OpenGL::setScissor.
	gl.setScissor(glrect, !state.canvases.empty());

	state.scissor = true;
	state.scissorRect = rect;
}

void Graphics::setScissor()
{
	flushStreamDraws();
	states.back().scissor = false;
	glDisable(GL_SCISSOR_TEST);
}

void Graphics::drawToStencilBuffer(StencilAction action, int value)
{
	writingToStencil = true;

	// Disable color writes but don't save the state for it.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	GLenum glaction = GL_REPLACE;

	switch (action)
	{
	case STENCIL_REPLACE:
	default:
		glaction = GL_REPLACE;
		break;
	case STENCIL_INCREMENT:
		glaction = GL_INCR;
		break;
	case STENCIL_DECREMENT:
		glaction = GL_DECR;
		break;
	case STENCIL_INCREMENT_WRAP:
		glaction = GL_INCR_WRAP;
		break;
	case STENCIL_DECREMENT_WRAP:
		glaction = GL_DECR_WRAP;
		break;
	case STENCIL_INVERT:
		glaction = GL_INVERT;
		break;
	}

	// The stencil test must be enabled in order to write to the stencil buffer.
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, value, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, glaction);
}

void Graphics::stopDrawToStencilBuffer()
{
	if (!writingToStencil)
		return;

	flushStreamDraws();

	writingToStencil = false;

	const DisplayState &state = states.back();

	// Revert the color write mask.
	setColorMask(state.colorMask);

	// Use the user-set stencil test state when writes are disabled.
	setStencilTest(state.stencilCompare, state.stencilTestValue);
}

void Graphics::setStencilTest(CompareMode compare, int value)
{
	DisplayState &state = states.back();
	state.stencilCompare = compare;
	state.stencilTestValue = value;

	if (writingToStencil)
		return;

	if (compare == COMPARE_ALWAYS)
	{
		glDisable(GL_STENCIL_TEST);
		return;
	}

	GLenum glcompare = GL_EQUAL;

	/**
	 * Q: Why are some of the compare modes inverted (e.g. COMPARE_LESS becomes
	 * GL_GREATER)?
	 *
	 * A: OpenGL / GPUs do the comparison in the opposite way that makes sense
	 * for this API. For example, if the compare function is GL_GREATER then the
	 * stencil test will pass if the reference value is greater than the value
	 * in the stencil buffer. With our API it's more intuitive to assume that
	 * setStencilTest(COMPARE_GREATER, 4) will make it pass if the stencil
	 * buffer has a value greater than 4.
	 **/

	switch (compare)
	{
	case COMPARE_LESS:
		glcompare = GL_GREATER;
		break;
	case COMPARE_LEQUAL:
		glcompare = GL_GEQUAL;
		break;
	case COMPARE_EQUAL:
	default:
		glcompare = GL_EQUAL;
		break;
	case COMPARE_GEQUAL:
		glcompare = GL_LEQUAL;
		break;
	case COMPARE_GREATER:
		glcompare = GL_LESS;
		break;
	case COMPARE_NOTEQUAL:
		glcompare = GL_NOTEQUAL;
		break;
	case COMPARE_ALWAYS:
		glcompare = GL_ALWAYS;
		break;
	}

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(glcompare, value, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void Graphics::setStencilTest()
{
	setStencilTest(COMPARE_ALWAYS, 0);
}

void Graphics::clearStencil()
{
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Graphics::setColor(Colorf c)
{
	c.r = std::min(std::max(c.r, 0.0f), 1.0f);
	c.g = std::min(std::max(c.g, 0.0f), 1.0f);
	c.b = std::min(std::max(c.b, 0.0f), 1.0f);
	c.a = std::min(std::max(c.a, 0.0f), 1.0f);

	gl.setConstantColor(c);

	states.back().color = c;
}

void Graphics::setColorMask(ColorMask mask)
{
	flushStreamDraws();

	glColorMask(mask.r, mask.g, mask.b, mask.a);
	states.back().colorMask = mask;
}

void Graphics::setBlendMode(BlendMode mode, BlendAlpha alphamode)
{
	flushStreamDraws();

	GLenum func   = GL_FUNC_ADD;
	GLenum srcRGB = GL_ONE;
	GLenum srcA   = GL_ONE;
	GLenum dstRGB = GL_ZERO;
	GLenum dstA   = GL_ZERO;

	if (mode == BLEND_LIGHTEN || mode == BLEND_DARKEN)
	{
		if (!isSupported(FEATURE_LIGHTEN))
			throw love::Exception("The 'lighten' and 'darken' blend modes are not supported on this system.");
	}

	if (alphamode != BLENDALPHA_PREMULTIPLIED)
	{
		const char *modestr = "unknown";
		switch (mode)
		{
		case BLEND_LIGHTEN:
		case BLEND_DARKEN:
		case BLEND_MULTIPLY:
			getConstant(mode, modestr);
			throw love::Exception("The '%s' blend mode must be used with premultiplied alpha.", modestr);
			break;
		default:
			break;
		}
	}

	switch (mode)
	{
	case BLEND_ALPHA:
		srcRGB = srcA = GL_ONE;
		dstRGB = dstA = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_MULTIPLY:
		srcRGB = srcA = GL_DST_COLOR;
		dstRGB = dstA = GL_ZERO;
		break;
	case BLEND_SUBTRACT:
		func = GL_FUNC_REVERSE_SUBTRACT;
	case BLEND_ADD:
		srcRGB = GL_ONE;
		srcA = GL_ZERO;
		dstRGB = dstA = GL_ONE;
		break;
	case BLEND_LIGHTEN:
		func = GL_MAX;
		break;
	case BLEND_DARKEN:
		func = GL_MIN;
		break;
	case BLEND_SCREEN:
		srcRGB = srcA = GL_ONE;
		dstRGB = dstA = GL_ONE_MINUS_SRC_COLOR;
		break;
	case BLEND_REPLACE:
	case BLEND_NONE:
	default:
		srcRGB = srcA = GL_ONE;
		dstRGB = dstA = GL_ZERO;
		break;
	}

	// We can only do alpha-multiplication when srcRGB would have been unmodified.
	if (srcRGB == GL_ONE && alphamode == BLENDALPHA_MULTIPLY && mode != BLEND_NONE)
		srcRGB = GL_SRC_ALPHA;

	glBlendEquation(func);
	glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);

	states.back().blendMode = mode;
	states.back().blendAlphaMode = alphamode;
}

void Graphics::setPointSize(float size)
{
	if (streamBufferState.primitiveMode == vertex::PrimitiveMode::POINTS)
		flushStreamDraws();

	gl.setPointSize(size * getCurrentPixelDensity());
	states.back().pointSize = size;
}

void Graphics::setWireframe(bool enable)
{
	// Not supported in OpenGL ES.
	if (GLAD_ES_VERSION_2_0)
		return;

	flushStreamDraws();

	glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
	states.back().wireframe = enable;
}

Graphics::RendererInfo Graphics::getRendererInfo() const
{
	RendererInfo info;

	if (GLAD_ES_VERSION_2_0)
		info.name = "OpenGL ES";
	else
		info.name = "OpenGL";

	const char *str = (const char *) glGetString(GL_VERSION);
	if (str)
		info.version = str;
	else
		throw love::Exception("Cannot retrieve renderer version information.");

	str = (const char *) glGetString(GL_VENDOR);
	if (str)
		info.vendor = str;
	else
		throw love::Exception("Cannot retrieve renderer vendor information.");

	str = (const char *) glGetString(GL_RENDERER);
	if (str)
		info.device = str;
	else
		throw love::Exception("Cannot retrieve renderer device information.");

	return info;
}

Graphics::Stats Graphics::getStats() const
{
	int drawcalls = gl.stats.drawCalls;

	if (streamBufferState.vertexCount > 0)
		drawcalls++;

	Stats stats;

	stats.drawCalls = drawcalls;
	stats.canvasSwitches = canvasSwitchCount;
	stats.shaderSwitches = gl.stats.shaderSwitches;
	stats.canvases = Canvas::canvasCount;
	stats.images = Image::imageCount;
	stats.fonts = Font::fontCount;
	stats.textureMemory = gl.stats.textureMemory;

	return stats;
}

double Graphics::getSystemLimit(SystemLimit limittype) const
{
	switch (limittype)
	{
	case Graphics::LIMIT_POINT_SIZE:
		return (double) gl.getMaxPointSize();
	case Graphics::LIMIT_TEXTURE_SIZE:
		return (double) gl.getMaxTextureSize();
	case Graphics::LIMIT_MULTI_CANVAS:
		return (double) gl.getMaxRenderTargets();
	case Graphics::LIMIT_CANVAS_MSAA:
		return (double) gl.getMaxRenderbufferSamples();
	case Graphics::LIMIT_ANISOTROPY:
		return (double) gl.getMaxAnisotropy();
	default:
		return 0.0;
	}
}

bool Graphics::isSupported(Feature feature) const
{
	switch (feature)
	{
	case FEATURE_MULTI_CANVAS_FORMATS:
		return Canvas::isMultiFormatMultiCanvasSupported();
	case FEATURE_CLAMP_ZERO:
		return gl.isClampZeroTextureWrapSupported();
	case FEATURE_LIGHTEN:
		return GLAD_VERSION_1_4 || GLAD_ES_VERSION_3_0 || GLAD_EXT_blend_minmax;
	case FEATURE_FULL_NPOT:
		return GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot;
	case FEATURE_PIXEL_SHADER_HIGHP:
		return gl.isPixelShaderHighpSupported();
	default:
		return false;
	}
}

} // opengl
} // graphics
} // love
