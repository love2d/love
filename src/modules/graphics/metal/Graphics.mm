/**
 * Copyright (c) 2006-2020 LOVE Development Team
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
#include "Shader.h"
#include "ShaderStage.h"
#include "window/Window.h"
#include "image/Image.h"
#include "common/memory.h"

#import <QuartzCore/CAMetalLayer.h>

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
#ifdef LOVE_MACOS
		case SamplerState::WRAP_CLAMP_ONE: return MTLSamplerAddressModeClampToBorderColor;
#else
		case SamplerState::WRAP_CLAMP_ONE: return MTLSamplerAddressModeClampToZero;
#endif
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
		case PRIMITIVE_TRIANGLE_FAN: return MTLPrimitiveTypeTriangle; // This needs to be emulated.
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
	: device(nil)
	, commandQueue(nil)
	, commandBuffer(nil)
	, renderEncoder(nil)
	, blitEncoder(nil)
	, metalLayer(nil)
	, activeDrawable(nil)
	, passDesc(nil)
	, dirtyRenderState(STATEBIT_ALL)
	, windowHasStencil(false)
	, uniformBufferOffset(0)
	, defaultAttributesBuffer(nullptr)
	, defaultTextures()
{ @autoreleasepool {
	graphicsInstance = this;
	device = MTLCreateSystemDefaultDevice();
	if (device == nil)
		throw love::Exception("Metal is not supported on this system.");

	commandQueue = [device newCommandQueue];
	passDesc = [MTLRenderPassDescriptor renderPassDescriptor];

	initCapabilities();

	uniformBuffer = CreateStreamBuffer(device, BUFFERTYPE_UNIFORM, 1024 * 1024 * 1);

	{
		std::vector<Buffer::DataDeclaration> dataformat = {
			{"floats", DATAFORMAT_FLOAT_VEC4, 0},
			{"ints", DATAFORMAT_INT32_VEC4, 0},
		};

		DefaultVertexAttributes defaults = {
			{0.0f, 0.0f, 0.0f, 1.0f},
			{0, 0, 0, 1},
		};

		Buffer::Settings attribsettings(Buffer::TYPEFLAG_VERTEX, BUFFERUSAGE_STATIC);

		defaultAttributesBuffer = newBuffer(attribsettings, dataformat, &defaults, sizeof(DefaultVertexAttributes), 0);
	}

	uint8 defaultpixel[] = {255, 255, 255, 255};
	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
	{
		Texture::Settings settings;
		settings.type = (TextureType) i;
		settings.format = PIXELFORMAT_RGBA8_UNORM;
		defaultTextures[i] = newTexture(settings);
		Rect r = {0, 0, 1, 1};
		defaultTextures[i]->replacePixels(defaultpixel, sizeof(defaultpixel), 0, 0, r, false);
	}

	auto window = Module::getInstance<love::window::Window>(M_WINDOW);

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

			void *context = nullptr; // TODO
			setMode(context, (int) dpiW, (int) dpiH, window->getPixelWidth(), window->getPixelHeight(), settings.stencil, settings.depth);
		}
	}
}}

Graphics::~Graphics()
{ @autoreleasepool {
	submitCommandBuffer();
	delete uniformBuffer;
	delete defaultAttributesBuffer;
	passDesc = nil;
	commandQueue = nil;
	device = nil;

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
		defaultTextures[i]->release();

	for (auto &kvp : cachedSamplers)
		CFBridgingRelease(kvp.second);

	for (auto &kvp : cachedDepthStencilStates)
		CFBridgingRelease(kvp.second);

	graphicsInstance = nullptr;
}}

love::graphics::StreamBuffer *Graphics::newStreamBuffer(BufferType type, size_t size)
{
	return CreateStreamBuffer(device, type, size);
}

love::graphics::Texture *Graphics::newTexture(const Texture::Settings &settings, const Texture::Slices *data)
{
	return new Texture(this, device, settings, data);
}

love::graphics::ShaderStage *Graphics::newShaderStageInternal(ShaderStage::StageType stage, const std::string &cachekey, const std::string &source, bool gles)
{
	return new ShaderStage(this, stage, source, gles, cachekey);
}

love::graphics::Shader *Graphics::newShaderInternal(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
{
	return new Shader(device, vertex, pixel);
}

love::graphics::Buffer *Graphics::newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
{
	return new Buffer(this, device, settings, format, data, size, arraylength);
}

void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight)
{
	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	if (!isRenderTargetActive())
	{
		dirtyRenderState |= STATEBIT_VIEWPORT | STATEBIT_SCISSOR;

		// Set up the projection matrix
		projectionMatrix = Matrix4::ortho(0.0, (float) width, (float) height, 0.0, -10.0f, 10.0f);
	}
}

bool Graphics::setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa)
{ @autoreleasepool {
	this->width = width;
	this->height = height;
	this->metalLayer = (__bridge CAMetalLayer *) context;

	this->windowHasStencil = windowhasstencil;

	metalLayer.device = device;
	metalLayer.pixelFormat = isGammaCorrect() ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;

	// This is set to NO when there are pending screen captures.
	metalLayer.framebufferOnly = YES;

	setViewportSize(width, height, pixelwidth, pixelheight);

	created = true;

	if (batchedDrawState.vb[0] == nullptr)
	{
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		batchedDrawState.vb[0] = CreateStreamBuffer(device, BUFFERTYPE_VERTEX, 1024 * 1024 * 1);
		batchedDrawState.vb[1] = CreateStreamBuffer(device, BUFFERTYPE_VERTEX, 256  * 1024 * 1);
		batchedDrawState.indexBuffer = CreateStreamBuffer(device, BUFFERTYPE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
	}

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
			stages.push_back(Shader::getDefaultCode(stype, ShaderStage::STAGE_VERTEX));
			stages.push_back(Shader::getDefaultCode(stype, ShaderStage::STAGE_PIXEL));
			Shader::standardShaders[i] = newShader(stages);
		}
	}

	// A shader should always be active, but the default shader shouldn't be
	// returned by getShader(), so we don't do setShader(defaultShader).
	if (!Shader::current)
		Shader::standardShaders[Shader::STANDARD_DEFAULT]->attach();

	return true;
}}

void Graphics::unSetMode()
{ @autoreleasepool {
	if (!isCreated())
		return;

	flushBatchedDraws();

	submitCommandBuffer();

	for (auto temp : temporaryTextures)
		temp.texture->release();

	temporaryTextures.clear();

	created = false;
	metalLayer = nil;
	activeDrawable = nil;
}}

void Graphics::setActive(bool enable)
{
	flushBatchedDraws();
	active = enable;
}

id<MTLCommandBuffer> Graphics::useCommandBuffer()
{
	if (commandBuffer == nil)
	{
		commandBuffer = [commandQueue commandBuffer];

		Graphics *pthis = this;
		pthis->retain();
		[commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull) {
			pthis->completeCommandBufferIndex.fetch_add(1, std::memory_order_relaxed);
			pthis->release();
		}];
	}

	return commandBuffer;
}

void Graphics::submitCommandBuffer()
{
	submitRenderEncoder();
	submitBlitEncoder();

	if (commandBuffer != nil)
	{
		[commandBuffer commit];
		commandBuffer = nil;
	}
}

id<MTLRenderCommandEncoder> Graphics::useRenderEncoder()
{
	if (renderEncoder == nil)
	{
		submitBlitEncoder();

		// Pass desc info for non-backbuffer render targets are set up in
		// setRenderTagetsInternal.
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

			passDesc.colorAttachments[0].texture = activeDrawable.texture;
			passDesc.colorAttachments[0].level = 0;
			passDesc.colorAttachments[0].slice = 0;
			passDesc.colorAttachments[0].depthPlane = 0;
		}

		renderEncoder = [useCommandBuffer() renderCommandEncoderWithDescriptor:passDesc];

		id<MTLBuffer> defaultbuffer = getMTLBuffer(defaultAttributesBuffer);
		[renderEncoder setVertexBuffer:defaultbuffer offset:0 atIndex:DEFAULT_VERTEX_BUFFER_BINDING];

		dirtyRenderState = STATEBIT_ALL;
	}

	return renderEncoder;
}

void Graphics::submitRenderEncoder()
{
	if (renderEncoder != nil)
	{
		[renderEncoder endEncoding];
		renderEncoder = nil;

		passDesc.colorAttachments[0].texture = nil;
	}
}

id<MTLBlitCommandEncoder> Graphics::useBlitEncoder()
{
	if (blitEncoder == nil)
	{
		submitRenderEncoder();
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

#ifdef LOVE_MACOS
	desc.borderColor = MTLSamplerBorderColorOpaqueWhite;
#endif

	desc.lodMinClamp = s.minLod;
	desc.lodMaxClamp = s.maxLod;

	if (s.depthSampleMode.hasValue)
		desc.compareFunction = getMTLCompareFunction(s.depthSampleMode.value);

	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:desc];

	if (sampler != nil)
		cachedSamplers[key] = (void *) CFBridgingRetain(sampler);

	return sampler;
}}

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

	stencildesc.stencilCompareFunction = getMTLCompareFunction(stencil.compare);
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

void Graphics::applyRenderState(id<MTLRenderCommandEncoder> encoder, const VertexAttributes &attributes)
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
			rtw = rt.texture->getPixelWidth();
			rth = rt.texture->getPixelHeight();
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
			// TODO: clamping
			double dpiscale = getCurrentDPIScale();
			rect.x = (NSUInteger)(state.scissorRect.x*dpiscale);
			rect.y = (NSUInteger)(state.scissorRect.y*dpiscale);
			rect.width = (NSUInteger)(state.scissorRect.w*dpiscale);
			rect.height = (NSUInteger)(state.scissorRect.h*dpiscale);
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
		// TODO
	}

	// TODO: attributes
	if ((dirtyState & pipelineStateBits) != 0 || true)
	{
//		Shader *shader = (Shader *) state.shader.get();
		Shader *shader = (Shader *) Shader::current;
		id<MTLRenderPipelineState> pipeline = nil;

		if (shader)
		{
			Shader::RenderPipelineKey key;

			key.vertexAttributes = attributes;
			key.blend = state.blend;
			key.colorChannelMask = state.colorMask;

			const auto &rts = state.renderTargets.colors;

			for (size_t i = 0; i < rts.size(); i++)
				key.colorRenderTargetFormats |= (rts[i].texture->getPixelFormat()) << (8 * i);

			if (state.renderTargets.getFirstTarget().texture.get() == nullptr)
				key.colorRenderTargetFormats = isGammaCorrect() ? PIXELFORMAT_BGRA8_UNORM_sRGB : PIXELFORMAT_BGRA8_UNORM;

			// TODO: depth/stencil

			pipeline = shader->getCachedRenderPipeline(key);
		}

		[encoder setRenderPipelineState:pipeline];
	}

	if (dirtyState & (STATEBIT_DEPTH | STATEBIT_STENCIL))
	{
		DepthState depth;
		depth.compare = state.depthTest;
		depth.write = state.depthWrite;

		StencilState stencil = state.stencil;

		if (stencil.action != STENCIL_KEEP)
		{
			// FIXME
			stencil.compare = COMPARE_ALWAYS;
		}

		id<MTLDepthStencilState> mtlstate = getCachedDepthStencilState(depth, stencil);

		[encoder setDepthStencilState:mtlstate];
	}

	if (dirtyState & STATEBIT_STENCIL)
		[encoder setStencilReferenceValue:state.stencil.value];

	dirtyRenderState = 0;
}

void Graphics::applyShaderUniforms(id<MTLRenderCommandEncoder> renderEncoder, love::graphics::Shader *shader)
{
	Shader *s = (Shader *)shader;

#ifdef LOVE_MACOS
	size_t alignment = 256;
#else
	size_t alignment = 16;
#endif

	size_t size = s->getLocalUniformBufferSize();
	uint8 *bufferdata = s->getLocalUniformBufferData();

	auto builtins = (Shader::BuiltinUniformData *) (bufferdata + s->getBuiltinUniformDataOffset());

	builtins->transformMatrix = getTransform();
	builtins->projectionMatrix = getProjection();

	// The normal matrix is the transpose of the inverse of the rotation portion
	// (top-left 3x3) of the transform matrix.
	{
		Matrix3 normalmatrix = Matrix3(builtins->transformMatrix).transposedInverse();
		const float *e = normalmatrix.getElements();
		for (int i = 0; i < 3; i++)
		{
			builtins->normalMatrix[i].x = e[i * 3 + 0];
			builtins->normalMatrix[i].y = e[i * 3 + 1];
			builtins->normalMatrix[i].z = e[i * 3 + 2];
			builtins->normalMatrix[i].w = 0.0f;
		}
	}

	// FIXME: should be active RT dimensions
	builtins->screenSizeParams = Vector4(getPixelWidth(), getPixelHeight(), 1.0f, 0.0f);

	builtins->constantColor = getColor();
	gammaCorrectColor(builtins->constantColor);

	if (uniformBuffer->getSize() < uniformBufferOffset + size)
	{
		size_t newsize = uniformBuffer->getSize() * 2;
		delete uniformBuffer;
		uniformBuffer = CreateStreamBuffer(device, BUFFERTYPE_UNIFORM, newsize);
		uniformBufferData = {};
		uniformBufferOffset = 0;
	}

	if (uniformBufferData.data == nullptr)
		uniformBufferData = uniformBuffer->map(uniformBuffer->getSize());

	memcpy(uniformBufferData.data + uniformBufferOffset, bufferdata, size);

	id<MTLBuffer> buffer = getMTLBuffer(uniformBuffer);
	int index = Shader::getUniformBufferBinding();

	// TODO: bind shader textures/samplers

	[renderEncoder setVertexBuffer:buffer offset:uniformBufferOffset atIndex:index];
	[renderEncoder setFragmentBuffer:buffer offset:uniformBufferOffset atIndex:index];

	uniformBufferOffset += alignUp(size, alignment);
}

static void setVertexBuffers(id<MTLRenderCommandEncoder> encoder, const BufferBindings *buffers)
{
	uint32 allbits = buffers->useBits;
	uint32 i = 0;
	while (allbits)
	{
		uint32 bit = 1u << i;

		if (buffers->useBits & bit)
		{
			auto b = buffers->info[i];
			id<MTLBuffer> buffer = getMTLBuffer(b.buffer);
			[encoder setVertexBuffer:buffer offset:b.offset atIndex:i + VERTEX_BUFFER_BINDING_START];
		}

		i++;
		allbits >>= 1;
	}
}

void Graphics::draw(const DrawCommand &cmd)
{ @autoreleasepool {
	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	applyRenderState(encoder, *cmd.attributes);
	applyShaderUniforms(encoder, Shader::current);

	love::graphics::Texture *texture = cmd.texture;
	if (texture == nullptr)
		texture = defaultTextures[TEXTURE_2D];

	id<MTLTexture> mtltexture = getMTLTexture(texture);

	[encoder setFragmentTexture:mtltexture atIndex:0];
	[encoder setFragmentSamplerState:((Texture *)texture)->getMTLSampler() atIndex:0];

	[encoder setCullMode:MTLCullModeNone];

	setVertexBuffers(encoder, cmd.buffers);

	[encoder drawPrimitives:getMTLPrimitiveType(cmd.primitiveType)
				vertexStart:cmd.vertexStart
				vertexCount:cmd.vertexCount
			  instanceCount:cmd.instanceCount];
}}

void Graphics::draw(const DrawIndexedCommand &cmd)
{ @autoreleasepool {
	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	applyRenderState(encoder, *cmd.attributes);
	applyShaderUniforms(encoder, Shader::current);

	love::graphics::Texture *texture = cmd.texture;
	if (texture == nullptr)
		texture = defaultTextures[TEXTURE_2D];

	id<MTLTexture> mtltexture = getMTLTexture(texture);

	[encoder setFragmentTexture:mtltexture atIndex:0];
	[encoder setFragmentSamplerState:((Texture *)texture)->getMTLSampler() atIndex:0];

	[encoder setCullMode:MTLCullModeNone];

	setVertexBuffers(encoder, cmd.buffers);

	auto indexType = cmd.indexType == INDEX_UINT32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;

	[encoder drawIndexedPrimitives:getMTLPrimitiveType(cmd.primitiveType)
						indexCount:cmd.indexCount
						 indexType:indexType
					   indexBuffer:getMTLBuffer(cmd.indexBuffer)
				 indexBufferOffset:cmd.indexBufferOffset
					 instanceCount:cmd.instanceCount];
}}

void Graphics::drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, love::graphics::Texture *texture)
{ @autoreleasepool {
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW    = MAX_VERTICES_PER_DRAW / 4;

	id<MTLRenderCommandEncoder> encoder = useRenderEncoder();

	applyRenderState(encoder, attributes);
	applyShaderUniforms(encoder, Shader::current);

	if (texture == nullptr)
		texture = defaultTextures[TEXTURE_2D];

	id<MTLTexture> mtltexture = getMTLTexture(texture);

	[encoder setFragmentTexture:mtltexture atIndex:0];
	[encoder setFragmentSamplerState:((Texture *)texture)->getMTLSampler() atIndex:0];

	[encoder setCullMode:MTLCullModeNone];

	setVertexBuffers(encoder, &buffers);

	id<MTLBuffer> ib = getMTLBuffer(quadIndexBuffer);

	// TODO: support for iOS devices that don't support base vertex.

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
}}

static inline void setAttachment(const Graphics::RenderTarget &rt, MTLRenderPassAttachmentDescriptor *desc)
{
	bool isvolume = rt.texture->getTextureType() == TEXTURE_VOLUME;

	desc.texture = getMTLRenderTarget(rt.texture);
	desc.level = rt.mipmap;
	desc.slice = isvolume ? 0 : rt.slice;
	desc.depthPlane = isvolume ? rt.slice : 0;

	// Default to load until clear or discard is called.
	desc.loadAction = MTLLoadActionLoad;
	desc.storeAction = MTLStoreActionStore;

	desc.resolveTexture = nil;

	if (rt.texture->getMSAA() > 1)
	{
		// TODO
		desc.resolveTexture = getMTLTexture(rt.texture);

		// TODO: This StoreAction is only supported sometimes.
		desc.storeAction = MTLStoreActionStoreAndMultisampleResolve;
	}
}

void Graphics::setRenderTargetsInternal(const RenderTargets &rts, int w, int h, int /*pixelw*/, int /*pixelh*/, bool /*hasSRGBtexture*/)
{ @autoreleasepool {
	endPass();

	// Set up render pass descriptor for the next useRenderEncoder call.
	// The backbuffer will be set up in useRenderEncoder rather than here.
	for (size_t i = 0; i < rts.colors.size(); i++)
	{
		auto desc = passDesc.colorAttachments[i];
		setAttachment(rts.colors[i], desc);
		passDesc.colorAttachments[i] = desc;
	}

	for (size_t i = rts.colors.size(); i < MAX_COLOR_RENDER_TARGETS; i++)
		passDesc.colorAttachments[i] = nil;

	passDesc.depthAttachment = nil;
	passDesc.stencilAttachment = nil;

	if (rts.depthStencil.texture)
	{
		if (isPixelFormatDepth(rts.depthStencil.texture->getPixelFormat()))
			setAttachment(rts.depthStencil, passDesc.depthAttachment);

		if (isPixelFormatStencil(rts.depthStencil.texture->getPixelFormat()))
			setAttachment(rts.depthStencil, passDesc.stencilAttachment);
	}

	projectionMatrix = Matrix4::ortho(0.0, (float) w, (float) h, 0.0, -10.0f, 10.0f);
	dirtyRenderState = STATEBIT_ALL;
}}

