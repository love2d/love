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

#ifndef LOVE_GRAPHICS_GRAPHICS_H
#define LOVE_GRAPHICS_GRAPHICS_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "common/StringMap.h"
#include "common/Vector.h"
#include "StreamBuffer.h"
#include "vertex.h"
#include "Color.h"
#include "Texture.h"
#include "Canvas.h"
#include "Font.h"
#include "Shader.h"
#include "Quad.h"
#include "math/Transform.h"
#include "font/Rasterizer.h"

// C++
#include <string>
#include <vector>

namespace love
{

class Reference;

namespace graphics
{

const int MAX_COLOR_RENDER_TARGETS = 8;

/**
 * Globally sets whether gamma correction is enabled. Ideally this should be set
 * prior to using any Graphics module function.
 **/
void setGammaCorrect(bool gammacorrect);

/**
 * Gets whether global gamma correction is enabled.
 **/
bool isGammaCorrect();

/**
 * Gamma-corrects a color (converts it from sRGB to linear RGB, if
 * gamma correction is enabled.)
 * The color's components are expected to be in the range of [0, 1].
 **/
void gammaCorrectColor(Colorf &c);

/**
 * Un-gamma-corrects a color (converts it from linear RGB to sRGB, if
 * gamma correction is enabled.)
 * The color's components are expected to be in the range of [0, 1].
 **/
void unGammaCorrectColor(Colorf &c);

Colorf gammaCorrectColor(const Colorf &c);
Colorf unGammaCorrectColor(const Colorf &c);

class Graphics : public Module
{
public:

	static love::Type type;

	enum DrawMode
	{
		DRAW_LINE,
		DRAW_FILL,
		DRAW_MAX_ENUM
	};

	enum ArcMode
	{
		ARC_OPEN,
		ARC_CLOSED,
		ARC_PIE,
		ARC_MAX_ENUM
	};

	enum BlendMode
	{
		BLEND_ALPHA,
		BLEND_ADD,
		BLEND_SUBTRACT,
		BLEND_MULTIPLY,
		BLEND_LIGHTEN,
		BLEND_DARKEN,
		BLEND_SCREEN,
		BLEND_REPLACE,
		BLEND_NONE,
		BLEND_MAX_ENUM
	};

	enum BlendAlpha
	{
		BLENDALPHA_MULTIPLY,
		BLENDALPHA_PREMULTIPLIED,
		BLENDALPHA_MAX_ENUM
	};

	enum LineStyle
	{
		LINE_ROUGH,
		LINE_SMOOTH,
		LINE_MAX_ENUM
	};

	enum LineJoin
	{
		LINE_JOIN_NONE,
		LINE_JOIN_MITER,
		LINE_JOIN_BEVEL,
		LINE_JOIN_MAX_ENUM
	};

	enum StencilAction
	{
		STENCIL_REPLACE,
		STENCIL_INCREMENT,
		STENCIL_DECREMENT,
		STENCIL_INCREMENT_WRAP,
		STENCIL_DECREMENT_WRAP,
		STENCIL_INVERT,
		STENCIL_MAX_ENUM
	};

	enum CompareMode
	{
		COMPARE_LESS,
		COMPARE_LEQUAL,
		COMPARE_EQUAL,
		COMPARE_GEQUAL,
		COMPARE_GREATER,
		COMPARE_NOTEQUAL,
		COMPARE_ALWAYS,
		COMPARE_MAX_ENUM
	};

	enum Feature
	{
		FEATURE_MULTI_CANVAS_FORMATS,
		FEATURE_CLAMP_ZERO,
		FEATURE_LIGHTEN,
		FEATURE_FULL_NPOT,
		FEATURE_PIXEL_SHADER_HIGHP,
		FEATURE_MAX_ENUM
	};

	enum Renderer
	{
		RENDERER_OPENGL = 0,
		RENDERER_OPENGLES,
		RENDERER_MAX_ENUM
	};

	enum SystemLimit
	{
		LIMIT_POINT_SIZE,
		LIMIT_TEXTURE_SIZE,
		LIMIT_MULTI_CANVAS,
		LIMIT_CANVAS_MSAA,
		LIMIT_ANISOTROPY,
		LIMIT_MAX_ENUM
	};

