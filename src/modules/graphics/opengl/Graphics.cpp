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
#include <common/Vector.h>

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
		: currentFont(0), lineWidth(1), matrixLimit(0), userMatrices(0)
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

		s.color = getColor();
		s.backgroundColor = getBackgroundColor();

		//store modes here
		GLint mode;
		//get blend mode
		glGetIntegerv(GL_BLEND_DST, &mode);
		//following syntax seems better than if-else every time
		s.blendMode = (mode == GL_ONE) ? Graphics::BLEND_ADDITIVE : Graphics::BLEND_ALPHA;
		//get color mode
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);
		s.colorMode = (mode == GL_MODULATE) ? Graphics::COLOR_MODULATE : Graphics::COLOR_REPLACE;
		//get line style
		//s.lineStyle = (glIsEnabled(GL_POLYGON_SMOOTH) == GL_TRUE) ? Graphics::LINE_SMOOTH : Graphics::LINE_ROUGH;
		s.lineStyle = Graphics::LINE_ROUGH;
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
		setLine(lineWidth, s.lineStyle);
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
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

		// FSAA
		if(fsaa > 0)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 ) ;
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa ) ;
			glEnable(GL_MULTISAMPLE);
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

		GLint buffers;
		GLint samples;

		glGetIntegerv( GL_SAMPLE_BUFFERS_ARB, & buffers ) ;
		glGetIntegerv( GL_SAMPLES_ARB, & samples ) ;

		// Don't fail because of this, but issue a warning.
		if ( (! buffers && fsaa) || (samples != fsaa))
		{
			std::cerr << "Warning, quality setting failed! (Result: buffers: " << buffers << ", samples: " << samples << ")" << std::endl;
			fsaa = !buffers ? 0 : samples;
		}

		// Okay, setup OpenGL.

		// Enable blending
		glEnable(GL_BLEND);

		// "Normal" blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable line/point smoothing.
		setLineStyle(LINE_SMOOTH);
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
		
		// Set pixel row alignment
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

		// Get the actual vsync status
		int real_vsync;
		SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &real_vsync);

		// Set the new display mode as the current display mode.
		currentMode.width = width;
		currentMode.height = height;
		currentMode.colorDepth = 32;
		currentMode.fsaa = fsaa;
		currentMode.fullscreen = fullscreen;
		currentMode.vsync = (real_vsync != 0);

		// Reload all volatile objects.
		if(!Volatile::loadAll())
			std::cerr << "Could not reload all volatile objects." << std::endl;

		// Restore the display state.
		restoreState(tempState);
		
		// Get the maximum number of matrices
		// subtract a few to give the engine some room.
		glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &matrixLimit);
		matrixLimit -= 5;

		return true;
	}

	void Graphics::getMode(int *width, int *height, bool *fullscreen, bool *vsync, int *fsaa)
	{
		*width = currentMode.width;
		*height = currentMode.height;
		*fullscreen = currentMode.fullscreen;
		*vsync = currentMode.vsync;
		*fsaa = currentMode.fsaa;
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
		discardStencil();
		Canvas::bindDefaultCanvas();
		restoreState(s);
	}

	void Graphics::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		PixelEffect::detach();
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

	int Graphics::getRenderHeight()
	{
		if (Canvas::current)
			return Canvas::current->getHeight();
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
		glScissor(x, getRenderHeight() - (y + height), width, height); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
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
		lua_pushnumber(L, getRenderHeight() - (scissor[1] + scissor[3])); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
		lua_pushnumber(L, scissor[2]);
		lua_pushnumber(L, scissor[3]);

		return 4;
	}

	void Graphics::defineStencil()
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}

	void Graphics::useStencil(bool invert)
	{
		glStencilFunc(GL_EQUAL, (int)(!invert), 1); // invert ? 0 : 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	void Graphics::discardStencil()
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDisable(GL_STENCIL_TEST);
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

	Quad * Graphics::newQuad(float x, float y, float w, float h, float sw, float sh)
	{
		Quad::Viewport v;
		v.x = x;
		v.y = y;
		v.w = w;
		v.h = h;
		return new Quad(v, sw, sh);
	}

	Font * Graphics::newFont(love::font::Rasterizer * r, const Image::Filter& filter)
	{
		Font * font = new Font(r, filter);

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
		SpriteBatch * t = NULL;
		try {
			t = new SpriteBatch(image, size, usage);
		} catch (love::Exception& e) {
			if (t) delete t;
			throw e;
		}
		return t;
	}

	ParticleSystem * Graphics::newParticleSystem(Image * image, int size)
	{
		return new ParticleSystem(image, size);
	}

	Canvas * Graphics::newCanvas(int width, int height)
	{
		Canvas * canvas = new Canvas(width, height);
		GLenum err = canvas->getStatus();

		// everything ok, reaturn canvas (early out)
		if (err == GL_FRAMEBUFFER_COMPLETE)
			return canvas;

		// create error message
		std::stringstream error_string;
		error_string << "Cannot create canvas: ";
		switch (err) {

			case GL_FRAMEBUFFER_UNSUPPORTED:
				error_string << "Not supported by your OpenGL implementation.";
				break;

			// remaining error codes are highly unlikely:
			case GL_FRAMEBUFFER_UNDEFINED:
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				error_string << "Error in implementation. Possible fix: Make canvas width and height powers of two.";
				break;

			default:
				// my intel hda card wrongly returns 0 to glCheckFramebufferStatus() but sets
				// no error flag. I think it meant to return GL_FRAMEBUFFER_UNSUPPORTED, but who
				// knows.
				if (glGetError() == GL_NO_ERROR)
					error_string << "May not be supported by your OpenGL implementation.";
				// the remaining error is an indication of a serious fuckup since it should
				// only be returned if glCheckFramebufferStatus() was called with the wrong
				// arguments.
				else
					error_string << "Cannot create canvas: Aliens did it (OpenGL error code: " << glGetError() << ")";
		}

		canvas->release();
		throw Exception(error_string.str().c_str());
		return NULL; // never reached
	}

	PixelEffect * Graphics::newPixelEffect(const std::string& code)
	{
		PixelEffect * effect = NULL;
		try {
			effect = new PixelEffect(code);
		} catch (love::Exception& e) {
			if (effect)
				delete effect;
			throw(e);
		}
		return effect;
	}

	void Graphics::setColor(const Color& c)
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

	void Graphics::setBackgroundColor(const Color& c)
	{
		glClearColor((float)c.r/255.0f, (float)c.g/255.0f, (float)c.b/255.0f, (float)c.a/255.0f);
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
		else // mode == BLEND_ADDITIVE || mode == BLEND_SUBTRACTIVE
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	void Graphics::setColorMode ( Graphics::ColorMode mode )
	{
		if(mode == COLOR_MODULATE)
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		else // mode = COLOR_REPLACE
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	Graphics::BlendMode Graphics::getBlendMode ()
	{
		GLint dst, src, equation;
		glGetIntegerv(GL_BLEND_DST, &dst);
		glGetIntegerv(GL_BLEND_SRC, &src);
		glGetIntegerv(GL_BLEND_EQUATION, &equation);

		if (equation == GL_FUNC_REVERSE_SUBTRACT) // && src == GL_SRC_ALPHA && dst == GL_ONE
			return BLEND_SUBTRACTIVE;
		else if(src == GL_SRC_ALPHA && dst == GL_ONE) // && equation == GL_FUNC_ADD
			return BLEND_ADDITIVE;
		else if (src == GL_SRC_ALPHA && dst == GL_ONE_MINUS_SRC_ALPHA) // && equation == GL_FUNC_ADD
			return BLEND_ALPHA;
		else // src == GL_DST_COLOR && dst == GL_ONE_MINUS_SRC_ALPHA && equation == GL_FUNC_ADD
			return BLEND_MULTIPLICATIVE;
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
		lineWidth = width;
	}

	void Graphics::setLineStyle(Graphics::LineStyle style )
	{
		//// XXX: actually enables antialiasing for _all_ polygons.
		//// may need investigation if wanted or not
		//// maybe rename to something else?
		//if(style == LINE_ROUGH)
		//	glDisable (GL_POLYGON_SMOOTH);
		//else // type == LINE_SMOOTH
		//{
		//	glEnable (GL_POLYGON_SMOOTH);
		//	glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		//}
	}

	void Graphics::setLine( float width, Graphics::LineStyle style )
	{
		setLineWidth(width);

		if(style == 0)
			return;
		setLineStyle(style);
	}

	float Graphics::getLineWidth()
	{
		float w;
		glGetFloatv(GL_LINE_WIDTH, &w);
		return w;
	}

	Graphics::LineStyle Graphics::getLineStyle()
	{
		//if(glIsEnabled(GL_POLYGON_SMOOTH) == GL_TRUE)
		//	return LINE_SMOOTH;
		//else
			return LINE_ROUGH;
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

	void Graphics::print( const char * str, float x, float y , float angle, float sx, float sy, float ox, float oy, float kx, float ky)
	{
		if(currentFont != 0)
		{
			std::string text(str);
			currentFont->print(text, x, y, angle, sx, sy, ox, oy, kx, ky);
		}
	}

	void Graphics::printf( const char * str, float x, float y, float wrap, AlignMode align)
	{
		if (currentFont == 0)
			return;

		using namespace std;
		string text(str);
		vector<string> lines_to_draw = currentFont->getWrap(text, wrap);

		// now for the actual printing
		vector<string>::const_iterator line_iter, line_end = lines_to_draw.end();
		for (line_iter = lines_to_draw.begin(); line_iter != line_end; ++line_iter) {
			float width = static_cast<float>(currentFont->getWidth( *line_iter ));
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

	// calculate line boundary intersection vertices for current line
	// dependent on the current *and next* line segment
	static void pushIntersectionPoints(Vector *vertices, int pos, float halfwidth, const Vector& p, const Vector& q, const Vector& r)
	{
		// calculate line directions
		Vector s = (q - p);
		Vector t = (r - q);

		// calculate vertex displacement vectors
		Vector d1 = s.getNormal();
		Vector d2 = t.getNormal();
		d1.normalize();
		d2.normalize();
		float det_norm = d1 ^ d2;
		d1 *= halfwidth;
		d2 *= halfwidth;

		// lines parallel -> assume intersection at displacement points
		if (fabs(det_norm) <= .03) {
			vertices[pos]     = q - d2;
			vertices[pos + 1] = q + d2;
			return;
		}

		// real intersection -> calculate boundary intersection points
		float det = s ^ t;
		Vector d = d1 - d2;
		Vector b = s - d; // s = q - p
		Vector c = s + d;
		float lambda = (b ^ t) / det;
		float mu     = (c ^ t) / det;

		// ordering for GL_TRIANGLE_STRIP
		vertices[pos]   = p - d1 + s * mu;
		vertices[pos+1] = p + d1 + s * lambda;
	}

	void Graphics::polyline(const float* coords, size_t count, bool looping)
	{
		Vector *vertices = new Vector[count]; // two vertices for every line end-point
		Vector p,q,r;

		r = Vector(coords[0], coords[1]);
		if (looping) q = Vector(coords[count-4], coords[count-3]);
		else         q = r * 2 - Vector(coords[2], coords[3]);

		for (size_t i = 0; i+3 < count; i += 2) {
			p = q;
			q = r;
			r = Vector(coords[i+2], coords[i+3]);
			pushIntersectionPoints(vertices, i, lineWidth/2, p,q,r);
		}

		p = q;
		q = r;
		if (looping) r = Vector(coords[2], coords[3]);
		else         r += (q-p);
		pushIntersectionPoints(vertices, count-2, lineWidth/2, p,q,r);

		glDisable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (const GLvoid*)vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
		glDisableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);

		delete[] vertices;
	}

	void Graphics::triangle(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3 )
	{
		float coords[] = { x1,y1, x2,y2, x3,y3, x1,y1 };
		polygon(mode, coords, 4 * 2);
	}

	void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h)
	{
		quad(mode, x,y, x,y+h, x+w,y+h, x+w,y);
	}

	void Graphics::quad(DrawMode mode, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 )
	{
		float coords[] = { x1,y1, x2,y2, x3,y3, x4,y4, x1,y1 };
		polygon(mode, coords, 5 * 2);
	}

	void Graphics::circle(DrawMode mode, float x, float y, float radius, int points)
	{
		float two_pi = static_cast<float>(LOVE_M_PI * 2);
		if(points <= 0) points = 1;
		float angle_shift = (two_pi / points);
		float phi = .0f;

		float *coords = new float[2 * (points + 1)];
		for (int i = 0; i < points; ++i, phi += angle_shift) {
			coords[2*i]   = x + radius * cos(phi);
			coords[2*i+1] = y + radius * sin(phi);
		}

		coords[2*points]   = coords[0];
		coords[2*points+1] = coords[1];

		polygon(mode, coords, (points + 1) * 2);

		delete[] coords;
	}
	
	void Graphics::arc(DrawMode mode, float x, float y, float radius, float angle1, float angle2, int points)
	{
		angle1 = fmod(angle1, 2.0f * (float)LOVE_M_PI);
		angle2 = fmod(angle2, 2.0f * (float)LOVE_M_PI);
		if (angle1 == angle2)
			return;
		else if (angle1 > angle2)
			angle2 += (float)LOVE_M_PI * 2.0f;


		if(points <= 0) points = 1;
		float angle_shift = ((angle2 - angle1) / points);
		float phi = angle1;

		// GL_POLYGON can only fill-draw convex polygons, so we need to do stuff manually here
		if (mode == DRAW_LINE) {
			float *coords = new float[(points + 3) * 2];
			coords[0] = coords[2 * points + 4] = x;
			coords[1] = coords[2 * points + 5] = y;
			for (int i = 0; i <= points; ++i, phi += angle_shift) {
				coords[2 * (i+1)]     = x + radius * cos(phi);
				coords[2 * (i+1) + 1] = y - radius * sin(phi);
			}
			polyline(coords, (points + 3) * 2); // artifacts at sharp angles if set to looping

			delete[] coords;
		} else {
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_TRIANGLE_FAN);
			glVertex2f(x, y);
			for (int i = 0; i <= points; ++i, phi += angle_shift)
				glVertex2f(x + radius * cos(phi), y - radius * sin(phi));
			glEnd();
			glEnable(GL_TEXTURE_2D);
		}

	}

	/// @param mode    the draw mode
	/// @param coords  the coordinate array
	/// @param count   the number of coordinates/size of the array
	void Graphics::polygon(DrawMode mode, const float* coords, size_t count)
	{
		// coords is an array of a closed loop of vertices, i.e.
		// coords[count-2] = coords[0], coords[count-1] = coords[1]
		if (mode == DRAW_LINE) {
			polyline(coords, count, true);
		} else {
			glDisable(GL_TEXTURE_2D);
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, (const GLvoid*)coords);
			glDrawArrays(GL_POLYGON, 0, count/2-1); // opengl will close the polygon for us
			glDisableClientState(GL_VERTEX_ARRAY);
			glEnable(GL_TEXTURE_2D);
		}
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
		if (userMatrices == matrixLimit)
			throw Exception("Maximum stack depth reached.");
		glPushMatrix();
		++userMatrices;
	}

	void Graphics::pop()
	{
		if (userMatrices < 1)
			throw Exception("Minimum stack depth reached. (More pops than pushes?)");
		glPopMatrix();
		--userMatrices;
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

	void Graphics::shear(float kx, float ky)
	{
		Matrix t;
		t.setShear(kx, ky);
		glMultMatrixf((const GLfloat*)t.getElements());
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
