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

#include "Graphics.h"
#include "StreamBuffer.h"
#include "Buffer.h"
#include "Texture.h"
#include "GraphicsReadback.h"
#include "Shader.h"
#include "ShaderStage.h"
#include "window/Window.h"
#include "image/Image.h"
#include "common/memory.h"

#import <QuartzCore/CAMetalLayer.h>

#ifdef LOVE_MACOS
// Needed for the GPU dynamic switching hack below.
#define GL_SILENCE_DEPRECATION 1
#import <Cocoa/Cocoa.h>
#endif

namespace love
{
namespace graphics
{
namespace metal
{

static MTLSamplerMinMagFilter getMTLSamplerFilter(SamplerState::FilterMode mode)
{
	switch (mode)
	{
		case SamplerState::FILTER_LINEAR: return MTLSamplerMinMagFilterLinear;
		case SamplerState::FILTER_NEAREST: return MTLSamplerMinMagFilterNearest;
		case SamplerState::FILTER_MAX_ENUM: return MTLSamplerMinMagFilterLinear;
	}
	return MTLSamplerMinMagFilterLinear;
}

static MTLSamplerMipFilter getMTLSamplerMipFilter(SamplerState::MipmapFilterMode mode)
{
	switch (mode)
	{
		case SamplerState::MIPMAP_FILTER_NONE: return MTLSamplerMipFilterNotMipmapped;
		case SamplerState::MIPMAP_FILTER_LINEAR: return MTLSamplerMipFilterLinear;
		case SamplerState::MIPMAP_FILTER_NEAREST: return MTLSamplerMipFilterNearest;
		case SamplerState::MIPMAP_FILTER_MAX_ENUM: return MTLSamplerMipFilterNotMipmapped;
	}
	return MTLSamplerMipFilterNotMipmapped;
}

static MTLSamplerAddressMode getMTLSamplerAddressMode(SamplerState::WrapMode mode)
{
	switch (mode)
	{
		case SamplerState::WRAP_CLAMP: return MTLSamplerAddressModeClampToEdge;
		case SamplerState::WRAP_CLAMP_ZERO: return MTLSamplerAddressModeClampToZero;
		case SamplerState::WRAP_CLAMP_ONE:
			if (@available(macOS 10.12, iOS 14.0, *))
				return MTLSamplerAddressModeClampToBorderColor;
			else
				return MTLSamplerAddressModeClampToZero;
		case SamplerState::WRAP_REPEAT: return MTLSamplerAddressModeRepeat;
		case SamplerState::WRAP_MIRRORED_REPEAT: return MTLSamplerAddressModeMirrorRepeat;
		case SamplerState::WRAP_MAX_ENUM: return MTLSamplerAddressModeClampToEdge;
	}
	return MTLSamplerAddressModeClampToEdge;
}

static MTLCompareFunction getMTLCompareFunction(CompareMode mode)
{
	switch (mode)
	{
		case COMPARE_LESS: return MTLCompareFunctionLess;
		case COMPARE_LEQUAL: return MTLCompareFunctionLessEqual;
		case COMPARE_EQUAL: return MTLCompareFunctionEqual;
		case COMPARE_GEQUAL: return MTLCompareFunctionGreaterEqual;
		case COMPARE_GREATER: return MTLCompareFunctionGreater;
		case COMPARE_NOTEQUAL: return MTLCompareFunctionNotEqual;
		case COMPARE_ALWAYS: return MTLCompareFunctionAlways;
		case COMPARE_NEVER: return MTLCompareFunctionNever;
		case COMPARE_MAX_ENUM: return MTLCompareFunctionNever;
	}
	return MTLCompareFunctionNever;
}

static MTLStencilOperation getMTLStencilOperation(StencilAction action)
{
	switch (action)
	{
		case STENCIL_KEEP: return MTLStencilOperationKeep;
		case STENCIL_ZERO: return MTLStencilOperationZero;
		case STENCIL_REPLACE: return MTLStencilOperationReplace;
		case STENCIL_INCREMENT: return MTLStencilOperationIncrementClamp;
		case STENCIL_DECREMENT: return MTLStencilOperationDecrementClamp;
		case STENCIL_INCREMENT_WRAP: return MTLStencilOperationIncrementWrap;
		case STENCIL_DECREMENT_WRAP: return MTLStencilOperationDecrementWrap;
		case STENCIL_INVERT: return MTLStencilOperationInvert;
		case STENCIL_MAX_ENUM: return MTLStencilOperationKeep;
	}
	return MTLStencilOperationKeep;
}

static MTLPrimitiveType getMTLPrimitiveType(PrimitiveType prim)
{
	switch (prim)
	{
		case PRIMITIVE_TRIANGLES: return MTLPrimitiveTypeTriangle;
		case PRIMITIVE_TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
		case PRIMITIVE_TRIANGLE_FAN: return MTLPrimitiveTypeTriangle; // This is emulated with an index buffer.
		case PRIMITIVE_POINTS: return MTLPrimitiveTypePoint;
		case PRIMITIVE_MAX_ENUM: return MTLPrimitiveTypeTriangle;
	}
	return MTLPrimitiveTypeTriangle;
}

static inline id<MTLTexture> getMTLTexture(love::graphics::Texture *tex)
{
	return tex ? (__bridge id<MTLTexture>)(void *) tex->getHandle() : nil;
}

static inline id<MTLTexture> getMTLRenderTarget(love::graphics::Texture *tex)
{
	return tex ? (__bridge id<MTLTexture>)(void *) tex->getRenderTargetHandle() : nil;
}

static inline id<MTLBuffer> getMTLBuffer(love::graphics::Resource *res)
{
	return res ? (__bridge id<MTLBuffer>)(void *) res->getHandle() : nil;
}

static inline void setBuffer(id<MTLRenderCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, ShaderStageType stage, int index, id<MTLBuffer> buffer, size_t offset)
{
	void *b = (__bridge void *)buffer;
	auto &binding = bindings.buffers[index][stage];
	if (binding.buffer != b)
	{
		binding.buffer = b;
		binding.offset = offset;
		if (stage == SHADERSTAGE_VERTEX)
			[encoder setVertexBuffer:buffer offset:offset atIndex:index];
		else if (stage == SHADERSTAGE_PIXEL)
			[encoder setFragmentBuffer:buffer offset:offset atIndex:index];
	}
	else if (binding.offset != offset)
	{
		binding.offset = offset;
		if (stage == SHADERSTAGE_VERTEX)
			[encoder setVertexBufferOffset:offset atIndex:index];
		else if (stage == SHADERSTAGE_PIXEL)
			[encoder setFragmentBufferOffset:offset atIndex:index];
	}
}

static inline void setBuffer(id<MTLComputeCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, int index, id<MTLBuffer> buffer, size_t offset)
{
	void *b = (__bridge void *)buffer;
	auto &binding = bindings.buffers[index][SHADERSTAGE_COMPUTE];
	if (binding.buffer != b)
	{
		binding.buffer = b;
		binding.offset = offset;
		[encoder setBuffer:buffer offset:offset atIndex:index];
	}
	else if (binding.offset != offset)
	{
		binding.offset = offset;
		[encoder setBufferOffset:offset atIndex:index];
	}
}

static inline void setTexture(id<MTLRenderCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, ShaderStageType stage, int index, id<MTLTexture> texture)
{
	void *t = (__bridge void *)texture;
	auto &binding = bindings.textures[index][stage];
	if (binding != t)
	{
		binding = t;
		if (stage == SHADERSTAGE_VERTEX)
			[encoder setVertexTexture:texture atIndex:index];
		else if (stage == SHADERSTAGE_PIXEL)
			[encoder setFragmentTexture:texture atIndex:index];
	}
}

static inline void setTexture(id<MTLComputeCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, int index, id<MTLTexture> texture)
{
	void *t = (__bridge void *)texture;
	auto &binding = bindings.textures[index][SHADERSTAGE_COMPUTE];
	if (binding != t)
	{
		binding = t;
		[encoder setTexture:texture atIndex:index];
	}
}

static inline void setSampler(id<MTLRenderCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, ShaderStageType stage, int index, love::graphics::Texture *samplertex)
{
	void *s = samplertex != nullptr ? (void *)samplertex->getSamplerHandle() : nullptr;
	auto &binding = bindings.samplers[index][stage];
	if (binding != s)
	{
		binding = s;
		id<MTLSamplerState> sampler = (__bridge id<MTLSamplerState>) s;
		if (stage == SHADERSTAGE_VERTEX)
			[encoder setVertexSamplerState:sampler atIndex:index];
		else if (stage == SHADERSTAGE_PIXEL)
			[encoder setFragmentSamplerState:sampler atIndex:index];
	}
}

static inline void setSampler(id<MTLComputeCommandEncoder> encoder, Graphics::RenderEncoderBindings &bindings, int index, love::graphics::Texture *samplertex)
{
	void *s = samplertex != nullptr ? (void *)samplertex->getSamplerHandle() : nullptr;
	auto &binding = bindings.samplers[index][SHADERSTAGE_COMPUTE];
	if (binding != s)
	{
		binding = s;
		id<MTLSamplerState> sampler = (__bridge id<MTLSamplerState>) s;
		[encoder setSamplerState:sampler atIndex:index];
	}
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
		printf("Cannot create Metal renderer: %s\n", e.what());
	}