	enum StackType
	{
		STACK_ALL,
		STACK_TRANSFORM,
		STACK_MAX_ENUM
	};

	struct RendererInfo
	{
		std::string name;
		std::string version;
		std::string vendor;
		std::string device;
	};

	struct Stats
	{
		int drawCalls;
		int canvasSwitches;
		int shaderSwitches;
		int canvases;
		int images;
		int fonts;
		size_t textureMemory;
	};

	struct ColorMask
	{
		bool r, g, b, a;

		ColorMask()
			: r(true), g(true), b(true), a(true)
		{}

		ColorMask(bool _r, bool _g, bool _b, bool _a)
			: r(_r), g(_g), b(_b), a(_a)
		{}

		bool operator == (const ColorMask &m) const
		{
			return r == m.r && g == m.g && b == m.b && a == m.a;
		}

		bool operator != (const ColorMask &m) const
		{
			return !(operator == (m));
		}
	};

	struct OptionalColorf
	{
		Colorf c;
		bool enabled;
	};

	struct StreamDrawRequest
	{
		vertex::PrimitiveMode primitiveMode = vertex::PrimitiveMode::TRIANGLES;
		vertex::CommonFormat formats[2];
		vertex::TriangleIndexMode indexMode = vertex::TriangleIndexMode::NONE;
		int vertexCount = 0;
		Texture *texture = nullptr;

		// FIXME: This is only needed for fonts. We should just change fonts to
		// use love.graphics Images instead of raw OpenGL textures.
		ptrdiff_t textureHandle = 0;

		StreamDrawRequest()
		{
			// VS2013 can't initialize arrays in the above manner...
			formats[1] = formats[0] = vertex::CommonFormat::NONE;
		}
	};

	struct StreamVertexData
	{
		void *stream[2];
	};

	class TempTransform
	{
	public:

		TempTransform(Graphics *gfx)
			: gfx(gfx)
		{
			gfx->pushTransform();
		}

		TempTransform(Graphics *gfx, const Matrix4 &t)
			: gfx(gfx)
		{
			gfx->pushTransform();
			gfx->transformStack.back() *= t;
		}

		~TempTransform()
		{
			gfx->popTransform();
		}

	private:
		Graphics *gfx;
	};

	typedef void (*ScreenshotCallback)(love::image::ImageData *i, Reference *ref, void *ud);

	struct ScreenshotInfo
	{
		ScreenshotCallback callback;
		Reference *ref;
	};

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_GRAPHICS; }

	Quad *newQuad(Quad::Viewport v, double sw, double sh);

	virtual Font *newFont(love::font::Rasterizer *data, const Texture::Filter &filter = Texture::defaultFilter) = 0;

	virtual Canvas *newCanvas(int width, int height, const Canvas::Settings &settings) = 0;

	virtual Shader *newShader(const Shader::ShaderSource &source) = 0;

	/**
	 * Resets the current color, background color, line style, and so forth.
	 **/
	void reset();

	virtual void clear(Colorf color) = 0;
	virtual void clear(const std::vector<OptionalColorf> &colors) = 0;

	virtual void discard(const std::vector<bool> &colorbuffers, bool depthstencil) = 0;

	/**
	 * Flips buffers. (Rendered geometry is presented on screen).
	 **/
	virtual void present(void *screenshotCallbackData) = 0;

	/**
	 * Sets the current graphics display viewport dimensions.
	 **/
	virtual void setViewportSize(int width, int height, int pixelwidth, int pixelheight) = 0;

	/**
	 * Sets the current graphics display viewport and initializes the renderer.
	 * @param width The viewport width.
	 * @param height The viewport height.
	 **/
	virtual bool setMode(int width, int height, int pixelwidth, int pixelheight) = 0;

	/**
	 * Un-sets the current graphics display mode (uninitializing objects if
	 * necessary.)
	 **/
	virtual void unSetMode() = 0;

	/**
	 * Sets whether the module is active (internal use only.)
	 **/
	virtual void setActive(bool active) = 0;

	/**
	 * Gets whether the module is active. Graphics module methods are only
	 * guaranteed to work when it is active. Calling them otherwise may cause
	 * the program to crash (or worse.)
	 * Normally the module will always be active as long as a window exists, it
	 * may be different on some platforms (especially mobile ones.)
	 **/
	bool isActive() const;

