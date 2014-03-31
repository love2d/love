/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

// LOVE
#include "common/config.h"
#include "graphics/Graphics.h"
#include "Window.h"

// C++
#include <iostream>
#include <vector>
#include <algorithm>

namespace love
{
namespace window
{
namespace sdl
{

Window::Window()
	: windowTitle("")
	, created(false)
	, mouseGrabbed(false)
	, window(0)
	, context(0)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		throw love::Exception("%s", SDL_GetError());
}

Window::~Window()
{
	if (curMode.icon)
		curMode.icon->release();

	if (window)
		SDL_DestroyWindow(window);

	if (context)
		SDL_GL_DeleteContext(context);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

Window::_currentMode::_currentMode()
	: width(800)
	, height(600)
	, settings()
	, icon(0)
{
}

bool Window::setWindow(int width, int height, WindowSettings *settings)
{
	graphics::Graphics *gfx = (graphics::Graphics *) Module::findInstance("love.graphics.");
	if (gfx != nullptr)
		gfx->unSetMode();

	WindowSettings f;

	if (settings)
		f = *settings;

	f.minwidth = std::max(f.minwidth, 1);
	f.minheight = std::max(f.minheight, 1);

	f.display = std::min(std::max(f.display, 0), getDisplayCount() - 1);

	// Use the desktop resolution if a width or height of 0 is specified.
	if (width == 0 || height == 0)
	{
		SDL_DisplayMode mode = {};
		SDL_GetDesktopDisplayMode(f.display, &mode);
		width = mode.w;
		height = mode.h;
	}

	Uint32 sdlflags = SDL_WINDOW_OPENGL;

	if (f.fullscreen)
	{
		if (f.fstype == FULLSCREEN_TYPE_DESKTOP)
			sdlflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
		{
			sdlflags |= SDL_WINDOW_FULLSCREEN;

			// Fullscreen window creation will bug out if no mode can be used.
			SDL_DisplayMode mode = {0, width, height, 0, 0};
			if (SDL_GetClosestDisplayMode(f.display, &mode, &mode) == 0)
				return false;
		}
	}

	if (f.resizable)
		sdlflags |= SDL_WINDOW_RESIZABLE;

	if (f.borderless)
		sdlflags |= SDL_WINDOW_BORDERLESS;

	// FIXME: disabled in Linux for runtime SDL 2.0.0 compatibility.
#if SDL_VERSION_ATLEAST(2,0,1) && !defined(LOVE_LINUX)
	if (f.highdpi)
		sdlflags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

	// Destroy and recreate the window if the dimensions or flags have changed.
	if (window)
	{
		int curdisplay = SDL_GetWindowDisplayIndex(window);
		Uint32 wflags = SDL_GetWindowFlags(window);

		Uint32 testflags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP
			| SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;

		// FIXME: disabled in Linux for runtime SDL 2.0.0 compatibility.
#if SDL_VERSION_ATLEAST(2,0,1) && !defined(LOVE_LINUX)
		testflags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

		wflags &= testflags;

		if (sdlflags != wflags || width != curMode.width || height != curMode.height
			|| f.display != curdisplay || f.fsaa != curMode.settings.fsaa)
		{
			SDL_DestroyWindow(window);
			window = 0;

			// The old window may have generated pending events which are no
			// longer relevant. Destroy them all!
			SDL_FlushEvent(SDL_WINDOWEVENT);
		}
	}

	int centeredpos = SDL_WINDOWPOS_CENTERED_DISPLAY(f.display);
	int uncenteredpos = SDL_WINDOWPOS_UNDEFINED_DISPLAY(f.display);

	if (!window)
	{
		// In Windows and Linux, some GL attributes are set on window creation.
		setWindowGLAttributes(f.fsaa, f.sRGB);

		const char *title = windowTitle.c_str();
		int pos = f.centered ? centeredpos : uncenteredpos;

		window = SDL_CreateWindow(title, pos, pos, width, height, sdlflags);

		if (!window && f.fsaa > 0)
		{
			// FSAA might have caused the failure, disable it and try again.
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

			window = SDL_CreateWindow(title, pos, pos, width, height, sdlflags);
			f.fsaa = 0;
		}

		// Make sure the window keeps any previously set icon.
		if (window && curMode.icon)
			setIcon(curMode.icon);
	}

	if (!window)
	{
		std::cerr << "Could not set video mode: " << SDL_GetError() << std::endl;
		return false;
	}

	// Enforce minimum window dimensions.
	SDL_SetWindowMinimumSize(window, f.minwidth, f.minheight);

	if (f.centered && !f.fullscreen)
		SDL_SetWindowPosition(window, centeredpos, centeredpos);

	SDL_RaiseWindow(window);

	if (!setContext(f.fsaa, f.vsync, f.sRGB))
		return false;

	created = true;

	updateSettings(f);

	if (gfx != nullptr)
	{
		int width = curMode.width;
		int height = curMode.height;

		// FIXME: disabled in Linux for runtime SDL 2.0.0 compatibility.
#if SDL_VERSION_ATLEAST(2,0,1) && !defined(LOVE_LINUX)
		SDL_GL_GetDrawableSize(window, &width, &height);
#endif

		gfx->setMode(width, height, curMode.settings.sRGB);
	}

	// Make sure the mouse keeps its previous grab setting.
	setMouseGrab(mouseGrabbed);

	return true;
}

bool Window::onWindowResize(int width, int height)
{
	if (!window)
		return false;

	curMode.width = width;
	curMode.height = height;

	return true;
}

bool Window::setContext(int fsaa, bool vsync, bool sRGB)
{
	// We would normally only need to recreate the context if FSAA changes or
	// SDL_GL_MakeCurrent is unsuccessful, but in Windows MakeCurrent can
	// sometimes claim success but the context will actually be trashed.
	if (context)
	{
		SDL_GL_DeleteContext(context);
		context = 0;
	}

	// Make sure the proper attributes are set.
	setWindowGLAttributes(fsaa, sRGB);

	context = SDL_GL_CreateContext(window);

	if (!context && fsaa > 0)
	{
		// FSAA might have caused the failure, disable it and try again.
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		context = SDL_GL_CreateContext(window);
	}

	if (!context)
	{
		int flags = 0;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &flags);
		if (flags & SDL_GL_CONTEXT_DEBUG_FLAG)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
			context = SDL_GL_CreateContext(window);
		}
	}

