/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include <common/config.h>
#include <common/math.h>

#include "Graphics.h"

#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace love
{
namespace graphics
{
namespace opengl
{

	Graphics::Graphics()
		: currentFont(0)
	{
		// Indicates that there is no screen
		// created yet.
		currentMode.width = 0;
		currentMode.height = 0;
		currentMode.fullscreen = 0;

		// Window should be centered.
		SDL_putenv(const_cast<char *>("SDL_VIDEO_CENTERED=center"));

		if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
			throw Exception(SDL_GetError());
	}

	Graphics::~Graphics()
	{
		if(currentFont != 0)
			currentFont->release();

		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	const char * Graphics::getName() const
	{
		return "love.graphics.opengl";
	}

	bool Graphics::checkMode(int width, int height, bool fullscreen)
	{
		Uint32 sdlflags = fullscreen ? (SDL_OPENGL | SDL_FULLSCREEN) : SDL_OPENGL;

		// Check if mode is supported
		int bpp = SDL_VideoModeOK(width, height, 32, sdlflags);

		return (bpp >= 16);
	}

	DisplayState Graphics::saveState()
	{
		DisplayState s;
		//create a table in which to store the color data in float format, before converting it
		float color[4];
		//get the color
		glGetFloatv(GL_CURRENT_COLOR, color);
		s.color.r = (GLubyte)(color[0]*255.0f);
		s.color.g = (GLubyte)(color[1]*255.0f);
		s.color.b = (GLubyte)(color[2]*255.0f);
		s.color.a = (GLubyte)(color[3]*255.0f);
		//get the background color
		glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
		s.backgroundColor.r = (GLubyte)(color[0]*255.0f);
		s.backgroundColor.g = (GLubyte)(color[1]*255.0f);
		s.backgroundColor.b = (GLubyte)(color[2]*255.0f);
		s.backgroundColor.a = (GLubyte)(color[3]*255.0f);
		//store modes here
		GLint mode;
		//get blend mode
		glGetIntegerv(GL_BLEND_DST, &mode);
		//following syntax seems better than if-else every time
		s.blendMode = (mode == GL_ONE) ? Graphics::BLEND_ADDITIVE : Graphics::BLEND_ALPHA;
		//get color mode
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);
		s.colorMode = (mode == GL_MODULATE) ? Graphics::COLOR_MODULATE : Graphics::COLOR_REPLACE;
		//get the line width (directly to corresponding variable)
		glGetFloatv(GL_LINE_WIDTH, &s.lineWidth);
		//get line style
		s.lineStyle = (glIsEnabled(GL_LINE_SMOOTH) == GL_TRUE) ? Graphics::LINE_SMOOTH : Graphics::LINE_ROUGH;
		//get line stipple
		s.stipple = (glIsEnabled(GL_LINE_STIPPLE) == GL_TRUE);
		if (s.stipple)
		{
			//get the stipple repeat
			glGetIntegerv(GL_LINE_STIPPLE_REPEAT, &s.stippleRepeat);
			//get the stipple pattern
			glGetIntegerv(GL_LINE_STIPPLE_PATTERN, &s.stipplePattern);
		}
		//get the point size
		glGetFloatv(GL_POINT_SIZE, &s.pointSize);
		//get point style
		s.pointStyle = (glIsEnabled(GL_POINT_SMOOTH) == GL_TRUE) ? Graphics::POINT_SMOOTH : Graphics::POINT_ROUGH;
		//get scissor status
		s.scissor = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
		//do we have scissor, if so, store the box
		if (s.scissor)
			glGetIntegerv(GL_SCISSOR_BOX, s.scissorBox);

		char *cap = 0;
		SDL_WM_GetCaption(&cap, 0);
		s.caption = cap;
		s.mouseVisible = (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) ? true : false;
		return s;
	}

	void Graphics::restoreState(const DisplayState & s)
	{
		setColor(s.color);
		setBackgroundColor(s.backgroundColor);
		setBlendMode(s.blendMode);
		setColorMode(s.colorMode);
		setLine(s.lineWidth, s.lineStyle);
		if (s.stipple)
			setLineStipple(s.stipplePattern, s.stippleRepeat);
		else
			setLineStipple();
		setPoint(s.pointSize, s.pointStyle);
		if (s.scissor)
			setScissor(s.scissorBox[0], s.scissorBox[1], s.scissorBox[2], s.scissorBox[3]);
		else
			setScissor();

		setCaption(s.caption.c_str());
		SDL_ShowCursor(s.mouseVisible ? SDL_ENABLE : SDL_DISABLE);
	}

	bool Graphics::setMode(int width, int height, bool fullscreen, bool vsync, int fsaa)
	{
		// This operation destroys the OpenGL context, so
		// we must save the state.
		DisplayState tempState;
		if (isCreated())
			tempState = saveState();

		// Unlad all volatile objects. These must be reloaded after
		// the display mode change.
		Volatile::unloadAll();

		// Get caption.

		// We need to restart the subsystem for two reasons:
		// 1) Special case for fullscreen -> windowed. Windows XP did not
		//    work well with "normal" display mode change in this case.
		//    The application window does leave fullscreen, but the desktop
		//    resolution does not revert to the correct one. Restarting the
		//    SDL video subsystem does the trick, though.
		// 2) Restart the event system (for whatever reason the event system
		//    started and stopped with SDL_INIT_VIDEO, see:
		//    http://sdl.beuc.net/sdl.wiki/Introduction_to_Events)
		//    because the mouse position will not be able to exceed
		//    the previous' video mode window size (i.e. alway
		//    love.mouse.getX() < 800 when switching from 800x600 to a
		//    higher resolution)
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			std::cout << "Could not init SDL_VIDEO: " << SDL_GetError() << std::endl;
			return false;
		}

		// Set caption.

		// Set GL attributes
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (vsync ? 1 : 0));

		// FSAA
		if(fsaa > 0)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 ) ;
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa ) ;
		}

		// Fullscreen?
		Uint32 sdlflags = fullscreen ? (SDL_OPENGL | SDL_FULLSCREEN) : SDL_OPENGL;

		if(!isCreated())
			setCaption("");

		// Have SDL set the video mode.
		if(SDL_SetVideoMode(width, height, 32, sdlflags ) == 0)
		{
			bool failed = true;
			if(fsaa > 0)
			{
				// FSAA might have failed, disable it and try again
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
				failed = SDL_SetVideoMode(width, height, 32, sdlflags ) == 0;
				if (failed)
				{
					// There might be no FSAA at all
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
					failed = SDL_SetVideoMode(width, height, 32, sdlflags ) == 0;
				}
			}
			if(failed)
			{
				std::cerr << "Could not set video mode: "  << SDL_GetError() << std::endl;
				return false;
			}
		}

		if (width == 0 || height == 0)
		{
			const SDL_VideoInfo* videoinfo = SDL_GetVideoInfo();
			width = videoinfo->current_w;
			height = videoinfo->current_h;
		}

		// Check if FSAA failed or not
		if(fsaa > 0)
		{
			GLint buffers;
			GLint samples;

			glGetIntegerv( GL_SAMPLE_BUFFERS_ARB, & buffers ) ;
			glGetIntegerv( GL_SAMPLES_ARB, & samples ) ;

			// Don't fail because of this, but issue a warning.
			if ( ! buffers || (samples != fsaa))
				std::cerr << "Warning, quality setting failed! (Result: buffers: " << buffers << ", samples: " << samples << ")" << std::endl;
		}

		// Okay, setup OpenGL.

		// Enable blending
		glEnable(GL_BLEND);

		// "Normal" blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable line/point smoothing.
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_POINT_SMOOTH);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

		// Enable textures
		glEnable(GL_TEXTURE_2D);

		// Set the viewport to top-left corner
		glViewport(0,0, width, height);

		// Reset the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Set up orthographic view (no depth)
		glOrtho(0.0, width, height,0.0, -1.0, 1.0);

		// Reset modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Set the new display mode as the current display mode.
		currentMode.width = width;
		currentMode.height = height;
		currentMode.colorDepth = 32;
		currentMode.fsaa = fsaa;
		currentMode.fullscreen = fullscreen;
		currentMode.vsync = vsync;

		// Reload all volatile objects.
		if(!Volatile::loadAll())
			std::cerr << "Could not reload all volatile objects." << std::endl;

		// Restore the display state.
		restoreState(tempState);

		return true;
	}

	bool Graphics::toggleFullscreen()
	{
		// Try to do the change.
		return setMode(currentMode.width,
			currentMode.height,
			!currentMode.fullscreen,
			currentMode.vsync,
			currentMode.fsaa);
	}


	void Graphics::reset()
	{
		DisplayState s;
		restoreState(s);
	}

	void Graphics::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
	}

	void Graphics::present()
	{
		SDL_GL_SwapBuffers();
	}

	void Graphics::setIcon(Image * image)
	{
		Uint32 rmask, gmask, bmask, amask;
#ifdef LOVE_BIG_ENDIAN
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;
#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0xFF000000;
#endif

		int w = static_cast<int>(image->getWidth());
		int h = static_cast<int>(image->getHeight());
		int pitch = static_cast<int>(image->getWidth() * 4);

		SDL_Surface * icon = SDL_CreateRGBSurfaceFrom(image->getData()->getData(), w, h, 32, pitch, rmask, gmask, bmask, amask);
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface(icon);
	}

	void Graphics::setCaption(const char * caption)
	{
		SDL_WM_SetCaption(caption, 0);
	}

	int Graphics::getCaption(lua_State * L)
	{
		char * title = 0;
		SDL_WM_GetCaption(&title, 0);
		lua_pushstring(L, title);
		return 1;
	}

	int Graphics::getWidth()
	{
		return currentMode.width;
	}

	int Graphics::getHeight()
	{
		return currentMode.height;
	}

	bool Graphics::isCreated()
	{
		return (currentMode.width > 0) || (currentMode.height > 0);
	}

	int Graphics::getModes(lua_State * L)
	{
		SDL_Rect ** modes = SDL_ListModes(0, SDL_OPENGL | SDL_FULLSCREEN);

		if(modes == (SDL_Rect **)0 || modes == (SDL_Rect **)-1)
			return 0;

		int index = 1;

		lua_newtable(L);

		for(int i=0;modes[i];++i)
		{
			lua_pushinteger(L, index);
			lua_newtable(L);

			// Inner table attribs.

			lua_pushstring(L, "width");
			lua_pushinteger(L, modes[i]->w);
			lua_settable(L, -3);

			lua_pushstring(L, "height");
			lua_pushinteger(L, modes[i]->h);
			lua_settable(L, -3);

			// Inner table attribs end.

			lua_settable(L, -3);

			index++;
		}

		return 1;
	}

	void Graphics::setScissor(int x, int y, int width, int height)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(x, getHeight() - (y + height), width, height); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
	}

	void Graphics::setScissor()
	{
		glDisable(GL_SCISSOR_TEST);
	}

	int Graphics::getScissor(lua_State * L)
	{
		if(glIsEnabled(GL_SCISSOR_TEST) == GL_FALSE)
			return 0;

		GLint scissor[4];
		glGetIntegerv(GL_SCISSOR_BOX, scissor);

		lua_pushnumber(L, scissor[0]);
		lua_pushnumber(L, getHeight() - (scissor[1] + scissor[3])); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
		lua_pushnumber(L, scissor[2]);
		lua_pushnumber(L, scissor[3]);

		return 4;
	}

	Image * Graphics::newImage(love::image::ImageData * data)
	{
		// Create the image.
		Image * image = new Image(data);
		bool success;
		try {
			success = image->load();
		} catch (love::Exception & e) {
			image->release();
			throw love::Exception(e.what());
		}
		if (!success) {
			image->release();
			return 0;
		}

		return image;
	}

	Quad * Graphics::newQuad(int x, int y, int w, int h, int sw, int sh)
	{
		Quad::Viewport v;
		v.x = x;
		v.y = y;
		v.w = w;
		v.h = h;
		return new Quad(v, sw, sh);
	}

	Font * Graphics::newFont(love::font::FontData * data, const Image::Filter& filter)
	{
		Font * font = new Font(data, filter);

		// Load it and check for errors.
		if(!font)
		{
			delete font;
			return 0;
		}

		return font;
	}

	SpriteBatch * Graphics::newSpriteBatch(Image * image, int size, int usage)
	{
		return new SpriteBatch(image, size, usage);
	}

	ParticleSystem * Graphics::newParticleSystem(Image * image, int size)
	{
		return new ParticleSystem(image, size);
	}

	Framebuffer * Graphics::newFramebuffer(int width, int height)
	{
		return new Framebuffer(width, height);
	}

	void Graphics::setColor(Color c)
	{
		glColor4ubv(&c.r);
	}

	Color Graphics::getColor()
	{
		float c[4];
		glGetFloatv(GL_CURRENT_COLOR, c);

		Color t;
		t.r = (unsigned char)(255.0f*c[0]);
		t.g = (unsigned char)(255.0f*c[1]);
		t.b = (unsigned char)(255.0f*c[2]);
		t.a = (unsigned char)(255.0f*c[3]);

		return t;
	}

	void Graphics::setBackgroundColor(Color c)
	{
		glClearColor((float)c.r/255.0f, (float)c.g/255.0f, (float)c.b/255.0f, 1.0f);
	}

	Color Graphics::getBackgroundColor()
	{
		float c[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, c);

		Color t;
		t.r = (unsigned char)(255.0f*c[0]);
		t.g = (unsigned char)(255.0f*c[1]);
		t.b = (unsigned char)(255.0f*c[2]);
		t.a = (unsigned char)(255.0f*c[3]);

		return t;
	}

	void Graphics::setFont( Font * font )
	{
		if(currentFont != 0)
			currentFont->release();

		currentFont = font;

		if(font != 0)
			currentFont->retain();
	}

	Font * Graphics::getFont()
	{
		return currentFont;
	}

	void Graphics::setBlendMode( Graphics::BlendMode mode )
	{
		glAlphaFunc(GL_GEQUAL, 0);

		if (mode == BLEND_SUBTRACTIVE)
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		else
			glBlendEquation(GL_FUNC_ADD);

		if (mode == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (mode == BLEND_MULTIPLICATIVE)
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	void Graphics::setColorMode ( Graphics::ColorMode mode )
	{
		if(mode == COLOR_MODULATE)
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		else // mode = COLOR_REPLACE
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	Graphics::BlendMode Graphics::getBlendMode()
	{
		GLint dst, src;
		glGetIntegerv(GL_BLEND_DST, &dst);
		glGetIntegerv(GL_BLEND_SRC, &src);

		if(src == GL_SRC_ALPHA && dst == GL_ONE)
			return BLEND_ADDITIVE;
		else // src == GL_SRC_ALPHA && dst == GL_ONE_MINUS_SRC_ALPHA
			return BLEND_ALPHA;
	}

	Graphics::ColorMode Graphics::getColorMode()
	{
		GLint mode;
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);

		if(mode == GL_MODULATE)
			return COLOR_MODULATE;
		else // // mode == GL_REPLACE
			return COLOR_REPLACE;
	}

	void Graphics::setLineWidth( float width )
	{
		glLineWidth(width);
	}

	void Graphics::setLineStyle(Graphics::LineStyle style )
	{
		if(style == LINE_ROUGH)
			glDisable (GL_LINE_SMOOTH);
		else // type == LINE_SMOOTH
		{
			glEnable (GL_LINE_SMOOTH);
			glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		}
	}

	void Graphics::setLine( float width, Graphics::LineStyle style )
	{
		glLineWidth(width);

		if(style == 0)
			return;

		if(style == LINE_ROUGH)
			glDisable (GL_LINE_SMOOTH);
		else // type == LINE_SMOOTH
		{
			glEnable (GL_LINE_SMOOTH);
			glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		}
	}

	void Graphics::setLineStipple()
	{
		glDisable(GL_LINE_STIPPLE);
	}

	void Graphics::setLineStipple(unsigned short pattern, int repeat)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple((GLint)repeat, (GLshort)pattern);
	}

	float Graphics::getLineWidth()
	{
		float w;
		glGetFloatv(GL_LINE_WIDTH, &w);
		return w;
	}

	Graphics::LineStyle Graphics::getLineStyle()
	{
		if(glIsEnabled(GL_LINE_SMOOTH) == GL_TRUE)
			return LINE_SMOOTH;
		else
			return LINE_ROUGH;
	}

	int Graphics::getLineStipple(lua_State * L)
	{
		if(glIsEnabled(GL_LINE_STIPPLE) == GL_FALSE)
			return 0;

		GLint factor, pattern;
		glGetIntegerv(GL_LINE_STIPPLE_PATTERN, &pattern);
		glGetIntegerv(GL_LINE_STIPPLE_REPEAT, &factor);
		lua_pushinteger(L, pattern);
		lua_pushinteger(L, factor);
		return 2;
	}

	void Graphics::setPointSize( float size )
	{
		glPointSize((GLfloat)size);
	}

	void Graphics::setPointStyle( Graphics::PointStyle style )
	{
		if( style == POINT_SMOOTH )
			glEnable(GL_POINT_SMOOTH);
		else // love::POINT_ROUGH
			glDisable(GL_POINT_SMOOTH);
	}

	void Graphics::setPoint( float size, Graphics::PointStyle style )
	{
		if( style == POINT_SMOOTH )
			glEnable(GL_POINT_SMOOTH);
		else // POINT_ROUGH
			glDisable(GL_POINT_SMOOTH);

		glPointSize((GLfloat)size);
	}

	float Graphics::getPointSize()
	{
		GLfloat size;
		glGetFloatv(GL_POINT_SIZE, &size);
		return (float)size;
	}

	Graphics::PointStyle Graphics::getPointStyle()
	{
		if(glIsEnabled(GL_POINT_SMOOTH) == GL_TRUE)
			return POINT_SMOOTH;
		else
			return POINT_ROUGH;
	}

	int Graphics::getMaxPointSize()
	{
		GLint max;
		glGetIntegerv(GL_POINT_SIZE_MAX, &max);
		return (int)max;
	}

	void Graphics::print( const char * str, float x, float y , float angle, float sx, float sy)
	{
		if(currentFont != 0)
		{
			std::string text(str);
			currentFont->print(text, x, y, angle, sx, sy);
		}
	}

	void Graphics::printf( const char * str, float x, float y, float wrap, AlignMode align)
	{
		if (currentFont == 0)
			return;

		using namespace std;
		string text(str);
		const float width_space = currentFont->getWidth(' ');
		vector<string> lines_to_draw;

		//split text at newlines
		istringstream iss( text );
		string line;
		while (getline(iss, line, '\n')) {
			// split line into words
			vector<string> words;
			istringstream word_iss(line);
			copy(istream_iterator<string>(word_iss), istream_iterator<string>(),
					back_inserter< vector<string> >(words));

			// put words back together until a wrap occurs
			float width = 0.0f;
			ostringstream string_builder;
			vector<string>::const_iterator word_iter;
			for (word_iter = words.begin(); word_iter != words.end(); ++word_iter) {
				string word( *word_iter );
				width += currentFont->getWidth( word );

				// on wordwrap, push line to line buffer and clear string builder
				if (width >= wrap) {
					lines_to_draw.push_back( string_builder.str() );
					string_builder.str( "" );
					width = currentFont->getWidth( word );
				}
				string_builder << word << " ";
				width += width_space;
			}
			// push last line
			lines_to_draw.push_back( string_builder.str() );
		}

		// now for the actual printing
		vector<string>::const_iterator line_iter, line_end = lines_to_draw.end();
		for (line_iter = lines_to_draw.begin(); line_iter != line_end; ++line_iter) {
			float width = currentFont->getWidth( *line_iter );
			switch (align) {
				case ALIGN_RIGHT:
					currentFont->print(*line_iter, ceil(x + wrap - width), ceil(y));
					break;
				case ALIGN_CENTER:
					currentFont->print(*line_iter, ceil(x + (wrap - width) / 2), ceil(y));
					break;
				case ALIGN_LEFT:
				default:
					currentFont->print(*line_iter, ceil(x), ceil(y));
					break;
			}
			y += currentFont->getHeight() * currentFont->getLineHeight();
		}
	}

	/**
	* Primitives
	**/

	void Graphics::point( float x, float y )
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_POINTS);
			glVertex2f(x, y);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}

	void Graphics::line( float x1, float y1, float x2, float y2 )
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
			glBegin(GL_LINES);
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
			glEnd();
		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
	}

	int Graphics::polyline( lua_State * L)
	{
		// Get number of params.
		int args = lua_gettop(L);
		bool table = false;

		if (args == 1) { // we've got a table, hopefully
			int type = lua_type(L, 1);
			if (type != LUA_TTABLE)
				return luaL_error(L, "Function requires a table or series of numbers");
			table = true;
			args = lua_objlen(L, 1);
		}

		if (args % 2) // an odd number of arguments, no good for a polyline
			return luaL_error(L, "Number of vertices must be a multiple of two");
		else if (args < 4)
			return luaL_error(L, "Need at least two vertices to draw a line");

		// right, let's draw this polyline, then
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_STRIP);
		if (table) {
			for (int i = 1; i < args; i += 2) {
				lua_pushnumber(L, i);   // x coordinate
				lua_rawget(L, 1);
				lua_pushnumber(L, i+1); // y coordinate
				lua_rawget(L, 1);
				glVertex2f((GLfloat)lua_tonumber(L, -2), (GLfloat)lua_tonumber(L, -1));
				lua_pop(L, 2);
			}
		} else {
			for (int i = 1; i < args; i+=2) {
				glVertex2f((GLfloat)lua_tonumber(L, i), (GLfloat)lua_tonumber(L, i+1));
			}
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
		return 0;
	}

	void Graphics::triangle(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3 )
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();

		switch(mode)
		{
		case DRAW_LINE:
			glBegin(GL_LINE_LOOP);
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
				glVertex2f(x3, y3);
			glEnd();
			break;

		default:
		case DRAW_FILL:
			glBegin(GL_TRIANGLES);
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
				glVertex2f(x3, y3);
			glEnd();
			break;
		}

		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
	}

	void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h)
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();

		switch(mode)
		{
		case DRAW_LINE:
			// offsets here because OpenGL is being a bitch about line drawings
			glBegin(GL_LINE_LOOP);
				glVertex2f(x, y);
				glVertex2f(x, y+h-1);
				glVertex2f(x+w-1, y+h-1);
				glVertex2f(x+w-1, y);
			glEnd();
			break;

		default:
		case DRAW_FILL:
			glBegin(GL_QUADS);
				glVertex2f(x, y);
				glVertex2f(x, y+h);
				glVertex2f(x+w, y+h);
				glVertex2f(x+w, y);
			glEnd();
			break;
		}

		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
	}

	void Graphics::quad(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 )
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();

		switch(mode)
		{
		case DRAW_LINE:
			glBegin(GL_LINE_LOOP);
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
				glVertex2f(x3, y3);
				glVertex2f(x4, y4);
			glEnd();
			break;

		default:
		case DRAW_FILL:
			glBegin(GL_QUADS);
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
				glVertex2f(x3, y3);
				glVertex2f(x4, y4);
			glEnd();
			break;
		}

		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
	}

	void Graphics::circle(DrawMode mode, float x, float y, float radius, int points )
	{
		float two_pi = static_cast<float>(LOVE_M_PI * 2);
		if(points <= 0) points = 1;
		float angle_shift = (two_pi / points);

		glDisable(GL_TEXTURE_2D);
		glPushMatrix();

		glTranslatef(x, y, 0.0f);

		switch(mode)
		{
		case DRAW_LINE:
			glBegin(GL_LINE_LOOP);

			for(float i = 0; i < two_pi; i+= angle_shift)
				glVertex2f(radius * sin(i),radius * cos(i));

			glEnd();
			break;

		default:
		case DRAW_FILL:
			glBegin(GL_TRIANGLE_FAN);

			for(float i = 0; i < two_pi; i+= angle_shift)
				glVertex2f(radius * sin(i),radius * cos(i));

			glEnd();
			break;
		}

		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
	}

	int Graphics::polygon( lua_State * L )
	{
		// Get number of params.
		int n = lua_gettop(L);

		// Need at least two params.
		if( n < 2 )
			return luaL_error(L, "Error: function needs at least two parameters.");

		DrawMode mode;

		const char * str = luaL_checkstring(L, 1);
		if(!getConstant(str, mode))
			return luaL_error(L, "Invalid draw mode: %s", str);

		// Get the type of the second argument.
		int luatype = lua_type(L, 2);

		// Perform additional type checking.
		switch(luatype)
		{
		case LUA_TNUMBER:
			if( n-1 < 6 ) return luaL_error(L, "Error: function requires at least 3 vertices.");
			if( ((n-1)%2) != 0 ) return luaL_error(L, "Error: number of vertices must be a multiple of two.");
			break;
		case LUA_TTABLE:
			if( (lua_objlen(L, 2)%2) != 0 ) return luaL_error(L, "Error: number of vertices must be a multiple of two.");
			break;
		default:
			return luaL_error(L, "Error: number type or table expected.");
		}


		glDisable(GL_TEXTURE_2D);

		glBegin((mode==DRAW_LINE) ? GL_LINE_LOOP : GL_POLYGON);

		switch(luatype)
		{
		case LUA_TNUMBER:
			for(int i = 2; i<n; i+=2)
				glVertex2f((GLfloat)lua_tonumber(L, i), (GLfloat)lua_tonumber(L, i+1));
			break;
		case LUA_TTABLE:
			lua_pushnil(L);
			while (true)
			{
				if(lua_next(L, 2) == 0) break;
				GLfloat x = (GLfloat)lua_tonumber(L, -1);
				lua_pop(L, 1); // pop value
				if(lua_next(L, 2) == 0) break;
				GLfloat y = (GLfloat)lua_tonumber(L, -1);
				lua_pop(L, 1); // pop value
				glVertex2f(x, y);
			}
			break;
		}

		glEnd();

		glEnable(GL_TEXTURE_2D);

		return 0;
	}

	love::image::ImageData * Graphics::newScreenshot(love::image::Image * image)
	{
		int w = getWidth();
		int h = getHeight();

		int row = 4*w;

		int size = row*h;

		GLubyte * pixels = new GLubyte[size];
		GLubyte * screenshot = new GLubyte[size];

		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// OpenGL sucks and reads pixels from the lower-left. Let's fix that.

		GLubyte *src = pixels - row, *dst = screenshot + size;

		for (int i = 0; i < h; ++i) {
			memcpy(dst-=row, src+=row, row);
		}

		love::image::ImageData * img = image->newImageData(w, h, (void*)screenshot);

		delete [] pixels;
		delete [] screenshot;

		return img;
	}

	void Graphics::push()
	{
		glPushMatrix();
	}

	void Graphics::pop()
	{
		glPopMatrix();
	}

	void Graphics::rotate(float r)
	{
		glRotatef(LOVE_TODEG(r), 0, 0, 1);
	}

	void Graphics::scale(float x, float y)
	{
		glScalef(x, y, 1);
	}

	void Graphics::translate(float x, float y)
	{
		glTranslatef(x, y, 0);
	}

	void Graphics::drawTest(Image * image, float x, float y, float a, float sx, float sy, float ox, float oy)
	{
		image->bind();

		// Buffer for transforming the image.
		vertex buf[4];

		Matrix t;
		t.translate(x, y);
		t.rotate(a);
		t.scale(sx, sy);
		t.translate(ox, oy);
		t.transform(buf, image->getVertices(), 4);

		const vertex * vertices = image->getVertices();

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&buf[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].s);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	bool Graphics::hasFocus()
	{
		return (SDL_GetAppState() & SDL_APPINPUTFOCUS) != 0;
	}
} // opengl
} // graphics
} // love