	/**
	 * True if a graphics viewport is set.
	 **/
	bool isCreated() const;

	int getWidth() const;
	int getHeight() const;
	int getPixelWidth() const;
	int getPixelHeight() const;

	double getCurrentPixelDensity() const;
	double getScreenPixelDensity() const;

	/**
	 * Sets the current constant color.
	 **/
	virtual void setColor(Colorf c) = 0;

	/**
	 * Gets current color.
	 **/
	Colorf getColor() const;

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

	void setCanvas(Canvas *canvas);
	virtual void setCanvas(const std::vector<Canvas *> &canvases) = 0;
	void setCanvas(const std::vector<StrongRef<Canvas>> &canvases);
	virtual void setCanvas() = 0;

	std::vector<Canvas *> getCanvas() const;
	bool isCanvasActive() const;
	bool isCanvasActive(Canvas *canvas) const;

	/**
	 * Scissor defines a box such that everything outside that box is discarded
	 * and not drawn. Scissoring is automatically enabled.
	 * @param rect The rectangle defining the scissor area.
	 **/
	virtual void setScissor(const Rect &rect) = 0;
	void intersectScissor(const Rect &rect);

	/**
	 * Clears any scissor that has been created.
	 **/
	virtual void setScissor() = 0;

	/**
	 * Gets the current scissor box.
	 * @return Whether the scissor is enabled.
	 */
	bool getScissor(Rect &rect) const;

	/**
	 * Enables or disables drawing to the stencil buffer. When enabled, the
	 * color buffer is disabled.
	 **/
	virtual void drawToStencilBuffer(StencilAction action, int value) = 0;
	virtual void stopDrawToStencilBuffer() = 0;

	/**
	 * Sets whether stencil testing is enabled.
	 **/
	virtual void setStencilTest(CompareMode compare, int value) = 0;
	virtual void setStencilTest() = 0;
	void getStencilTest(CompareMode &compare, int &value);

	/**
	 * Clear the stencil buffer in the active Canvas(es.)
	 **/
	virtual void clearStencil() = 0;

	/**
	 * Sets the enabled color components when rendering.
	 **/
	virtual void setColorMask(ColorMask mask) = 0;

	/**
	 * Gets the current color mask.
	 **/
	ColorMask getColorMask() const;

	/**
	 * Sets the current blend mode.
	 **/
	virtual void setBlendMode(BlendMode mode, BlendAlpha alphamode) = 0;

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
	float getLineWidth() const;

	/**
	 * Sets the line style.
	 * @param style LINE_ROUGH or LINE_SMOOTH.
	 **/
	void setLineStyle(LineStyle style);
	LineStyle getLineStyle() const;

	/**
	 * Sets the line join mode.
	 **/
	void setLineJoin(LineJoin style);
	LineJoin getLineJoin() const;

	/**
	 * Sets the size of points.
	 **/
	virtual void setPointSize(float size) = 0;

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
	virtual void setWireframe(bool enable) = 0;

	/**
	 * Gets whether wireframe drawing mode is enabled.
	 **/
	bool isWireframe() const;

	void captureScreenshot(const ScreenshotInfo &info);

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

	/**
	 * Draws a series of lines connecting the given vertices.
	 * @param coords Vertex components (x1, y1, ..., xn, yn). If x1,y1 == xn,yn the line will be drawn closed.
	 * @param count Number of items in the array, i.e. count = 2 * n
	 **/
	void polyline(const float *coords, size_t count);

	/**
	 * Draws a rectangle.
	 * @param x Position along x-axis for top-left corner.
	 * @param y Position along y-axis for top-left corner.
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 **/
	void rectangle(DrawMode mode, float x, float y, float w, float h);