	return instance;
}

struct DefaultVertexAttributes
{
	float floats[4];
	int ints[4];
};

Graphics *Graphics::graphicsInstance = nullptr;

Graphics::Graphics()
	: love::graphics::Graphics("love.graphics.metal")
	, device(nil)
	, commandQueue(nil)
	, commandBuffer(nil)
	, renderEncoder(nil)
	, blitEncoder(nil)
	, metalLayer(nil)
	, activeDrawable(nil)
	, passDesc(nil)
	, dirtyRenderState(STATEBIT_ALL)
	, lastCullMode(CULL_MAX_ENUM)
	, lastRenderPipelineKey()
	, shaderSwitches(0)
	, requestedBackbufferMSAA(0)
	, attachmentStoreActions()
	, renderBindings()
	, uniformBufferOffset(0)
	, uniformBufferGPUStart(0)
	, defaultAttributesBuffer(nullptr)
	, families()
	, isVMDevice(false)
{ @autoreleasepool {
	if (@available(macOS 10.15, iOS 13.0, *))
	{
		graphicsInstance = this;
#ifdef LOVE_MACOS
		if (isLowPowerPreferred())
		{
			for (id<MTLDevice> dev in MTLCopyAllDevices())
			{
				if (dev.isLowPower)
				{
					device = dev;
					break;
				}
			}
		}
#endif
		if (device == nil)
			device = MTLCreateSystemDefaultDevice();
		if (device == nil)
			throw love::Exception("Metal is not supported on this system.");
	}
	else
	{
		throw love::Exception("LOVE's Metal graphics backend requires macOS 10.15+ or iOS 13+.");
	}

	isVMDevice = [device.name containsString:@("Apple Paravirtual device")];

#ifdef LOVE_MACOS
	// On multi-GPU macOS systems with a low and high power GPU (e.g. a 2016
	// Macbook Pro), the OS doesn't activate the high power GPU for final
	// monitor blitting even when MTLCreateSystemDefaultDevice is called, on
	// newer macOS versions. https://developer.apple.com/forums/thread/675411
	// But it does when an OpenGL context is created, so this big hack forces it
	// when we want it to happen.
	if (!device.isLowPower && MTLCopyAllDevices().count > 1)
	{
		NSOpenGLPixelFormatAttribute attributes[] =
		{
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
			NSOpenGLPFAColorSize, 24,
			NSOpenGLPFAAlphaSize, 8,
			NSOpenGLPFADepthSize, 24,
			NSOpenGLPFAStencilSize, 8,
			NSOpenGLPFASampleBuffers, 0,
			0,
		};

		auto pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
		auto glcontext = [[NSOpenGLContext alloc] initWithFormat:pixelformat shareContext:nullptr];
		LOVE_UNUSED(glcontext);
	}
#endif

	commandQueue = [device newCommandQueue];
	passDesc = [MTLRenderPassDescriptor new];

	initCapabilities();

	uniformBuffer = CreateStreamBuffer(device, BUFFERUSAGE_UNIFORM, 1024 * 512 * 1);

	{
		std::vector<Buffer::DataDeclaration> dataformat = {
			{"floats", DATAFORMAT_FLOAT_VEC4, 0},
			{"ints", DATAFORMAT_INT32_VEC4, 0},
		};

		DefaultVertexAttributes defaults = {
			{0.0f, 0.0f, 0.0f, 1.0f},
			{0, 0, 0, 1},
		};

		Buffer::Settings attribsettings(BUFFERUSAGEFLAG_VERTEX, BUFFERDATAUSAGE_STATIC);
		attribsettings.debugName = "Default Vertex Attributes";

		defaultAttributesBuffer = newBuffer(attribsettings, dataformat, &defaults, sizeof(DefaultVertexAttributes), 0);
	}

	if (batchedDrawState.vb[0] == nullptr)
	{
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		batchedDrawState.vb[0] = CreateStreamBuffer(device, BUFFERUSAGE_VERTEX, 1024 * 1024 * 1);
		batchedDrawState.vb[1] = CreateStreamBuffer(device, BUFFERUSAGE_VERTEX, 256  * 1024 * 1);
		batchedDrawState.indexBuffer = CreateStreamBuffer(device, BUFFERUSAGE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
	}

	createQuadIndexBuffer();
	createFanIndexBuffer();

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
			Shader::standardShaders[i] = newShader(stages, opts);
		}
	}

	// A shader should always be active, but the default shader shouldn't be
	// returned by getShader(), so we don't do setShader(defaultShader).
	if (!Shader::current)
		Shader::standardShaders[Shader::STANDARD_DEFAULT]->attach();

	auto window = Module::getInstance<love::window::Window>(M_WINDOW);

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
}}

Graphics::~Graphics()
{ @autoreleasepool {
	submitCommandBuffer(SUBMIT_DONE);
	processCompletedCommandBuffers();

	// Wait for all active command buffers to complete before returning from
	// the destructor.
	for (id<MTLCommandBuffer> cmd : activeCommandBuffers)
	{
		if (cmd.status == MTLCommandBufferStatusNotEnqueued || cmd.status == MTLCommandBufferStatusEnqueued)
			[cmd commit];

		[cmd waitUntilCompleted];
	}

	uniformBuffer->release();
	defaultAttributesBuffer->release();
	passDesc = nil;
	commandQueue = nil;
	device = nil;

	for (auto &kvp : cachedSamplers)
		CFBridgingRelease(kvp.second);

	for (auto &kvp : cachedDepthStencilStates)
		CFBridgingRelease(kvp.second);

	graphicsInstance = nullptr;
}}

love::graphics::StreamBuffer *Graphics::newStreamBuffer(BufferUsage usage, size_t size)
{
	return CreateStreamBuffer(device, usage, size);
}

love::graphics::Texture *Graphics::newTexture(const Texture::Settings &settings, const Texture::Slices *data)
{
	return new Texture(this, device, settings, data);
}

love::graphics::Texture *Graphics::newTextureView(love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
{
	return new Texture(this, device, base, viewsettings);
}

love::graphics::ShaderStage *Graphics::newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles)
{
	return new ShaderStage(this, stage, source, gles, cachekey);
}

love::graphics::Shader *Graphics::newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const Shader::CompileOptions &options)
{
	return new Shader(device, stages, options);
}

love::graphics::Buffer *Graphics::newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
{
	return new Buffer(this, device, settings, format, data, size, arraylength);
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
	bool sizechanged = width != this->width || height != this->height
		|| pixelwidth != this->pixelWidth || pixelheight != this->pixelHeight;

	bool dschanged = backbufferstencil != this->backbufferHasStencil || backbufferdepth != this->backbufferHasDepth;
	bool msaachanged = msaa != this->requestedBackbufferMSAA;

	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	this->backbufferHasStencil = backbufferstencil;
	this->backbufferHasDepth = backbufferdepth;
	this->requestedBackbufferMSAA = msaa;

	if (!isRenderTargetActive())
	{
		dirtyRenderState |= STATEBIT_VIEWPORT | STATEBIT_SCISSOR;
		resetProjection();
	}

	Texture::Settings settings;
	settings.width = width;
	settings.height = height;
	settings.dpiScale = (float)pixelheight / (float)height;
	settings.msaa = getRequestedBackbufferMSAA();
	settings.renderTarget = true;
	settings.readable.set(false);

	if (sizechanged || msaachanged)
	{
		backbufferMSAA.set(nullptr);
		if (settings.msaa > 1)
		{
			settings.format = isGammaCorrect() ? PIXELFORMAT_BGRA8_sRGB : PIXELFORMAT_BGRA8_UNORM;
			backbufferMSAA.set(newTexture(settings), Acquire::NORETAIN);
		}
	}

	if (sizechanged || msaachanged || dschanged)
	{
		backbufferDepthStencil.set(nullptr);
		if (backbufferstencil || backbufferdepth)
		{
			if (backbufferstencil && backbufferdepth)
				settings.format = PIXELFORMAT_DEPTH24_UNORM_STENCIL8;
			else if (backbufferstencil)
				settings.format = PIXELFORMAT_STENCIL8;
			else if (backbufferdepth)
				settings.format = PIXELFORMAT_DEPTH24_UNORM;
			backbufferDepthStencil.set(newTexture(settings), Acquire::NORETAIN);
		}
	}
}

bool Graphics::setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa)
{ @autoreleasepool {
	this->width = width;
	this->height = height;
	this->metalLayer = (__bridge CAMetalLayer *) context;

	metalLayer.device = device;
	metalLayer.pixelFormat = isGammaCorrect() ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;

	// This is set to NO when there are pending screen captures.
	metalLayer.framebufferOnly = YES;

#ifdef LOVE_MACOS
	// Matches behaviour of SDL's OpenGL context when using the GL renderer.
	metalLayer.magnificationFilter = kCAFilterNearest;
#endif

	backbufferChanged(width, height, pixelwidth, pixelheight, backbufferstencil, backbufferdepth, msaa);

	created = true;

	// Restore the graphics state.
	restoreState(states.back());

	return true;
}}

void Graphics::unSetMode()
{ @autoreleasepool {
	if (!isCreated())
		return;

	flushBatchedDraws();

	submitCommandBuffer(SUBMIT_DONE);

	clearTemporaryResources();

	created = false;
	metalLayer = nil;
	activeDrawable = nil;
}}

void Graphics::setActive(bool enable)
{
	flushBatchedDraws();
	active = enable;
}

void Graphics::setShaderChanged()
{
	dirtyRenderState |= STATEBIT_SHADER;
	++shaderSwitches;
}

id<MTLCommandBuffer> Graphics::useCommandBuffer()
{
	if (commandBuffer == nil)
	{
		commandBuffer = [commandQueue commandBuffer];
		activeCommandBuffers.push_back(commandBuffer);
	}

	return commandBuffer;
}

void Graphics::processCompletedCommandBuffers()
{
	for (int i = (int) activeCommandBuffers.size() - 1; i >= 0; i--)
	{
		auto status = activeCommandBuffers[i].status;
		if (status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError)
			activeCommandBuffers.erase(activeCommandBuffers.begin() + i);
	}
}

void Graphics::submitCommandBuffer(SubmitType type)
{
	submitAllEncoders(type);

	if (commandBuffer != nil)
	{
		[commandBuffer commit];
		commandBuffer = nil;
	}
}

void Graphics::submitAllEncoders(SubmitType type)
{
	submitRenderEncoder(type);
	submitBlitEncoder();
	submitComputeEncoder();
}

