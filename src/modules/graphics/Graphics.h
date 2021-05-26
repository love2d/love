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

#ifndef LOVE_GRAPHICS_GRAPHICS_H
#define LOVE_GRAPHICS_GRAPHICS_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "common/StringMap.h"
#include "common/Vector.h"
#include "common/Optional.h"
#include "common/int.h"
#include "common/Color.h"
#include "StreamBuffer.h"
#include "vertex.h"
#include "Texture.h"
#include "Font.h"
#include "ShaderStage.h"
#include "Shader.h"
#include "Quad.h"
#include "Mesh.h"
#include "Deprecations.h"
#include "renderstate.h"
#include "math/Transform.h"
#include "font/Rasterizer.h"
#include "font/Font.h"
#include "video/VideoStream.h"
#include "data/HashFunction.h"

// C++
#include <string>
#include <vector>

namespace love
{

namespace graphics
{

class SpriteBatch;
class ParticleSystem;
class Text;
class Video;
class Buffer;

typedef Optional<Colorf> OptionalColorf;

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

bool isDebugEnabled();

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

	enum Feature
	{
		FEATURE_MULTI_RENDER_TARGET_FORMATS,
		FEATURE_CLAMP_ZERO,
		FEATURE_BLEND_MINMAX,
		FEATURE_LIGHTEN, // Deprecated
		FEATURE_FULL_NPOT,
		FEATURE_PIXEL_SHADER_HIGHP,
		FEATURE_SHADER_DERIVATIVES,
		FEATURE_GLSL3,
		FEATURE_GLSL4,
		FEATURE_INSTANCING,
		FEATURE_TEXEL_BUFFER,
		FEATURE_COPY_BUFFER,
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
		LIMIT_VOLUME_TEXTURE_SIZE,
		LIMIT_CUBE_TEXTURE_SIZE,
		LIMIT_TEXTURE_LAYERS,
		LIMIT_TEXEL_BUFFER_SIZE,
		LIMIT_SHADER_STORAGE_BUFFER_SIZE,
		LIMIT_THREADGROUPS_X,
		LIMIT_THREADGROUPS_Y,
		LIMIT_THREADGROUPS_Z,
		LIMIT_RENDER_TARGETS,
		LIMIT_TEXTURE_MSAA,
		LIMIT_ANISOTROPY,
		LIMIT_MAX_ENUM
	};

	enum StackType
	{
		STACK_ALL,
		STACK_TRANSFORM,
		STACK_MAX_ENUM
	};

	enum TemporaryRenderTargetFlags
	{
		TEMPORARY_RT_DEPTH   = (1 << 0),
		TEMPORARY_RT_STENCIL = (1 << 1),
	};

	struct Capabilities
	{
		double limits[LIMIT_MAX_ENUM];
		bool features[FEATURE_MAX_ENUM];
		bool textureTypes[TEXTURE_MAX_ENUM];
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
		int drawCallsBatched;
		int renderTargetSwitches;
		int shaderSwitches;
		int textures;
		int fonts;
		int64 textureMemory;
	};

	struct DrawCommand
	{
		PrimitiveType primitiveType = PRIMITIVE_TRIANGLES;

		const VertexAttributes *attributes;
		const BufferBindings *buffers;

		int vertexStart = 0;
		int vertexCount = 0;
		int instanceCount = 1;

		Texture *texture = nullptr;

		// TODO: This should be moved out to a state transition API?
		CullMode cullMode = CULL_NONE;

		DrawCommand(const VertexAttributes *attribs, const BufferBindings *buffers)
			: attributes(attribs)
			, buffers(buffers)
		{}
	};

	struct DrawIndexedCommand
	{
		PrimitiveType primitiveType = PRIMITIVE_TRIANGLES;

		const VertexAttributes *attributes;
		const BufferBindings *buffers;

		int indexCount = 0;
		int instanceCount = 1;

		IndexDataType indexType = INDEX_UINT16;
		Resource *indexBuffer;
		size_t indexBufferOffset = 0;

