/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "GraphicsReadback.h"
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
#include <SDL3/SDL_video.h>
#endif

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum getGLBlendOperation(BlendOperation op)
{
	switch (op)
	{
		case BLENDOP_ADD: return GL_FUNC_ADD;
		case BLENDOP_SUBTRACT: return GL_FUNC_SUBTRACT;
		case BLENDOP_REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
		case BLENDOP_MIN: return GL_MIN;
		case BLENDOP_MAX: return GL_MAX;
		case BLENDOP_MAX_ENUM: return 0;
	}
	return 0;
}

static GLenum getGLBlendFactor(BlendFactor factor)
{
	switch (factor)
	{
		case BLENDFACTOR_ZERO: return GL_ZERO;
		case BLENDFACTOR_ONE: return GL_ONE;
		case BLENDFACTOR_SRC_COLOR: return GL_SRC_COLOR;
		case BLENDFACTOR_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case BLENDFACTOR_SRC_ALPHA: return GL_SRC_ALPHA;
		case BLENDFACTOR_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case BLENDFACTOR_DST_COLOR: return GL_DST_COLOR;
		case BLENDFACTOR_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case BLENDFACTOR_DST_ALPHA: return GL_DST_ALPHA;
		case BLENDFACTOR_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case BLENDFACTOR_SRC_ALPHA_SATURATED: return GL_SRC_ALPHA_SATURATE;
		case BLENDFACTOR_MAX_ENUM: return 0;
	}
	return 0;
}

love::graphics::Graphics *createInstance()
{
	love::graphics::Graphics *instance = nullptr;

	try
	{
		instance = new Graphics();
	}
	catch (love::Exception &e)
	{
		printf("Cannot create OpenGL renderer: %s\n", e.what());
	}

	return instance;
}

Graphics::Graphics()
	: love::graphics::Graphics("love.graphics.opengl")
	, windowHasStencil(false)
	, mainVAO(0)
	, internalBackbufferFBO(0)
	, requestedBackbufferMSAA(0)
	, bufferMapMemory(nullptr)
	, bufferMapMemorySize(2 * 1024 * 1024)
	, pixelFormatUsage()
{
	gl = OpenGL();

	try
	{
		bufferMapMemory = new char[bufferMapMemorySize];
	}
	catch (std::exception &)
	{
		// Handled in getBufferMapMemory.
	}

	auto window = getInstance<love::window::Window>(M_WINDOW);

	if (window != nullptr)
	{
		window->setGraphics(this);

		// Recreate the window using the current renderer, if needed.
		if (window->isOpen())
		{
			int w, h;
			love::window::WindowSettings settings;
			window->getWindow(w, h, settings);
			window->setWindow(w, h, &settings);
		}
	}
}

Graphics::~Graphics()
{
	delete[] bufferMapMemory;
}

love::graphics::StreamBuffer *Graphics::newStreamBuffer(BufferUsage type, size_t size)
{
	return CreateStreamBuffer(type, size);
}

love::graphics::Texture *Graphics::newTexture(const Texture::Settings &settings, const Texture::Slices *data)
{
	return new Texture(this, settings, data);
}

love::graphics::Texture *Graphics::newTextureView(love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
{
	return new Texture(this, base, viewsettings);
}

love::graphics::ShaderStage *Graphics::newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles)
{
	return new ShaderStage(this, stage, source, gles, cachekey);
}

love::graphics::Shader *Graphics::newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const Shader::CompileOptions &options)
{
	return new Shader(stages, options);
}

love::graphics::Buffer *Graphics::newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
{
	return new Buffer(this, settings, format, data, size, arraylength);
}

love::graphics::GraphicsReadback *Graphics::newReadbackInternal(ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset)
{
	return new GraphicsReadback(this, method, buffer, offset, size, dest, destoffset);
}

love::graphics::GraphicsReadback *Graphics::newReadbackInternal(ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty)
{
	return new GraphicsReadback(this, method, texture, slice, mipmap, rect, dest, destx, desty);
}

void Graphics::backbufferChanged(int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa)
{
	bool changed = width != this->width || height != this->height
		|| pixelwidth != this->pixelWidth || pixelheight != this->pixelHeight;

	changed |= backbufferstencil != this->backbufferHasStencil || backbufferdepth != this->backbufferHasDepth;
	changed |= msaa != this->requestedBackbufferMSAA;

	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	this->backbufferHasStencil = backbufferstencil;
	this->backbufferHasDepth = backbufferdepth;
	this->requestedBackbufferMSAA = msaa;

	if (!isRenderTargetActive())
	{
		// Set the viewport to top-left corner.
		gl.setViewport({0, 0, pixelwidth, pixelheight});

		// Re-apply the scissor if it was active, since the rectangle passed to
		// glScissor is affected by the viewport dimensions.
		if (states.back().scissor)
			setScissor(states.back().scissorRect);

		resetProjection();
	}

	if (!changed)
		return;

	bool useinternalbackbuffer = false;
	if (msaa > 1)
		useinternalbackbuffer = true;

	GLuint prevFBO = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);
	bool restoreFBO = prevFBO != getInternalBackbufferFBO();

	if (useinternalbackbuffer)
	{
		Texture::Settings settings;
		settings.width = width;
		settings.height = height;
		settings.dpiScale = (float)pixelheight / (float)height;
		settings.msaa = msaa;
		settings.renderTarget = true;
		settings.readable.set(false);

		settings.format = isGammaCorrect() ? PIXELFORMAT_RGBA8_sRGB : PIXELFORMAT_RGBA8_UNORM;
		internalBackbuffer.set(newTexture(settings), Acquire::NORETAIN);

		internalBackbufferDepthStencil.set(nullptr);
		if (backbufferstencil || backbufferdepth)
		{
			if (backbufferstencil && backbufferdepth)
				settings.format = PIXELFORMAT_DEPTH24_UNORM_STENCIL8;
			else if (backbufferstencil)
				settings.format = PIXELFORMAT_STENCIL8;
			else if (backbufferdepth)
				settings.format = PIXELFORMAT_DEPTH24_UNORM;
			internalBackbufferDepthStencil.set(newTexture(settings), Acquire::NORETAIN);
		}

		RenderTargets rts;
		rts.colors.push_back(internalBackbuffer.get());
		rts.depthStencil.texture = internalBackbufferDepthStencil;

		internalBackbufferFBO = bindCachedFBO(rts);
	}
	else
	{
		internalBackbuffer.set(nullptr);
		internalBackbufferDepthStencil.set(nullptr);
		internalBackbufferFBO = 0;
	}

	if (restoreFBO)
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, prevFBO);
}