static inline void setAttachment(const Graphics::RenderTarget &rt, MTLRenderPassAttachmentDescriptor *desc, MTLStoreAction &storeaction, bool setload = true)
{
	bool isvolume = rt.texture->getTextureType() == TEXTURE_VOLUME;

	desc.texture = getMTLRenderTarget(rt.texture);
	desc.level = rt.mipmap;
	desc.slice = isvolume ? 0 : rt.slice;
	desc.depthPlane = isvolume ? rt.slice : 0;

	if (setload)
	{
		// Default to load until clear or discard is called.
		desc.loadAction = MTLLoadActionLoad;
	}

	desc.storeAction = MTLStoreActionUnknown;
	storeaction = MTLStoreActionStore;

	desc.resolveTexture = nil;

	if (rt.texture->getMSAA() > 1 && rt.texture->isReadable())
	{
		storeaction = MTLStoreActionStoreAndMultisampleResolve;
		desc.resolveTexture = getMTLTexture(rt.texture);
	}
}

id<MTLRenderCommandEncoder> Graphics::useRenderEncoder()
{
	if (renderEncoder == nil)
	{
		submitAllEncoders(SUBMIT_STORE);

		// Pass desc info for non-backbuffer render targets are set up in
		// setRenderTargetsInternal.
		const auto &rts = states.back().renderTargets;
		if (rts.getFirstTarget().texture.get() == nullptr)
		{
			if (activeDrawable == nil)
			{
				// This is reset to YES after each frame.
				// TODO: Does setting this reallocate memory?
				if (!pendingScreenshotCallbacks.empty())
					metalLayer.framebufferOnly = NO;

				activeDrawable = [metalLayer nextDrawable];
			}

			if (backbufferMSAA.get())
			{
				attachmentStoreActions.color[0] = MTLStoreActionMultisampleResolve;
				passDesc.colorAttachments[0].texture = getMTLRenderTarget(backbufferMSAA);
				passDesc.colorAttachments[0].resolveTexture = activeDrawable.texture;
			}
			else
			{
				attachmentStoreActions.color[0] = MTLStoreActionStore;
				passDesc.colorAttachments[0].texture = activeDrawable.texture;
				passDesc.colorAttachments[0].resolveTexture = nil;
			}

			passDesc.colorAttachments[0].storeAction = MTLStoreActionUnknown;
			passDesc.colorAttachments[0].level = 0;
			passDesc.colorAttachments[0].slice = 0;
			passDesc.colorAttachments[0].depthPlane = 0;

			RenderTarget rt(backbufferDepthStencil);
			if (rt.texture != nullptr && isPixelFormatDepth(rt.texture->getPixelFormat()))
				setAttachment(rt, passDesc.depthAttachment, attachmentStoreActions.depth, false);
			if (rt.texture != nullptr && isPixelFormatStencil(rt.texture->getPixelFormat()))
				setAttachment(rt, passDesc.stencilAttachment, attachmentStoreActions.stencil, false);
			attachmentStoreActions.depth = MTLStoreActionDontCare;
			attachmentStoreActions.stencil = MTLStoreActionDontCare;

			auto &key = lastRenderPipelineKey;
			key.colorRenderTargetFormats = isGammaCorrect() ? PIXELFORMAT_BGRA8_sRGB : PIXELFORMAT_BGRA8_UNORM;
			if (backbufferDepthStencil.get())
				key.depthStencilFormat = backbufferDepthStencil->getPixelFormat();
			else
				key.depthStencilFormat = PIXELFORMAT_UNKNOWN;
			key.msaa = backbufferMSAA ? (uint8) backbufferMSAA->getMSAA() : 1;
		}

		renderEncoder = [useCommandBuffer() renderCommandEncoderWithDescriptor:passDesc];

		renderBindings = {};

		id<MTLBuffer> defaultbuffer = getMTLBuffer(defaultAttributesBuffer);
		setBuffer(renderEncoder, renderBindings, SHADERSTAGE_VERTEX, DEFAULT_VERTEX_BUFFER_BINDING, defaultbuffer, 0);

		dirtyRenderState = STATEBIT_ALL;
		lastCullMode = CULL_MAX_ENUM;
	}

	return renderEncoder;
}

void Graphics::submitRenderEncoder(SubmitType type)
{
	if (renderEncoder != nil)
	{
		bool store = type == SUBMIT_STORE;
		const auto &actions = attachmentStoreActions;
		const auto &rts = states.back().renderTargets;
		bool isbackbuffer = rts.getFirstTarget().texture.get() == nullptr;

		if (isbackbuffer)
			[renderEncoder setColorStoreAction:(store ? MTLStoreActionStore : actions.color[0]) atIndex:0];

		for (size_t i = 0; i < rts.colors.size(); i++)
			[renderEncoder setColorStoreAction:(store ? MTLStoreActionStore : actions.color[i]) atIndex:i];

		love::graphics::Texture *ds = rts.depthStencil.texture.get();
		if (isbackbuffer)
			ds = backbufferDepthStencil;

		if ((rts.temporaryRTFlags & TEMPORARY_RT_DEPTH) != 0 || (ds != nullptr && isPixelFormatDepth(ds->getPixelFormat())))
			[renderEncoder setDepthStoreAction:store ? MTLStoreActionStore : actions.depth];

		if ((rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) != 0 || (ds != nullptr && isPixelFormatStencil(ds->getPixelFormat())))
			[renderEncoder setStencilStoreAction:store ? MTLStoreActionStore : actions.stencil];

		[renderEncoder endEncoding];
		renderEncoder = nil;

		// Reset actions to load. The next clear/discard/etc will set more
		// appropriate actions if necessary.
		for (int i = 0; i < MAX_COLOR_RENDER_TARGETS; i++)
		{
			passDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
			if (type == SUBMIT_DONE)
			{
				passDesc.colorAttachments[i].texture = nil;
				passDesc.colorAttachments[i].resolveTexture = nil;
			}
		}

		passDesc.depthAttachment.loadAction = MTLLoadActionLoad;
		passDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
		if (type == SUBMIT_DONE)
		{
			passDesc.depthAttachment.texture = nil;
			passDesc.depthAttachment.resolveTexture = nil;
			passDesc.stencilAttachment.texture = nil;
			passDesc.stencilAttachment.resolveTexture = nil;
		}
	}
}

id<MTLBlitCommandEncoder> Graphics::useBlitEncoder()
{
	if (blitEncoder == nil)
	{
		submitAllEncoders(SUBMIT_STORE);
		blitEncoder = [useCommandBuffer() blitCommandEncoder];
	}

	return blitEncoder;
}

void Graphics::submitBlitEncoder()
{
	if (blitEncoder != nil)
	{
		[blitEncoder endEncoding];
		blitEncoder = nil;
	}
}

id<MTLComputeCommandEncoder> Graphics::useComputeEncoder()
{
	if (computeEncoder == nil)
	{
		submitAllEncoders(SUBMIT_STORE);
		computeEncoder = [useCommandBuffer() computeCommandEncoder];
		renderBindings = {};
	}

	return computeEncoder;
}

void Graphics::submitComputeEncoder()
{
	if (computeEncoder != nil)
	{
		[computeEncoder endEncoding];
		computeEncoder = nil;
	}
}

static bool isClampOne(SamplerState::WrapMode w)
{
	return w == SamplerState::WRAP_CLAMP_ONE;
}

id<MTLSamplerState> Graphics::getCachedSampler(const SamplerState &s)
{ @autoreleasepool {
	uint64 key = s.toKey();

	auto it = cachedSamplers.find(key);
	if (it != cachedSamplers.end())
		return (__bridge id<MTLSamplerState>) it->second;

	MTLSamplerDescriptor *desc = [MTLSamplerDescriptor new];

	desc.minFilter = getMTLSamplerFilter(s.minFilter);
	desc.magFilter = getMTLSamplerFilter(s.magFilter);
	desc.mipFilter = getMTLSamplerMipFilter(s.mipmapFilter);
	desc.maxAnisotropy = std::max(1.0f, std::min((float)s.maxAnisotropy, 16.0f));

	desc.sAddressMode = getMTLSamplerAddressMode(s.wrapU);
	desc.tAddressMode = getMTLSamplerAddressMode(s.wrapV);
	desc.rAddressMode = getMTLSamplerAddressMode(s.wrapW);

	if (isClampOne(s.wrapU) || isClampOne(s.wrapV) || isClampOne(s.wrapW))
	{
		if (capabilities.features[FEATURE_CLAMP_ONE])
			desc.borderColor = MTLSamplerBorderColorOpaqueWhite;
		else
		{
			if (isClampOne(s.wrapU))
				desc.sAddressMode = MTLSamplerAddressModeClampToZero;
			if (isClampOne(s.wrapV))
				desc.tAddressMode = MTLSamplerAddressModeClampToZero;
			if (isClampOne(s.wrapW))
				desc.rAddressMode = MTLSamplerAddressModeClampToZero;
		}
	}

	desc.lodMinClamp = s.minLod;
	desc.lodMaxClamp = s.maxLod;

	// This isn't supported on some older iOS devices. Texture code checks for support.
	if (s.depthSampleMode.hasValue)
		desc.compareFunction = getMTLCompareFunction(getReversedCompareMode(s.depthSampleMode.value));

	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:desc];

	if (sampler != nil)
		cachedSamplers[key] = (void *) CFBridgingRetain(sampler);

	return sampler;
}}

bool Graphics::isDepthCompareSamplerSupported() const
{
	return families.mac[1] || families.macCatalyst[1] || families.apple[3];
}

