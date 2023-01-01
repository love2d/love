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

#include "Image.h"
#include "Canvas.h"
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

	void setColorMask(ColorMask mask) override;

	void setBlendMode(BlendMode mode, BlendAlpha alphamode) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;

	bool isCanvasFormatSupported(PixelFormat format) const override;
	bool isCanvasFormatSupported(PixelFormat format, bool readable) const override;
	bool isImageFormatSupported(PixelFormat format, bool sRGB) const override;
	Renderer getRenderer() const override;
	RendererInfo getRendererInfo() const override;

	Shader::Language getShaderLanguageTarget() const override;

	// Internal use.
	void cleanupCanvas(Canvas *canvas);

private:

	struct CachedFBOHasher
	{
		size_t operator() (const RenderTargets &rts) const
		{
			RenderTarget hashtargets[MAX_COLOR_RENDER_TARGETS + 1];
			int hashcount = 0;

			for (size_t i = 0; i < rts.colors.size(); i++)
				hashtargets[hashcount++] = rts.colors[i];

			if (rts.depthStencil.canvas != nullptr)
				hashtargets[hashcount++] = rts.depthStencil;
			else if (rts.temporaryRTFlags != 0)
				hashtargets[hashcount++] = RenderTarget(nullptr, -1, rts.temporaryRTFlags);

			return XXH32(hashtargets, sizeof(RenderTarget) * hashcount, 0);
		}
	};

	love::graphics::ShaderStage *newShaderStageInternal(ShaderStage::StageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	love::graphics::Shader *newShaderInternal(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel) override;
	love::graphics::StreamBuffer *newStreamBuffer(BufferType type, size_t size) override;
	void setCanvasInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBcanvas) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;

	void endPass();
	void bindCachedFBO(const RenderTargets &targets);
	void discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil);

	void setDebug(bool enable);

	std::unordered_map<RenderTargets, GLuint, CachedFBOHasher> framebufferObjects;
	bool windowHasStencil;
	GLuint mainVAO;

}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