GLuint Graphics::getInternalBackbufferFBO() const
{
	if (internalBackbufferFBO != 0)
		return internalBackbufferFBO;
	else
		return getSystemBackbufferFBO();
}

GLuint Graphics::getSystemBackbufferFBO() const
{
#ifdef LOVE_IOS
	// Hack: iOS uses a custom FBO.
	SDL_PropertiesID props = SDL_GetWindowProperties(SDL_GL_GetCurrentWindow());
	GLuint resolveframebuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RESOLVE_FRAMEBUFFER_NUMBER, 0);
	if (resolveframebuffer != 0)
		return resolveframebuffer;
	else
		return (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0);
#else
	return 0;
#endif
}

bool Graphics::setMode(void */*context*/, int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa)
{
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

	// Enable blending
	gl.setEnableState(OpenGL::ENABLE_BLEND, true);

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

	if (!GLAD_ES_VERSION_2_0)
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	gl.setTextureUnit(0);

	// Set pixel row alignment - code that calls glTexSubImage and glReadPixels
	// assumes there's no row alignment, but OpenGL defaults to 4 bytes.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// Always enable seamless cubemap filtering when possible.
	if (GLAD_VERSION_3_2 || GLAD_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Set whether drawing converts input from linear -> sRGB colorspace.
	if (!gl.bugs.brokenSRGB)
	{
		if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
			gl.setEnableState(OpenGL::ENABLE_FRAMEBUFFER_SRGB, isGammaCorrect());
	}
	else
		setGammaCorrect(false);

	setDebug(isDebugEnabled());

	backbufferChanged(width, height, pixelwidth, pixelheight, backbufferstencil, backbufferdepth, msaa);

	if (batchedDrawState.vb[0] == nullptr)
	{
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		batchedDrawState.vb[0] = CreateStreamBuffer(BUFFERUSAGE_VERTEX, 1024 * 1024 * 1);
		batchedDrawState.vb[1] = CreateStreamBuffer(BUFFERUSAGE_VERTEX, 256  * 1024 * 1);
		batchedDrawState.indexBuffer = CreateStreamBuffer(BUFFERUSAGE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
	}

	// Reload all volatile objects.
	if (!Volatile::loadAll())
		::printf("Could not reload all volatile objects.\n");

	createQuadIndexBuffer();

	// Restore the graphics state.
	restoreState(states.back());

	// We always need a default shader.
	for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++)
	{
		auto stype = (Shader::StandardShader) i;

		if (!Shader::standardShaders[i])
		{
			std::vector<std::string> stages;
			Shader::CompileOptions opts;
			stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_VERTEX));
			stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_PIXEL));

			try
			{
				Shader::standardShaders[i] = newShader(stages, opts);
			}
			catch (love::Exception &)
			{
				// Attempted workaround for nvidia driver bug affecting old GPUs
				// on Windows (e.g. the 300 series).
				if (!isUsingNoTextureCubeShadowBiasHack())
				{
					usingNoTextureCubeShadowBiasHack = true;
					Shader::standardShaders[i] = newShader(stages, opts);
				}
				else
				{
					throw;
				}
			}
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

	flushBatchedDraws();

	internalBackbuffer.set(nullptr);
	internalBackbufferDepthStencil.set(nullptr);

	// Unload all volatile objects. These must be reloaded after the display
	// mode change.
	Volatile::unloadAll();

	clearTemporaryResources();

	for (const auto &pair : framebufferObjects)
		gl.deleteFramebuffer(pair.second);

	framebufferObjects.clear();

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
	flushBatchedDraws();

	// Make sure all pending OpenGL commands have fully executed before
	// returning, when going from active to inactive. This is required on iOS.
	if (isCreated() && this->active && !enable)
		glFinish();

	active = enable;
}