id<MTLDepthStencilState> Graphics::getCachedDepthStencilState(const DepthState &depth, const StencilState &stencil)
{
	uint64 key = (depth.compare << 0) | ((uint32)depth.write << 8)
		| (stencil.action << 16) | (stencil.compare << 24)
		| ((uint64)std::max(0, std::min(255, stencil.value)) << 32)
		| ((uint64)std::min(255u, stencil.readMask) << 40)
		| ((uint64)std::min(255u, stencil.writeMask) << 48);

	auto it = cachedDepthStencilStates.find(key);
	if (it != cachedDepthStencilStates.end())
		return (__bridge id<MTLDepthStencilState>) it->second;

	MTLStencilDescriptor *stencildesc = [MTLStencilDescriptor new];

	/**
	 * GPUs do the comparison opposite to what makes sense for love's API. For
	 * example, if the compare function is GREATER then the stencil test will
	 * pass if the reference value is greater than the value in the stencil
	 * buffer. With our API it's more intuitive to assume that
	 * setStencilState(STENCIL_KEEP, COMPARE_GREATER, 4) will make it pass if the
	 * stencil buffer has a value greater than 4.
	 **/
	stencildesc.stencilCompareFunction = getMTLCompareFunction(getReversedCompareMode(stencil.compare));
	stencildesc.stencilFailureOperation = MTLStencilOperationKeep;
	stencildesc.depthFailureOperation = MTLStencilOperationKeep;
	stencildesc.depthStencilPassOperation = getMTLStencilOperation(stencil.action);
	stencildesc.readMask = stencil.readMask;
	stencildesc.writeMask = stencil.writeMask;

	MTLDepthStencilDescriptor *desc = [MTLDepthStencilDescriptor new];

	desc.depthCompareFunction = getMTLCompareFunction(depth.compare);
	desc.depthWriteEnabled = depth.write;
	desc.frontFaceStencil = stencildesc;
	desc.backFaceStencil = stencildesc;

	id<MTLDepthStencilState> mtlstate = [device newDepthStencilStateWithDescriptor:desc];

	if (mtlstate != nil)
		cachedDepthStencilStates[key] = (void *) CFBridgingRetain(mtlstate);

	return mtlstate;
}

void Graphics::applyRenderState(id<MTLRenderCommandEncoder> encoder, VertexAttributesID attributesID)
{
	const uint32 pipelineStateBits = STATEBIT_SHADER | STATEBIT_BLEND | STATEBIT_COLORMASK;

	uint32 dirtyState = dirtyRenderState;
	const auto &state = states.back();

	if (dirtyState & (STATEBIT_VIEWPORT | STATEBIT_SCISSOR))
	{
		int rtw = 0;
		int rth = 0;

		const auto &rt = state.renderTargets.getFirstTarget();
		if (rt.texture.get())
		{
			rtw = rt.texture->getPixelWidth(rt.mipmap);
			rth = rt.texture->getPixelHeight(rt.mipmap);
		}
		else
		{
			rtw = getPixelWidth();
			rth = getPixelHeight();
		}

		if (dirtyState & STATEBIT_VIEWPORT)
		{
			MTLViewport view;
			view.originX = 0.0;
			view.originY = 0.0;
			view.width = rtw;
			view.height = rth;
			view.znear = 0.0;
			view.zfar = 1.0;
			[encoder setViewport:view];
		}

		MTLScissorRect rect = {0, 0, (NSUInteger)rtw, (NSUInteger)rth};

		if (state.scissor)
		{
			double dpiscale = getCurrentDPIScale();
			rect.x = (NSUInteger)(state.scissorRect.x*dpiscale);
			rect.y = (NSUInteger)(state.scissorRect.y*dpiscale);
			rect.width = (NSUInteger)(state.scissorRect.w*dpiscale);
			rect.height = (NSUInteger)(state.scissorRect.h*dpiscale);

			if (rtw > 0 && (int)rect.x >= rtw)
				rect.x = rtw - 1;
			if (rth > 0 && (int)rect.y >= rth)
				rect.y = rth - 1;

			rect.width = std::min(rect.width, rtw - rect.x);
			rect.height = std::min(rect.height, rth - rect.y);
		}

		[encoder setScissorRect:rect];
	}

	if (dirtyState & STATEBIT_FACEWINDING)
	{
		auto winding = state.winding == WINDING_CCW ? MTLWindingCounterClockwise : MTLWindingClockwise;
		[encoder setFrontFacingWinding:winding];
	}

	if (dirtyState & STATEBIT_WIREFRAME)
	{
		auto mode = state.wireframe ? MTLTriangleFillModeLines : MTLTriangleFillModeFill;
		[encoder setTriangleFillMode:mode];
	}

	if (dirtyState & STATEBIT_CULLMODE)
	{
		auto mode = lastCullMode == CULL_BACK ? MTLCullModeBack :
			lastCullMode == CULL_FRONT ? MTLCullModeFront : MTLCullModeNone;
		[encoder setCullMode:mode];
	}

	if ((dirtyState & pipelineStateBits) != 0 || attributesID != lastRenderPipelineKey.vertexAttributesID)
	{
		auto &key = lastRenderPipelineKey;

		key.vertexAttributesID = attributesID;

		Shader *shader = (Shader *) Shader::current;
		id<MTLRenderPipelineState> pipeline = nil;

		if (shader)
		{
			key.blendStateKey = state.blend.toKey();
			key.colorChannelMask = state.colorMask;

			pipeline = shader->getCachedRenderPipeline(this, key);
		}

		[encoder setRenderPipelineState:pipeline];
	}

	if (dirtyState & (STATEBIT_DEPTH | STATEBIT_STENCIL))
	{
		DepthState depth;
		depth.compare = state.depthTest;
		depth.write = state.depthWrite;

		id<MTLDepthStencilState> mtlstate = getCachedDepthStencilState(depth, state.stencil);

		[encoder setDepthStencilState:mtlstate];
	}

	if (dirtyState & STATEBIT_STENCIL)
		[encoder setStencilReferenceValue:state.stencil.value];

	dirtyRenderState = 0;
}

bool Graphics::applyShaderUniforms(id<MTLComputeCommandEncoder> encoder, love::graphics::Shader *shader)
{
	Shader *s = (Shader *)shader;

#if defined(LOVE_MACOS) || TARGET_OS_SIMULATOR || TARGET_OS_MACCATALYST
	size_t alignment = 256;
#else
	size_t alignment = 16;
#endif

	size_t size = s->getLocalUniformBufferSize();
	uint8 *bufferdata = s->getLocalUniformBufferData();

	if (uniformBuffer->getSize() < uniformBufferOffset + size)
	{
		size_t newsize = uniformBuffer->getSize() * 2;
		if (uniformBufferOffset > 0)
			uniformBuffer->nextFrame();
		uniformBuffer->release();
		uniformBuffer = CreateStreamBuffer(device, BUFFERUSAGE_UNIFORM, newsize);
		uniformBufferData = {};
		uniformBufferOffset = 0;
	}

	if (uniformBufferData.data == nullptr)
	{
		uniformBufferData = uniformBuffer->map(uniformBuffer->getSize());
		uniformBufferGPUStart = uniformBuffer->getGPUReadOffset();
	}

	memcpy(uniformBufferData.data + uniformBufferOffset, bufferdata, size);

	id<MTLBuffer> buffer = getMTLBuffer(uniformBuffer);
	int uniformindex = Shader::getUniformBufferBinding();

	auto &bindings = renderBindings;
	setBuffer(encoder, bindings, uniformindex, buffer, uniformBufferGPUStart + uniformBufferOffset);

	uniformBufferOffset += alignUp(size, alignment);

	bool allWritableVariablesSet = true;

	for (const Shader::TextureBinding &b : s->getTextureBindings())
	{
		id<MTLTexture> texture = b.texture;
		auto samplertex = b.samplerTexture;

		uint8 texindex = b.textureStages[SHADERSTAGE_COMPUTE];
		uint8 sampindex = b.samplerStages[SHADERSTAGE_COMPUTE];

		if (texindex != LOVE_UINT8_MAX)
		{
			setTexture(encoder, bindings, texindex, texture);
			if ((b.access & Shader::ACCESS_WRITE) != 0 && texture == nil)
				allWritableVariablesSet = false;
		}

		if (sampindex != LOVE_UINT8_MAX)
			setSampler(encoder, bindings, sampindex, samplertex);
	}

	for (const Shader::BufferBinding &b : s->getBufferBindings())
	{
		uint8 index = b.stages[SHADERSTAGE_COMPUTE];
		if (index != LOVE_UINT8_MAX)
		{
			setBuffer(encoder, bindings, index, b.buffer, 0);
			if ((b.access & Shader::ACCESS_WRITE) != 0 && b.buffer == nil)
				allWritableVariablesSet = false;
		}
	}

	return allWritableVariablesSet;
}

