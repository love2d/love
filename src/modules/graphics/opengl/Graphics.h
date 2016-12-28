/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "math/Transform.h"

#include "Font.h"
#include "Image.h"
#include "graphics/Quad.h"
#include "graphics/Texture.h"
#include "SpriteBatch.h"
#include "ParticleSystem.h"
#include "Canvas.h"
#include "Shader.h"
#include "Mesh.h"
#include "Text.h"
#include "Video.h"

namespace love
{

class Reference;

namespace graphics
{
namespace opengl
{

class Graphics final : public love::graphics::Graphics
{
public:

	typedef void (*ScreenshotCallback)(love::image::ImageData *i, Reference *ref, void *ud);

	struct ScreenshotInfo
	{
		ScreenshotCallback callback;
		Reference *ref;
	};

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	const char *getName() const override;

	void setViewportSize(int width, int height) override;
	bool setMode(int width, int height) override;
	void unSetMode() override;

	void setActive(bool active) override;
	bool isActive() const override;

	void flushStreamDraws() override;

	/**
	 * Resets the current color, background color,
	 * line style, and so forth. (This will be called
	 * when the game reloads.
	 **/
	void reset();

	void clear(Colorf color);
	void clear(const std::vector<OptionalColorf> &colors);

	void discard(const std::vector<bool> &colorbuffers, bool depthstencil);

	bool isCanvasActive() const override;
	bool isCanvasActive(Canvas *canvas) const;

	/**
	 * Flips buffers. (Rendered geometry is presented on screen).
	 **/
	void present(void *screenshotCallbackData);

	/**
	 * Gets the width of the current graphics viewport.
	 **/
	int getWidth() const;

	/**
	 * Gets the height of the current graphics viewport.
	 **/
	int getHeight() const;

	/**
	 * True if a graphics viewport is set.
	 **/
	bool isCreated() const;

	void setCanvas(Canvas *canvas);
	void setCanvas(const std::vector<Canvas *> &canvases);
	void setCanvas(const std::vector<StrongRef<Canvas>> &canvases);
	void setCanvas();

	std::vector<Canvas *> getCanvas() const;

	/**
	 * Scissor defines a box such that everything outside that box is discarded and not drawn.
	 * Scissoring is automatically enabled.
	 * @param rect The rectangle defining the scissor area.
	 **/
	void setScissor(const Rect &rect);
	void intersectScissor(const Rect &rect);

	/**
	 * Clears any scissor that has been created.
	 **/
	void setScissor();

	/**
	 * Gets the current scissor box.
	 * @return Whether the scissor is enabled.
	 */
	bool getScissor(Rect &rect) const;

	/**
	 * Enables or disables drawing to the stencil buffer. When enabled, the
	 * color buffer is disabled.
	 **/
	void drawToStencilBuffer(StencilAction action, int value);
	void stopDrawToStencilBuffer();

	/**
	 * Sets whether stencil testing is enabled.
	 **/
	void setStencilTest(CompareMode compare, int value);
	void setStencilTest();
	void getStencilTest(CompareMode &compare, int &value);

	/**
	 * Clear the stencil buffer in the active Canvas(es.)
	 **/
	void clearStencil();

	/**
	 * Creates an Image object with padding and/or optimization.
	 **/
	Image *newImage(const std::vector<love::image::ImageData *> &data, const Image::Flags &flags);
	Image *newImage(const std::vector<love::image::CompressedImageData *> &cdata, const Image::Flags &flags);

	Quad *newQuad(Quad::Viewport v, double sw, double sh);

	/**
	 * Creates a Font object.
	 **/
	Font *newFont(love::font::Rasterizer *data, const Texture::Filter &filter = Texture::getDefaultFilter());

	SpriteBatch *newSpriteBatch(Texture *texture, int size, Mesh::Usage usage);

	ParticleSystem *newParticleSystem(Texture *texture, int size);

	Canvas *newCanvas(int width, int height, PixelFormat format = PIXELFORMAT_NORMAL, int msaa = 0);

	Shader *newShader(const Shader::ShaderSource &source);

	Mesh *newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode drawmode, Mesh::Usage usage);
	Mesh *newMesh(int vertexcount, Mesh::DrawMode drawmode, Mesh::Usage usage);

	Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, int vertexcount, Mesh::DrawMode drawmode, Mesh::Usage usage);
	Mesh *newMesh(const std::vector<Mesh::AttribFormat> &vertexformat, const void *data, size_t datasize, Mesh::DrawMode drawmode, Mesh::Usage usage);

	Text *newText(Font *font, const std::vector<Font::ColoredString> &text = {});

	Video *newVideo(love::video::VideoStream *stream);

	bool isGammaCorrect() const;

	/**
	 * Sets the current constant color.
	 **/
	void setColor(Colorf c);

	/**
	 * Gets current color.
	 **/
	Colorf getColor() const override;

	/**
	 * Sets the background Color.
	 **/
	void setBackgroundColor(Colorf c);

	/**
	 * Gets the current background color.
	 **/
	Colorf getBackgroundColor() const;

	void setFont(Font *font);
	Font *getFont();

	void setShader(Shader *shader);
	void setShader();

	Shader *getShader() const;

	/**
	 * Sets the enabled color components when rendering.
	 **/
	void setColorMask(ColorMask mask);

	/**
	 * Gets the current color mask.
	 **/
	ColorMask getColorMask() const;

	/**
	 * Sets the current blend mode.
	 **/
	void setBlendMode(BlendMode mode, BlendAlpha alphamode);

	/**
	 * Gets the current blend mode.
	 **/
	BlendMode getBlendMode(BlendAlpha &alphamode) const;

	/**
	 * Sets the default filter for images, canvases, and fonts.
	 **/
	void setDefaultFilter(const Texture::Filter &f);