static bool computeDispatchBarriers(Shader *shader, GLbitfield &preDispatchBarriers, GLbitfield &postDispatchBarriers)
{
	for (auto buffer : shader->getActiveWritableStorageBuffers())
	{
		if (buffer == nullptr)
			return false;

		auto usage = buffer->getUsageFlags();

		postDispatchBarriers |= GL_BUFFER_UPDATE_BARRIER_BIT;

		if (usage & BUFFERUSAGEFLAG_SHADER_STORAGE)
		{
			preDispatchBarriers |= GL_SHADER_STORAGE_BARRIER_BIT;
			postDispatchBarriers |= GL_SHADER_STORAGE_BARRIER_BIT;
		}

		// TODO: does this need a pre dispatch barrier too?
		if (usage & BUFFERUSAGEFLAG_INDIRECT_ARGUMENTS)
			postDispatchBarriers |= GL_COMMAND_BARRIER_BIT;

		if (usage & BUFFERUSAGEFLAG_TEXEL)
			postDispatchBarriers |= GL_TEXTURE_FETCH_BARRIER_BIT;

		if (usage & BUFFERUSAGEFLAG_INDEX)
			postDispatchBarriers |= GL_ELEMENT_ARRAY_BARRIER_BIT;

		if (usage & BUFFERUSAGEFLAG_VERTEX)
			postDispatchBarriers |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;

		postDispatchBarriers |= GL_PIXEL_BUFFER_BARRIER_BIT;
	}

	for (const auto &binding : shader->getStorageTextureBindings())
	{
		if (binding.texture == nullptr)
			return false;

		if (binding.access == GL_READ_ONLY)
			continue;

		preDispatchBarriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;

		postDispatchBarriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
			| GL_TEXTURE_UPDATE_BARRIER_BIT
			| GL_TEXTURE_FETCH_BARRIER_BIT;

		if (binding.texture->isRenderTarget())
			postDispatchBarriers |= GL_FRAMEBUFFER_BARRIER_BIT;
	}

	return true;
}

bool Graphics::dispatch(love::graphics::Shader *s, int x, int y, int z)
{
	auto shader = (Shader *) s;

	GLbitfield preDispatchBarriers = 0;
	GLbitfield postDispatchBarriers = 0;

	if (!computeDispatchBarriers(shader, preDispatchBarriers, postDispatchBarriers))
		return false;

	// glMemoryBarrier before dispatch to make sure non-compute-read ->
	// compute-write is synced.
	// TODO: is this needed? spec language around GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
	// makes me think so.
	// This is overly conservative (dispatch -> dispatch will have redundant
	// barriers).
	if (preDispatchBarriers != 0)
		glMemoryBarrier(preDispatchBarriers);

	glDispatchCompute(x, y, z);

	// Not as (theoretically) efficient as issuing the barrier right before
	// they're used later, but much less complicated.
	if (postDispatchBarriers != 0)
		glMemoryBarrier(postDispatchBarriers);

	return true;
}

bool Graphics::dispatch(love::graphics::Shader *s, love::graphics::Buffer *indirectargs, size_t argsoffset)
{
	auto shader = (Shader *) s;

	GLbitfield preDispatchBarriers = 0;
	GLbitfield postDispatchBarriers = 0;

	if (!computeDispatchBarriers(shader, preDispatchBarriers, postDispatchBarriers))
		return false;

	if (preDispatchBarriers != 0)
		glMemoryBarrier(preDispatchBarriers);

	// Note: OpenGL has separate bind points for draw versus dispatch indirect
	// buffers. Our gl.bindBuffer wrapper uses the draw bind point, so we can't
	// use it here.
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, (GLuint)indirectargs->getHandle());
	glDispatchComputeIndirect(argsoffset);

	// Not as (theoretically) efficient as issuing the barrier right before
	// they're used later, but much less complicated.
	if (postDispatchBarriers != 0)
		glMemoryBarrier(postDispatchBarriers);

	return true;
}

void Graphics::draw(const DrawCommand &cmd)
{
	VertexAttributes attributes;
	findVertexAttributes(cmd.attributesID, attributes);

	gl.prepareDraw(this);
	gl.setVertexAttributes(attributes, *cmd.buffers);
	gl.bindTextureToUnit(cmd.texture, 0, false);
	gl.setCullMode(cmd.cullMode);

	GLenum glprimitivetype = OpenGL::getGLPrimitiveType(cmd.primitiveType);

	if (cmd.indirectBuffer != nullptr)
	{
		gl.bindBuffer(BUFFERUSAGE_INDIRECT_ARGUMENTS, (GLuint) cmd.indirectBuffer->getHandle());
		glDrawArraysIndirect(glprimitivetype, BUFFER_OFFSET(cmd.indirectBufferOffset));
	}
	else if (cmd.instanceCount > 1)
		glDrawArraysInstanced(glprimitivetype, cmd.vertexStart, cmd.vertexCount, cmd.instanceCount);
	else
		glDrawArrays(glprimitivetype, cmd.vertexStart, cmd.vertexCount);

	++drawCalls;
}