void Graphics::applyShaderUniforms(id<MTLRenderCommandEncoder> renderEncoder, love::graphics::Shader *shader, love::graphics::Texture *maintex)
{
	Shader *s = (Shader *)shader;

#if defined(LOVE_MACOS) || TARGET_OS_SIMULATOR || TARGET_OS_MACCATALYST
	size_t alignment = 256;
#else
	size_t alignment = 16;
#endif

	size_t size = s->getLocalUniformBufferSize();
	uint8 *bufferdata = s->getLocalUniformBufferData();

	auto builtins = (Shader::BuiltinUniformData *) (bufferdata + s->getBuiltinUniformDataOffset());

	builtins->transformMatrix = getTransform();
	builtins->projectionMatrix = getDeviceProjection();

	builtins->scaleParams.x = (float) getCurrentDPIScale();
	builtins->scaleParams.y = getPointSize();

	uint32 flags = Shader::CLIP_TRANSFORM_Z_NEG1_1_TO_0_1;
	builtins->clipSpaceParams = Shader::computeClipSpaceParams(flags);

	builtins->screenSizeParams = Vector4(getPixelWidth(), getPixelHeight(), 1.0f, 0.0f);
	auto rt = states.back().renderTargets.getFirstTarget();
	if (rt.texture.get())
	{
		builtins->screenSizeParams.x = rt.texture->getPixelWidth(rt.mipmap);
		builtins->screenSizeParams.y = rt.texture->getPixelHeight(rt.mipmap);
	}

	builtins->constantColor = getColor();
	gammaCorrectColor(builtins->constantColor);

	if (uniformBuffer->getSize() < uniformBufferOffset + size)
	{
		size_t newsize = uniformBuffer->getSize() * 2;
		if (uniformBufferOffset > 0)
			uniformBuffer->nextFrame();
		uniformBuffer->release();
		uniformBuffer = CreateStreamBuffer(device, BUFFERUSAGE_UNIFORM, newsize);
		uniformBufferData = {};
		uniformBufferOffset = 0;
	}

	if (uniformBufferData.data == nullptr)
	{
		uniformBufferData = uniformBuffer->map(uniformBuffer->getSize());
		uniformBufferGPUStart = uniformBuffer->getGPUReadOffset();
	}

	memcpy(uniformBufferData.data + uniformBufferOffset, bufferdata, size);

	id<MTLBuffer> buffer = getMTLBuffer(uniformBuffer);
	int uniformindex = Shader::getUniformBufferBinding();

	auto &bindings = renderBindings;
	setBuffer(renderEncoder, bindings, SHADERSTAGE_VERTEX, uniformindex, buffer, uniformBufferGPUStart + uniformBufferOffset);
	setBuffer(renderEncoder, bindings, SHADERSTAGE_PIXEL, uniformindex, buffer, uniformBufferGPUStart + uniformBufferOffset);

	uniformBufferOffset += alignUp(size, alignment);

	for (const Shader::TextureBinding &b : s->getTextureBindings())
	{
		id<MTLTexture> texture = b.texture;
		auto samplertex = b.samplerTexture;

		if (b.isMainTexture)
		{
			texture = getMTLTexture(maintex);
			samplertex = maintex;
		}

		uint8 texindex = b.textureStages[SHADERSTAGE_VERTEX];
		uint8 sampindex = b.samplerStages[SHADERSTAGE_VERTEX];

		if (texindex != LOVE_UINT8_MAX)
			setTexture(renderEncoder, bindings, SHADERSTAGE_VERTEX, texindex, texture);
		if (sampindex != LOVE_UINT8_MAX)
			setSampler(renderEncoder, bindings, SHADERSTAGE_VERTEX, sampindex, samplertex);

		texindex = b.textureStages[SHADERSTAGE_PIXEL];
		sampindex = b.samplerStages[SHADERSTAGE_PIXEL];

		if (texindex != LOVE_UINT8_MAX)
			setTexture(renderEncoder, bindings, SHADERSTAGE_PIXEL, texindex, texture);
		if (sampindex != LOVE_UINT8_MAX)
			setSampler(renderEncoder, bindings, SHADERSTAGE_PIXEL, sampindex, samplertex);
	}

	for (const Shader::BufferBinding &b : s->getBufferBindings())
	{
		uint8 index = b.stages[SHADERSTAGE_VERTEX];
		if (index != LOVE_UINT8_MAX)
			setBuffer(renderEncoder, bindings, SHADERSTAGE_VERTEX, index, b.buffer, 0);
		index = b.stages[SHADERSTAGE_PIXEL];
		if (index != LOVE_UINT8_MAX)
			setBuffer(renderEncoder, bindings, SHADERSTAGE_PIXEL, index, b.buffer, 0);
	}
}

static void setVertexBuffers(id<MTLRenderCommandEncoder> encoder, love::graphics::Shader *shader, const BufferBindings *buffers, Graphics::RenderEncoderBindings &bindings)
{
	Shader *s = (Shader *)shader;
	int firstBinding = s->getFirstVertexBufferBinding();
	uint32 allbits = buffers->useBits;
	uint32 i = 0;
	while (allbits)
	{
		uint32 bit = 1u << i;

		if (buffers->useBits & bit)
		{
			auto b = buffers->info[i];
			id<MTLBuffer> buffer = getMTLBuffer(b.buffer);
			setBuffer(encoder, bindings, SHADERSTAGE_VERTEX, firstBinding + i, buffer, b.offset);
		}

		i++;
		allbits >>= 1;
	}
}

void Graphics::draw(const DrawCommand &cmd)
{ @autoreleasepool {
	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	if (cmd.cullMode != lastCullMode)
	{
		lastCullMode = cmd.cullMode;
		dirtyRenderState |= STATEBIT_CULLMODE;
	}

	applyRenderState(encoder, cmd.attributesID);
	applyShaderUniforms(encoder, Shader::current, cmd.texture);

	setVertexBuffers(encoder, Shader::current, cmd.buffers, renderBindings);

	if (cmd.indirectBuffer != nullptr)
	{
		[encoder drawPrimitives:getMTLPrimitiveType(cmd.primitiveType)
				 indirectBuffer:getMTLBuffer(cmd.indirectBuffer)
		   indirectBufferOffset:cmd.indirectBufferOffset];
	}
	else
	{
		[encoder drawPrimitives:getMTLPrimitiveType(cmd.primitiveType)
					vertexStart:cmd.vertexStart
					vertexCount:cmd.vertexCount
				  instanceCount:cmd.instanceCount];
	}

	++drawCalls;
}}

void Graphics::draw(const DrawIndexedCommand &cmd)
{ @autoreleasepool {
	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	if (cmd.cullMode != lastCullMode)
	{
		lastCullMode = cmd.cullMode;
		dirtyRenderState |= STATEBIT_CULLMODE;
	}

	applyRenderState(encoder, cmd.attributesID);
	applyShaderUniforms(encoder, Shader::current, cmd.texture);

	setVertexBuffers(encoder, Shader::current, cmd.buffers, renderBindings);

	auto indexType = cmd.indexType == INDEX_UINT32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;

	if (cmd.indirectBuffer != nullptr)
	{
		[encoder drawIndexedPrimitives:getMTLPrimitiveType(cmd.primitiveType)
							 indexType:indexType
						   indexBuffer:getMTLBuffer(cmd.indexBuffer)
					 indexBufferOffset:cmd.indexBufferOffset
						indirectBuffer:getMTLBuffer(cmd.indirectBuffer)
				  indirectBufferOffset:cmd.indexBufferOffset];
	}
	else
	{
		[encoder drawIndexedPrimitives:getMTLPrimitiveType(cmd.primitiveType)
							indexCount:cmd.indexCount
							 indexType:indexType
						   indexBuffer:getMTLBuffer(cmd.indexBuffer)
					 indexBufferOffset:cmd.indexBufferOffset
						 instanceCount:cmd.instanceCount];
	}

	++drawCalls;
}}

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
{ @autoreleasepool {
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW    = MAX_VERTICES_PER_DRAW / 4;

	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	if (lastCullMode != CULL_NONE)
	{
		lastCullMode = CULL_NONE;
		dirtyRenderState |= STATEBIT_CULLMODE;
	}

	applyRenderState(encoder, attributesID);
	applyShaderUniforms(encoder, Shader::current, texture);

	id<MTLBuffer> ib = getMTLBuffer(quadIndexBuffer);

	// Some older iOS devices don't support base vertex rendering.
	if (families.apple[3] || families.mac[1] || families.macCatalyst[1])
	{
		setVertexBuffers(encoder, Shader::current, &buffers, renderBindings);

		int basevertex = start * 4;

		for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
		{
			int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

			[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
								indexCount:quadcount * 6
								 indexType:MTLIndexTypeUInt16
							   indexBuffer:ib
						 indexBufferOffset:0
							 instanceCount:1
								baseVertex:basevertex
							  baseInstance:0];

			++drawCalls;
			basevertex += quadcount * 4;
		}
	}
	else
	{
		VertexAttributes attributes;
		findVertexAttributes(attributesID, attributes);

		BufferBindings bufferscopy = buffers;
		if (start > 0)
			advanceVertexOffsets(attributes, bufferscopy, start * 4);

		for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
		{
			setVertexBuffers(encoder, Shader::current, &bufferscopy, renderBindings);

			int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

			[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
								indexCount:quadcount * 6
								 indexType:MTLIndexTypeUInt16
							   indexBuffer:ib
						 indexBufferOffset:0];

			++drawCalls;

			if (count > MAX_QUADS_PER_DRAW)
				advanceVertexOffsets(attributes, bufferscopy, quadcount * 4);
		}
	}
}}

bool Graphics::dispatch(love::graphics::Shader *s, int x, int y, int z)
{ @autoreleasepool {
	auto shader = (Shader *) s;

	int tX, tY, tZ;
	shader->getLocalThreadgroupSize(&tX, &tY, &tZ);

	id<MTLComputePipelineState> pipeline = shader->getComputePipeline();
	if (pipeline == nil)
		return false;

	id<MTLComputeCommandEncoder> computeEncoder = useComputeEncoder();

	if (!applyShaderUniforms(computeEncoder, shader))
		return false;

	// TODO: track this state?
	[computeEncoder setComputePipelineState:pipeline];

	[computeEncoder dispatchThreadgroups:MTLSizeMake(x, y, z)
				   threadsPerThreadgroup:MTLSizeMake(tX, tY, tZ)];

	return true;
}}

bool Graphics::dispatch(love::graphics::Shader *s, love::graphics::Buffer *indirectargs, size_t argsoffset)
{
	auto shader = (Shader *) s;

	int tX, tY, tZ;
	shader->getLocalThreadgroupSize(&tX, &tY, &tZ);

	id<MTLComputePipelineState> pipeline = shader->getComputePipeline();
	if (pipeline == nil)
		return false;

	id<MTLComputeCommandEncoder> computeEncoder = useComputeEncoder();

	if (!applyShaderUniforms(computeEncoder, shader))
		return false;

	// TODO: track this state?
	[computeEncoder setComputePipelineState:pipeline];

	[computeEncoder dispatchThreadgroupsWithIndirectBuffer:getMTLBuffer(indirectargs)
									  indirectBufferOffset:argsoffset
									 threadsPerThreadgroup:MTLSizeMake(tX, tY, tZ)];

	return true;
}

