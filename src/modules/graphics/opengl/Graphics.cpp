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
#include "StreamBuffer.h"
#include "math/MathModule.h"
#include "window/Window.h"
#include "Buffer.h"
#include "Text.h"

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
	, windowHasStencil(false)
	, mainVAO(0)
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
			int w, h;
			love::window::WindowSettings settings;
			window->getWindow(w, h, settings);

			double dpiW = w;
			double dpiH = h;
			window->windowToDPICoords(&dpiW, &dpiH);

			setMode((int) dpiW, (int) dpiH, window->getPixelWidth(), window->getPixelHeight(), settings.stencil);
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

love::graphics::StreamBuffer *Graphics::newStreamBuffer(BufferType type, size_t size)
{
	return CreateStreamBuffer(type, size);
}

love::graphics::Image *Graphics::newImage(const Image::Slices &data, const Image::Settings &settings)
{
	return new Image(data, settings);
}

love::graphics::Image *Graphics::newImage(TextureType textype, PixelFormat format, int width, int height, int slices, const Image::Settings &settings)
{
	return new Image(textype, format, width, height, slices, settings);
}

love::graphics::SpriteBatch *Graphics::newSpriteBatch(Texture *texture, int size, vertex::Usage usage)
{
	return new SpriteBatch(this, texture, size, usage);
}

love::graphics::ParticleSystem *Graphics::newParticleSystem(Texture *texture, int size)
{
	return new ParticleSystem(this, texture, size);
}

love::graphics::Canvas *Graphics::newCanvas(const Canvas::Settings &settings)
{
	return new Canvas(settings);
}


love::graphics::Shader *Graphics::newShader(const Shader::ShaderSource &source)
{
	return new Shader(source);
}

love::graphics::Buffer *Graphics::newBuffer(size_t size, const void *data, BufferType type, vertex::Usage usage, uint32 mapflags)
{
	return new Buffer(size, data, type, usage, mapflags);
}

love::graphics::Mesh *Graphics::newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(this, vertices, drawmode, usage);
}

love::graphics::Mesh *Graphics::newMesh(int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(this, vertexcount, drawmode, usage);
}

love::graphics::Mesh *Graphics::newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(this, vertexformat, vertexcount, drawmode, usage);
}

love::graphics::Mesh *Graphics::newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, const void *data, size_t datasize, Mesh::DrawMode drawmode, vertex::Usage usage)
{
	return new Mesh(this, vertexformat, data, datasize, drawmode, usage);
}

love::graphics::Text *Graphics::newText(graphics::Font *font, const std::vector<Font::ColoredString> &text)
{
	return new Text(this, font, text);
}