void Graphics::draw(const DrawIndexedCommand &cmd)
{
	VertexAttributes attributes;
	findVertexAttributes(cmd.attributesID, attributes);

	gl.prepareDraw(this);
	gl.setVertexAttributes(attributes, *cmd.buffers);
	gl.bindTextureToUnit(cmd.texture, 0, false);
	gl.setCullMode(cmd.cullMode);

	const void *gloffset = BUFFER_OFFSET(cmd.indexBufferOffset);
	GLenum glprimitivetype = OpenGL::getGLPrimitiveType(cmd.primitiveType);
	GLenum gldatatype = OpenGL::getGLIndexDataType(cmd.indexType);

	gl.bindBuffer(BUFFERUSAGE_INDEX, cmd.indexBuffer->getHandle());

	if (cmd.indirectBuffer != nullptr)
	{
		// Note: OpenGL doesn't support indirect indexed draws with a non-zero
		// index buffer offset.
		gl.bindBuffer(BUFFERUSAGE_INDIRECT_ARGUMENTS, (GLuint) cmd.indirectBuffer->getHandle());
		glDrawElementsIndirect(glprimitivetype, gldatatype, BUFFER_OFFSET(cmd.indirectBufferOffset));
	}
	else if (cmd.instanceCount > 1)
		glDrawElementsInstanced(glprimitivetype, cmd.indexCount, gldatatype, gloffset, cmd.instanceCount);
	else
		glDrawElements(glprimitivetype, cmd.indexCount, gldatatype, gloffset);

	++drawCalls;
}

static inline void advanceVertexOffsets(const VertexAttributes &attributes, BufferBindings &buffers, int vertexcount)
{
	// TODO: Figure out a better way to avoid touching the same buffer multiple
	// times, if multiple attributes share the buffer.
	uint32 touchedbuffers = 0;

	for (unsigned int i = 0; i < VertexAttributes::MAX; i++)
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

void Graphics::drawQuads(int start, int count, VertexAttributesID attributesID, const BufferBindings &buffers, love::graphics::Texture *texture)
{
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW    = MAX_VERTICES_PER_DRAW / 4;

	VertexAttributes attributes;
	findVertexAttributes(attributesID, attributes);

	gl.prepareDraw(this);
	gl.bindTextureToUnit(texture, 0, false);
	gl.setCullMode(CULL_NONE);

	gl.bindBuffer(BUFFERUSAGE_INDEX, quadIndexBuffer->getHandle());

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
		BufferBindings bufferscopy = buffers;
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
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	// Human-readable strings for the debug info.
	const char *sourceStr = OpenGL::debugSourceString(source);
	const char *typeStr = OpenGL::debugTypeString(type);
	const char *severityStr = OpenGL::debugSeverityString(severity);

	const char *fmt = "OpenGL: [source=%s, type=%s, severity=%s, id=%d]: %s\n";
	printf(fmt, sourceStr, typeStr, severityStr, id, msg);
}

void Graphics::setDebug(bool enable)
{
	// Make sure debug output is supported. The AMD ext. is a bit different
	// so we don't make use of it, since AMD drivers now support KHR_debug.
	if (!(GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2 || GLAD_KHR_debug || GLAD_ARB_debug_output))
		return;

	// TODO: We don't support GL_KHR_debug in GLES yet.
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_2)
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
		if (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2 || GLAD_KHR_debug)
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

	if (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2 || GLAD_KHR_debug)
		glEnable(GL_DEBUG_OUTPUT);

	::printf("OpenGL debug output enabled (LOVE_GRAPHICS_DEBUG=1)\n");
}

void Graphics::setRenderTargetsInternal(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture)
{
	const DisplayState &state = states.back();

	OpenGL::TempDebugGroup debuggroup("setRenderTargets");

	endPass(false);

	bool iswindow = rts.getFirstTarget().texture == nullptr;
	Winding vertexwinding = state.winding;

	if (iswindow)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getInternalBackbufferFBO());
	}
	else
	{
		bindCachedFBO(rts);

		// Flip front face winding when rendering to a texture, since our
		// projection matrix is flipped.
		// Note: projection matrix is set at a higher level.
		vertexwinding = vertexwinding == WINDING_CW ? WINDING_CCW : WINDING_CW;
	}

	glFrontFace(vertexwinding == WINDING_CW ? GL_CW : GL_CCW);

	gl.setViewport({0, 0, pixelw, pixelh});

	// Re-apply the scissor if it was active, since the rectangle passed to
	// glScissor is affected by the viewport dimensions.
	if (state.scissor)
		setScissor(state.scissorRect, !iswindow);

	// Make sure the correct sRGB setting is used when drawing to the textures.
	if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
	{
		if (hasSRGBtexture != gl.isStateEnabled(OpenGL::ENABLE_FRAMEBUFFER_SRGB))
			gl.setEnableState(OpenGL::ENABLE_FRAMEBUFFER_SRGB, hasSRGBtexture);
	}
}