	if (!context)
	{
		std::cerr << "Could not set video mode: " << SDL_GetError() << std::endl;
		return false;
	}

	// Set vertical synchronization.
	SDL_GL_SetSwapInterval(vsync ? 1 : 0);

	// Verify FSAA setting.
	int buffers;
	int samples;
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);

	// Don't fail because of this, but issue a warning.
	if ((!buffers && fsaa) || (samples != fsaa))
	{
		std::cerr << "Warning, FSAA setting failed! (Result: buffers: " << buffers << ", samples: " << samples << ")" << std::endl;
		fsaa = (buffers > 0) ? samples : 0;
	}

	curMode.settings.fsaa = fsaa;
	curMode.settings.vsync = SDL_GL_GetSwapInterval() != 0;

	return true;
}

void Window::setWindowGLAttributes(int fsaa, bool /* sRGB */) const
{
	// Set GL window attributes.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	// FSAA.
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (fsaa > 0) ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, (fsaa > 0) ? fsaa : 0);

	/* FIXME: Enable this code but make sure to try to re-create the window and
	 * context with this disabled, if creation fails with it enabled.
	 * We can leave this out for now because in practice the framebuffer will
	 * already be sRGB-capable (on desktops at least.)
#if SDL_VERSION_ATLEAST(2,0,1)
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, sRGB ? 1 : 0);
#endif
	 */

	// Do we want a debug context?
	const char *debugenv = SDL_GetHint("LOVE_GRAPHICS_DEBUG");
	if (debugenv && *debugenv == '1')
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
	}
}