void Graphics::setRenderTargetsInternal(const RenderTargets &rts, int /*pixelw*/, int /*pixelh*/, bool /*hasSRGBtexture*/)
{ @autoreleasepool {
	endPass(false);

	bool isbackbuffer = rts.getFirstTarget().texture == nullptr;

	// Set up render pass descriptor for the next useRenderEncoder call.
	// The backbuffer will be set up in useRenderEncoder rather than here.
	for (size_t i = 0; i < rts.colors.size(); i++)
	{
		auto desc = passDesc.colorAttachments[i];
		setAttachment(rts.colors[i], desc, attachmentStoreActions.color[i]);
		passDesc.colorAttachments[i] = desc;
	}

	for (size_t i = rts.colors.size(); i < MAX_COLOR_RENDER_TARGETS; i++)
	{
		passDesc.colorAttachments[i] = nil;
		passDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
	}

	passDesc.depthAttachment = nil;
	passDesc.depthAttachment.loadAction = MTLLoadActionLoad;
	passDesc.stencilAttachment = nil;
	passDesc.stencilAttachment.loadAction = MTLLoadActionLoad;

	auto ds = rts.depthStencil.texture;
	if (isbackbuffer && ds == nullptr)
		ds = backbufferDepthStencil;

	PixelFormat dsformat = PIXELFORMAT_UNKNOWN;
	if (ds != nullptr)
	{
		RenderTarget rt = rts.depthStencil;
		rt.texture = ds;
		dsformat = ds->getPixelFormat();

		if (isPixelFormatDepth(dsformat))
			setAttachment(rt, passDesc.depthAttachment, attachmentStoreActions.depth);

		if (isPixelFormatStencil(dsformat))
			setAttachment(rt, passDesc.stencilAttachment, attachmentStoreActions.stencil);
	}

	if (!isbackbuffer)
	{
		lastRenderPipelineKey.colorRenderTargetFormats = 0;
		for (size_t i = 0; i < rts.colors.size(); i++)
			lastRenderPipelineKey.colorRenderTargetFormats |= (rts.colors[i].texture->getPixelFormat()) << (8 * i);

		lastRenderPipelineKey.msaa = (uint8) rts.getFirstTarget().texture->getMSAA();
	}

	lastRenderPipelineKey.depthStencilFormat = dsformat;
	lastRenderPipelineKey.vertexAttributesID = VertexAttributesID();

	dirtyRenderState = STATEBIT_ALL;
}}

void Graphics::endPass(bool presenting)
{
	// Make sure the encoder gets set up, if nothing else has done it yet.
	useRenderEncoder();

	flushBatchedDraws();

	auto &rts = states.back().renderTargets;
	love::graphics::Texture *depthstencil = rts.depthStencil.texture.get();

	// Discard the depth/stencil buffer if we're using an internal cached one,
	// or if we're presenting the backbuffer to the display.
	if ((depthstencil == nullptr && (rts.temporaryRTFlags & (TEMPORARY_RT_DEPTH | TEMPORARY_RT_STENCIL)) != 0)
		|| (presenting && !rts.getFirstTarget().texture.get()))
	{
		attachmentStoreActions.depth = MTLStoreActionDontCare;
		attachmentStoreActions.stencil = MTLStoreActionDontCare;
	}

	submitRenderEncoder(SUBMIT_DONE);
}

void Graphics::clear(OptionalColorD c, OptionalInt stencil, OptionalDouble depth)
{ @autoreleasepool {
	if (c.hasValue || stencil.hasValue || depth.hasValue)
	{
		flushBatchedDraws();

		// Handle clearing mid-pass by starting a new pass.
		if (renderEncoder != nil)
		{
			submitRenderEncoder(SUBMIT_STORE);
			useRenderEncoder();
		}
	}

	if (c.hasValue)
	{
		Colorf cf((float)c.value.r, (float)c.value.g, (float)c.value.b, (float)c.value.a);
		gammaCorrectColor(cf);
		auto color = MTLClearColorMake(cf.r, cf.g, cf.b, cf.a);
		for (int i = 0; i < MAX_COLOR_RENDER_TARGETS; i++)
		{
			passDesc.colorAttachments[i].clearColor = color;
			passDesc.colorAttachments[i].loadAction = MTLLoadActionClear;
		}
	}

	if (stencil.hasValue)
	{
		passDesc.stencilAttachment.clearStencil = stencil.value;
		passDesc.stencilAttachment.loadAction = MTLLoadActionClear;
	}

	if (depth.hasValue)
	{
		passDesc.depthAttachment.clearDepth = depth.value;
		passDesc.depthAttachment.loadAction = MTLLoadActionClear;
	}
}}

void Graphics::clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth)
{ @autoreleasepool {
	if (colors.size() == 0 && !stencil.hasValue && !depth.hasValue)
		return;

	int ncolorcanvases = (int) states.back().renderTargets.colors.size();
	int ncolors = (int) colors.size();

	if (ncolors <= 1 && ncolorcanvases <= 1)
	{
		clear(ncolors > 0 ? colors[0] : OptionalColorD(), stencil, depth);
		return;
	}

	flushBatchedDraws();

	// Handle clearing mid-pass by starting a new pass.
	if (renderEncoder != nil)
	{
		submitRenderEncoder(SUBMIT_STORE);
		useRenderEncoder();
	}

	for (int i = 0; i < ncolors; i++)
	{
		if (!colors[i].hasValue)
			continue;

		const ColorD &cd = colors[i].value;
		Colorf cf((float)cd.r, (float)cd.g, (float)cd.b, (float)cd.a);
		gammaCorrectColor(cf);

		passDesc.colorAttachments[i].clearColor = MTLClearColorMake(cf.r, cf.g, cf.b, cf.a);
		passDesc.colorAttachments[i].loadAction = MTLLoadActionClear;
	}

	if (stencil.hasValue)
	{
		passDesc.stencilAttachment.clearStencil = stencil.value;
		passDesc.stencilAttachment.loadAction = MTLLoadActionClear;
	}

	if (depth.hasValue)
	{
		passDesc.depthAttachment.clearDepth = depth.value;
		passDesc.depthAttachment.loadAction = MTLLoadActionClear;
	}
}}

void Graphics::discard(const std::vector<bool> &colorbuffers, bool depthstencil)
{ @autoreleasepool {
	flushBatchedDraws();

	// TODO
	if (renderEncoder != nil)
		return;

	size_t maxcolor = isRenderTargetActive() ? states.back().renderTargets.colors.size() : 1;
	size_t ncolor = std::min(maxcolor, colorbuffers.size());

	for (size_t i = 0; i < ncolor; i++)
	{
		if (colorbuffers[i])
			passDesc.colorAttachments[i].loadAction = MTLLoadActionDontCare;
	}

	if (depthstencil)
	{
		passDesc.stencilAttachment.loadAction = MTLLoadActionDontCare;
		passDesc.depthAttachment.loadAction = MTLLoadActionDontCare;
	}
}}

void Graphics::present(void *screenshotCallbackData)
{ @autoreleasepool {
	if (!isActive())
		return;

	if (isRenderTargetActive())
		throw love::Exception("present cannot be called while a render target is active.");

	deprecations.draw(this);

	// endPass calls useRenderEncoder, which makes sure activeDrawable is set
	// when possible.
	endPass(true);

	id<MTLBuffer> screenshotbuffer = nil;

	int w = activeDrawable.texture.width;
	int h = activeDrawable.texture.height;

	if (!pendingScreenshotCallbacks.empty())
	{
		size_t size = w * h * 4;

		screenshotbuffer = [device newBufferWithLength:size options:MTLResourceStorageModeShared];
		if (screenshotbuffer == nil)
			throw love::Exception("Out of graphics memory.");

		auto blitencoder = useBlitEncoder();

		[blitencoder copyFromTexture:activeDrawable.texture
						 sourceSlice:0
						 sourceLevel:0
						sourceOrigin:MTLOriginMake(0, 0, 0)
						  sourceSize:MTLSizeMake(w, h, 1)
							toBuffer:screenshotbuffer
				   destinationOffset:0
			  destinationBytesPerRow:w * 4
			destinationBytesPerImage:size];

		submitBlitEncoder();
	}

	for (StreamBuffer *buffer : batchedDrawState.vb)
		buffer->nextFrame();
	batchedDrawState.indexBuffer->nextFrame();

	uniformBuffer->nextFrame();
	uniformBufferData = {};
	uniformBufferOffset = 0;

	id<MTLCommandBuffer> cmd = getCommandBuffer();

	if (cmd != nil && activeDrawable != nil)
		[cmd presentDrawable:activeDrawable];

	submitCommandBuffer(SUBMIT_DONE);

	activeDrawable = nil;

	if (!pendingScreenshotCallbacks.empty())
	{
		[cmd waitUntilCompleted];

		size_t size = w * h * 4;

		auto imagemodule = Module::getInstance<love::image::Image>(M_IMAGE);

		for (int i = 0; i < (int) pendingScreenshotCallbacks.size(); i++)
		{
			const auto &info = pendingScreenshotCallbacks[i];
			image::ImageData *img = nullptr;

			try
			{
				img = imagemodule->newImageData(w, h, PIXELFORMAT_RGBA8_UNORM, screenshotbuffer.contents);
			}
			catch (love::Exception &)
			{
				info.callback(&info, nullptr, nullptr);
				for (int j = i + 1; j < (int) pendingScreenshotCallbacks.size(); j++)
				{
					const auto &ninfo = pendingScreenshotCallbacks[j];
					ninfo.callback(&ninfo, nullptr, nullptr);
				}
				pendingScreenshotCallbacks.clear();
				throw;
			}

			uint8 *screenshot = (uint8 *) img->getData();

			// Convert from BGRA to RGBA and replace alpha with full opacity.
			for (size_t i = 0; i < size; i += 4)
			{
				uint8 r = screenshot[i + 2];
				screenshot[i + 2] = screenshot[i + 0];
				screenshot[i + 0] = r;
				screenshot[i + 3] = 255;
			}

			info.callback(&info, img, screenshotCallbackData);
			img->release();
		}

		pendingScreenshotCallbacks.clear();
	}

	auto window = Module::getInstance<love::window::Window>(M_WINDOW);
	if (window != nullptr)
		window->swapBuffers();

	// This is set to NO when there are pending screen captures.
	metalLayer.framebufferOnly = YES;

	// Reset the per-frame stat counts.
	drawCalls = 0;
	shaderSwitches = 0;
	renderTargetSwitchCount = 0;
	drawCallsBatched = 0;

	updatePendingReadbacks();
	updateTemporaryResources();
	processCompletedCommandBuffers();
}}

