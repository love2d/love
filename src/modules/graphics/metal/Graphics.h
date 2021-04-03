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

#pragma once

#include "graphics/Graphics.h"
#include "Metal.h"

#include <map>

@class CAMetalLayer;
@protocol CAMetalDrawable;

namespace love
{
namespace graphics
{
namespace metal
{

class Graphics final : public love::graphics::Graphics
{
public:

	struct RenderEncoderBindings
	{
		void *textures[32][ShaderStage::STAGE_MAX_ENUM];
		void *samplers[32][ShaderStage::STAGE_MAX_ENUM];
		struct
		{
			void *buffer;
			size_t offset;
		} buffers[32][ShaderStage::STAGE_MAX_ENUM];
	};

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	const char *getName() const override { return "love.graphics.metal"; }

	love::graphics::Texture *newTexture(const Texture::Settings &settings, const Texture::Slices *data = nullptr) override;
	love::graphics::Buffer *newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength) override;

	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override;
	void unSetMode() override;

	void setActive(bool active) override;

	void draw(const DrawCommand &cmd) override;
	void draw(const DrawIndexedCommand &cmd) override;
	void drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, love::graphics::Texture *texture) override;

	void clear(OptionalColorf color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth) override;

	void discard(const std::vector<bool> &colorbuffers, bool depthstencil) override;

	void present(void *screenshotCallbackData) override;

	int getRequestedBackbufferMSAA() const override;
	int getBackbufferMSAA() const override;

	void setColor(Colorf c) override;

	void setScissor(const Rect &rect) override;
	void setScissor() override;

	void drawToStencilBuffer(StencilAction action, int value) override;
	void stopDrawToStencilBuffer() override;

	void setStencilTest(CompareMode compare, int value) override;

	void setDepthMode(CompareMode compare, bool write) override;

	void setFrontFaceWinding(Winding winding) override;

	void setColorMask(ColorChannelMask mask) override;

	void setBlendState(const BlendState &state) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;
	
	PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const override;
	bool isPixelFormatSupported(PixelFormat format, bool rendertarget, bool readable, bool sRGB = false) override;
	Renderer getRenderer() const override;
	bool usesGLSLES() const override;
	RendererInfo getRendererInfo() const override;

	void attachShader(love::graphics::Shader *shader);

	id<MTLCommandBuffer> useCommandBuffer();
	id<MTLCommandBuffer> getCommandBuffer() const { return commandBuffer; }
	void submitCommandBuffer();

	id<MTLRenderCommandEncoder> useRenderEncoder();
	id<MTLRenderCommandEncoder> getRenderEncoder() const { return renderEncoder; }
	void submitRenderEncoder();

	id<MTLBlitCommandEncoder> useBlitEncoder();
	id<MTLBlitCommandEncoder> getBlitEncoder() const { return blitEncoder; }
	void submitBlitEncoder();

	id<MTLSamplerState> getCachedSampler(const SamplerState &s);

	StreamBuffer *getUniformBuffer() const { return uniformBuffer; }
	Buffer *getDefaultAttributesBuffer() const { return defaultAttributesBuffer; }
	Texture *getDefaultTexture(TextureType textype) const { return defaultTextures[textype]; }

	static Graphics *getInstance() { return graphicsInstance; }

	id<MTLDevice> device;

private:

	static Graphics *graphicsInstance;

	enum StateType
	{
		STATE_BLEND,
		STATE_VIEWPORT,
		STATE_SCISSOR,
		STATE_STENCIL,
		STATE_DEPTH,
		STATE_SHADER,
		STATE_COLORMASK,
		STATE_CULLMODE,
		STATE_FACEWINDING,
		STATE_WIREFRAME,
	};

	enum StateBit
	{
		STATEBIT_BLEND = 1 << STATE_BLEND,
		STATEBIT_VIEWPORT = 1 << STATE_VIEWPORT,
		STATEBIT_SCISSOR = 1 << STATE_SCISSOR,
		STATEBIT_STENCIL = 1 << STATE_STENCIL,
		STATEBIT_DEPTH = 1 << STATE_DEPTH,
		STATEBIT_SHADER = 1 << STATE_SHADER,
		STATEBIT_COLORMASK = 1 << STATE_COLORMASK,
		STATEBIT_CULLMODE = 1 << STATE_CULLMODE,
		STATEBIT_FACEWINDING = 1 << STATE_FACEWINDING,
		STATEBIT_WIREFRAME = 1 << STATE_WIREFRAME,
		STATEBIT_ALL = 0xFFFFFFFF
	};

	struct DeviceFamilies
	{
		// All arrays are 1-indexed for convenience
		bool apple[7+1];
		bool mac[2+1];
		bool common[3+1];
		bool macCatalyst[2+1];
	};

	struct AttachmentStoreActions
	{
		MTLStoreAction color[MAX_COLOR_RENDER_TARGETS];
		MTLStoreAction depth;
		MTLStoreAction stencil;
	};

	love::graphics::ShaderStage *newShaderStageInternal(ShaderStage::StageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	love::graphics::Shader *newShaderInternal(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel) override;
	love::graphics::StreamBuffer *newStreamBuffer(BufferUsage usage, size_t size) override;
	void setRenderTargetsInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBcanvas) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;

	void endPass();

	id<MTLDepthStencilState> getCachedDepthStencilState(const DepthState &depth, const StencilState &stencil);
	void applyRenderState(id<MTLRenderCommandEncoder> renderEncoder, const VertexAttributes &attributes);
	void applyShaderUniforms(id<MTLRenderCommandEncoder> renderEncoder, Shader *shader, Texture *maintex);

	id<MTLCommandQueue> commandQueue;

	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> renderEncoder;
	id<MTLBlitCommandEncoder> blitEncoder;

	CAMetalLayer *metalLayer;
	id<CAMetalDrawable> activeDrawable;
	MTLRenderPassDescriptor *passDesc;

	uint32 dirtyRenderState;
	VertexAttributes lastVertexAttributes;
	bool windowHasStencil;

	StrongRef<love::graphics::Texture> backbufferMSAA;
	StrongRef<love::graphics::Texture> backbufferDepthStencil;
	int requestedBackbufferMSAA;

	AttachmentStoreActions attachmentStoreActions;

	RenderEncoderBindings renderBindings;

	StreamBuffer *uniformBuffer;
	StreamBuffer::MapInfo uniformBufferData;
	size_t uniformBufferOffset;

	Buffer *defaultAttributesBuffer;

	Texture *defaultTextures[TEXTURE_MAX_ENUM];

	std::map<uint64, void *> cachedSamplers;
	std::unordered_map<uint64, void *> cachedDepthStencilStates;

	std::atomic<int64> completeCommandBufferIndex;

	DeviceFamilies families;

}; // Graphics

} // metal
} // graphics
} // love
