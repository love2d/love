/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "ShaderStage.h"

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
	: windowHasStencil(false)
	, mainVAO(0)
{
	gl = OpenGL();
	Canvas::resetFormatSupport();

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

love::graphics::Canvas *Graphics::newCanvas(const Canvas::Settings &settings)
{
	return new Canvas(settings);
}

love::graphics::ShaderStage *Graphics::newShaderStageInternal(ShaderStage::StageType stage, const std::string &cachekey, const std::string &source, bool gles)
{
	return new ShaderStage(this, stage, source, gles, cachekey);
}

love::graphics::Shader *Graphics::newShaderInternal(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
{
	return new Shader(vertex, pixel);
}

love::graphics::Buffer *Graphics::newBuffer(size_t size, const void *data, BufferType type, vertex::Usage usage, uint32 mapflags)
{
	return new Buffer(size, data, type, usage, mapflags);
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
		projectionMatrix = Matrix4::ortho(0.0, (float) width, (float) height, 0.0, -10.0f, 10.0f);
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
	initCapabilities();

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
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// Always enable seamless cubemap filtering when possible.
	if (GLAD_VERSION_3_2 || GLAD_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Set whether drawing converts input from linear -> sRGB colorspace.
	if (!gl.bugs.brokenSRGB && (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_sRGB
		|| GLAD_EXT_framebuffer_sRGB || GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB))
	{
		if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
			gl.setEnableState(OpenGL::ENABLE_FRAMEBUFFER_SRGB, isGammaCorrect());
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

	createQuadIndexBuffer();

	// Restore the graphics state.
	restoreState(states.back());

	int gammacorrect = isGammaCorrect() ? 1 : 0;
	Shader::Language target = getShaderLanguageTarget();

	// We always need a default shader.
	for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++)
	{
		if (i == Shader::STANDARD_ARRAY && !capabilities.textureTypes[TEXTURE_2D_ARRAY])
			continue;

		// Apparently some intel GMA drivers on windows fail to compile shaders
		// which use array textures despite claiming support for the extension.
		try
		{
			if (!Shader::standardShaders[i])
			{
				const auto &code = defaultShaderCode[i][target][gammacorrect];
				Shader::standardShaders[i] = love::graphics::Graphics::newShader(code.source[ShaderStage::STAGE_VERTEX], code.source[ShaderStage::STAGE_PIXEL]);
			}
		}
		catch (love::Exception &)
		{
			if (i == Shader::STANDARD_ARRAY)
				capabilities.textureTypes[TEXTURE_2D_ARRAY] = false;
			else
				throw;
		}
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

	for (auto temp : temporaryCanvases)
		temp.canvas->release();

	framebufferObjects.clear();
	temporaryCanvases.clear();

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

void Graphics::draw(const DrawCommand &cmd)
{
	gl.prepareDraw();
	gl.setVertexAttributes(*cmd.attributes, *cmd.buffers);
	gl.bindTextureToUnit(cmd.texture, 0, false);
	gl.setCullMode(cmd.cullMode);

	GLenum glprimitivetype = OpenGL::getGLPrimitiveType(cmd.primitiveType);

	if (cmd.instanceCount > 1)
		glDrawArraysInstanced(glprimitivetype, cmd.vertexStart, cmd.vertexCount, cmd.instanceCount);
	else
		glDrawArrays(glprimitivetype, cmd.vertexStart, cmd.vertexCount);

	++drawCalls;
}

void Graphics::draw(const DrawIndexedCommand &cmd)
{
	gl.prepareDraw();
	gl.setVertexAttributes(*cmd.attributes, *cmd.buffers);
	gl.bindTextureToUnit(cmd.texture, 0, false);
	gl.setCullMode(cmd.cullMode);

	const void *gloffset = BUFFER_OFFSET(cmd.indexBufferOffset);
	GLenum glprimitivetype = OpenGL::getGLPrimitiveType(cmd.primitiveType);
	GLenum gldatatype = OpenGL::getGLIndexDataType(cmd.indexType);

	gl.bindBuffer(BUFFER_INDEX, cmd.indexBuffer->getHandle());

	if (cmd.instanceCount > 1)
		glDrawElementsInstanced(glprimitivetype, cmd.indexCount, gldatatype, gloffset, cmd.instanceCount);
	else
		glDrawElements(glprimitivetype, cmd.indexCount, gldatatype, gloffset);

	++drawCalls;
}

static inline void advanceVertexOffsets(const vertex::Attributes &attributes, vertex::BufferBindings &buffers, int vertexcount)
{
	// TODO: Figure out a better way to avoid touching the same buffer multiple
	// times, if multiple attributes share the buffer.
	uint32 touchedbuffers = 0;

	for (unsigned int i = 0; i < vertex::Attributes::MAX; i++)
	{
		if (!attributes.isEnabled(i))
			continue;

		auto &attrib = attributes.attribs[i];

		uint32 bufferbit = 1u << attrib.bufferIndex;
		if ((touchedbuffers & bufferbit) == 0)
		{
			touchedbuffers |= bufferbit;
			const auto &layout = attributes.bufferLayouts[attrib.bufferIndex];
			buffers.info[attrib.bufferIndex].offset += layout.stride * vertexcount;
		}
	}
}

void Graphics::drawQuads(int start, int count, const vertex::Attributes &attributes, const vertex::BufferBindings &buffers, love::graphics::Texture *texture)
{
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW    = MAX_VERTICES_PER_DRAW / 4;

	gl.prepareDraw();
	gl.bindTextureToUnit(texture, 0, false);
	gl.setCullMode(CULL_NONE);

	gl.bindBuffer(BUFFER_INDEX, quadIndexBuffer->getHandle());

	if (gl.isBaseVertexSupported())
	{
		gl.setVertexAttributes(attributes, buffers);

		int basevertex = start * 4;

		for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
		{
			int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

			glDrawElementsBaseVertex(GL_TRIANGLES, quadcount * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), basevertex);
			++drawCalls;

			basevertex += quadcount * 4;
		}
	}
	else
	{
		vertex::BufferBindings bufferscopy = buffers;
		if (start > 0)
			advanceVertexOffsets(attributes, bufferscopy, start * 4);

		for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
		{
			gl.setVertexAttributes(attributes, bufferscopy);

			int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

			glDrawElements(GL_TRIANGLES, quadcount * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
			++drawCalls;

			if (count > MAX_QUADS_PER_DRAW)
				advanceVertexOffsets(attributes, bufferscopy, quadcount * 4);
		}
	}
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

void Graphics::setCanvasInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBcanvas)
{
	const DisplayState &state = states.back();

	OpenGL::TempDebugGroup debuggroup("setCanvas");

	flushStreamDraws();
	endPass();

	bool iswindow = rts.getFirstTarget().canvas == nullptr;
	vertex::Winding vertexwinding = state.winding;

	if (iswindow)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, gl.getDefaultFBO());

		// The projection matrix is flipped compared to rendering to a canvas, due
		// to OpenGL considering (0,0) bottom-left instead of top-left.
		projectionMatrix = Matrix4::ortho(0.0, (float) w, (float) h, 0.0, -10.0f, 10.0f);
	}
	else
	{
		bindCachedFBO(rts);

		projectionMatrix = Matrix4::ortho(0.0, (float) w, 0.0, (float) h, -10.0f, 10.0f);

		// Flip front face winding when rendering to a canvas, since our
		// projection matrix is flipped.
		vertexwinding = vertexwinding == vertex::WINDING_CW ? vertex::WINDING_CCW : vertex::WINDING_CW;
	}

	glFrontFace(vertexwinding == vertex::WINDING_CW ? GL_CW : GL_CCW);

	gl.setViewport({0, 0, pixelw, pixelh});

	// Re-apply the scissor if it was active, since the rectangle passed to
	// glScissor is affected by the viewport dimensions.
	if (state.scissor)
		setScissor(state.scissorRect);

	// Make sure the correct sRGB setting is used when drawing to the canvases.
	if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
	{
		if (hasSRGBcanvas != gl.isStateEnabled(OpenGL::ENABLE_FRAMEBUFFER_SRGB))
			gl.setEnableState(OpenGL::ENABLE_FRAMEBUFFER_SRGB, hasSRGBcanvas);
	}
}

void Graphics::endPass()
{
	auto &rts = states.back().renderTargets;
	love::graphics::Canvas *depthstencil = rts.depthStencil.canvas.get();

	// Discard the depth/stencil buffer if we're using an internal cached one.
	if (depthstencil == nullptr && (rts.temporaryRTFlags & (TEMPORARY_RT_DEPTH | TEMPORARY_RT_STENCIL)) != 0)
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

	if (depthstencil != nullptr && depthstencil->getMSAA() > 1 && depthstencil->isReadable())
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, ((Canvas *) depthstencil)->getFBO());

		if (GLAD_APPLE_framebuffer_multisample)
			glResolveMultisampleFramebufferAPPLE();
		else
		{
			int mip = rts.depthStencil.mipmap;
			int w = depthstencil->getPixelWidth(mip);
			int h = depthstencil->getPixelHeight(mip);
			PixelFormat format = depthstencil->getPixelFormat();

			GLbitfield mask = 0;

			if (isPixelFormatDepth(format))
				mask |= GL_DEPTH_BUFFER_BIT;
			if (isPixelFormatStencil(format))
				mask |= GL_STENCIL_BUFFER_BIT;

			if (mask != 0)
				glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, mask, GL_NEAREST);
		}
	}

	for (const auto &rt : rts.colors)
	{
		if (rt.canvas->getMipmapMode() == Canvas::MIPMAPS_AUTO && rt.mipmap == 0)
			rt.canvas->generateMipmaps();
	}
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

	bool hadDepthWrites = gl.hasDepthWrites();

	if (depth.hasValue)
	{
		if (!hadDepthWrites) // glDepthMask also affects glClear.
			gl.setDepthWrites(true);

		gl.clearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
		glClear(flags);

	if (depth.hasValue && !hadDepthWrites)
		gl.setDepthWrites(hadDepthWrites);

	if (c.hasValue && gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
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

	int ncolorcanvases = (int) states.back().renderTargets.colors.size();
	int ncolors = (int) colors.size();

	if (ncolors <= 1 && ncolorcanvases <= 1)
	{
		clear(ncolors > 0 ? colors[0] : OptionalColorf(), stencil, depth);
		return;
	}

	flushStreamDraws();

	bool drawbuffersmodified = false;
	ncolors = std::min(ncolors, ncolorcanvases);

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

		for (int i = 0; i < ncolorcanvases; i++)
			bufs[i] = GL_COLOR_ATTACHMENT0 + i;

		glDrawBuffers(ncolorcanvases, bufs);
	}

	GLbitfield flags = 0;

	if (stencil.hasValue)
	{
		glClearStencil(stencil.value);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	bool hadDepthWrites = gl.hasDepthWrites();

	if (depth.hasValue)
	{
		if (!hadDepthWrites) // glDepthMask also affects glClear.
			gl.setDepthWrites(true);

		gl.clearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
		glClear(flags);

	if (depth.hasValue && !hadDepthWrites)
		gl.setDepthWrites(hadDepthWrites);

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

void Graphics::cleanupCanvas(Canvas *canvas)
{
	for (auto it = framebufferObjects.begin(); it != framebufferObjects.end(); /**/)
	{
		bool hascanvas = false;
		const auto &rts = it->first;

		for (const RenderTarget &rt : rts.colors)
		{
			if (rt.canvas == canvas)
			{
				hascanvas = true;
				break;
			}
		}

		hascanvas = hascanvas || rts.depthStencil.canvas == canvas;

		if (hascanvas)
		{
			if (isCreated())
				gl.deleteFramebuffer(it->second);
			it = framebufferObjects.erase(it);
		}
		else
			++it;
	}
}

void Graphics::bindCachedFBO(const RenderTargets &targets)
{
	GLuint fbo = framebufferObjects[targets];

	if (fbo != 0)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);
	}
	else
	{
		int msaa = targets.getFirstTarget().canvas->getMSAA();
		bool hasDS = targets.depthStencil.canvas != nullptr;

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

		if (hasDS)
			attachCanvas(targets.depthStencil);

		if (ncolortargets > 1)
			glDrawBuffers(ncolortargets, drawbuffers);
		else if (ncolortargets == 0 && hasDS && (GLAD_ES_VERSION_3_0 || !GLAD_ES_VERSION_2_0))
		{
			// glDrawBuffers is an ext in GL2. glDrawBuffer doesn't exist in ES3.
			GLenum none = GL_NONE;
			if (GLAD_ES_VERSION_3_0)
				glDrawBuffers(1, &none);
			else
				glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			gl.deleteFramebuffer(fbo);
			const char *sstr = OpenGL::framebufferStatusString(status);
			throw love::Exception("Could not create Framebuffer Object! %s", sstr);
		}

		framebufferObjects[targets] = fbo;
	}
}

void Graphics::present(void *screenshotCallbackData)
{
	if (!isActive())
		return;

	if (isCanvasActive())
		throw love::Exception("present cannot be called while a Canvas is active.");

	deprecations.draw(this);

	flushStreamDraws();
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
				info.callback(&info, nullptr, nullptr);
				for (int j = i + 1; j < (int) pendingScreenshotCallbacks.size(); j++)
				{
					const auto &ninfo = pendingScreenshotCallbacks[j];
					ninfo.callback(&ninfo, nullptr, nullptr);
				}
				pendingScreenshotCallbacks.clear();
				throw;
			}

			info.callback(&info, img, screenshotCallbackData);
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

	for (StreamBuffer *buffer : streamBufferState.vb)
		buffer->nextFrame();
	streamBufferState.indexBuffer->nextFrame();

	auto window = getInstance<love::window::Window>(M_WINDOW);
	if (window != nullptr)
		window->swapBuffers();

	// Reset the per-frame stat counts.
	drawCalls = 0;
	gl.stats.shaderSwitches = 0;
	canvasSwitchCount = 0;
	drawCallsBatched = 0;

	// This assumes temporary canvases will only be used within a render pass.
	for (int i = (int) temporaryCanvases.size() - 1; i >= 0; i--)
	{
		if (temporaryCanvases[i].framesSinceUse >= MAX_TEMPORARY_CANVAS_UNUSED_FRAMES)
		{
			temporaryCanvases[i].canvas->release();
			temporaryCanvases[i] = temporaryCanvases.back();
			temporaryCanvases.pop_back();
		}
		else
			temporaryCanvases[i].framesSinceUse++;
	}
}

void Graphics::setScissor(const Rect &rect)
{
	flushStreamDraws();

	DisplayState &state = states.back();

	if (!gl.isStateEnabled(OpenGL::ENABLE_SCISSOR_TEST))
		gl.setEnableState(OpenGL::ENABLE_SCISSOR_TEST, true);

	double dpiscale = getCurrentDPIScale();

	Rect glrect;
	glrect.x = (int) (rect.x * dpiscale);
	glrect.y = (int) (rect.y * dpiscale);
	glrect.w = (int) (rect.w * dpiscale);
	glrect.h = (int) (rect.h * dpiscale);

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

	if (gl.isStateEnabled(OpenGL::ENABLE_SCISSOR_TEST))
		gl.setEnableState(OpenGL::ENABLE_SCISSOR_TEST, false);
}

void Graphics::drawToStencilBuffer(StencilAction action, int value)
{
	const auto &rts = states.back().renderTargets;
	love::graphics::Canvas *dscanvas = rts.depthStencil.canvas.get();

	if (!isCanvasActive() && !windowHasStencil)
		throw love::Exception("The window must have stenciling enabled to draw to the main screen's stencil buffer.");
	else if (isCanvasActive() && (rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) == 0 && (dscanvas == nullptr || !isPixelFormatStencil(dscanvas->getPixelFormat())))
		throw love::Exception("Drawing to the stencil buffer with a Canvas active requires either stencil=true or a custom stencil-type Canvas to be used, in setCanvas.");

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
	if (!gl.isStateEnabled(OpenGL::ENABLE_STENCIL_TEST))
		gl.setEnableState(OpenGL::ENABLE_STENCIL_TEST, true);

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

	if (state.stencilCompare != compare || state.stencilTestValue != value)
		flushStreamDraws();

	state.stencilCompare = compare;
	state.stencilTestValue = value;

	if (writingToStencil)
		return;

	if (compare == COMPARE_ALWAYS)
	{
		if (gl.isStateEnabled(OpenGL::ENABLE_STENCIL_TEST))
			gl.setEnableState(OpenGL::ENABLE_STENCIL_TEST, false);
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

	if (!gl.isStateEnabled(OpenGL::ENABLE_STENCIL_TEST))
		gl.setEnableState(OpenGL::ENABLE_STENCIL_TEST, true);

	glStencilFunc(glcompare, value, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
	DisplayState &state = states.back();

	if (state.depthTest != compare || state.depthWrite != write)
		flushStreamDraws();

	state.depthTest = compare;
	state.depthWrite = write;

	bool depthenable = compare != COMPARE_ALWAYS || write;

	if (depthenable != gl.isStateEnabled(OpenGL::ENABLE_DEPTH_TEST))
		gl.setEnableState(OpenGL::ENABLE_DEPTH_TEST, depthenable);

	if (depthenable)
	{
		glDepthFunc(OpenGL::getGLCompareMode(compare));
		gl.setDepthWrites(write);
	}
}

void Graphics::setFrontFaceWinding(vertex::Winding winding)
{
	DisplayState &state = states.back();

	if (state.winding != winding)
		flushStreamDraws();

	state.winding = winding;

	if (isCanvasActive())
		winding = winding == vertex::WINDING_CW ? vertex::WINDING_CCW : vertex::WINDING_CW;

	glFrontFace(winding == vertex::WINDING_CW ? GL_CW : GL_CCW);
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

	if (mode == BLEND_LIGHTEN || mode == BLEND_DARKEN)
	{
		if (!capabilities.features[FEATURE_LIGHTEN])
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

	GLenum func   = GL_FUNC_ADD;
	GLenum srcRGB = GL_ONE;
	GLenum srcA   = GL_ONE;
	GLenum dstRGB = GL_ZERO;
	GLenum dstA   = GL_ZERO;

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
	if (streamBufferState.primitiveMode == PRIMITIVE_POINTS)
		flushStreamDraws();

	gl.setPointSize(size * getCurrentDPIScale());
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

void Graphics::getAPIStats(int &shaderswitches) const
{
	shaderswitches = gl.stats.shaderSwitches;
}

void Graphics::initCapabilities()
{
	capabilities.features[FEATURE_MULTI_CANVAS_FORMATS] = Canvas::isMultiFormatMultiCanvasSupported();
	capabilities.features[FEATURE_CLAMP_ZERO] = gl.isClampZeroTextureWrapSupported();
	capabilities.features[FEATURE_LIGHTEN] = GLAD_VERSION_1_4 || GLAD_ES_VERSION_3_0 || GLAD_EXT_blend_minmax;
	capabilities.features[FEATURE_FULL_NPOT] = GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = gl.isPixelShaderHighpSupported();
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_standard_derivatives;
	capabilities.features[FEATURE_GLSL3] = GLAD_ES_VERSION_3_0 || gl.isCoreProfile();
	capabilities.features[FEATURE_INSTANCING] = gl.isInstancingSupported();
	static_assert(FEATURE_MAX_ENUM == 8, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	capabilities.limits[LIMIT_POINT_SIZE] = gl.getMaxPointSize();
	capabilities.limits[LIMIT_TEXTURE_SIZE] = gl.getMax2DTextureSize();
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = gl.getMaxTextureLayers();
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = gl.getMax3DTextureSize();
	capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = gl.getMaxCubeTextureSize();
	capabilities.limits[LIMIT_MULTI_CANVAS] = gl.getMaxRenderTargets();
	capabilities.limits[LIMIT_CANVAS_MSAA] = gl.getMaxRenderbufferSamples();
	capabilities.limits[LIMIT_ANISOTROPY] = gl.getMaxAnisotropy();
	static_assert(LIMIT_MAX_ENUM == 8, "Graphics::initCapabilities must be updated when adding a new system limit!");

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
		capabilities.textureTypes[i] = gl.isTextureTypeSupported((TextureType) i);
}

bool Graphics::isCanvasFormatSupported(PixelFormat format) const
{
	return Canvas::isFormatSupported(format);
}

bool Graphics::isCanvasFormatSupported(PixelFormat format, bool readable) const
{
	return Canvas::isFormatSupported(format, readable);
}

bool Graphics::isImageFormatSupported(PixelFormat format, bool sRGB) const
{
	return Image::isFormatSupported(format, sRGB);
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
