/**
* Copyright (c) 2006-2009 LOVE Development Team
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

// SDL
#include <SDL.h>
#include "GLee.h"
#include <SDL_opengl.h>

// LOVE
#include <graphics/Graphics.h>

#include "Image.h"
#include "TrueTypeFont.h"
#include "ImageFont.h"
#include "Frame.h"
#include "SpriteBatch.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	struct Color
	{
		unsigned char r, g, b, a;
	};

	struct DisplayMode
	{
		int width, height; // The size of the screen.
		int colorDepth; // The color depth of the display mode.
		bool fullscreen; // Fullscreen (true), or windowed (false).
		bool vsync; // Vsync enabled (true), or disabled (false).
		int fsaa; // 0 for no FSAA, otherwise 1, 2 or 4.
	};

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
		float lineWidth;
		Graphics::LineStyle lineStyle;
		bool stipple;
		int stippleRepeat;
		int stipplePattern;

		// Point.
		float pointSize;
		Graphics::PointStyle pointStyle;

		// Scissor.
		bool scissor;
		GLint scissorBox[4];

		// Default values.
		DisplayState()
		{
			color.r = 255; 
			color.g = 255; 
			color.b = 255; 
			color.a = 255;
			backgroundColor.r = 0; 
			backgroundColor.g = 0; 
			backgroundColor.b = 0; 
			backgroundColor.a = 255;
			blendMode = Graphics::BLEND_ALPHA;
			colorMode = Graphics::COLOR_MODULATE;
			lineWidth = 1.0f;
			lineStyle = Graphics::LINE_SMOOTH;
			stipple = false;
			pointSize = 1.0f;
			pointStyle = Graphics::POINT_SMOOTH;
			scissor = false;
		}

	};

	class Graphics : public love::graphics::Graphics
	{
	private:

		Font * currentFont;
		DisplayMode currentMode;

	public:

		Graphics();
		virtual ~Graphics();
		
		// Implements Module.
		const char * getName() const;

		/**
		* Checks whether a display mode is supported or not. Note
		* that fullscreen is assumed, because windowed modes are
		* generally supported regardless of size.
		* @param width The window width.
		* @param height The window height.
		**/
		bool checkMode(int width, int height, bool fullscreen);

		DisplayState saveState();

		void restoreState(const DisplayState & s);

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
		* Sets the windows caption.
		**/
		void setCaption(const char * caption);

		int getCaption(lua_State * L);

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
		int getModes(lua_State * L);

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
		int getScissor(lua_State * L);

		/**
		* Creates an Image object with padding and/or optimization.
		**/
		Image * newImage(love::filesystem::File * file);
		Image * newImage(love::image::ImageData * data);
		
		/**
		* Creates a Frame
		**/
		Frame * newFrame(int x, int y, int w, int h, int sw, int sh);

		/**
		* Creates a Font object.
		**/
		Font * newFont(Data * data, int size);

		/**
		* Creates an ImageFont object.
		**/
		Font * newImageFont(Image * image, const char * glyphs, float spacing = 1);
		
		SpriteBatch * newSpriteBatch(Image * image, int size, int usage);
			
		/**
		* Sets the foreground color.
		**/
		void setColor(Color c);

		/**
		* Gets current color.
		**/
		Color getColor();

		/**
		* Sets the background Color. 
		**/
		void setBackgroundColor(Color c);

		/**
		* Gets the current background color.
		* @param c Array of size 3 (r,g,b).
		**/
		Color getBackgroundColor();

		/**
		* Sets the current font.
		* @parm font A Font object.
		**/
		void setFont( Font * font );

		/**
		* Sets a default font. The font is
		* loaded and sent to the GPU every time this is called, 
		* so no over-using.
		* @param data Data 
		* @param size The size of the font.
		**/
		void setFont( Data * data, int size = 12);

		/**
		* Gets the current Font, or nil if none.
		**/
		Font * getFont();

		/**
		* Sets the current blend mode.
		**/
		void setBlendMode( int mode );

		/**
		* Sets the current color mode.
		**/
		void setColorMode ( int mode );

		/**
		* Gets the current blend mode.
		**/
		int getBlendMode();

		/**
		* Gets the current color mode.
		**/
		int getColorMode();

		/**
		* Sets the line width.
		* @param width The new width of the line.
		**/
		void setLineWidth( float width );

		/**
		* Sets the line style.
		* @param style LINE_ROUGH or LINE_SMOOTH.
		**/
		void setLineStyle( int style );

		/**
		* Sets the type of line used to draw primitives.
		* A shorthand for setLineWidth and setLineStyle.
		**/
		void setLine( float width, int style = 0 );

		/**
		* Disables line stippling.
		**/
		void setLineStipple();

		/**
		* Sets a line stipple pattern.
		**/
		void setLineStipple(unsigned short pattern, int repeat = 1);

		/**
		* Gets the line width.
		**/
		float getLineWidth();

		/**
		* Gets the line style.
		**/
		int getLineStyle();

		/**
		* Gets the line stipple pattern and repeat factor.
		* @return pattern The stipplie bit-pattern.
		* @return repeat The reapeat factor.
		**/
		int getLineStipple(lua_State * L);

		/**
		* Sets the size of points.
		**/
		void setPointSize( float size );

		/**
		* Sets the style of points.
		* @param style POINT_SMOOTH or POINT_ROUGH.
		**/
		void setPointStyle( int style );

		/**
		* Shorthand for setPointSize and setPointStyle.
		**/
		void setPoint( float size, int style );

		/**
		* Gets the point size.
		**/
		float getPointSize();

		/**
		* Gets the point style.
		**/
		int getPointStyle();

		/**
		* Gets the maximum point size supported. 
		* This may vary from computer to computer.
		**/
		int getMaxPointSize();

		/**
		* Draw text on screen at the specified coordiantes (automatically breaks \n characters).
		*
		* @param str A string of text.
		* @param x The x-coordiante.
		* @param y The y-coordiante.
		**/
		void print( const char * str, float x, float y );

		/**
		* Draws text at the specified coordinates, with rotation.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param angle The amount of rotation.
		**/
		void print( const char * str, float x, float y , float angle );

		/**
		* Draws text at the specified coordinates, with rotation and 
		* scaling.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param angle The amount of rotation.
		* @param s The scale factor. (1 = normal).
		**/
		void print( const char * str, float x, float y , float angle, float s );

		/**
		* Draws text at the specified coordinates, with rotation and 
		* scaling along both axes.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param angle The amount of rotation.
		* @param sx The scale factor along the x-axis. (1 = normal).
		* @param sy The scale factor along the y-axis. (1 = normal).
		**/
		void print( const char * str, float x, float y , float angle, float sx, float sy);

		/**
		* Draw formatted text on screen at the specified coordinates.
		*
		* @param str A string of text.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param wrap The maximum width of the text area.
		* @param align Where to align the text.
		**/
		void printf( const char * str, float x, float y, float wrap, int align = 0 );

		/**
		* Draws a point at (x,y).
		* @param x Point along x-axis.
		* @param y Point along y-axis.
		**/
		void point( float x, float y );

		/**
		* Draws a line from (x1,y1) to (x2,y2).
		* @param x1 First x-coordinate.
		* @param y1 First y-coordinate.
		* @param x2 Second x-coordinate.
		* @param y2 Second y-coordinate.
		**/
		void line( float x1, float y1, float x2, float y2 );

		/**
		* Draws a triangle using the three coordinates passed.
		* @param type The type of drawing (line/filled).
		* @param x1 First x-coordinate.
		* @param y1 First y-coordinate.
		* @param x2 Second x-coordinate.
		* @param y2 Second y-coordinate.
		* @param x3 Third x-coordinate.
		* @param y3 Third y-coordinate.
		**/
		void triangle( int type, float x1, float y1, float x2, float y2, float x3, float y3 );

		/**
		* Draws a rectangle.
		* @param x Position along x-axis for top-left corner.
		* @param y Position along y-axis for top-left corner.
		* @param w The width of the rectangle.
		* @param h The height of the rectangle.
		**/
		void rectangle( int type, float x, float y, float w, float h );

		/**
		* Draws a quadrilateral using the four coordinates passed.
		* @param type The type of drawing (line/filled).
		* @param x1 First x-coordinate.
		* @param y1 First y-coordinate.
		* @param x2 Second x-coordinate.
		* @param y2 Second y-coordinate.
		* @param x3 Third x-coordinate.
		* @param y3 Third y-coordinate.
		* @param x4 Fourth x-coordinate.
		* @param y4 Fourth y-coordinate.
		**/
		void quad( int type, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 );

		/**
		* Draws a circle using the specified arguments.
		* @param type The type of drawing (line/filled).
		* @param x X-coordinate.
		* @param y Y-coordinate.
		* @param radius Radius of the circle.
		* @param points Amount of points to use to draw the circle.
		**/
		void circle( int type, float x, float y, float radius, int points = 10 );

		/**
		* Draws a polygon with an arbitrary number of vertices.
		* @param type The type of drawing (line/filled).
		* @param ... Vertex components (x1, y1, x2, y2, etc).
		**/
		int polygon( lua_State * L );
		int polygong( lua_State * L );

		/**
		* Creates a screenshot of the view and saves it to the default folder.
		* @param file The file to write the screenshot to.
		**/
		bool screenshot(love::filesystem::File * file);

		void push();
		void pop();
		void rotate(float r);
		void scale(float x, float y = 1.0f);
		void translate(float x, float y);

		void drawTest(Image * image, float x, float y, float a, float sx, float sy, float ox, float oy);

	}; // Graphics

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GRAPHICS_H