	/**
	 * Variant of rectangle that draws a rounded rectangle.
	 * @param mode The mode of drawing (line/filled).
	 * @param x X-coordinate of top-left corner
	 * @param y Y-coordinate of top-left corner
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 * @param rx The radius of the corners on the x axis
	 * @param ry The radius of the corners on the y axis
	 * @param points The number of points to use per corner
	 **/
	void rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry, int points);
	void rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry);

	/**
	 * Draws a circle using the specified arguments.
	 * @param mode The mode of drawing (line/filled).
	 * @param x X-coordinate.
	 * @param y Y-coordinate.
	 * @param radius Radius of the circle.
	 * @param points Number of points to use to draw the circle.
	 **/
	void circle(DrawMode mode, float x, float y, float radius, int points);
	void circle(DrawMode mode, float x, float y, float radius);

	/**
	 * Draws an ellipse using the specified arguments.
	 * @param mode The mode of drawing (line/filled).
	 * @param x X-coordinate of center
	 * @param y Y-coordinate of center
	 * @param a Radius in x-direction
	 * @param b Radius in y-direction
	 * @param points Number of points to use to draw the circle.
	 **/
	void ellipse(DrawMode mode, float x, float y, float a, float b, int points);
	void ellipse(DrawMode mode, float x, float y, float a, float b);

	/**
	 * Draws an arc using the specified arguments.
	 * @param drawmode The mode of drawing (line/filled).
	 * @param arcmode The type of arc.
	 * @param x X-coordinate.
	 * @param y Y-coordinate.
	 * @param radius Radius of the arc.
	 * @param angle1 The angle at which the arc begins.
	 * @param angle2 The angle at which the arc terminates.
	 * @param points Number of points to use to draw the arc.
	 **/
	void arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2, int points);
	void arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2);

	/**
	 * Draws a polygon with an arbitrary number of vertices.
	 * @param mode The type of drawing (line/filled).
	 * @param coords Vertex components (x1, y1, x2, y2, etc.)
	 * @param count Coord array size
	 **/
	void polygon(DrawMode mode, const float *coords, size_t count);

	/**
	 * Gets whether a graphics feature is supported on this system.
	 **/
	virtual bool isSupported(Feature feature) const = 0;

	/**
	 * Gets the system-dependent numeric limit for the specified parameter.
	 **/
	virtual double getSystemLimit(SystemLimit limittype) const = 0;

	/**
	 * Returns system-dependent renderer information.
	 * Returned strings can vary greatly between systems! Do not rely on it for
	 * anything!
	 **/
	virtual RendererInfo getRendererInfo() const = 0;

	/**
	 * Returns performance-related statistics.
	 **/
	virtual Stats getStats() const = 0;

	void push(StackType type = STACK_TRANSFORM);
	void pop();

	const Matrix4 &getTransform() const;
	const Matrix4 &getProjection() const;

	void rotate(float r);
	void scale(float x, float y = 1.0f);
	void translate(float x, float y);
	void shear(float kx, float ky);
	void origin();

	void applyTransform(love::math::Transform *transform);
	void replaceTransform(love::math::Transform *transform);

	Vector transformPoint(Vector point);
	Vector inverseTransformPoint(Vector point);

	virtual void flushStreamDraws() = 0;
	StreamVertexData requestStreamDraw(const StreamDrawRequest &request);

	template <typename T>
	T *getScratchBuffer(size_t count)
	{
		size_t bytes = sizeof(T) * count;

		if (scratchBuffer.size() < bytes)
			scratchBuffer.resize(bytes);

		return (T *) scratchBuffer.data();
	}

	static bool getConstant(const char *in, DrawMode &out);
	static bool getConstant(DrawMode in, const char *&out);

	static bool getConstant(const char *in, ArcMode &out);
	static bool getConstant(ArcMode in, const char *&out);

	static bool getConstant(const char *in, BlendMode &out);
	static bool getConstant(BlendMode in, const char *&out);

	static bool getConstant(const char *in, BlendAlpha &out);
	static bool getConstant(BlendAlpha in, const char *&out);

	static bool getConstant(const char *in, LineStyle &out);
	static bool getConstant(LineStyle in, const char *&out);

	static bool getConstant(const char *in, LineJoin &out);
	static bool getConstant(LineJoin in, const char *&out);

	static bool getConstant(const char *in, StencilAction &out);
	static bool getConstant(StencilAction in, const char *&out);

	static bool getConstant(const char *in, CompareMode &out);
	static bool getConstant(CompareMode in, const char *&out);

	static bool getConstant(const char *in, Feature &out);
	static bool getConstant(Feature in, const char *&out);

	static bool getConstant(const char *in, SystemLimit &out);
	static bool getConstant(SystemLimit in, const char *&out);

	static bool getConstant(const char *in, StackType &out);
	static bool getConstant(StackType in, const char *&out);

	// Default shader code (a shader is always required internally.)
	static Shader::ShaderSource defaultShaderCode[RENDERER_MAX_ENUM][2];
	static Shader::ShaderSource defaultVideoShaderCode[RENDERER_MAX_ENUM][2];