void Window::updateSettings(const WindowSettings &newsettings)
{
	Uint32 wflags = SDL_GetWindowFlags(window);

	// Set the new display mode as the current display mode.
	SDL_GetWindowSize(window, &curMode.width, &curMode.height);

	if ((wflags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		curMode.settings.fullscreen = true;
		curMode.settings.fstype = FULLSCREEN_TYPE_DESKTOP;
	}
	else if ((wflags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
	{
		curMode.settings.fullscreen = true;
		curMode.settings.fstype = FULLSCREEN_TYPE_NORMAL;
	}
	else
	{
		curMode.settings.fullscreen = false;
		curMode.settings.fstype = newsettings.fstype;
	}

	// The min width/height is set to 0 internally in SDL when in fullscreen.
	if (curMode.settings.fullscreen)
	{
		curMode.settings.minwidth = newsettings.minwidth;
		curMode.settings.minheight = newsettings.minheight;
	}
	else
		SDL_GetWindowMinimumSize(window, &curMode.settings.minwidth, &curMode.settings.minheight);

	curMode.settings.resizable = (wflags & SDL_WINDOW_RESIZABLE) != 0;
	curMode.settings.borderless = (wflags & SDL_WINDOW_BORDERLESS) != 0;
	curMode.settings.centered = newsettings.centered;
	curMode.settings.display = std::max(SDL_GetWindowDisplayIndex(window), 0);

#if SDL_VERSION_ATLEAST(2,0,1)
	curMode.settings.highdpi = (wflags & SDL_WINDOW_ALLOW_HIGHDPI) != 0;
#else
	curMode.settings.highdpi = false;
#endif

	// Only minimize on focus loss if the window is in exclusive-fullscreen
	// mode.
	if (curMode.settings.fullscreen && curMode.settings.fstype == FULLSCREEN_TYPE_NORMAL)
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1");
	else
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	curMode.settings.sRGB = newsettings.sRGB;
}

void Window::getWindow(int &width, int &height, WindowSettings &settings)
{
	// The window might have been modified (moved, resized, etc.) by the user.
	if (window)
		updateSettings(curMode.settings);

	width = curMode.width;
	height = curMode.height;
	settings = curMode.settings;
}

bool Window::setFullscreen(bool fullscreen, Window::FullscreenType fstype)
{
	if (!window)
		return false;

	WindowSettings newsettings = curMode.settings;
	newsettings.fullscreen = fullscreen;
	newsettings.fstype = fstype;

	Uint32 sdlflags = 0;

	if (fullscreen)
	{
		if (fstype == FULLSCREEN_TYPE_DESKTOP)
			sdlflags = SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
		{
			sdlflags = SDL_WINDOW_FULLSCREEN;

			SDL_DisplayMode mode = {};
			mode.w = curMode.width;
			mode.h = curMode.height;

			SDL_GetClosestDisplayMode(SDL_GetWindowDisplayIndex(window), &mode, &mode);
			SDL_SetWindowDisplayMode(window, &mode);
		}
	}

	if (SDL_SetWindowFullscreen(window, sdlflags) == 0)
	{
		SDL_GL_MakeCurrent(window, context);
		updateSettings(newsettings);

		// Update the viewport size now instead of waiting for event polling.
		graphics::Graphics *gfx = (graphics::Graphics *) Module::findInstance("love.graphics.");
		if (gfx != nullptr)
		{
			int width = curMode.width;
			int height = curMode.height;

			// FIXME: disabled in Linux for runtime SDL 2.0.0 compatibility.
#if SDL_VERSION_ATLEAST(2,0,1) && !defined(LOVE_LINUX)
			SDL_GL_GetDrawableSize(window, &width, &height);
#endif

			gfx->setViewportSize(width, height);
		}

		return true;
	}

	return false;
}

bool Window::setFullscreen(bool fullscreen)
{
	return setFullscreen(fullscreen, curMode.settings.fstype);
}

int Window::getDisplayCount() const
{
	return SDL_GetNumVideoDisplays();
}

typedef Window::WindowSize WindowSize;

std::vector<WindowSize> Window::getFullscreenSizes(int displayindex) const
{
	std::vector<WindowSize> sizes;

	SDL_DisplayMode mode = {};
	std::vector<WindowSize>::const_iterator it;
	for (int i = 0; i < SDL_GetNumDisplayModes(displayindex); i++)
	{
		SDL_GetDisplayMode(displayindex, i, &mode);

		// SDL2's display mode list has multiple entries for modes of the same
		// size with different bits per pixel, so we need to filter those out.
		bool alreadyhassize = false;
		for (it = sizes.begin(); it != sizes.end(); ++it)
		{
			if (it->width == mode.w && it->height == mode.h)
			{
				alreadyhassize = true;
				break;
			}
		}

		if (!alreadyhassize)
		{
			WindowSize w = {mode.w, mode.h};
			sizes.push_back(w);
		}
	}

	return sizes;
}

int Window::getWidth() const
{
	return curMode.width;
}

int Window::getHeight() const
{
	return curMode.height;
}

void Window::getDesktopDimensions(int displayindex, int &width, int &height) const
{
	if (displayindex >= 0 && displayindex < getDisplayCount())
	{
		SDL_DisplayMode mode = {};
		SDL_GetDesktopDisplayMode(displayindex, &mode);
		width = mode.w;
		height = mode.h;
	}
	else
	{
		width = 0;
		height = 0;
	}
}

bool Window::isCreated() const
{
	return created;
}

void Window::setWindowTitle(const std::string &title)
{
	windowTitle = title;

	if (window)
		SDL_SetWindowTitle(window, title.c_str());
}

const std::string &Window::getWindowTitle() const
{
	return windowTitle;
}

bool Window::setIcon(love::image::ImageData *imgd)
{
	if (!imgd)
		return false;

	imgd->retain();
	if (curMode.icon)
		curMode.icon->release();
	curMode.icon = imgd;

	if (!window)
		return false;

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

	int w = imgd->getWidth();
	int h = imgd->getHeight();
	int pitch = imgd->getWidth() * 4;

	SDL_Surface *sdlicon = 0;

	{
		// We don't want another thread modifying the ImageData mid-copy.
		love::thread::Lock lock(imgd->getMutex());
		sdlicon = SDL_CreateRGBSurfaceFrom(imgd->getData(), w, h, 32, pitch, rmask, gmask, bmask, amask);
	}

	if (!sdlicon)
		return false;

	SDL_SetWindowIcon(window, sdlicon);
	SDL_FreeSurface(sdlicon);

	return true;
}

love::image::ImageData *Window::getIcon()
{
	return curMode.icon;
}

void Window::swapBuffers()
{
	SDL_GL_SwapWindow(window);
}

bool Window::hasFocus() const
{
	return (window && SDL_GetKeyboardFocus() == window);
}

bool Window::hasMouseFocus() const
{
	return (window && SDL_GetMouseFocus() == window);
}

bool Window::isVisible() const
{
	return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_SHOWN) != 0;
}

void Window::setMouseVisible(bool visible)
{
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

bool Window::getMouseVisible() const
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

void Window::setMouseGrab(bool grab)
{
	mouseGrabbed = grab;
	if (window)
		SDL_SetWindowGrab(window, (SDL_bool) grab);
}

bool Window::isMouseGrabbed() const
{
	if (window)
		return (bool) SDL_GetWindowGrab(window);
	else
		return mouseGrabbed;
}

double Window::getPixelScale() const
{
	double scale = 1.0;

	// FIXME: disabled in Linux for runtime SDL 2.0.0 compatibility.
#if SDL_VERSION_ATLEAST(2,0,1) && !defined(LOVE_LINUX)
	if (window)
	{
		int wheight;
		SDL_GetWindowSize(window, nullptr, &wheight);

		int dheight = wheight;
		SDL_GL_GetDrawableSize(window, nullptr, &dheight);

		scale = (double) dheight / wheight;
	}
#endif

	return scale;
}

const void *Window::getHandle() const
{
	return window;
}

love::window::Window *Window::createSingleton()
{
	if (!singleton)
		singleton = new Window();
	else
		singleton->retain();

	return singleton;
}

love::window::Window *Window::getSingleton()
{
	return singleton;
}

const char *Window::getName() const
{
	return "love.window.sdl";
}

} // sdl
} // window
} // love