		Texture *texture = nullptr;

		// TODO: This should be moved out to a state transition API?
		CullMode cullMode = CULL_NONE;

		DrawIndexedCommand(const VertexAttributes *attribs, const BufferBindings *buffers, Resource *indexbuffer)
			: attributes(attribs)
			, buffers(buffers)
			, indexBuffer(indexbuffer)
		{}
	};

	struct BatchedDrawCommand
	{
		PrimitiveType primitiveMode = PRIMITIVE_TRIANGLES;
		CommonFormat formats[2];
		TriangleIndexMode indexMode = TRIANGLEINDEX_NONE;
		int vertexCount = 0;
		Texture *texture = nullptr;
		Shader::StandardShader standardShaderType = Shader::STANDARD_DEFAULT;

		BatchedDrawCommand()
		{
			// VS2013 can't initialize arrays in the above manner...
			formats[1] = formats[0] = CommonFormat::NONE;
		}
	};

	struct BatchedVertexData
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

	struct ScreenshotInfo;
	typedef void (*ScreenshotCallback)(const ScreenshotInfo *info, love::image::ImageData *i, void *ud);

	struct ScreenshotInfo
	{
		ScreenshotCallback callback = nullptr;
		void *data = nullptr;
	};

	struct RenderTargetStrongRef;

	struct RenderTarget
	{
		Texture *texture;
		int slice;
		int mipmap;

		RenderTarget(Texture *texture, int slice = 0, int mipmap = 0)
			: texture(texture)
			, slice(slice)
			, mipmap(mipmap)
		{}

		RenderTarget()
			: texture(nullptr)
			, slice(0)
			, mipmap(0)
		{}

		bool operator != (const RenderTarget &other) const
		{
			return texture != other.texture || slice != other.slice || mipmap != other.mipmap;
		}

		bool operator != (const RenderTargetStrongRef &other) const
		{
			return texture != other.texture.get() || slice != other.slice || mipmap != other.mipmap;
		}
	};

	struct RenderTargetStrongRef
	{
		StrongRef<Texture> texture;
		int slice = 0;
		int mipmap = 0;

		RenderTargetStrongRef(Texture *texture, int slice = 0, int mipmap = 0)
			: texture(texture)
			, slice(slice)
			, mipmap(mipmap)
		{}

		bool operator != (const RenderTargetStrongRef &other) const
		{
			return texture.get() != other.texture.get() || slice != other.slice || mipmap != other.mipmap;
		}

		bool operator != (const RenderTarget &other) const
		{
			return texture.get() != other.texture || slice != other.slice || mipmap != other.mipmap;
		}
	};

	struct RenderTargets
	{
		std::vector<RenderTarget> colors;
		RenderTarget depthStencil;
		uint32 temporaryRTFlags;

		RenderTargets()
			: depthStencil(nullptr)
			, temporaryRTFlags(0)
		{}

		const RenderTarget &getFirstTarget() const
		{
			return colors.empty() ? depthStencil : colors[0];
		}

		bool operator == (const RenderTargets &other) const
		{
			size_t ncolors = colors.size();
			if (ncolors != other.colors.size())
				return false;

			for (size_t i = 0; i < ncolors; i++)
			{
				if (colors[i] != other.colors[i])
					return false;
			}

			if (depthStencil != other.depthStencil || temporaryRTFlags != other.temporaryRTFlags)
				return false;

			return true;
		}
	};

	struct RenderTargetsStrongRef
	{
		std::vector<RenderTargetStrongRef> colors;
		RenderTargetStrongRef depthStencil;
		uint32 temporaryRTFlags;

		RenderTargetsStrongRef()
			: depthStencil(nullptr)
			, temporaryRTFlags(0)
		{}

		const RenderTargetStrongRef &getFirstTarget() const
		{
			return colors.empty() ? depthStencil : colors[0];
		}
	};

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_GRAPHICS; }

