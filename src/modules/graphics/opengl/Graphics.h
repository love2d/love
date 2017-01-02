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

#include "video/VideoStream.h"

#include "Image.h"
#include "SpriteBatch.h"
#include "ParticleSystem.h"
#include "Canvas.h"
#include "Shader.h"
#include "Mesh.h"
#include "Text.h"
#include "Video.h"

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

	Image *newImage(const std::vector<love::image::ImageData *> &data, const Image::Settings &settings);
	Image *newImage(const std::vector<love::image::CompressedImageData *> &cdata, const Image::Settings &settings);

	love::graphics::Font *newFont(love::font::Rasterizer *data, const Texture::Filter &filter = Texture::defaultFilter) override;

	SpriteBatch *newSpriteBatch(Texture *texture, int size, vertex::Usage usage);

	ParticleSystem *newParticleSystem(Texture *texture, int size);

	love::graphics::Canvas *newCanvas(int width, int height, const Canvas::Settings &settings) override;

	love::graphics::Shader *newShader(const Shader::ShaderSource &source) override;

	Mesh *newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode drawmode, vertex::Usage usage);
	Mesh *newMesh(int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage);

	Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, int vertexcount, Mesh::DrawMode drawmode, vertex::Usage usage);
	Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, const void *data, size_t datasize, Mesh::DrawMode drawmode, vertex::Usage usage);

	Text *newText(love::graphics::Font *font, const std::vector<Font::ColoredString> &text = {});

	Video *newVideo(love::video::VideoStream *stream, float pixeldensity);

	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(int width, int height, int pixelwidth, int pixelheight) override;
	void unSetMode() override;

	void setActive(bool active) override;

	void flushStreamDraws() override;

	void clear(Colorf color) override;
	void clear(const std::vector<OptionalColorf> &colors) override;

	void discard(const std::vector<bool> &colorbuffers, bool depthstencil) override;

	void present(void *screenshotCallbackData) override;

	void setColor(Colorf c) override;

	void setCanvas(const std::vector<love::graphics::Canvas *> &canvases) override;
	void setCanvas() override;

	void setScissor(const Rect &rect) override;
	void setScissor() override;

	void drawToStencilBuffer(StencilAction action, int value) override;
	void stopDrawToStencilBuffer() override;

	void setStencilTest(CompareMode compare, int value) override;
	void setStencilTest() override;

	void clearStencil() override;

	void setColorMask(ColorMask mask) override;

	void setBlendMode(BlendMode mode, BlendAlpha alphamode) override;

	void setPointSize(float size) override;

	void setWireframe(bool enable) override;

	bool isSupported(Feature feature) const override;
	double getSystemLimit(SystemLimit limittype) const override;
	RendererInfo getRendererInfo() const override;
	Stats getStats() const override;

private:

	struct CachedRenderbuffer
	{
		int w;
		int h;
		int samples;
		GLenum attachments[2];
		GLuint renderbuffer;
	};

	void endPass();
	void bindCachedFBO(const std::vector<love::graphics::Canvas *> &canvases);
	void discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil);
	GLuint attachCachedStencilBuffer(int w, int h, int samples);

	void setDebug(bool enable);

	std::unordered_map<uint32, GLuint> framebufferObjects;
	std::vector<CachedRenderbuffer> stencilBuffers;

	QuadIndices *quadIndices;

}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
