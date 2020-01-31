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

namespace love
{
namespace graphics
{
namespace metal
{

class Graphics final : public love::graphics::Graphics
{
public:

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	const char *getName() const override { return "love.graphics.metal"; }

	love::graphics::Image *newImage(const Image::Slices &data, const Image::Settings &settings) override;
	love::graphics::Image *newImage(TextureType textype, PixelFormat format, int width, int height, int slices, const Image::Settings &settings) override;
	love::graphics::Canvas *newCanvas(const Canvas::Settings &settings) override;
	love::graphics::Buffer *newBuffer(size_t size, const void *data, BufferType type, vertex::Usage usage, uint32 mapflags) override;

	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil) override;
	void unSetMode() override;

	void setActive(bool active) override;

	void draw(const DrawCommand &cmd) override;
	void draw(const DrawIndexedCommand &cmd) override;
	void drawQuads(int start, int count, const vertex::Attributes &attributes, const vertex::BufferBindings &buffers, Texture *texture) override;

	void clear(OptionalColorf color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth) override;

	void discard(const std::vector<bool> &colorbuffers, bool depthstencil) override;

	void present(void *screenshotCallbackData) override;

	void setColor(Colorf c) override;

	void setScissor(const Rect &rect) override;
	void setScissor() override;

	void drawToStencilBuffer(StencilAction action, int value) override;
	void stopDrawToStencilBuffer() override;

	void setStencilTest(CompareMode compare, int value) override;

	void setDepthMode(CompareMode compare, bool write) override;

	void setFrontFaceWinding(vertex::Winding winding) override;

	void setColorMask(ColorChannelMask mask) override;

	void setBlendState(const BlendState &state) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;

	bool isCanvasFormatSupported(PixelFormat format) const override;
	bool isCanvasFormatSupported(PixelFormat format, bool readable) const override;
	bool isImageFormatSupported(PixelFormat format, bool sRGB) const override;
	Renderer getRenderer() const override;
	bool usesGLSLES() const override;
	RendererInfo getRendererInfo() const override;

	Shader::Language getShaderLanguageTarget() const override;

	id<MTLCommandBuffer> useCommandBuffer();
	id<MTLCommandBuffer> getCommandBuffer() const { return commandBuffer; }
	void submitCommandBuffer();

	id<MTLRenderCommandEncoder> useRenderEncoder();
	id<MTLRenderCommandEncoder> getRenderEncoder() const { return renderEncoder; }
	void submitRenderEncoder();

	id<MTLBlitCommandEncoder> useBlitEncoder();
	id<MTLBlitCommandEncoder> getBlitEncoder() const { return blitEncoder; }
	void submitBlitEncoder();

	id<MTLSamplerState> getCachedSampler(const Texture::Filter &f, const Texture::Wrap &w, float maxAnisotropy, Optional<CompareMode> depthSampleMode);

	static Graphics *getInstance() { return Module::getInstance<Graphics>(M_GRAPHICS); }

	id<MTLDevice> device;

private:

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

	struct RenderState
	{
		Rect viewport = {0, 0, 0, 0};
		ScissorState scissor;
		BlendState blend;
		DepthState depth;
		StencilState stencil;
		ColorChannelMask colorChannelMask;
		Shader *shader;
	};

	struct PipelineState
	{
		vertex::Attributes vertexAttributes;
		BlendState blend;
		ColorChannelMask colorChannelMask;
		Shader *shader;
	};

	love::graphics::ShaderStage *newShaderStageInternal(ShaderStage::StageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	love::graphics::Shader *newShaderInternal(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel) override;
	love::graphics::StreamBuffer *newStreamBuffer(BufferType type, size_t size) override;
	void setCanvasInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBcanvas) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;

	void endPass();

	id<MTLRenderPipelineState> getCachedRenderPipelineState(const PipelineState &state);
	id<MTLDepthStencilState> getCachedDepthStencilState(const DepthState &depth, const StencilState &stencil);
	void applyRenderState(id<MTLRenderCommandEncoder> renderEncoder);

	id<MTLCommandQueue> commandQueue;

	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> renderEncoder;
	id<MTLBlitCommandEncoder> blitEncoder;

	MTLRenderPassDescriptor *passDesc;

	uint32 dirtyRenderState;
	bool windowHasStencil;

}; // Graphics

} // metal
} // graphics
} // love