	virtual Texture *newTexture(const Texture::Settings &settings, const Texture::Slices *data = nullptr) = 0;

	Quad *newQuad(Quad::Viewport v, double sw, double sh);
	Font *newFont(love::font::Rasterizer *data);
	Font *newDefaultFont(int size, font::TrueTypeRasterizer::Hinting hinting);
	Video *newVideo(love::video::VideoStream *stream, float dpiscale);

	SpriteBatch *newSpriteBatch(Texture *texture, int size, BufferDataUsage usage);
	ParticleSystem *newParticleSystem(Texture *texture, int size);

	Shader *newShader(const std::vector<std::string> &stagessource);

	virtual Buffer *newBuffer(const Buffer::Settings &settings, const std::vector<Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength) = 0;
	virtual Buffer *newBuffer(const Buffer::Settings &settings, DataFormat format, const void *data, size_t size, size_t arraylength);

	Mesh *newMesh(const std::vector<Buffer::DataDeclaration> &vertexformat, int vertexcount, PrimitiveType drawmode, BufferDataUsage usage);
	Mesh *newMesh(const std::vector<Buffer::DataDeclaration> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, BufferDataUsage usage);
	Mesh *newMesh(const std::vector<Mesh::BufferAttribute> &attributes, PrimitiveType drawmode);

	Text *newText(Font *font, const std::vector<Font::ColoredString> &text = {});

	bool validateShader(bool gles, const std::vector<std::string> &stages, std::string &err);

	/**
	 * Resets the current color, background color, line style, and so forth.
	 **/
	void reset();

	virtual void clear(OptionalColorf color, OptionalInt stencil, OptionalDouble depth) = 0;
	virtual void clear(const std::vector<OptionalColorf> &colors, OptionalInt stencil, OptionalDouble depth) = 0;

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
	virtual bool setMode(int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) = 0;

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

	double getCurrentDPIScale() const;
	double getScreenDPIScale() const;

	virtual int getRequestedBackbufferMSAA() const = 0;
	virtual int getBackbufferMSAA() const = 0;

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

	void setRenderTarget(RenderTarget rt, uint32 temporaryRTFlags);
	void setRenderTargets(const RenderTargets &rts);
	void setRenderTargets(const RenderTargetsStrongRef &rts);
	void setRenderTarget();

	RenderTargets getRenderTargets() const;
	bool isRenderTargetActive() const;
	bool isRenderTargetActive(Texture *texture) const;
	bool isRenderTargetActive(Texture *texture, int slice) const;

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
	void setStencilTest();
	void getStencilTest(CompareMode &compare, int &value) const;

	virtual void setDepthMode(CompareMode compare, bool write) = 0;
	void setDepthMode();
	void getDepthMode(CompareMode &compare, bool &write) const;

	void setMeshCullMode(CullMode cull);
	CullMode getMeshCullMode() const;

	virtual void setFrontFaceWinding(Winding winding) = 0;
	Winding getFrontFaceWinding() const;

	/**
	 * Sets the enabled color components when rendering.
	 **/
	virtual void setColorMask(ColorChannelMask mask) = 0;

	/**
	 * Gets the current color mask.
	 **/
	ColorChannelMask getColorMask() const;

	/**
	 * High-level blend mode.
	 **/
	void setBlendMode(BlendMode mode, BlendAlpha alphamode);
	BlendMode getBlendMode(BlendAlpha &alphamode) const;

	/**
	 * Low-level blend state.
	 **/
	virtual void setBlendState(const BlendState &blend) = 0;
	const BlendState &getBlendState() const;

	/**
	 * Sets the default sampler state for textures, videos, and fonts.
	 **/
	void setDefaultSamplerState(const SamplerState &s);

	/**
	 * Gets the default sampler state for textures, videos, and fonts.
	 **/
	const SamplerState &getDefaultSamplerState() const;

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

	void copyBuffer(Buffer *source, Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size);

