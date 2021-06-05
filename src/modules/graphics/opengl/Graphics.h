/**
 * Copyright (c) 2006-2021 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_GRAPHICS_H
#define LOVE_GRAPHICS_OPENGL_GRAPHICS_H

// STD
#include <stack>
#include <vector>
#include <unordered_map>

// OpenGL
#include "OpenGL.h"

// LOVE
#include "graphics/Graphics.h"
#include "common/Color.h"

#include "image/Image.h"
#include "image/ImageData.h"

#include "Texture.h"
#include "Shader.h"

#include "libraries/xxHash/xxhash.h"

namespace love
{

namespace graphics
{
namespace opengl
{

class Graphics final : public love::graphics::Graphics
{
public:

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	const char *getName() const override;

	love::graphics::Texture *newTexture(const Texture::Settings &settings, const Texture::Slices *data = nullptr) override;
	love::graphics::Buffer *newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength) override;

	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override;
	void unSetMode() override;

	void setActive(bool active) override;

	void dispatch(int x, int y, int z) override;

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

	void setBlendState(const BlendState &blend) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;

	PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const override;
	bool isPixelFormatSupported(PixelFormat format, bool rendertarget, bool readable, bool sRGB = false) override;
	Renderer getRenderer() const override;
	RendererInfo getRendererInfo() const override;

	// Internal use.
	void cleanupRenderTexture(love::graphics::Texture *texture);

	void *getBufferMapMemory(size_t size);
	void releaseBufferMapMemory(void *mem);

private:

	struct CachedFBOHasher
	{
		size_t operator() (const RenderTargets &rts) const
		{
			RenderTarget hashtargets[MAX_COLOR_RENDER_TARGETS + 1];
			int hashcount = 0;

			for (size_t i = 0; i < rts.colors.size(); i++)
				hashtargets[hashcount++] = rts.colors[i];

			if (rts.depthStencil.texture != nullptr)
				hashtargets[hashcount++] = rts.depthStencil;
			else if (rts.temporaryRTFlags != 0)
				hashtargets[hashcount++] = RenderTarget(nullptr, -1, rts.temporaryRTFlags);

			return XXH32(hashtargets, sizeof(RenderTarget) * hashcount, 0);
		}
	};

	love::graphics::ShaderStage *newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	love::graphics::Shader *newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM]) override;
	love::graphics::StreamBuffer *newStreamBuffer(BufferUsage type, size_t size) override;
	void setRenderTargetsInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBtexture) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;

	void endPass();
	GLuint bindCachedFBO(const RenderTargets &targets);
	void discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil);

	void updateBackbuffer(int width, int height, int pixelwidth, int pixelheight, int msaa);
	GLuint getInternalBackbufferFBO() const;
	GLuint getSystemBackbufferFBO() const;

	void setDebug(bool enable);

	std::unordered_map<RenderTargets, GLuint, CachedFBOHasher> framebufferObjects;
	bool windowHasStencil;
	GLuint mainVAO;

	StrongRef<love::graphics::Texture> internalBackbuffer;
	StrongRef<love::graphics::Texture> internalBackbufferDepthStencil;
	GLuint internalBackbufferFBO;
	int requestedBackbufferMSAA;

	char *bufferMapMemory;
	size_t bufferMapMemorySize;

	// Only needed for buffer types that can be bound to shaders.
	StrongRef<love::graphics::Buffer> defaultBuffers[BUFFERUSAGE_MAX_ENUM];

	// [rendertarget][readable][srgb]
	OptionalBool supportedFormats[PIXELFORMAT_MAX_ENUM][2][2][2];

}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