	/**
	 * Gets the default filter for images, canvases, and fonts.
	 **/
	const Texture::Filter &getDefaultFilter() const;

	/**
	 * Default Image mipmap filter mode and sharpness values.
	 **/
	void setDefaultMipmapFilter(Texture::FilterMode filter, float sharpness);
	void getDefaultMipmapFilter(Texture::FilterMode *filter, float *sharpness) const;

	/**
	 * Sets the line width.
	 * @param width The new width of the line.
	 **/
	void setLineWidth(float width);

	/**
	 * Sets the line style.
	 * @param style LINE_ROUGH or LINE_SMOOTH.
	 **/
	void setLineStyle(LineStyle style);

	/**
	 * Sets the line join mode.
	 **/
	void setLineJoin(LineJoin style);

	float getLineWidth() const override;
	LineStyle getLineStyle() const override;
	LineJoin getLineJoin() const override;

	/**
	 * Sets the size of points.
	 **/
	void setPointSize(float size);

	/**
	 * Gets the point size.
	 **/
	float getPointSize() const;

	/**
	 * Sets whether graphics will be drawn as wireframe lines instead of filled
	 * triangles (has no effect for drawn points.)
	 * This should only be used as a debugging tool. The wireframe lines do not
	 * behave the same as regular love.graphics lines.
	 **/
	void setWireframe(bool enable);

	/**
	 * Gets whether wireframe drawing mode is enabled.
	 **/
	bool isWireframe() const;

	void draw(Drawable *drawable, const Matrix4 &m);
	void draw(Texture *texture, Quad *quad, const Matrix4 &m);

	/**
	 * Draws text at the specified coordinates
	 **/
	void print(const std::vector<Font::ColoredString> &str, const Matrix4 &m);

	/**
	 * Draws formatted text on screen at the specified coordinates.
	 **/
	void printf(const std::vector<Font::ColoredString> &str, float wrap, Font::AlignMode align, const Matrix4 &m);

	/**
	 * Draws a point at (x,y).
	 * @param x Point along x-axis.
	 * @param y Point along y-axis.
	 **/
	void points(const float *coords, const Colorf *colors, size_t numpoints);

	void captureScreenshot(const ScreenshotInfo &info);

	/**
	 * Returns system-dependent renderer information.
	 * Returned strings can vary greatly between systems! Do not rely on it for
	 * anything!
	 **/
	RendererInfo getRendererInfo() const;

	/**
	 * Returns performance-related statistics.
	 **/
	Stats getStats() const;

	/**
	 * Gets the system-dependent numeric limit for the specified parameter.
	 **/
	double getSystemLimit(SystemLimit limittype) const;

	/**
	 * Gets whether a graphics feature is supported on this system.
	 **/
	bool isSupported(Feature feature) const;

	void push(StackType type = STACK_TRANSFORM);
	void pop();

	void rotate(float r);
	void scale(float x, float y = 1.0f);
	void translate(float x, float y);
	void shear(float kx, float ky);
	void origin();

	void applyTransform(love::math::Transform *transform);
	void replaceTransform(love::math::Transform *transform);

	Vector transformPoint(Vector point);
	Vector inverseTransformPoint(Vector point);

private:

	struct DisplayState
	{
		Colorf color = Colorf(1.0, 1.0, 1.0, 1.0);
		Colorf gammaCorrectedColor = Colorf(1.0f, 1.0f, 1.0f, 1.0f);
		Colorf backgroundColor = Colorf(0.0, 0.0, 0.0, 1.0);

		BlendMode blendMode = BLEND_ALPHA;
		BlendAlpha blendAlphaMode = BLENDALPHA_MULTIPLY;

		float lineWidth = 1.0f;
		LineStyle lineStyle = LINE_SMOOTH;
		LineJoin lineJoin = LINE_JOIN_MITER;

		float pointSize = 1.0f;

		bool scissor = false;
		Rect scissorRect = Rect();

		// Stencil.
		CompareMode stencilCompare = COMPARE_ALWAYS;
		int stencilTestValue = 0;

		StrongRef<Font> font;
		StrongRef<Shader> shader;

		std::vector<StrongRef<Canvas>> canvases;

		ColorMask colorMask = ColorMask(true, true, true, true);

		bool wireframe = false;

		Texture::Filter defaultFilter = Texture::Filter();

		Texture::FilterMode defaultMipmapFilter = Texture::FILTER_NEAREST;
		float defaultMipmapSharpness = 0.0f;
	};

	struct CachedRenderbuffer
	{
		int w;
		int h;
		int samples;
		GLenum attachments[2];
		GLuint renderbuffer;
	};

	void restoreState(const DisplayState &s);
	void restoreStateChecked(const DisplayState &s);

	void endPass();
	void bindCachedFBO(const std::vector<Canvas *> &canvases);
	void discard(OpenGL::FramebufferTarget target, const std::vector<bool> &colorbuffers, bool depthstencil);
	GLuint attachCachedStencilBuffer(int w, int h, int samples);

	void setDebug(bool enable);

	void checkSetDefaultFont();

	StrongRef<Font> defaultFont;

	std::vector<ScreenshotInfo> pendingScreenshotCallbacks;

	std::unordered_map<uint32, GLuint> framebufferObjects;
	std::vector<CachedRenderbuffer> stencilBuffers;

	QuadIndices *quadIndices;

	int width;
	int height;
	bool created;
	bool active;

	bool writingToStencil;

	std::vector<DisplayState> states;
	std::vector<StackType> stackTypes; // Keeps track of the pushed stack types.

	int canvasSwitchCount;

	static const size_t MAX_USER_STACK_DEPTH = 64;

}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