void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight)
{
	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	if (!isCanvasActive())
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

bool Graphics::setMode(int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil)
{
	this->width = width;
	this->height = height;

	this->windowHasStencil = windowhasstencil;

	// Okay, setup OpenGL.
	gl.initContext();

	if (gl.isCoreProfile())
	{
		glGenVertexArrays(1, &mainVAO);
		glBindVertexArray(mainVAO);
	}

	gl.setupContext();

	created = true;

	setViewportSize(width, height, pixelwidth, pixelheight);

	// Enable blending
	glEnable(GL_BLEND);

	// Auto-generated mipmaps should be the best quality possible
	if (!gl.isCoreProfile())
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	if (!GLAD_ES_VERSION_2_0 && !gl.isCoreProfile())
	{
		// Make sure antialiasing works when set elsewhere
		glEnable(GL_MULTISAMPLE);

		// Enable texturing
		glEnable(GL_TEXTURE_2D);
	}

	gl.setTextureUnit(0);

	// Set pixel row alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Always enable seamless cubemap filtering when possible.
	if (GLAD_VERSION_3_2 || GLAD_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Set whether drawing converts input from linear -> sRGB colorspace.
	if (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB
		|| GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB)
	{
		if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
			gl.setFramebufferSRGB(isGammaCorrect());
	}
	else
		setGammaCorrect(false);

	setDebug(isDebugEnabled());

	if (streamBufferState.vb[0] == nullptr)
	{
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		streamBufferState.vb[0] = CreateStreamBuffer(BUFFER_VERTEX, 1024 * 1024 * 1);
		streamBufferState.vb[1] = CreateStreamBuffer(BUFFER_VERTEX, 256  * 1024 * 1);
		streamBufferState.indexBuffer = CreateStreamBuffer(BUFFER_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
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
		quadIndices = new QuadIndices(this, 20);

	// Restore the graphics state.
	restoreState(states.back());

	int gammacorrect = isGammaCorrect() ? 1 : 0;
	Shader::Language target = getShaderLanguageTarget();

	// We always need a default shader.
	for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++)
	{
		if (i == Shader::STANDARD_ARRAY && !isSupported(FEATURE_ARRAY_TEXTURE))
			continue;

		if (!Shader::standardShaders[i])
			Shader::standardShaders[i] = newShader(defaultShaderCode[i][target][gammacorrect]);
	}

	// A shader should always be active, but the default shader shouldn't be
	// returned by getShader(), so we don't do setShader(defaultShader).
	if (!Shader::current)
		Shader::standardShaders[Shader::STANDARD_DEFAULT]->attach();

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

	for (love::graphics::Canvas *c : stencilBuffers)
		c->release();

	framebufferObjects.clear();
	stencilBuffers.clear();

	if (mainVAO != 0)
	{
		glDeleteVertexArrays(1, &mainVAO);
		mainVAO = 0;
	}

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

	auto &sbstate = streamBufferState;

	if (sbstate.vertexCount == 0 && sbstate.indexCount == 0)
		return;

	love::graphics::Shader *prevdefaultshader = nullptr;

	if (sbstate.texture.get())
	{
		TextureType textype = sbstate.texture->getTextureType();

		if (textype == TEXTURE_2D_ARRAY && Shader::isDefaultActive())
		{
			if (!Shader::standardShaders[Shader::STANDARD_ARRAY])
				throw love::Exception("Standard array texture shader has not been initialized!");

			prevdefaultshader = Shader::current;
			Shader::standardShaders[Shader::STANDARD_ARRAY]->attach();
		}
	}

	OpenGL::TempDebugGroup debuggroup("Stream vertices flush and draw");

	uint32 attribs = 0;
	size_t usedsizes[3] = {0, 0, 0};

	for (int i = 0; i < 2; i++)
	{
		if (sbstate.formats[i] == CommonFormat::NONE)
			continue;

		usedsizes[i] = getFormatStride(sbstate.formats[i]) * sbstate.vertexCount;

		love::graphics::StreamBuffer *buffer = sbstate.vb[i];

		gl.bindBuffer(BUFFER_VERTEX, (GLuint) buffer->getHandle());
		size_t offset = buffer->unmap(usedsizes[i]);

		sbstate.vbMap[i] = StreamBuffer::MapInfo();

		gl.setVertexPointers(sbstate.formats[i], offset);
		attribs |= getFormatFlags(sbstate.formats[i]);
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
	gl.bindTextureToUnit(sbstate.texture, 0, false);

	gl.useVertexAttribArrays(attribs);

	if (sbstate.indexCount > 0)
	{
		usedsizes[2] = sizeof(uint16) * sbstate.indexCount;

		gl.bindBuffer(BUFFER_INDEX, (GLuint) sbstate.indexBuffer->getHandle());
		size_t offset = sbstate.indexBuffer->unmap(usedsizes[2]);

		sbstate.indexBufferMap = StreamBuffer::MapInfo();

		gl.drawElements(glmode, sbstate.indexCount, GL_UNSIGNED_SHORT, BUFFER_OFFSET(offset));
	}
	else
		gl.drawArrays(glmode, 0, sbstate.vertexCount);

	for (int i = 0; i < 2; i++)
	{
		if (usedsizes[i] > 0)
			sbstate.vb[i]->markUsed(usedsizes[i]);
	}

	if (usedsizes[2] > 0)
		sbstate.indexBuffer->markUsed(usedsizes[2]);

	popTransform();

	if (attribs & ATTRIB_CONSTANTCOLOR)
		gl.setConstantColor(nc);

	streamBufferState.vertexCount = 0;
	streamBufferState.indexCount = 0;

	if (prevdefaultshader != nullptr)
		prevdefaultshader->attach();
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

void Graphics::setCanvas(const RenderTargets &rts)
{
	DisplayState &state = states.back();
	int ncanvases = (int) rts.colors.size();

	if (ncanvases == 0 && rts.depthStencil.canvas == nullptr)
		return setCanvas();
	else if (ncanvases == 0)
		throw love::Exception("At least one color render target is required when using a custom depth/stencil buffer.");

	const auto &prevRTs = state.renderTargets;

	if (ncanvases == (int) prevRTs.colors.size())
	{
		bool modified = false;

		for (int i = 0; i < ncanvases; i++)
		{
			if (rts.colors[i].canvas != prevRTs.colors[i].canvas.get()
				|| rts.colors[i].slice != prevRTs.colors[i].slice
				|| rts.colors[i].mipmap != prevRTs.colors[i].mipmap)
			{
				modified = true;
				break;
			}
		}

		if (!modified && (rts.depthStencil.canvas != prevRTs.depthStencil.canvas
			|| rts.depthStencil.slice != prevRTs.depthStencil.slice
			|| rts.depthStencil.mipmap != prevRTs.depthStencil.mipmap))
		{
			modified = true;
		}

		if (!modified)
			return;
	}

	if (ncanvases > gl.getMaxRenderTargets())
		throw love::Exception("This system can't simultaneously render to %d canvases.", ncanvases);

	love::graphics::Canvas *firstcanvas = rts.colors[0].canvas;

	bool multiformatsupported = Canvas::isMultiFormatMultiCanvasSupported();
	PixelFormat firstformat = firstcanvas->getPixelFormat();

	if (isPixelFormatDepthStencil(firstformat))
		throw love::Exception("Depth/stencil format Canvases must be used with the 'depthstencil' field of the table passed into setCanvas.");

	if (rts.colors[0].mipmap < 0 || rts.colors[0].mipmap >= firstcanvas->getMipmapCount())
		throw love::Exception("Invalid mipmap level %d.", rts.colors[0].mipmap + 1);

	bool hasSRGBcanvas = firstformat == PIXELFORMAT_sRGBA8;
	int pixelwidth = firstcanvas->getPixelWidth(rts.colors[0].mipmap);
	int pixelheight = firstcanvas->getPixelHeight(rts.colors[0].mipmap);

	for (int i = 1; i < ncanvases; i++)
	{
		love::graphics::Canvas *c = rts.colors[i].canvas;
		PixelFormat format = c->getPixelFormat();
		int mip = rts.colors[i].mipmap;

		if (mip < 0 || mip >= c->getMipmapCount())
			throw love::Exception("Invalid mipmap level %d.", mip + 1);

		if (c->getPixelWidth(mip) != pixelwidth || c->getPixelHeight(mip) != pixelheight)
			throw love::Exception("All canvases must have the same pixel dimensions.");

		if (!multiformatsupported && format != firstformat)
			throw love::Exception("This system doesn't support multi-canvas rendering with different canvas formats.");

		if (c->getRequestedMSAA() != firstcanvas->getRequestedMSAA())
			throw love::Exception("All Canvases must have the same MSAA value.");

		if (isPixelFormatDepthStencil(format))
			throw love::Exception("Depth/stencil format Canvases must be used with the 'depthstencil' field of the table passed into setCanvas.");

		if (format == PIXELFORMAT_sRGBA8)
			hasSRGBcanvas = true;
	}

	if (rts.depthStencil.canvas != nullptr)
	{
		love::graphics::Canvas *c = rts.depthStencil.canvas;
		int mip = rts.depthStencil.mipmap;

		if (!isPixelFormatDepthStencil(c->getPixelFormat()))
			throw love::Exception("Only depth/stencil format Canvases can be used with the 'depthstencil' field of the table passed into setCanvas.");

		if (c->getPixelWidth(mip) != pixelwidth || c->getPixelHeight(mip) != pixelheight)
			throw love::Exception("All canvases must have the same pixel dimensions.");

		if (c->getRequestedMSAA() != firstcanvas->getRequestedMSAA())
			throw love::Exception("All Canvases must have the same MSAA value.");

		if (mip < 0 || mip >= c->getMipmapCount())
			throw love::Exception("Invalid mipmap level %d.", mip + 1);
	}

	OpenGL::TempDebugGroup debuggroup("setCanvas(...)");

	endPass();

	bindCachedFBO(rts);

	gl.setViewport({0, 0, pixelwidth, pixelheight});

	// Re-apply the scissor if it was active, since the rectangle passed to
	// glScissor is affected by the viewport dimensions.
	if (state.scissor)
		setScissor(state.scissorRect);

	int w = firstcanvas->getWidth(rts.colors[0].mipmap);
	int h = firstcanvas->getHeight(rts.colors[0].mipmap);
	projectionMatrix = Matrix4::ortho(0.0, (float) w, 0.0, (float) h);

	// Make sure the correct sRGB setting is used when drawing to the canvases.
	if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
	{
		if (hasSRGBcanvas && !gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(true);
		else if (!hasSRGBcanvas && gl.hasFramebufferSRGB())
			gl.setFramebufferSRGB(false);
	}

	RenderTargetsStrongRef refs;
	refs.colors.reserve(rts.colors.size());

	for (auto c : rts.colors)
		refs.colors.emplace_back(c.canvas, c.slice);

	refs.depthStencil = RenderTargetStrongRef(rts.depthStencil.canvas, rts.depthStencil.slice);

	std::swap(state.renderTargets, refs);

	canvasSwitchCount++;
}

void Graphics::setCanvas()
{
	DisplayState &state = states.back();

	if (state.renderTargets.colors.empty() && state.renderTargets.depthStencil.canvas == nullptr)
		return;

	OpenGL::TempDebugGroup debuggroup("setCanvas()");

	endPass();

	state.renderTargets = RenderTargetsStrongRef();

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

	auto &rts = states.back().renderTargets;
	love::graphics::Canvas *depthstencil = rts.depthStencil.canvas.get();

	// Discard the stencil buffer if we're using an internal cached one.
	if (depthstencil == nullptr)
		discard({}, true);

	// Resolve MSAA buffers. MSAA is only supported for 2D render targets so we
	// don't have to worry about resolving to slices.
	if (rts.colors.size() > 0 && rts.colors[0].canvas->getMSAA() > 1)
	{
		int mip = rts.colors[0].mipmap;
		int w = rts.colors[0].canvas->getPixelWidth(mip);
		int h = rts.colors[0].canvas->getPixelHeight(mip);

		for (int i = 0; i < (int) rts.colors.size(); i++)
		{
			Canvas *c = (Canvas *) rts.colors[i].canvas.get();

			if (!c->isReadable())
				continue;

			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

			gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, c->getFBO());

			if (GLAD_APPLE_framebuffer_multisample)
				glResolveMultisampleFramebufferAPPLE();
			else
				glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	for (const auto &rt : rts.colors)
	{
		if (rt.canvas->getMipmapMode() == Canvas::MIPMAPS_AUTO && rt.mipmap == 0)
			rt.canvas->generateMipmaps();
	}

	int dsmipmap = rts.depthStencil.mipmap;
	if (depthstencil != nullptr && depthstencil->getMipmapMode() == Canvas::MIPMAPS_AUTO && dsmipmap == 0)
		depthstencil->generateMipmaps();
}

void Graphics::clear(OptionalColorf c, OptionalInt stencil, OptionalDouble depth)
{
	if (c.hasValue || stencil.hasValue || depth.hasValue)
		flushStreamDraws();

	GLbitfield flags = 0;

	if (c.hasValue)
	{
		gammaCorrectColor(c.value);
		glClearColor(c.value.r, c.value.g, c.value.b, c.value.a);
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (stencil.hasValue)
	{
		glClearStencil(stencil.value);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	if (depth.hasValue)
	{
		glClearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
		glClear(flags);

	if (flags != 0 && gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
	{
		// This seems to be enough to fix the bug for me. Other methods I've
		// tried (e.g. dummy draws) don't work in all cases.
		gl.useProgram(0);
		gl.useProgram((GLuint) Shader::current->getHandle());
	}
}

void Graphics::clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth)
{
	if (colors.size() == 0 && !stencil.hasValue && !depth.hasValue)
		return;

	int ncanvases = (int) states.back().renderTargets.colors.size();
	int ncolors = std::min((int) colors.size(), ncanvases);

	if (ncolors <= 1 && ncanvases <= 1)
	{
		if (colors[0].hasValue)
			clear(colors[0].value, stencil, depth);

		return;
	}

	flushStreamDraws();

	bool drawbuffersmodified = false;

	for (int i = 0; i < ncolors; i++)
	{
		if (!colors[i].hasValue)
			continue;

		Colorf c = colors[i].value;
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

	GLbitfield flags = 0;

	if (stencil.hasValue)
	{
		glClearStencil(stencil.value);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	if (depth.hasValue)
	{
		glClearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
		glClear(flags);

	if (gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
	{
		// This seems to be enough to fix the bug for me. Other methods I've
		// tried (e.g. dummy draws) don't work in all cases.
		gl.useProgram(0);
		gl.useProgram((GLuint) Shader::current->getHandle());
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
	if (!isCanvasActive() && gl.getDefaultFBO() == 0)
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
		int rendertargetcount = std::max((int) states.back().renderTargets.colors.size(), 1);

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

void Graphics::bindCachedFBO(const RenderTargets &targets)
{
	RenderTarget hashtargets[MAX_COLOR_RENDER_TARGETS + 1];
	int hashcount = 0;

	for (int i = 0; i < (int) targets.colors.size(); i++)
		hashtargets[hashcount++] = targets.colors[i];

	if (targets.depthStencil.canvas != nullptr)
		hashtargets[hashcount++] = targets.depthStencil;

	uint32 hash = XXH32(hashtargets, sizeof(RenderTarget) * hashcount, 0);
	GLuint fbo = framebufferObjects[hash];

	if (fbo != 0)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);
	}
	else
	{
		int mip = targets.colors[0].mipmap;
		int w = targets.colors[0].canvas->getPixelWidth(mip);
		int h = targets.colors[0].canvas->getPixelHeight(mip);
		int msaa = targets.colors[0].canvas->getMSAA();
		int reqmsaa = targets.colors[0].canvas->getRequestedMSAA();

		RenderTarget depthstencil = targets.depthStencil;

		if (depthstencil.canvas == nullptr)
		{
			depthstencil.canvas = getCachedStencilBuffer(w, h, reqmsaa);
			depthstencil.slice = 0;
		}

		glGenFramebuffers(1, &fbo);
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

		int ncolortargets = 0;
		GLenum drawbuffers[MAX_COLOR_RENDER_TARGETS];

		auto attachCanvas = [&](const RenderTarget &rt)
		{
			bool renderbuffer = msaa > 1 || !rt.canvas->isReadable();
			bool srgb = false;
			OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(rt.canvas->getPixelFormat(), renderbuffer, srgb);

			if (fmt.framebufferAttachments[0] == GL_COLOR_ATTACHMENT0)
			{
				fmt.framebufferAttachments[0] = GL_COLOR_ATTACHMENT0 + ncolortargets;
				drawbuffers[ncolortargets] = fmt.framebufferAttachments[0];
				ncolortargets++;
			}

			GLuint handle = (GLuint) rt.canvas->getRenderTargetHandle();

			for (GLenum attachment : fmt.framebufferAttachments)
			{
				if (attachment == GL_NONE)
					continue;
				else if (renderbuffer)
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, handle);
				else
				{
					TextureType textype = rt.canvas->getTextureType();

					int layer = textype == TEXTURE_CUBE ? 0 : rt.slice;
					int face = textype == TEXTURE_CUBE ? rt.slice : 0;
					int level = rt.mipmap;

					gl.framebufferTexture(attachment, textype, handle, level, layer, face);
				}
			}
		};

		for (const auto &rt : targets.colors)
			attachCanvas(rt);

		if (depthstencil.canvas != nullptr)
			attachCanvas(depthstencil);

		if (ncolortargets > 1)
			glDrawBuffers(ncolortargets, drawbuffers);

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

love::graphics::Canvas *Graphics::getCachedStencilBuffer(int w, int h, int samples)
{
	love::graphics::Canvas *canvas = nullptr;

	for (love::graphics::Canvas *c : stencilBuffers)
	{
		if (c->getPixelWidth() == w && c->getPixelHeight() == h && c->getRequestedMSAA() == samples)
		{
			canvas = c;
			break;
		}
	}

	if (canvas == nullptr)
	{
		Canvas::Settings settings;
		settings.format = PIXELFORMAT_STENCIL8;
		settings.width = w;
		settings.height = h;
		settings.msaa = samples;

		canvas = newCanvas(settings);

		stencilBuffers.push_back(canvas);
	}

	return canvas;
}

void Graphics::present(void *screenshotCallbackData)
{
	if (!isActive())
		return;

	if (isCanvasActive())
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

			// We need to do an explicit MSAA resolve on iOS, because it uses
			// GLES FBOs rather than a system framebuffer.
			if (GLAD_ES_VERSION_3_0)
				glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
	gl.setScissor(glrect, isCanvasActive());

	state.scissor = true;
	state.scissorRect = rect;
}

void Graphics::setScissor()
{
	if (states.back().scissor)
		flushStreamDraws();

	states.back().scissor = false;
	glDisable(GL_SCISSOR_TEST);
}

void Graphics::drawToStencilBuffer(StencilAction action, int value)
{
	if (!isCanvasActive() && !windowHasStencil)
		throw love::Exception("The window must have stenciling enabled to draw to the main screen's stencil buffer.");

	flushStreamDraws();

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
	if (compare != COMPARE_ALWAYS && !isCanvasActive() && !windowHasStencil)
		throw love::Exception("The window must have stenciling enabled to use setStencilTest on the main screen.");

	DisplayState &state = states.back();

	if (state.stencilCompare != compare || state.stencilTestValue != value)
		flushStreamDraws();

	state.stencilCompare = compare;
	state.stencilTestValue = value;

	if (writingToStencil)
		return;

	if (compare == COMPARE_ALWAYS)
	{
		glDisable(GL_STENCIL_TEST);
		return;
	}

	/**
	 * OpenGL / GPUs do the comparison in the opposite way that makes sense
	 * for this API. For example, if the compare function is GL_GREATER then the
	 * stencil test will pass if the reference value is greater than the value
	 * in the stencil buffer. With our API it's more intuitive to assume that
	 * setStencilTest(COMPARE_GREATER, 4) will make it pass if the stencil
	 * buffer has a value greater than 4.
	 **/
	GLenum glcompare = OpenGL::getGLCompareMode(getReversedCompareMode(compare));

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(glcompare, value, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void Graphics::setStencilTest()
{
	setStencilTest(COMPARE_ALWAYS, 0);
}

void Graphics::clearStencil(int value)
{
	glClearStencil(value);
	glClear(GL_STENCIL_BUFFER_BIT);
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
	if (mode != states.back().blendMode || alphamode != states.back().blendAlphaMode)
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

Graphics::Renderer Graphics::getRenderer() const
{
	return GLAD_ES_VERSION_2_0 ? RENDERER_OPENGLES : RENDERER_OPENGL;
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
	case LIMIT_POINT_SIZE:
		return (double) gl.getMaxPointSize();
	case LIMIT_TEXTURE_SIZE:
		return (double) gl.getMax2DTextureSize();
	case LIMIT_TEXTURE_LAYERS:
		return (double) gl.getMaxTextureLayers();
	case LIMIT_VOLUME_TEXTURE_SIZE:
		return (double) gl.getMax3DTextureSize();
	case LIMIT_CUBE_TEXTURE_SIZE:
		return (double) gl.getMaxCubeTextureSize();
	case LIMIT_MULTI_CANVAS:
		return (double) gl.getMaxRenderTargets();
	case LIMIT_CANVAS_MSAA:
		return (double) gl.getMaxRenderbufferSamples();
	case LIMIT_ANISOTROPY:
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
	case FEATURE_ARRAY_TEXTURE:
		return gl.isTextureTypeSupported(TEXTURE_2D_ARRAY);
	case FEATURE_VOLUME_TEXTURE:
		return gl.isTextureTypeSupported(TEXTURE_VOLUME);
	case FEATURE_GLSL3:
		return GLAD_ES_VERSION_3_0 || gl.isCoreProfile();
	case FEATURE_INSTANCING:
		return gl.isInstancingSupported();
	default:
		return false;
	}
}

bool Graphics::isCanvasFormatSupported(PixelFormat format) const
{
	return Canvas::isFormatSupported(format);
}

bool Graphics::isCanvasFormatSupported(PixelFormat format, bool readable) const
{
	return Canvas::isFormatSupported(format, readable);
}

bool Graphics::isImageFormatSupported(PixelFormat format) const
{
	return Image::isFormatSupported(format);
}

Shader::Language Graphics::getShaderLanguageTarget() const
{
	if (gl.isCoreProfile())
		return Shader::LANGUAGE_GLSL3;
	else if (GLAD_ES_VERSION_3_0)
		return Shader::LANGUAGE_ESSL3;
	else if (GLAD_ES_VERSION_2_0)
		return Shader::LANGUAGE_ESSL1;
	else
		return Shader::LANGUAGE_GLSL1;
}

} // opengl
} // graphics
} // love
