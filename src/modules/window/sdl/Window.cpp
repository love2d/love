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
	if (window)
		SDL_DestroyWindow(window);

	if (context)
		SDL_GL_DeleteContext(context);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

bool Window::setWindow(int width, int height, WindowSettings *settings)
{
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
			SDL_DisplayMode mode = {0, width, height, 0, 0};

			// Fullscreen window creation will bug out if no mode can be used.
			if (SDL_GetClosestDisplayMode(f.display, &mode, &mode) == nullptr)
			{
				// GetClosestDisplayMode will fail if we request a size larger
				// than the largest available display mode, so we'll try to use
				// the largest (first) mode in that case.
				if (SDL_GetDisplayMode(f.display, 0, &mode) < 0)
					return false;
			}

			width = mode.w;
			height = mode.h;
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

	graphics::Graphics *gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
		gfx->unSetMode();

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
			|| f.display != curdisplay || f.msaa != curMode.settings.msaa)
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
		created = false;

		// In Windows and Linux, some GL attributes are set on window creation.
		setWindowGLAttributes(f.msaa, f.sRGB);

		const char *title = windowTitle.c_str();
		int pos = f.centered ? centeredpos : uncenteredpos;

		window = SDL_CreateWindow(title, pos, pos, width, height, sdlflags);

		if (!window && f.msaa > 0)
		{
			// MSAA might have caused the failure, disable it and try again.
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

			window = SDL_CreateWindow(title, pos, pos, width, height, sdlflags);
			f.msaa = 0;
		}

		// Make sure the window keeps any previously set icon.
		if (window && curMode.icon.get())
			setIcon(curMode.icon.get());
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

	if (!setContext(f.msaa, f.vsync, f.sRGB))
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

bool Window::setContext(int msaa, bool vsync, bool sRGB)
{
	// We would normally only need to recreate the context if MSAA changes or
	// SDL_GL_MakeCurrent is unsuccessful, but in Windows MakeCurrent can
	// sometimes claim success but the context will actually be trashed.
	if (context)
	{
		SDL_GL_DeleteContext(context);
		context = 0;
	}

	// Make sure the proper attributes are set.
	setWindowGLAttributes(msaa, sRGB);

	context = SDL_GL_CreateContext(window);

	if (!context && msaa > 0)
	{
		// MSAA might have caused the failure, disable it and try again.
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

	// Verify MSAA setting.
	int buffers;
	int samples;
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);

	// Don't fail because of this, but issue a warning.
	if ((!buffers && msaa) || (samples != msaa))
	{
		std::cerr << "Warning, MSAA setting failed! (Result: buffers: " << buffers << ", samples: " << samples << ")" << std::endl;
		msaa = (buffers > 0) ? samples : 0;
	}

	curMode.settings.msaa = msaa;
	curMode.settings.vsync = SDL_GL_GetSwapInterval() != 0;

	return true;
}

void Window::setWindowGLAttributes(int msaa, bool /* sRGB */) const
{
	// Set GL window attributes.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

	// MSAA.
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (msaa > 0) ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, (msaa > 0) ? msaa : 0);

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
		curMode.settings.fstype = FULLSCREEN_TYPE_EXCLUSIVE;
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
	if (curMode.settings.fullscreen && curMode.settings.fstype == FULLSCREEN_TYPE_EXCLUSIVE)
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1");
	else
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	curMode.settings.sRGB = newsettings.sRGB;

	SDL_DisplayMode dmode = {};
	SDL_GetCurrentDisplayMode(curMode.settings.display, &dmode);

	// May be 0 if the refresh rate can't be determined.
	curMode.settings.refreshrate = (double) dmode.refresh_rate;
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
		graphics::Graphics *gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
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

const char *Window::getDisplayName(int displayindex) const
{
	const char *name = SDL_GetDisplayName(displayindex);

	if (name == nullptr)
		throw love::Exception("Invalid display index: %d", displayindex + 1);

	return name;
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

	curMode.icon.set(imgd);

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
	return curMode.icon.get();
}

void Window::minimize()
{
	if (window != nullptr)
		SDL_MinimizeWindow(window);
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

double Window::toPixels(double x) const
{
	return x * getPixelScale();
}

void Window::toPixels(double wx, double wy, double &px, double &py) const
{
	double scale = getPixelScale();
	px = wx * scale;
	py = wy * scale;
}

double Window::fromPixels(double x) const
{
	return x / getPixelScale();
}

void Window::fromPixels(double px, double py, double &wx, double &wy) const
{
	double scale = getPixelScale();
	wx = px / scale;
	wy = py / scale;
}

const void *Window::getHandle() const
{
	return window;
}

SDL_MessageBoxFlags Window::convertMessageBoxType(MessageBoxType type) const
{
	switch (type)
	{
	case MESSAGEBOX_ERROR:
		return SDL_MESSAGEBOX_ERROR;
	case MESSAGEBOX_WARNING:
		return SDL_MESSAGEBOX_WARNING;
	case MESSAGEBOX_INFO:
	default:
		return SDL_MESSAGEBOX_INFORMATION;
	}
}

bool Window::showMessageBox(MessageBoxType type, const std::string &title, const std::string &message, bool attachtowindow)
{
	SDL_MessageBoxFlags flags = convertMessageBoxType(type);
	SDL_Window *sdlwindow = attachtowindow ? window : nullptr;

	return SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), sdlwindow) >= 0;
}

int Window::showMessageBox(const MessageBoxData &data)
{
	SDL_MessageBoxData sdldata = {};

	sdldata.flags = convertMessageBoxType(data.type);
	sdldata.title = data.title.c_str();
	sdldata.message = data.message.c_str();
	sdldata.window = data.attachToWindow ? window : nullptr;

	sdldata.numbuttons = (int) data.buttons.size();

	std::vector<SDL_MessageBoxButtonData> sdlbuttons;

	for (size_t i = 0; i < data.buttons.size(); i++)
	{
		SDL_MessageBoxButtonData sdlbutton = {};

		sdlbutton.buttonid = (int) i;
		sdlbutton.text = data.buttons[i].c_str();

		if ((int) i == data.enterButtonIndex)
			sdlbutton.flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;

		if ((int) i == data.escapeButtonIndex)
			sdlbutton.flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;

		sdlbuttons.push_back(sdlbutton);
	}

	sdldata.buttons = &sdlbuttons[0];

	int pressedbutton = -2;
	SDL_ShowMessageBox(&sdldata, &pressedbutton);

	return pressedbutton;
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
