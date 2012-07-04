/**
 * Copyright (c) 2006-2012 LOVE Development Team
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
#include <iostream>
#include <cmath>

// OpenGL
#include "GLee.h"

// LOVE
#include "graphics/Graphics.h"
#include "graphics/Color.h"

#include "image/Image.h"
#include "image/ImageData.h"

#include "window/Window.h"

#include "OpenGL.h"
#include "Font.h"
#include "Image.h"
#include "Quad.h"
#include "SpriteBatch.h"
#include "ParticleSystem.h"
#include "Canvas.h"
#include "PixelEffect.h"

namespace love
{
namespace graphics
{
namespace opengl
{

// During display mode changing, certain
// variables about the OpenGL context are
// lost.
struct DisplayState
{
	// Colors.
	Color color;
	Color backgroundColor;

	// Blend and color modes.
	Graphics::BlendMode blendMode;
	Graphics::ColorMode colorMode;

	// Line.
	Graphics::LineStyle lineStyle;

	// Point.
	float pointSize;
	Graphics::PointStyle pointStyle;

	// Scissor.
	bool scissor;
	GLint scissorBox[4];

	// Window info.
	std::string caption;
	bool mouseVisible;

	// Default values.
	DisplayState()
	{
		color.set(255,255,255,255);
		backgroundColor.r = 0;
		backgroundColor.g = 0;
		backgroundColor.b = 0;
		backgroundColor.a = 255;
		blendMode = Graphics::BLEND_ALPHA;
		colorMode = Graphics::COLOR_MODULATE;
		lineStyle = Graphics::LINE_SMOOTH;
		pointSize = 1.0f;
		pointStyle = Graphics::POINT_SMOOTH;
		scissor = false;
		caption = "";
		mouseVisible = true;
	}

};

class Graphics : public love::graphics::Graphics
{
public:

	Graphics();
	virtual ~Graphics();

	// Implements Module.
	const char *getName() const;

	/**
	 * Checks whether a display mode is supported or not. Note
	 * that fullscreen is assumed, because windowed modes are
	 * generally supported regardless of size.
	 * @param width The window width.
	 * @param height The window height.
	 **/
	bool checkMode(int width, int height, bool fullscreen);

	DisplayState saveState();

	void restoreState(const DisplayState &s);

	/**
	 * Sets the current display mode.
	 * @param width The window width.
	 * @param height The window height.
	 * @param fullscreen True if fullscreen, false otherwise.
	 * @param vsync True if we should wait for vsync, false otherwise.
	 * @param fsaa Number of full scene anti-aliasing buffer, or 0 for disabled.
	 **/
	bool setMode(int width, int height, bool fullscreen, bool vsync, int fsaa);

	/**
	 * Gets the current display mode.
	 * @param width Pointer to an integer for the window width.
	 * @param height Pointer to an integer for the window height.
	 * @param fullscreen Pointer to a boolean for the fullscreen status.
	 * @param vsync Pointer to a boolean for the vsync status.
	 * @param fsaa Pointer to an integer for the current number of full scene anti-aliasing buffers.
	 **/
	void getMode(int &width, int &height, bool &fullscreen, bool &vsync, int &fsaa);

	/**
	 * Toggles fullscreen. Note that this also needs to reload the
	 * entire OpenGL context.
	 **/
	bool toggleFullscreen();

	/**
	 * Resets the current color, background color,
	 * line style, and so forth. (This will be called
	 * when the game reloads.
	 **/
	void reset();

	/**
	 * Clears the screen.
	 **/
	void clear();

	/**
	 * Flips buffers. (Rendered geometry is
	 * presented on screen).
	 **/
	void present();

	/**
	 * Sets the window's icon.
	 **/
	void setIcon(Image *image);

	/**
	 * Sets the window's caption.
	 **/
	void setCaption(const char *caption);

	int getCaption(lua_State *L);

	/**
	 * Gets the width of the current display mode.
	 **/
	int getWidth();

	/**
	 * Gets the height of the current display mode.
	 **/
	int getHeight();

	/**
	 * True if some display mode is set.
	 **/
	bool isCreated();

	/**
	 * This native Lua function gets available modes
	 * from SDL and returns them as a table on the following format:
	 *
	 * {
	 *   { width = 800, height = 600 },
	 *   { width = 1024, height = 768 },
	 *   ...
	 * }
	 *
	 * Only fullscreen modes are returned here, as all
	 * window sizes are supported (normally).
	 **/
	int getModes(lua_State *L);

	/**
	 * Scissor defines a box such that everything outside that box is discared and not drawn.
	 * Scissoring is automatically enabled.
	 * @param x The x-coordinate of the top-left corner.
	 * @param y The y-coordinate of the top-left corner.
	 * @param width The width of the box.
	 * @param height The height of the box.
	 **/
	void setScissor(int x, int y, int width, int height);

	/**
	 * Clears any scissor that has been created.
	 **/
	void setScissor();

	/**
	 * This native Lua function gets the current scissor box in the order of:
	 * x, y, width, height
	 **/
	int getScissor(lua_State *L);

	/**
	 * Enables the stencil buffer and set stencil function to fill it
	 */
	void defineStencil();

	/**
	 * Set stencil function to mask the following drawing calls using
	 * the current stencil buffer
	 * @param invert Invert the mask, i.e. draw everywhere expect where
	 *               the mask is defined.
	 */
	void useStencil(bool invert = false);

	/**
	 * Disables the stencil buffer
	 */
	void discardStencil();

	/**
	 * Creates an Image object with padding and/or optimization.
	 **/
	Image *newImage(love::filesystem::File *file);
	Image *newImage(love::image::ImageData *data);

	/**
	 * Creates a Quad object.
	 **/
	Quad *newQuad(float x, float y, float w, float h, float sw, float sh);

	/**
	 * Creates a Font object.
	 **/
	Font *newFont(love::font::Rasterizer *data, const Image::Filter &filter = Image::Filter());

	SpriteBatch *newSpriteBatch(Image *image, int size, int usage);

	ParticleSystem *newParticleSystem(Image *image, int size);

	Canvas *newCanvas(int width, int height);

	PixelEffect *newPixelEffect(const std::string &code);

	/**
	 * Sets the foreground color.
	 * @param c The new foreground color.
	 **/
	void setColor(const Color &c);

	/**
	 * Gets current color.
	 **/
	Color getColor();

	/**
	 * Sets the background Color.
	 **/
	void setBackgroundColor(const Color &c);

	/**
	 * Gets the current background color.
	 **/
	Color getBackgroundColor();

	/**
	 * Sets the current font.
	 * @param font A Font object.
	 **/
	void setFont(Font *font);
	/**
	 * Gets the current Font, or nil if none.
	 **/
	Font *getFont();

	/**
	 * Sets the current blend mode.
	 **/
	void setBlendMode(BlendMode mode);

	/**
	 * Sets the current color mode.
	 **/
	void setColorMode(ColorMode mode);

	/**
	 * Sets the current image filter.
	 **/
	void setDefaultImageFilter(const Image::Filter &f);

	/**
	 * Gets the current blend mode.
	 **/
	BlendMode getBlendMode();

	/**
	 * Gets the current color mode.
	 **/
	ColorMode getColorMode();

	/**
	 * Gets the current image filter.
	 **/
	const Image::Filter &getDefaultImageFilter() const;

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
	 * Sets the type of line used to draw primitives.
	 * A shorthand for setLineWidth and setLineStyle.
	 **/
	void setLine(float width, LineStyle style);

	/**
	 * Gets the line width.
	 **/
	float getLineWidth();

	/**
	 * Gets the line style.
	 **/
	LineStyle getLineStyle();

	/**
	 * Sets the size of points.
	 **/
	void setPointSize(float size);

	/**
	 * Sets the style of points.
	 * @param style POINT_SMOOTH or POINT_ROUGH.
	 **/
	void setPointStyle(PointStyle style);

	/**
	 * Shorthand for setPointSize and setPointStyle.
	 **/
	void setPoint(float size, PointStyle style);

	/**
	 * Gets the point size.
	 **/
	float getPointSize();

	/**
	 * Gets the point style.
	 **/
	PointStyle getPointStyle();

	/**
	 * Gets the maximum point size supported.
	 * This may vary from computer to computer.
	 **/
	int getMaxPointSize();

	/**
	 * Draws text at the specified coordinates, with rotation and
	 * scaling along both axes.
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 * @param angle The amount of rotation.
	 * @param sx The scale factor along the x-axis. (1 = normal).
	 * @param sy The scale factor along the y-axis. (1 = normal).
	 * @param ox The origin offset along the x-axis.
	 * @param oy The origin offset along the y-axis.
	 * @param kx Shear along the x-axis.
	 * @param ky Shear along the y-axis.
	 **/
	void print(const char *str, float x, float y , float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * Draw formatted text on screen at the specified coordinates.
	 *
	 * @param str A string of text.
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 * @param wrap The maximum width of the text area.
	 * @param align Where to align the text.
	 **/
	void printf(const char *str, float x, float y, float wrap, AlignMode align);

	/**
	 * Draws a point at (x,y).
	 * @param x Point along x-axis.
	 * @param y Point along y-axis.
	 **/
	void point(float x, float y);

	/**
	 * Draws a series of lines connecting the given vertices.
	 * @param coords Vertex components (x1, y1, ..., xn, yn). If x1,y1 == xn,yn the line will be drawn closed.
	 * @param count Number of items in the array, i.e. count = 2 * n
	 **/
	void polyline(const float *coords, size_t count);

	/**
	 * Draws a triangle using the three coordinates passed.
	 * @param mode The mode of drawing (line/filled).
	 * @param x1 First x-coordinate.
	 * @param y1 First y-coordinate.
	 * @param x2 Second x-coordinate.
	 * @param y2 Second y-coordinate.
	 * @param x3 Third x-coordinate.
	 * @param y3 Third y-coordinate.
	 **/
	void triangle(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3);

	/**
	 * Draws a rectangle.
	 * @param x Position along x-axis for top-left corner.
	 * @param y Position along y-axis for top-left corner.
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 **/
	void rectangle(DrawMode mode, float x, float y, float w, float h);

	/**
	 * Draws a quadrilateral using the four coordinates passed.
	 * @param mode The mode of drawing (line/filled).
	 * @param x1 First x-coordinate.
	 * @param y1 First y-coordinate.
	 * @param x2 Second x-coordinate.
	 * @param y2 Second y-coordinate.
	 * @param x3 Third x-coordinate.
	 * @param y3 Third y-coordinate.
	 * @param x4 Fourth x-coordinate.
	 * @param y4 Fourth y-coordinate.
	 **/
	void quad(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

	/**
	 * Draws a circle using the specified arguments.
	 * @param mode The mode of drawing (line/filled).
	 * @param x X-coordinate.
	 * @param y Y-coordinate.
	 * @param radius Radius of the circle.
	 * @param points Number of points to use to draw the circle.
	 **/
	void circle(DrawMode mode, float x, float y, float radius, int points = 10);

	/**
	 * Draws an arc using the specified arguments.
	 * @param mode The mode of drawing (line/filled).
	 * @param x X-coordinate.
	 * @param y Y-coordinate.
	 * @param radius Radius of the arc.
	 * @param angle1 The angle at which the arc begins.
	 * @param angle2 The angle at which the arc terminates.
	 * @param points Number of points to use to draw the arc.
	 **/
	void arc(DrawMode mode, float x, float y, float radius, float angle1, float angle2, int points = 10);

	/**
	 * Draws a polygon with an arbitrary number of vertices.
	 * @param type The type of drawing (line/filled).
	 * @param coords Vertex components (x1, y1, x2, y2, etc.)
	 * @param count Coord array size
	 **/
	void polygon(DrawMode mode, const float *coords, size_t count);

	/**
	 * Creates a screenshot of the view and saves it to the default folder.
	 * @param image The love.image module.
	 **/
	love::image::ImageData *newScreenshot(love::image::Image *image);

	void push();
	void pop();
	void rotate(float r);
	void scale(float x, float y = 1.0f);
	void translate(float x, float y);
	void shear(float kx, float ky);

	void drawTest(Image *image, float x, float y, float a, float sx, float sy, float ox, float oy);

	bool hasFocus();
private:

	Font *currentFont;
	love::window::Window *currentWindow;

	LineStyle lineStyle;
	float lineWidth;
	GLint matrixLimit;
	GLint userMatrices;

	int getRenderHeight();
}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
