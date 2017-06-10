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
#include "graphics/Color.h"

#include "image/Image.h"
#include "image/ImageData.h"

#include "Image.h"
#include "SpriteBatch.h"
#include "ParticleSystem.h"
#include "Canvas.h"
#include "Shader.h"
#include "Mesh.h"

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

	love::graphics::SpriteBatch *newSpriteBatch(Texture *texture, int size, vertex::Usage usage) override;
	love::graphics::ParticleSystem *newParticleSystem(Texture *texture, int size) override;

	love::graphics::Canvas *newCanvas(const Canvas::Settings &settings) override;
	love::graphics::Shader *newShader(const Shader::ShaderSource &source) override;

	love::graphics::Buffer *newBuffer(size_t size, const void *data, BufferType type, vertex::Usage usage, uint32 mapflags) override;

	love::graphics::Mesh *newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode drawmode, vertex::Usage usage) override;
	love::graphics::Mesh *newMesh(int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage) override;

	love::graphics::Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage) override;
	love::graphics::Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, const void *data, size_t datasize, Mesh::DrawMode drawmode, vertex::Usage usage) override;

	love::graphics::Text *newText(love::graphics::Font *font, const std::vector<Font::ColoredString> &text = {}) override;

	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil) override;
	void unSetMode() override;

	void setActive(bool active) override;

	void flushStreamDraws() override;

	void clear(OptionalColorf color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth) override;

	void discard(const std::vector<bool> &colorbuffers, bool depthstencil) override;

	void present(void *screenshotCallbackData) override;

	void setColor(Colorf c) override;

	void setCanvas(const RenderTargets &rts) override;
	void setCanvas() override;

	void setScissor(const Rect &rect) override;
	void setScissor() override;

	void drawToStencilBuffer(StencilAction action, int value) override;
	void stopDrawToStencilBuffer() override;

	void setStencilTest(CompareMode compare, int value) override;
	void setStencilTest() override;

	void clearStencil(int value) override;

	void setColorMask(ColorMask mask) override;

	void setBlendMode(BlendMode mode, BlendAlpha alphamode) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;

	bool isSupported(Feature feature) const override;
	double getSystemLimit(SystemLimit limittype) const override;
	bool isCanvasFormatSupported(PixelFormat format) const override;
	bool isCanvasFormatSupported(PixelFormat format, bool readable) const override;
	bool isImageFormatSupported(PixelFormat format) const override;
	Renderer getRenderer() const override;
	RendererInfo getRendererInfo() const override;

	Shader::Language getShaderLanguageTarget() const override;

private:

	love::graphics::StreamBuffer *newStreamBuffer(BufferType type, size_t size) override;
	void getAPIStats(int &drawcalls, int &shaderswitches) const override;

	void endPass();
	void bindCachedFBO(const RenderTargets &targets);
	void discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil);

	void setDebug(bool enable);

	std::unordered_map<uint32, GLuint> framebufferObjects;
	QuadIndices *quadIndices;
	bool windowHasStencil;
	GLuint mainVAO;

}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