int Graphics::getRequestedBackbufferMSAA() const
{
	return requestedBackbufferMSAA;
}

int Graphics::getBackbufferMSAA() const
{
	return backbufferMSAA.get() ? backbufferMSAA->getMSAA() : 0;
}

void Graphics::setColor(Colorf c)
{
	c.r = std::min(std::max(c.r, 0.0f), 1.0f);
	c.g = std::min(std::max(c.g, 0.0f), 1.0f);
	c.b = std::min(std::max(c.b, 0.0f), 1.0f);
	c.a = std::min(std::max(c.a, 0.0f), 1.0f);

	states.back().color = c;
}

void Graphics::setScissor(const Rect &rect)
{
	flushBatchedDraws();

	DisplayState &state = states.back();
	state.scissor = true;
	state.scissorRect = rect;
	dirtyRenderState |= STATEBIT_SCISSOR;
}

void Graphics::setScissor()
{
	DisplayState &state = states.back();
	if (state.scissor)
	{
		flushBatchedDraws();
		state.scissor = false;
		dirtyRenderState |= STATEBIT_SCISSOR;
	}
}

void Graphics::setStencilState(const StencilState &s)
{
	validateStencilState(s);

	flushBatchedDraws();

	states.back().stencil = s;
	dirtyRenderState |= STATEBIT_STENCIL;
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
	validateDepthState(write);

	DisplayState &state = states.back();

	if (state.depthTest != compare || state.depthWrite != write)
	{
		flushBatchedDraws();
		state.depthTest = compare;
		state.depthWrite = write;
		dirtyRenderState |= STATEBIT_DEPTH;
	}
}

void Graphics::setFrontFaceWinding(Winding winding)
{
	if (states.back().winding != winding)
	{
		flushBatchedDraws();
		states.back().winding = winding;
		dirtyRenderState |= STATEBIT_FACEWINDING;
	}
}

void Graphics::setColorMask(ColorChannelMask mask)
{
	if (states.back().colorMask != mask)
	{
		flushBatchedDraws();
		states.back().colorMask = mask;
		dirtyRenderState |= STATEBIT_COLORMASK;
	}
}

void Graphics::setBlendState(const BlendState &blend)
{
	if (!(blend == states.back().blend))
	{
		flushBatchedDraws();
		states.back().blend = blend;
		dirtyRenderState |= STATEBIT_BLEND;
	}
}

void Graphics::setPointSize(float size)
{
	if (size != states.back().pointSize)
		flushBatchedDraws();

	states.back().pointSize = size;
}

void Graphics::setWireframe(bool enable)
{
	if (enable != states.back().wireframe)
	{
		flushBatchedDraws();
		states.back().wireframe = enable;
		dirtyRenderState |= STATEBIT_WIREFRAME;
	}
}

bool Graphics::isPixelFormatSupported(PixelFormat format, uint32 usage)
{
	format = getSizedFormat(format);

	const uint32 sample = PIXELFORMATUSAGEFLAGS_SAMPLE;
	const uint32 filter = PIXELFORMATUSAGEFLAGS_LINEAR;
	const uint32 rt = PIXELFORMATUSAGEFLAGS_RENDERTARGET;
	const uint32 blend = PIXELFORMATUSAGEFLAGS_BLEND;
	const uint32 msaa = PIXELFORMATUSAGEFLAGS_MSAA;
	const uint32 commonsample = PIXELFORMATUSAGEFLAGS_SAMPLE | PIXELFORMATUSAGEFLAGS_LINEAR;
	const uint32 commonrender = PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_BLEND | PIXELFORMATUSAGEFLAGS_MSAA;
	const uint32 computewrite = PIXELFORMATUSAGEFLAGS_COMPUTEWRITE;
	const uint32 all = commonsample | commonrender | computewrite;

	uint32 flags = PIXELFORMATUSAGEFLAGS_NONE;

	if (isPixelFormatCompressed(format) && (usage & rt) != 0)
		return false;

	// https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
	switch (format)
	{
		case PIXELFORMAT_UNKNOWN:
		case PIXELFORMAT_NORMAL:
		case PIXELFORMAT_HDR:
			break;

		case PIXELFORMAT_R8_UNORM:
			flags |= all;
			break;
		case PIXELFORMAT_R16_UNORM:
			if (families.apple[1])
				flags |= commonsample | commonrender | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= all;
			break;
		case PIXELFORMAT_R16_FLOAT:
			flags |= all;
			break;
		case PIXELFORMAT_R32_FLOAT:
			if (families.apple[1])
				flags |= sample | rt | blend | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= (all & ~(msaa | filter));
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				if (device.supports32BitFloatFiltering)
					flags |= filter;
				if (device.supports32BitMSAA)
					flags |= msaa;
			}
			break;

		case PIXELFORMAT_RG8_UNORM:
			flags |= all;
			break;
		case PIXELFORMAT_LA8_UNORM:
			// Requires texture swizzle support.
			if (@available(macOS 10.15, iOS 13, *))
			{
				// As of early 2024, the VM device doesn't properly support texture swizzles
				// (observed on GitHub's runners) which is required for LA8 support.
				if (!isVMDevice)
				{
					if (families.apple[1] || families.mac[2] || families.macCatalyst[2])
						flags |= commonsample;
				}
			}
			break;
		case PIXELFORMAT_RG16_UNORM:
			if (families.apple[1])
				flags |= commonsample | rt | blend | msaa | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= all;
			break;
		case PIXELFORMAT_RG16_FLOAT:
			flags |= all;
			break;
		case PIXELFORMAT_RG32_FLOAT:
			if (families.apple[1])
				flags |= sample | rt | blend | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= (all & ~(msaa | filter));
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				if (device.supports32BitFloatFiltering)
					flags |= filter;
				if (device.supports32BitMSAA)
					flags |= msaa;
			}
			break;

		case PIXELFORMAT_RGBA8_UNORM:
		case PIXELFORMAT_BGRA8_UNORM:
			flags |= all;
			break;
		case PIXELFORMAT_RGBA8_sRGB:
		case PIXELFORMAT_BGRA8_sRGB:
			if (families.apple[1] || families.mac[1] || families.macCatalyst[1])
				flags |= commonsample | commonrender;
			if (families.apple[2])
				flags |= all;
			break;

		case PIXELFORMAT_RGBA16_UNORM:
			if (families.apple[1])
				flags |= commonsample | rt | msaa | blend | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= all;
			break;
		case PIXELFORMAT_RGBA16_FLOAT:
			flags |= all;
			break;
		case PIXELFORMAT_RGBA32_FLOAT:
			if (families.apple[1])
				flags |= sample | rt | computewrite;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= (all & ~(msaa | filter));
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				if (device.supports32BitFloatFiltering)
					flags |= filter;
				if (device.supports32BitMSAA)
					flags |= msaa;
			}
			break;

		case PIXELFORMAT_R8_INT:
		case PIXELFORMAT_R8_UINT:
		case PIXELFORMAT_RG8_INT:
		case PIXELFORMAT_RG8_UINT:
		case PIXELFORMAT_RGBA8_INT:
		case PIXELFORMAT_RGBA8_UINT:
		case PIXELFORMAT_R16_INT:
		case PIXELFORMAT_R16_UINT:
		case PIXELFORMAT_RG16_INT:
		case PIXELFORMAT_RG16_UINT:
		case PIXELFORMAT_RGBA16_INT:
		case PIXELFORMAT_RGBA16_UINT:
		case PIXELFORMAT_R32_INT:
		case PIXELFORMAT_R32_UINT:
		case PIXELFORMAT_RG32_INT:
		case PIXELFORMAT_RG32_UINT:
		case PIXELFORMAT_RGBA32_INT:
		case PIXELFORMAT_RGBA32_UINT:
			// If MSAA support for int formats is added this should be split up.
			flags |= sample | rt | computewrite;
			break;

		case PIXELFORMAT_RGBA4_UNORM:
		case PIXELFORMAT_RGB5A1_UNORM:
		case PIXELFORMAT_RGB565_UNORM:
			if (families.apple[1])
				flags |= commonsample | rt | blend | msaa; // | resolve
			break;
		case PIXELFORMAT_RGB10A2_UNORM:
		case PIXELFORMAT_RG11B10_FLOAT:
			if (families.apple[1])
				flags |= commonsample | rt | blend | msaa; // | resolve
			if (families.apple[3])
				flags |= all;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= all;
			break;

		case PIXELFORMAT_STENCIL8:
			flags |= rt | sample | msaa;
			break;
		case PIXELFORMAT_DEPTH16_UNORM:
			flags |= rt | commonsample | msaa;
			//if (families.apple[3] || families.mac[1] || families.macCatalyst[1])
			//	flags |= resolve;
			break;
		case PIXELFORMAT_DEPTH24_UNORM: // This is emulated via 32f.
		case PIXELFORMAT_DEPTH32_FLOAT:
			if (families.apple[1])
				flags |= rt | sample | msaa;
			if (families.apple[3])
				flags |= rt | sample | msaa; // | resolve;
			if (families.mac[1] || families.macCatalyst[1])
				flags |= rt | commonsample | msaa; // | resolve;
			break;
		case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
			// Note: this falls back to 32f_s8 on some systems.
			flags |= rt | sample | msaa;
			break;
		case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
			if (families.apple[1])
				flags |= rt | sample | msaa;
			if (families.apple[3])
				flags |= rt | sample | msaa; // | resolve
			if (families.mac[1] || families.macCatalyst[1])
				flags |= rt | commonsample | msaa; // | resolve
			break;

		case PIXELFORMAT_DXT1_UNORM:
		case PIXELFORMAT_DXT1_sRGB:
		case PIXELFORMAT_DXT3_UNORM:
		case PIXELFORMAT_DXT3_sRGB:
		case PIXELFORMAT_DXT5_UNORM:
		case PIXELFORMAT_DXT5_sRGB:
		case PIXELFORMAT_BC4_UNORM:
		case PIXELFORMAT_BC4_SNORM:
		case PIXELFORMAT_BC5_UNORM:
		case PIXELFORMAT_BC5_SNORM:
		case PIXELFORMAT_BC6H_UFLOAT:
		case PIXELFORMAT_BC6H_FLOAT:
		case PIXELFORMAT_BC7_UNORM:
		case PIXELFORMAT_BC7_sRGB:
			if (families.mac[1] || families.macCatalyst[1])
				flags |= commonsample;
			break;

		case PIXELFORMAT_PVR1_RGB2_UNORM:
		case PIXELFORMAT_PVR1_RGB2_sRGB:
		case PIXELFORMAT_PVR1_RGB4_UNORM:
		case PIXELFORMAT_PVR1_RGB4_sRGB:
		case PIXELFORMAT_PVR1_RGBA2_UNORM:
		case PIXELFORMAT_PVR1_RGBA2_sRGB:
		case PIXELFORMAT_PVR1_RGBA4_UNORM:
		case PIXELFORMAT_PVR1_RGBA4_sRGB:
			if (families.apple[1])
				flags |= commonsample;
			break;

		case PIXELFORMAT_ETC1_UNORM:
		case PIXELFORMAT_ETC2_RGB_UNORM:
		case PIXELFORMAT_ETC2_RGB_sRGB:
		case PIXELFORMAT_ETC2_RGBA_UNORM:
		case PIXELFORMAT_ETC2_RGBA_sRGB:
		case PIXELFORMAT_ETC2_RGBA1_UNORM:
		case PIXELFORMAT_ETC2_RGBA1_sRGB:
		case PIXELFORMAT_EAC_R_UNORM:
		case PIXELFORMAT_EAC_R_SNORM:
		case PIXELFORMAT_EAC_RG_UNORM:
		case PIXELFORMAT_EAC_RG_SNORM:
			if (families.apple[1])
				flags |= commonsample;
			break;

		case PIXELFORMAT_ASTC_4x4_UNORM:
		case PIXELFORMAT_ASTC_5x4_UNORM:
		case PIXELFORMAT_ASTC_5x5_UNORM:
		case PIXELFORMAT_ASTC_6x5_UNORM:
		case PIXELFORMAT_ASTC_6x6_UNORM:
		case PIXELFORMAT_ASTC_8x5_UNORM:
		case PIXELFORMAT_ASTC_8x6_UNORM:
		case PIXELFORMAT_ASTC_8x8_UNORM:
		case PIXELFORMAT_ASTC_10x5_UNORM:
		case PIXELFORMAT_ASTC_10x6_UNORM:
		case PIXELFORMAT_ASTC_10x8_UNORM:
		case PIXELFORMAT_ASTC_10x10_UNORM:
		case PIXELFORMAT_ASTC_12x10_UNORM:
		case PIXELFORMAT_ASTC_12x12_UNORM:
		case PIXELFORMAT_ASTC_4x4_sRGB:
		case PIXELFORMAT_ASTC_5x4_sRGB:
		case PIXELFORMAT_ASTC_5x5_sRGB:
		case PIXELFORMAT_ASTC_6x5_sRGB:
		case PIXELFORMAT_ASTC_6x6_sRGB:
		case PIXELFORMAT_ASTC_8x5_sRGB:
		case PIXELFORMAT_ASTC_8x6_sRGB:
		case PIXELFORMAT_ASTC_8x8_sRGB:
		case PIXELFORMAT_ASTC_10x5_sRGB:
		case PIXELFORMAT_ASTC_10x6_sRGB:
		case PIXELFORMAT_ASTC_10x8_sRGB:
		case PIXELFORMAT_ASTC_10x10_sRGB:
		case PIXELFORMAT_ASTC_12x10_sRGB:
		case PIXELFORMAT_ASTC_12x12_sRGB:
			if (families.apple[2])
				flags |= commonsample;
			break;

		case PIXELFORMAT_MAX_ENUM:
			break;
	}

	return (usage & flags) == usage;
}