void Graphics::endPass()
{
	flushBatchedDraws();

	auto &rts = states.back().renderTargets;
	love::graphics::Texture *depthstencil = rts.depthStencil.texture.get();

	// Discard the depth/stencil buffer if we're using an internal cached one.
	if (depthstencil == nullptr && (rts.temporaryRTFlags & (TEMPORARY_RT_DEPTH | TEMPORARY_RT_STENCIL)) != 0)
		discard({}, true);

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

			// TODO
		}
	}

	if (depthstencil != nullptr && depthstencil->getMSAA() > 1 && depthstencil->isReadable())
	{
		// TODO
	}

	for (const auto &rt : rts.colors)
	{
		if (rt.texture->getMipmapsMode() == Texture::MIPMAPS_AUTO && rt.mipmap == 0)
			rt.texture->generateMipmaps();
	}

	int dsmipmap = rts.depthStencil.mipmap;
	if (depthstencil != nullptr && depthstencil->getMipmapsMode() == Texture::MIPMAPS_AUTO && dsmipmap == 0)
		depthstencil->generateMipmaps();
}

void Graphics::clear(OptionalColorf c, OptionalInt stencil, OptionalDouble depth)
{ @autoreleasepool {
	if (c.hasValue || stencil.hasValue || depth.hasValue)
		flushBatchedDraws();

	// TODO: handle clearing mid-pass
	if (c.hasValue)
	{
		gammaCorrectColor(c.value);
		auto color = MTLClearColorMake(c.value.r, c.value.g, c.value.b, c.value.a);
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

void Graphics::clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth)
{ @autoreleasepool {
	if (colors.size() == 0 && !stencil.hasValue && !depth.hasValue)
		return;

	int ncolorcanvases = (int) states.back().renderTargets.colors.size();
	int ncolors = (int) colors.size();

	if (ncolors <= 1 && ncolorcanvases <= 1)
	{
		clear(ncolors > 0 ? colors[0] : OptionalColorf(), stencil, depth);
		return;
	}

	flushBatchedDraws();

	// TODO: handle clearing mid-pass
	for (int i = 0; i < ncolors; i++)
	{
		if (!colors[i].hasValue)
			continue;

		Colorf c = colors[i].value;
		gammaCorrectColor(c);

		passDesc.colorAttachments[i].clearColor = MTLClearColorMake(c.r, c.g, c.b, c.a);
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
{
	flushBatchedDraws();
	// TODO
}

void Graphics::present(void *screenshotCallbackData)
{ @autoreleasepool {
	if (!isActive())
		return;

	if (isRenderTargetActive())
		throw love::Exception("present cannot be called while a render target is active.");

	deprecations.draw(this);

	endPass();

	if (!pendingScreenshotCallbacks.empty())
	{
		int w = getPixelWidth();
		int h = getPixelHeight();

		size_t row = 4 * w;
		size_t size = row * h;

		uint8 *screenshot = nullptr;

		try
		{
			screenshot = new uint8[size];
		}
		catch (std::exception &)
		{
			delete[] screenshot;
			throw love::Exception("Out of memory.");
		}

		// TODO

		// Replace alpha values with full opacity.
		for (size_t i = 3; i < size; i += 4)
			screenshot[i] = 255;

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

	for (StreamBuffer *buffer : batchedDrawState.vb)
		buffer->nextFrame();
	batchedDrawState.indexBuffer->nextFrame();

	uniformBuffer->nextFrame();
	uniformBufferData = {};
	uniformBufferOffset = 0;

	id<MTLCommandBuffer> cmd = getCommandBuffer();

	if (cmd != nil && activeDrawable != nil)
		[cmd presentDrawable:activeDrawable];

	submitCommandBuffer();

	auto window = Module::getInstance<love::window::Window>(M_WINDOW);
	if (window != nullptr)
		window->swapBuffers();

	// This is set to NO when there are pending screen captures.
	metalLayer.framebufferOnly = YES;

	activeDrawable = nil;

	// Reset the per-frame stat counts.
	drawCalls = 0;
	//gl.stats.shaderSwitches = 0;
	renderTargetSwitchCount = 0;
	drawCallsBatched = 0;

	// This assumes temporary canvases will only be used within a render pass.
	for (int i = (int) temporaryTextures.size() - 1; i >= 0; i--)
	{
		if (temporaryTextures[i].framesSinceUse >= MAX_TEMPORARY_TEXTURE_UNUSED_FRAMES)
		{
			temporaryTextures[i].texture->release();
			temporaryTextures[i] = temporaryTextures.back();
			temporaryTextures.pop_back();
		}
		else
			temporaryTextures[i].framesSinceUse++;
	}
}}

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

void Graphics::drawToStencilBuffer(StencilAction action, int value)
{
	DisplayState &state = states.back();
	const auto &rts = state.renderTargets;
	love::graphics::Texture *dstexture = rts.depthStencil.texture.get();

	if (!isRenderTargetActive() && !windowHasStencil)
		throw love::Exception("The window must have stenciling enabled to draw to the main screen's stencil buffer.");
	else if (isRenderTargetActive() && (rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) == 0 && (dstexture == nullptr || !isPixelFormatStencil(dstexture->getPixelFormat())))
		throw love::Exception("Drawing to the stencil buffer with a Canvas active requires either stencil=true or a custom stencil-type Canvas to be used, in setCanvas.");

	flushBatchedDraws();

	state.stencil.action = action;
	state.stencil.value = value;

	dirtyRenderState |= STATEBIT_STENCIL;
	// TODO
}

void Graphics::stopDrawToStencilBuffer()
{
	DisplayState &state = states.back();

	if (state.stencil.action == STENCIL_KEEP)
		return;

	flushBatchedDraws();

	state.stencil.action = STENCIL_KEEP;

	// Revert the color write mask.
	setColorMask(state.colorMask);

	// Use the user-set stencil test state when writes are disabled.
	setStencilTest(state.stencil.compare, state.stencil.value);

	dirtyRenderState |= STATEBIT_STENCIL;
}

void Graphics::setStencilTest(CompareMode compare, int value)
{
	// TODO
	DisplayState &state = states.back();
	if (state.stencil.compare != compare || state.stencil.value != value)
	{
		state.stencil.compare = compare;
		state.stencil.value = value;
		dirtyRenderState |= STATEBIT_STENCIL;
	}
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
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
	// TODO
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

PixelFormat Graphics::getSizedFormat(PixelFormat format, bool /*rendertarget*/, bool /*readable*/, bool /*sRGB*/) const
{
	switch (format)
	{
	case PIXELFORMAT_NORMAL:
		if (isGammaCorrect())
			return PIXELFORMAT_RGBA8_UNORM_sRGB;
		else
			return PIXELFORMAT_RGBA8_UNORM;
	case PIXELFORMAT_HDR:
		return PIXELFORMAT_RGBA16_FLOAT;
	default:
		return format;
	}
}

bool Graphics::isPixelFormatSupported(PixelFormat format, bool rendertarget, bool readable, bool sRGB)
{
	return true; // TODO
}

Graphics::Renderer Graphics::getRenderer() const
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
	info.version = "1"; // TODO
	info.vendor = ""; // TODO
	info.device = device.name.UTF8String;
	return info;
}

void Graphics::initCapabilities()
{
	int msaa = 1;
	const int checkmsaa[] = {32, 16, 8, 4, 2};
	for (int samples : checkmsaa)
	{
		if ([device supportsTextureSampleCount:samples])
		{
			msaa = samples;
			break;
		}
	}

	capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = true;
	capabilities.features[FEATURE_CLAMP_ZERO] = true;
	capabilities.features[FEATURE_BLEND_MINMAX] = true;
	capabilities.features[FEATURE_LIGHTEN] = true;
	capabilities.features[FEATURE_FULL_NPOT] = true;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = true;
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = true;
	static_assert(FEATURE_MAX_ENUM == 11, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	// https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
	capabilities.limits[LIMIT_POINT_SIZE] = 511;
	capabilities.limits[LIMIT_TEXTURE_SIZE] = 16384; // TODO
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = 2048;
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = 2048;
	capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = 16384; // TODO
	capabilities.limits[LIMIT_TEXEL_BUFFER_SIZE] = 128 * 1024 * 1024; // TODO
	capabilities.limits[LIMIT_RENDER_TARGETS] = 8; // TODO
	capabilities.limits[LIMIT_TEXTURE_MSAA] = msaa;
	capabilities.limits[LIMIT_ANISOTROPY] = 16.0f;
	static_assert(LIMIT_MAX_ENUM == 9, "Graphics::initCapabilities must be updated when adding a new system limit!");

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
		capabilities.textureTypes[i] = true;
}

void Graphics::getAPIStats(int &shaderswitches) const
{
	shaderswitches = 0;
}

} // metal
} // graphics
} // love