	void draw(Drawable *drawable, const Matrix4 &m);
	void draw(Texture *texture, Quad *quad, const Matrix4 &m);
	void drawLayer(Texture *texture, int layer, const Matrix4 &m);
	void drawLayer(Texture *texture, int layer, Quad *quad, const Matrix4 &m);
	void drawInstanced(Mesh *mesh, const Matrix4 &m, int instancecount);

	/**
	 * Draws text at the specified coordinates
	 **/
	void print(const std::vector<Font::ColoredString> &str, const Matrix4 &m);
	void print(const std::vector<Font::ColoredString> &str, Font *font, const Matrix4 &m);

	/**
	 * Draws formatted text on screen at the specified coordinates.
	 **/
	void printf(const std::vector<Font::ColoredString> &str, float wrap, Font::AlignMode align, const Matrix4 &m);
	void printf(const std::vector<Font::ColoredString> &str, Font *font, float wrap, Font::AlignMode align, const Matrix4 &m);

	/**
	 * Draws a series of points at the specified positions.
	 **/
	void points(const Vector2 *positions, const Colorf *colors, size_t numpoints);

	/**
	 * Draws a series of lines connecting the given vertices.
	 * @param coords Vertex positions (v1, ..., vn). If v1 == vn the line will be drawn closed.
	 * @param count Number of vertices.
	 **/
	void polyline(const Vector2 *vertices, size_t count);

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
	 * @param coords Vertex positions.
	 * @param count Vertex array size.
	 **/
	void polygon(DrawMode mode, const Vector2 *vertices, size_t count, bool skipLastFilledVertex = true);

	/**
	 * Gets the graphics capabilities (feature support, limit values, and
	 * supported texture types) of this system.
	 **/
	const Capabilities &getCapabilities() const;

	/**
	 * Converts PIXELFORMAT_NORMAL and PIXELFORMAT_HDR into a real format.
	 **/
	virtual PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const = 0;

	/**
	 * Gets whether the specified pixel format is supported.
	 **/
	virtual bool isPixelFormatSupported(PixelFormat format, bool rendertarget, bool readable, bool sRGB = false) = 0;

	/**
	 * Gets the renderer used by love.graphics.
	 **/
	virtual Renderer getRenderer() const = 0;

	/**
	 * Returns system-dependent renderer information.
	 * Returned strings can vary greatly between systems! Do not rely on it for
	 * anything!
	 **/
	virtual RendererInfo getRendererInfo() const = 0;

	/**
	 * Returns performance-related statistics.
	 **/
	Stats getStats() const;

	size_t getStackDepth() const;
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

	Vector2 transformPoint(Vector2 point);
	Vector2 inverseTransformPoint(Vector2 point);

	virtual void draw(const DrawCommand &cmd) = 0;
	virtual void draw(const DrawIndexedCommand &cmd) = 0;
	virtual void drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, Texture *texture) = 0;

	void flushBatchedDraws();
	BatchedVertexData requestBatchedDraw(const BatchedDrawCommand &command);

	static void flushBatchedDrawsGlobal();

	void cleanupCachedShaderStage(ShaderStageType type, const std::string &cachekey);

	template <typename T>
	T *getScratchBuffer(size_t count)
	{
		size_t bytes = sizeof(T) * count;

		if (scratchBuffer.size() < bytes)
			scratchBuffer.resize(bytes);

		return (T *) scratchBuffer.data();
	}

	STRINGMAP_CLASS_DECLARE(DrawMode);
	STRINGMAP_CLASS_DECLARE(ArcMode);
	STRINGMAP_CLASS_DECLARE(LineStyle);
	STRINGMAP_CLASS_DECLARE(LineJoin);
	STRINGMAP_CLASS_DECLARE(Feature);
	STRINGMAP_CLASS_DECLARE(SystemLimit);
	STRINGMAP_CLASS_DECLARE(StackType);