protected:

	struct DisplayState
	{
		Colorf color = Colorf(1.0, 1.0, 1.0, 1.0);
		Colorf backgroundColor = Colorf(0.0, 0.0, 0.0, 1.0);

		BlendMode blendMode = BLEND_ALPHA;
		BlendAlpha blendAlphaMode = BLENDALPHA_MULTIPLY;

		float lineWidth = 1.0f;
		LineStyle lineStyle = LINE_SMOOTH;
		LineJoin lineJoin = LINE_JOIN_MITER;

		float pointSize = 1.0f;

		bool scissor = false;
		Rect scissorRect = Rect();

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

	struct StreamBufferState
	{
		StreamBuffer *vb[2];
		StreamBuffer *indexBuffer;
		vertex::PrimitiveMode primitiveMode;
		vertex::CommonFormat formats[2];
		StrongRef<Texture> texture;
		ptrdiff_t textureHandle = 0;
		int vertexCount;
		int indexCount;
	};

	void restoreState(const DisplayState &s);
	void restoreStateChecked(const DisplayState &s);

	void pushTransform();
	void pushIdentityTransform();
	void popTransform();

	int width;
	int height;
	int pixelWidth;
	int pixelHeight;

	bool created;
	bool active;

	bool writingToStencil;

	StrongRef<love::graphics::Font> defaultFont;

	std::vector<ScreenshotInfo> pendingScreenshotCallbacks;

	StreamBufferState streamBufferState;

	std::vector<Matrix4> transformStack;
	Matrix4 projectionMatrix;

	std::vector<double> pixelScaleStack;

	std::vector<DisplayState> states;
	std::vector<StackType> stackTypeStack;

	int canvasSwitchCount;

	static const size_t MAX_USER_STACK_DEPTH = 64;

private:

	void checkSetDefaultFont();
	int calculateEllipsePoints(float rx, float ry) const;

	std::vector<uint8> scratchBuffer;

	static StringMap<DrawMode, DRAW_MAX_ENUM>::Entry drawModeEntries[];
	static StringMap<DrawMode, DRAW_MAX_ENUM> drawModes;

	static StringMap<ArcMode, ARC_MAX_ENUM>::Entry arcModeEntries[];
	static StringMap<ArcMode, ARC_MAX_ENUM> arcModes;

	static StringMap<BlendMode, BLEND_MAX_ENUM>::Entry blendModeEntries[];
	static StringMap<BlendMode, BLEND_MAX_ENUM> blendModes;

	static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM>::Entry blendAlphaEntries[];
	static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM> blendAlphaModes;

	static StringMap<LineStyle, LINE_MAX_ENUM>::Entry lineStyleEntries[];
	static StringMap<LineStyle, LINE_MAX_ENUM> lineStyles;

	static StringMap<LineJoin, LINE_JOIN_MAX_ENUM>::Entry lineJoinEntries[];
	static StringMap<LineJoin, LINE_JOIN_MAX_ENUM> lineJoins;

	static StringMap<StencilAction, STENCIL_MAX_ENUM>::Entry stencilActionEntries[];
	static StringMap<StencilAction, STENCIL_MAX_ENUM> stencilActions;

	static StringMap<CompareMode, COMPARE_MAX_ENUM>::Entry compareModeEntries[];
	static StringMap<CompareMode, COMPARE_MAX_ENUM> compareModes;

	static StringMap<Feature, FEATURE_MAX_ENUM>::Entry featureEntries[];
	static StringMap<Feature, FEATURE_MAX_ENUM> features;

	static StringMap<SystemLimit, LIMIT_MAX_ENUM>::Entry systemLimitEntries[];
	static StringMap<SystemLimit, LIMIT_MAX_ENUM> systemLimits;

	static StringMap<StackType, STACK_MAX_ENUM>::Entry stackTypeEntries[];
	static StringMap<StackType, STACK_MAX_ENUM> stackTypes;

}; // Graphics

} // graphics
} // love

#endif // LOVE_GRAPHICS_GRAPHICS_H