void Graphics::endPass(bool presenting)
{
	auto &rts = states.back().renderTargets;
	love::graphics::Texture *depthstencil = rts.depthStencil.texture.get();

	// Discard the depth/stencil buffer if we're using an internal cached one,
	// or if we're presenting the backbuffer to the display.
	if ((depthstencil == nullptr && (rts.temporaryRTFlags & (TEMPORARY_RT_DEPTH | TEMPORARY_RT_STENCIL)) != 0)
		|| (presenting && !rts.getFirstTarget().texture.get()))
	{
		discard({}, true);
	}

	// Resolve MSAA buffers. MSAA is only supported for 2D render targets so we
	// don't have to worry about resolving to slices.
	if (rts.colors.size() > 0 && rts.colors[0].texture->getMSAA() > 1)
	{
		int mip = rts.colors[0].mipmap;
		int w = rts.colors[0].texture->getPixelWidth(mip);
		int h = rts.colors[0].texture->getPixelHeight(mip);

		for (int i = 0; i < (int) rts.colors.size(); i++)
		{
			Texture *c = (Texture *) rts.colors[i].texture.get();

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
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, ((Texture *) depthstencil)->getFBO());

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
}

void Graphics::clear(OptionalColorD c, OptionalInt stencil, OptionalDouble depth)
{
	if (c.hasValue)
	{
		bool hasintegerformat = false;

		const auto &rts = states.back().renderTargets;
		for (const auto &rt : rts.colors)
		{
			if (rt.texture.get() && isPixelFormatInteger(rt.texture->getPixelFormat()))
				hasintegerformat = true;
		}

		// This variant of clear() uses glClear() which can't clear integer formats,
		// so we switch to the MRT variant if needed.
		if (hasintegerformat)
		{
			std::vector<OptionalColorD> colors(rts.colors.size());
			for (size_t i = 0; i < colors.size(); i++)
				colors[i] = c;

			clear(colors, stencil, depth);
			return;
		}
	}

	if (c.hasValue || stencil.hasValue || depth.hasValue)
		flushBatchedDraws();

	GLbitfield flags = 0;

	if (c.hasValue)
	{
		Colorf cf((float)c.value.r, (float)c.value.g, (float)c.value.b, (float)c.value.a);
		gammaCorrectColor(cf);
		glClearColor(cf.r, cf.g, cf.b, cf.a);
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (stencil.hasValue)
	{
		glClearStencil(stencil.value);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	if (depth.hasValue)
	{
		gl.clearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
	{
		OpenGL::CleanClearState cs(flags);
		glClear(flags);
	}

	if (c.hasValue && gl.bugs.clearRequiresDriverTextureStateUpdate && Shader::current)
	{
		// This seems to be enough to fix the bug for me. Other methods I've
		// tried (e.g. dummy draws) don't work in all cases.
		gl.useProgram(0);
		gl.useProgram((GLuint) Shader::current->getHandle());
	}
}

void Graphics::clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth)
{
	if (colors.size() == 0 && !stencil.hasValue && !depth.hasValue)
		return;

	const auto &rts = states.back().renderTargets.colors;

	int ncolorRTs = (int) rts.size();
	int ncolors = (int) colors.size();

	if (ncolors <= 1 && (ncolorRTs == 0 || (ncolorRTs == 1 && rts[0].texture != nullptr && !isPixelFormatInteger(rts[0].texture->getPixelFormat()))))
	{
		clear(ncolors > 0 ? colors[0] : OptionalColorD(), stencil, depth);
		return;
	}

	flushBatchedDraws();

	ncolors = std::min(ncolors, ncolorRTs);

	for (int i = 0; i < ncolors; i++)
	{
		if (!colors[i].hasValue)
			continue;

		PixelFormatType datatype = PIXELFORMATTYPE_UNORM;
		if (rts[i].texture != nullptr)
			datatype = getPixelFormatInfo(rts[i].texture->getPixelFormat()).dataType;

		ColorD c = colors[i].value;

		if (datatype == PIXELFORMATTYPE_SINT)
		{
			const GLint carray[] = {(GLint)c.r, (GLint)c.g, (GLint)c.b, (GLint)c.a};
			glClearBufferiv(GL_COLOR, i, carray);
		}
		else if (datatype == PIXELFORMATTYPE_UINT)
		{
			const GLuint carray[] = {(GLuint)c.r, (GLuint)c.g, (GLuint)c.b, (GLuint)c.a};
			glClearBufferuiv(GL_COLOR, i, carray);
		}
		else
		{
			Colorf cf((float)c.r, (float)c.g, (float)c.b, (float)c.a);
			gammaCorrectColor(cf);
			const GLfloat carray[] = {cf.r, cf.g, cf.b, cf.a};
			glClearBufferfv(GL_COLOR, i, carray);
		}
	}

	GLbitfield flags = 0;

	if (stencil.hasValue)
	{
		glClearStencil(stencil.value);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	if (depth.hasValue)
	{
		gl.clearDepth(depth.value);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags != 0)
	{
		OpenGL::CleanClearState cs(flags);
		glClear(flags);
	}

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
	flushBatchedDraws();
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
	if (gl.getFramebuffer(target) == 0)
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

void Graphics::cleanupRenderTexture(love::graphics::Texture *texture)
{
	if (!texture->isRenderTarget())
		return;

	for (auto it = framebufferObjects.begin(); it != framebufferObjects.end(); /**/)
	{
		bool hastexture = false;
		const auto &rts = it->first;

		for (const RenderTarget &rt : rts.colors)
		{
			if (rt.texture == texture)
			{
				hastexture = true;
				break;
			}
		}

		hastexture = hastexture || rts.depthStencil.texture == texture;

		if (hastexture)
		{
			if (isCreated())
				gl.deleteFramebuffer(it->second);
			it = framebufferObjects.erase(it);
		}
		else
			++it;
	}
}

GLuint Graphics::bindCachedFBO(const RenderTargets &targets)
{
	GLuint fbo = framebufferObjects[targets];

	if (fbo != 0)
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);
	}
	else
	{
		int msaa = targets.getFirstTarget().texture->getMSAA();
		bool hasDS = targets.depthStencil.texture != nullptr;

		glGenFramebuffers(1, &fbo);
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

		int ncolortargets = 0;
		GLenum drawbuffers[MAX_COLOR_RENDER_TARGETS];

		auto attachRT = [&](const RenderTarget &rt)
		{
			bool renderbuffer = msaa > 1 || !rt.texture->isReadable();
			OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(rt.texture->getPixelFormat());

			if (fmt.framebufferAttachments[0] == GL_COLOR_ATTACHMENT0)
			{
				fmt.framebufferAttachments[0] = GL_COLOR_ATTACHMENT0 + ncolortargets;
				drawbuffers[ncolortargets] = fmt.framebufferAttachments[0];
				ncolortargets++;
			}

			GLuint handle = (GLuint) rt.texture->getRenderTargetHandle();

			for (GLenum attachment : fmt.framebufferAttachments)
			{
				if (attachment == GL_NONE)
					continue;
				else if (renderbuffer)
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, handle);
				else
				{
					TextureType textype = rt.texture->getTextureType();

					int layer = textype == TEXTURE_CUBE ? 0 : rt.slice;
					int face = textype == TEXTURE_CUBE ? rt.slice : 0;
					int level = rt.mipmap;

					gl.framebufferTexture(attachment, textype, handle, level, layer, face);
				}
			}
		};

		for (const auto &rt : targets.colors)
			attachRT(rt);

		if (hasDS)
			attachRT(targets.depthStencil);

		if (ncolortargets > 1)
			glDrawBuffers(ncolortargets, drawbuffers);
		else if (ncolortargets == 0 && hasDS)
		{
			GLenum none = GL_NONE;
			glDrawBuffers(1, &none);
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

	return fbo;
}

void Graphics::present(void *screenshotCallbackData)
{
	if (!isActive())
		return;

	if (isRenderTargetActive())
		throw love::Exception("present cannot be called while a render target is active.");

	deprecations.draw(this);

	flushBatchedDraws();

	endPass(true);

	int w = getPixelWidth();
	int h = getPixelHeight();

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getInternalBackbufferFBO());

	// Copy internal backbuffer to system backbuffer. When MSAA is used this
	// is a direct MSAA resolve.
	if (internalBackbuffer.get())
	{
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_DRAW, getSystemBackbufferFBO());

		// Discard system backbuffer to prevent it from copying its contents
		// from VRAM to chip memory.
		discard(OpenGL::FRAMEBUFFER_DRAW, {true}, true);

		// updateBackbuffer checks for glBlitFramebuffer support.
		if (GLAD_APPLE_framebuffer_multisample && internalBackbuffer->getMSAA() > 1)
			glResolveMultisampleFramebufferAPPLE();
		else
			glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Discarding the internal backbuffer directly after resolving it should
		// eliminate any copy back to vram it might need to do.
		discard(OpenGL::FRAMEBUFFER_READ, {true}, false);
	}

	if (!pendingScreenshotCallbacks.empty())
	{
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

		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getSystemBackbufferFBO());
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
				img = imagemodule->newImageData(w, h, PIXELFORMAT_RGBA8_UNORM, screenshot);
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
	SDL_PropertiesID props = SDL_GetWindowProperties(SDL_GL_GetCurrentWindow());
	GLuint colorbuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
#endif

	for (StreamBuffer *buffer : batchedDrawState.vb)
		buffer->nextFrame();
	batchedDrawState.indexBuffer->nextFrame();

	auto window = getInstance<love::window::Window>(M_WINDOW);
	if (window != nullptr)
		window->swapBuffers();

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getInternalBackbufferFBO());

	// Reset the per-frame stat counts.
	drawCalls = 0;
	gl.stats.shaderSwitches = 0;
	renderTargetSwitchCount = 0;
	drawCallsBatched = 0;

	updatePendingReadbacks();
	updateTemporaryResources();
}

int Graphics::getRequestedBackbufferMSAA() const
{
	return requestedBackbufferMSAA;
}

int Graphics::getBackbufferMSAA() const
{
	return internalBackbuffer.get() ? internalBackbuffer->getMSAA() : 0;
}

void Graphics::setScissor(const Rect &rect, bool rtActive)
{
	flushBatchedDraws();

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
	gl.setScissor(glrect, rtActive);

	state.scissor = true;
	state.scissorRect = rect;
}

void Graphics::setScissor(const Rect &rect)
{
	setScissor(rect, isRenderTargetActive());
}

void Graphics::setScissor()
{
	if (states.back().scissor)
		flushBatchedDraws();

	states.back().scissor = false;

	if (gl.isStateEnabled(OpenGL::ENABLE_SCISSOR_TEST))
		gl.setEnableState(OpenGL::ENABLE_SCISSOR_TEST, false);
}

void Graphics::setStencilState(const StencilState &s)
{
	validateStencilState(s);

	flushBatchedDraws();

	bool enablestencil = s.action != STENCIL_KEEP || s.compare != COMPARE_ALWAYS;
	if (enablestencil != gl.isStateEnabled(OpenGL::ENABLE_STENCIL_TEST))
		gl.setEnableState(OpenGL::ENABLE_STENCIL_TEST, enablestencil);

	GLenum glaction = GL_KEEP;

	switch (s.action)
	{
	case STENCIL_KEEP:
		glaction = GL_KEEP;
		break;
	case STENCIL_ZERO:
		glaction = GL_ZERO;
		break;
	case STENCIL_REPLACE:
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
	case STENCIL_MAX_ENUM:
		glaction = GL_KEEP;
		break;
	}

	/**
	 * GPUs do the comparison opposite to what makes sense for love's API. For
	 * example, if the compare function is GREATER then the stencil test will
	 * pass if the reference value is greater than the value in the stencil
	 * buffer. With our API it's more intuitive to assume that
	 * setStencilState(STENCIL_KEEP, COMPARE_GREATER, 4) will make it pass if the
	 * stencil buffer has a value greater than 4.
	 **/
	GLenum glcompare = OpenGL::getGLCompareMode(getReversedCompareMode(s.compare));

	if (enablestencil)
	{
		glStencilFunc(glcompare, s.value, s.readMask);
		glStencilOp(GL_KEEP, GL_KEEP, glaction);
	}

	if (s.writeMask != gl.getStencilWriteMask())
		gl.setStencilWriteMask(s.writeMask);

	states.back().stencil = s;
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
	validateDepthState(write);

	DisplayState &state = states.back();

	if (state.depthTest != compare || state.depthWrite != write)
		flushBatchedDraws();

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

void Graphics::setFrontFaceWinding(Winding winding)
{
	DisplayState &state = states.back();

	if (state.winding != winding)
		flushBatchedDraws();

	state.winding = winding;

	if (isRenderTargetActive())
		winding = winding == WINDING_CW ? WINDING_CCW : WINDING_CW;

	glFrontFace(winding == WINDING_CW ? GL_CW : GL_CCW);
}

void Graphics::setColor(Colorf c)
{
	c.r = std::min(std::max(c.r, 0.0f), 1.0f);
	c.g = std::min(std::max(c.g, 0.0f), 1.0f);
	c.b = std::min(std::max(c.b, 0.0f), 1.0f);
	c.a = std::min(std::max(c.a, 0.0f), 1.0f);

	states.back().color = c;
}

void Graphics::setColorMask(ColorChannelMask mask)
{
	flushBatchedDraws();

	uint32 maskbits =
		((mask.r ? 1 : 0) << 0) | ((mask.g ? 1 : 0) << 1) |
		((mask.b ? 1 : 0) << 2) | ((mask.a ? 1 : 0) << 3);

	gl.setColorWriteMask(maskbits);
	states.back().colorMask = mask;
}

void Graphics::setBlendState(const BlendState &blend)
{
	if (!(blend == states.back().blend))
		flushBatchedDraws();

	if (blend.enable != gl.isStateEnabled(OpenGL::ENABLE_BLEND))
		gl.setEnableState(OpenGL::ENABLE_BLEND, blend.enable);

	if (blend.enable)
	{
		GLenum opRGB  = getGLBlendOperation(blend.operationRGB);
		GLenum opA    = getGLBlendOperation(blend.operationA);
		GLenum srcRGB = getGLBlendFactor(blend.srcFactorRGB);
		GLenum srcA   = getGLBlendFactor(blend.srcFactorA);
		GLenum dstRGB = getGLBlendFactor(blend.dstFactorRGB);
		GLenum dstA   = getGLBlendFactor(blend.dstFactorA);

		glBlendEquationSeparate(opRGB, opA);
		glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
	}

	states.back().blend = blend;
}

void Graphics::setPointSize(float size)
{
	if (size != states.back().pointSize)
		flushBatchedDraws();

	states.back().pointSize = size;
}

void Graphics::setWireframe(bool enable)
{
	// Not supported in OpenGL ES.
	if (GLAD_ES_VERSION_2_0)
		return;

	flushBatchedDraws();

	glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
	states.back().wireframe = enable;
}

void *Graphics::getBufferMapMemory(size_t size)
{
	// We don't need anything more complicated because get/release calls are
	// never interleaved (as of when this comment was written.)
	if (bufferMapMemory == nullptr || size > bufferMapMemorySize)
		return malloc(size);
	return bufferMapMemory;
}

void Graphics::releaseBufferMapMemory(void *mem)
{
	if (mem != bufferMapMemory)
		free(mem);
}

Renderer Graphics::getRenderer() const
{
	return RENDERER_OPENGL;
}

bool Graphics::usesGLSLES() const
{
	return GLAD_ES_VERSION_2_0;
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
	capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = true;
	capabilities.features[FEATURE_CLAMP_ZERO] = gl.isClampZeroOneTextureWrapSupported();
	capabilities.features[FEATURE_CLAMP_ONE] = gl.isClampZeroOneTextureWrapSupported();
	capabilities.features[FEATURE_LIGHTEN] = true;
	capabilities.features[FEATURE_FULL_NPOT] = true;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = GLAD_ES_VERSION_3_1 || (gl.isCoreProfile() && GLAD_VERSION_4_3);
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = gl.isBufferUsageSupported(BUFFERUSAGE_TEXEL);
	capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = gl.isCopyTextureToBufferSupported();
	capabilities.features[FEATURE_INDIRECT_DRAW] = capabilities.features[FEATURE_GLSL4];
	static_assert(FEATURE_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	capabilities.limits[LIMIT_POINT_SIZE] = gl.getMaxPointSize();
	capabilities.limits[LIMIT_TEXTURE_SIZE] = gl.getMax2DTextureSize();
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = gl.getMaxTextureLayers();
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = gl.getMax3DTextureSize();
	capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = gl.getMaxCubeTextureSize();
	capabilities.limits[LIMIT_TEXEL_BUFFER_SIZE] = gl.getMaxTexelBufferSize();
	capabilities.limits[LIMIT_SHADER_STORAGE_BUFFER_SIZE] = gl.getMaxShaderStorageBufferSize();
	capabilities.limits[LIMIT_THREADGROUPS_X] = gl.getMaxComputeWorkGroupsX();
	capabilities.limits[LIMIT_THREADGROUPS_Y] = gl.getMaxComputeWorkGroupsY();
	capabilities.limits[LIMIT_THREADGROUPS_Z] = gl.getMaxComputeWorkGroupsZ();
	capabilities.limits[LIMIT_RENDER_TARGETS] = gl.getMaxRenderTargets();
	capabilities.limits[LIMIT_TEXTURE_MSAA] = gl.getMaxSamples();
	capabilities.limits[LIMIT_ANISOTROPY] = gl.getMaxAnisotropy();
	static_assert(LIMIT_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new system limit!");

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
		capabilities.textureTypes[i] = true;

	for (int i = 0; i < PIXELFORMAT_MAX_ENUM; i++)
	{
		auto format = (PixelFormat) i;
		pixelFormatUsage[i][0] = computePixelFormatUsage(format, false);
		pixelFormatUsage[i][1] = computePixelFormatUsage(format, true);
	}

#ifdef LOVE_ANDROID
	// This can't be done in initContext with the rest of the bug checks because
	// isPixelFormatSupported relies on state initialized here / after init.
	if (GLAD_ES_VERSION_3_0 && !isPixelFormatSupported(PIXELFORMAT_R8_UNORM, PIXELFORMATUSAGEFLAGS_SAMPLE | PIXELFORMATUSAGEFLAGS_RENDERTARGET))
	{
		gl.bugs.brokenR8PixelFormat = true;
		pixelFormatUsage[PIXELFORMAT_R8_UNORM][0] = computePixelFormatUsage(PIXELFORMAT_R8_UNORM, false);
		pixelFormatUsage[PIXELFORMAT_R8_UNORM][1] = computePixelFormatUsage(PIXELFORMAT_R8_UNORM, true);
	}
#endif
}

uint32 Graphics::computePixelFormatUsage(PixelFormat format, bool readable)
{
	uint32 usage = OpenGL::getPixelFormatUsageFlags(format);

	if (readable && (usage & PIXELFORMATUSAGEFLAGS_SAMPLE) == 0)
		return 0;

	// Even though we might have the necessary OpenGL version or extension,
	// drivers are still allowed to throw FRAMEBUFFER_UNSUPPORTED when attaching
	// a texture to a FBO whose format the driver doesn't like. So we should
	// test with an actual FBO.
	// Avoid the test for depth/stencil formats - not every GL version
	// guarantees support for depth/stencil-only render targets (which we would
	// need for the test below to work), and we already do some finagling in
	// convertPixelFormat to try to use the best-supported internal
	// depth/stencil format for a particular driver.
	if ((usage & PIXELFORMATUSAGEFLAGS_RENDERTARGET) != 0 && !isPixelFormatDepthStencil(format))
	{
		GLuint texture = 0;
		GLuint renderbuffer = 0;

		OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format);

		GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

		// Make sure at least something is bound to a color attachment. I believe
		// this is required on ES2 but I'm not positive.
		if (isPixelFormatDepthStencil(format))
		{
			love::graphics::Texture *tex = getDefaultTexture(TEXTURE_2D, DATA_BASETYPE_FLOAT, false);
			gl.framebufferTexture(GL_COLOR_ATTACHMENT0, TEXTURE_2D, (GLuint) tex->getHandle(), 0, 0, 0);
		}

		if (readable)
		{
			glGenTextures(1, &texture);
			gl.bindTextureToUnit(TEXTURE_2D, texture, 0, false);

			SamplerState s;
			s.minFilter = s.magFilter = SamplerState::FILTER_NEAREST;
			gl.setSamplerState(TEXTURE_2D, s);

			gl.rawTexStorage(TEXTURE_2D, 1, format, 1, 1);
		}
		else
		{
			glGenRenderbuffers(1, &renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, fmt.internalformat, 1, 1);
		}

		for (GLenum attachment : fmt.framebufferAttachments)
		{
			if (attachment == GL_NONE)
				continue;

			if (readable)
				gl.framebufferTexture(attachment, TEXTURE_2D, texture, 0, 0, 0);
			else
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			usage &= ~PIXELFORMATUSAGEFLAGS_RENDERTARGET;

		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
		gl.deleteFramebuffer(fbo);

		if (texture != 0)
			gl.deleteTexture(texture);

		if (renderbuffer != 0)
			glDeleteRenderbuffers(1, &renderbuffer);
	}

	return usage;
}

bool Graphics::isPixelFormatSupported(PixelFormat format, uint32 usage)
{
	format = getSizedFormat(format);

	bool readable = (usage & PIXELFORMATUSAGEFLAGS_SAMPLE) != 0;
	return (usage & pixelFormatUsage[format][readable ? 1 : 0]) == usage;
}

} // opengl
} // graphics
} // love