protected:

	struct DisplayState
	{
		DisplayState();

		Colorf color = Colorf(1.0, 1.0, 1.0, 1.0);
		Colorf backgroundColor = Colorf(0.0, 0.0, 0.0, 1.0);

		BlendState blend = computeBlendState(BLEND_ALPHA, BLENDALPHA_MULTIPLY);

		float lineWidth = 1.0f;
		LineStyle lineStyle = LINE_SMOOTH;
		LineJoin lineJoin = LINE_JOIN_MITER;

		float pointSize = 1.0f;

		bool scissor = false;
		Rect scissorRect = Rect();

		CompareMode stencilCompare = COMPARE_ALWAYS;
		int stencilTestValue = 0;

		CompareMode depthTest = COMPARE_ALWAYS;
		bool depthWrite = false;

		CullMode meshCullMode = CULL_NONE;
		Winding winding = WINDING_CCW;

		StrongRef<Font> font;
		StrongRef<Shader> shader;

		RenderTargetsStrongRef renderTargets;

		ColorChannelMask colorMask;

		bool wireframe = false;

		// Default mipmap filter is set in the DisplayState constructor.
		SamplerState defaultSamplerState = SamplerState();
	};

	struct BatchedDrawState
	{
		StreamBuffer *vb[2];
		StreamBuffer *indexBuffer = nullptr;

		PrimitiveType primitiveMode = PRIMITIVE_TRIANGLES;
		CommonFormat formats[2];
		StrongRef<Texture> texture;
		Shader::StandardShader standardShaderType = Shader::STANDARD_DEFAULT;
		int vertexCount = 0;
		int indexCount = 0;

		StreamBuffer::MapInfo vbMap[2];
		StreamBuffer::MapInfo indexBufferMap = StreamBuffer::MapInfo();

		BatchedDrawState()
		{
			vb[0] = vb[1] = nullptr;
			formats[0] = formats[1] = CommonFormat::NONE;
			vbMap[0] = vbMap[1] = StreamBuffer::MapInfo();
		}
	};

	struct TemporaryTexture
	{
		Texture *texture;
		int framesSinceUse;

		TemporaryTexture(Texture *tex)
			: texture(tex)
			, framesSinceUse(0)
		{}
	};

	ShaderStage *newShaderStage(ShaderStageType stage, const std::string &source, const Shader::SourceInfo &info);
	virtual ShaderStage *newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles) = 0;
	virtual Shader *newShaderInternal(ShaderStage *vertex, ShaderStage *pixel) = 0;
	virtual StreamBuffer *newStreamBuffer(BufferUsage type, size_t size) = 0;

	virtual void setRenderTargetsInternal(const RenderTargets &rts, int w, int h, int pixelw, int pixelh, bool hasSRGBtexture) = 0;

	virtual void initCapabilities() = 0;
	virtual void getAPIStats(int &shaderswitches) const = 0;

	void createQuadIndexBuffer();

	Texture *getTemporaryTexture(PixelFormat format, int w, int h, int samples);

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

	BatchedDrawState batchedDrawState;

	std::vector<Matrix4> transformStack;
	Matrix4 projectionMatrix;

	std::vector<double> pixelScaleStack;

	std::vector<DisplayState> states;
	std::vector<StackType> stackTypeStack;

	std::vector<TemporaryTexture> temporaryTextures;

	int renderTargetSwitchCount;
	int drawCalls;
	int drawCallsBatched;

	Buffer *quadIndexBuffer;

	Capabilities capabilities;

	Deprecations deprecations;

	static const size_t MAX_USER_STACK_DEPTH = 128;
	static const int MAX_TEMPORARY_TEXTURE_UNUSED_FRAMES = 16;

private:

	void checkSetDefaultFont();
	int calculateEllipsePoints(float rx, float ry) const;

	std::vector<uint8> scratchBuffer;

	std::unordered_map<std::string, ShaderStage *> cachedShaderStages[SHADERSTAGE_MAX_ENUM];

}; // Graphics

} // graphics
} // love

#endif // LOVE_GRAPHICS_GRAPHICS_H