Renderer Graphics::getRenderer() const
{
	return RENDERER_METAL;
}

bool Graphics::usesGLSLES() const
{
#ifdef LOVE_IOS
	return true;
#else
	return false;
#endif
}

Graphics::RendererInfo Graphics::getRendererInfo() const
{
	RendererInfo info;
	info.name = "Metal";
	info.version = "2.1"; // TODO
	info.vendor = ""; // TODO
	info.device = device.name.UTF8String;
	return info;
}

int Graphics::getClosestMSAASamples(int requestedsamples)
{
	// We currently rely on StoreAndMultisampleResolve (unfortunately), which
	// isn't supported by old phone GPUs.
	if (!families.apple[3] && !families.mac[1] && !families.macCatalyst[1])
		return 1;

	const int checkmsaa[] = {32, 16, 8, 4, 2};
	for (int samples : checkmsaa)
	{
		if (samples <= requestedsamples && [device supportsTextureSampleCount:samples])
			return samples;
	}
	return 1;
}

void Graphics::initCapabilities()
{
	if (@available(macOS 10.15, iOS 13.0, *))
	{
		for (NSInteger i = 0; i < 7; i++)
		{
			MTLGPUFamily family = (MTLGPUFamily) (MTLGPUFamilyApple1 + i);
			if ([device supportsFamily:family])
				families.apple[1 + i] = true;
		}

		for (NSInteger i = 0; i < 2; i++)
		{
			MTLGPUFamily family = (MTLGPUFamily) (MTLGPUFamilyMac1 + i);
			if ([device supportsFamily:family])
				families.mac[1 + i] = true;
		}

		for (NSInteger i = 0; i < 3; i++)
		{
			MTLGPUFamily family = (MTLGPUFamily) (MTLGPUFamilyCommon1 + i);
			if ([device supportsFamily:family])
				families.common[1 + i] = true;
		}

		for (NSInteger i = 0; i < 2; i++)
		{
			MTLGPUFamily family = (MTLGPUFamily) (MTLGPUFamilyMacCatalyst1 + i);
			if ([device supportsFamily:family])
				families.macCatalyst[1 + i] = true;
		}
	}

	capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = true;
	capabilities.features[FEATURE_CLAMP_ZERO] = true;
	capabilities.features[FEATURE_CLAMP_ONE] = false;
	if (@available(macOS 10.12, iOS 14.0, *))
	{
		// Requires "border color" feature.
		if (families.mac[1] || families.macCatalyst[1] || families.apple[7])
			capabilities.features[FEATURE_CLAMP_ONE] = true;
	}
	capabilities.features[FEATURE_LIGHTEN] = true;
	capabilities.features[FEATURE_FULL_NPOT] = true;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = true;
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = true;
	capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = true;

	if (families.mac[1] || families.macCatalyst[1] || families.apple[3])
		capabilities.features[FEATURE_INDIRECT_DRAW] = true;
	else
		capabilities.features[FEATURE_INDIRECT_DRAW] = false;
	
	static_assert(FEATURE_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	// https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
	capabilities.limits[LIMIT_POINT_SIZE] = 511;
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = 2048;
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = 2048;
	if (families.mac[1] || families.macCatalyst[1] || families.apple[3])
	{
		capabilities.limits[LIMIT_TEXTURE_SIZE] = 16384;
		capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = 16384;
	}
	else
	{
		capabilities.limits[LIMIT_TEXTURE_SIZE] = 8192;
		capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = 8192;
	}

	// TODO: metal doesn't have a good API to query this?
	capabilities.limits[LIMIT_TEXEL_BUFFER_SIZE] = 128 * 1024 * 1024;

	if (@available(macOS 10.14, iOS 12.0, *))
	{
		NSUInteger buffersize = [device maxBufferLength];
		capabilities.limits[LIMIT_SHADER_STORAGE_BUFFER_SIZE] = buffersize;
	}
	else
	{
		capabilities.limits[LIMIT_SHADER_STORAGE_BUFFER_SIZE] = 128 * 1024 * 1024;
	}

	capabilities.limits[LIMIT_THREADGROUPS_X] = LOVE_INT32_MAX; // TODO: is there a real limit?
	capabilities.limits[LIMIT_THREADGROUPS_Y] = LOVE_INT32_MAX;
	capabilities.limits[LIMIT_THREADGROUPS_Z] = LOVE_INT32_MAX;

	if (families.mac[1] || families.macCatalyst[1] || families.apple[2])
		capabilities.limits[LIMIT_RENDER_TARGETS] = 8;
	else
		capabilities.limits[LIMIT_RENDER_TARGETS] = 4;
	capabilities.limits[LIMIT_TEXTURE_MSAA] = getClosestMSAASamples(32);
	capabilities.limits[LIMIT_ANISOTROPY] = 16.0f;
	static_assert(LIMIT_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new system limit!");

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
		capabilities.textureTypes[i] = true;
}

void Graphics::getAPIStats(int &shaderswitches) const
{
	shaderswitches = shaderSwitches;
}

} // metal
} // graphics
} // love
