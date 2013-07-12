/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "Window.h"

// SDL
#include <SDL.h>

// STL
#include <iostream>

namespace love
{
namespace window
{
namespace sdl
{

Window::Window()
	: windowTitle("")
	, created(false)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		throw love::Exception(SDL_GetError());
}

Window::~Window()
{
	if (currentMode.icon)
		currentMode.icon->release();

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

Window::_currentMode::_currentMode()
	: width(800)
	, height(600)
	, flags()
	, icon(0)
{
}

bool Window::setWindow(int width, int height, graphics::Graphics *graphics, WindowFlags *flags)
{
	if (graphics)
		graphics->unSetMode();

	bool fullscreen = false;
	bool vsync = true;
	int fsaa = 0;
	bool resizable = false;
	bool borderless = false;
	bool centered = true;

	if (flags)
	{
		fullscreen = flags->fullscreen;
		vsync = flags->vsync;
		fsaa = flags->fsaa;
		resizable = flags->resizable;
		borderless = flags->borderless;
		centered = flags->centered;
	}

	bool mouseVisible = getMouseVisible();
	int keyrepeatDelay, keyrepeatInterval;
	SDL_GetKeyRepeat(&keyrepeatDelay, &keyrepeatInterval);

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

	if (centered) // Window should be centered.
		SDL_putenv(const_cast<char *>("SDL_VIDEO_CENTERED=center"));
	else
		SDL_putenv(const_cast<char *>("SDL_VIDEO_CENTERED="));

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Could not init SDL_VIDEO: " << SDL_GetError() << std::endl;
		return false;
	}

	// Set caption.
	setWindowTitle(windowTitle);
	setMouseVisible(mouseVisible);
	setIcon(currentMode.icon);
	SDL_EnableKeyRepeat(keyrepeatDelay, keyrepeatInterval);

	// Set GL attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (vsync ? 1 : 0));
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	// FSAA
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (fsaa > 0) ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, (fsaa > 0) ? fsaa : 0);

	Uint32 sdlflags = SDL_OPENGL;
	// Flags
	if (fullscreen)
		sdlflags |= SDL_FULLSCREEN;
	if (resizable)
		sdlflags |= SDL_RESIZABLE;
	if (borderless)
		sdlflags |= SDL_NOFRAME;

	// Have SDL set the video mode.
	SDL_Surface *surface;
	if ((surface = SDL_SetVideoMode(width, height, 32, sdlflags)) == 0)
	{
		bool failed = true;
		if (fsaa > 0)
		{
			// FSAA might have caused the failure, disable it and try again
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			failed = (surface = SDL_SetVideoMode(width, height, 32, sdlflags)) == 0;
		}
		if (failed)
		{
			std::cerr << "Could not set video mode: "  << SDL_GetError() << std::endl;
			return false;
		}
	}

	created = true;

	const SDL_VideoInfo *videoinfo = SDL_GetVideoInfo();
	width = videoinfo->current_w;
	height = videoinfo->current_h;

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

	// Get the actual vsync status
	int real_vsync;
	SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &real_vsync);

	// Set the new display mode as the current display mode.
	currentMode.width = width;
	currentMode.height = height;
	currentMode.flags.fsaa = fsaa;
	currentMode.flags.fullscreen = fullscreen;
	currentMode.flags.vsync = (real_vsync != 0);
	currentMode.flags.resizable = ((surface->flags & SDL_RESIZABLE) != 0);
	currentMode.flags.borderless = ((surface->flags & SDL_NOFRAME) != 0);
	currentMode.flags.centered = centered;

	if (graphics)
		graphics->setMode(width, height);

	return true;
}

void Window::getWindow(int &width, int &height, WindowFlags &flags) const
{
	width = currentMode.width;
	height = currentMode.height;
	flags = currentMode.flags;
}

bool Window::checkWindowSize(int width, int height, bool fullscreen) const
{
	Uint32 sdlflags = fullscreen ? (SDL_OPENGL | SDL_FULLSCREEN) : SDL_OPENGL;

	// Check if mode is supported
	int bpp = SDL_VideoModeOK(width, height, 32, sdlflags);

	return (bpp >= 16);
}

typedef Window::WindowSize WindowSize;

WindowSize *Window::getFullscreenSizes(int &n) const
{
	SDL_Rect **modes = SDL_ListModes(0, SDL_OPENGL | SDL_FULLSCREEN);

	if (modes == (SDL_Rect **)0 || modes == (SDL_Rect **)-1)
	{
		n = 0;
		return NULL;
	}

	n = 0;
	for (int i = 0; modes[i]; i++)
		n++;

	WindowSize *sizes = new WindowSize[n];

	for (int i = 0; i < n; i++)
	{
		WindowSize w = {modes[i]->w, modes[i]->h};
		sizes[i] = w;
	}

	return sizes;
}

int Window::getWidth() const
{
	return currentMode.width;
}

int Window::getHeight() const
{
	return currentMode.height;
}

bool Window::isCreated() const
{
	return created;
}

void Window::setWindowTitle(const std::string &title)
{
	windowTitle = title;
	SDL_WM_SetCaption(windowTitle.c_str(), 0);
}

std::string Window::getWindowTitle() const
{
	// not a reference
	// because we want this untouched
	// const std::string& might be an option
	return windowTitle;
}

bool Window::setIcon(love::image::ImageData *imgd)
{
	if (!imgd)
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

	SDL_Surface *icon = 0;

	{
		// We don't want another thread modifying the ImageData mid-copy.
		love::thread::Lock lock(imgd->getMutex());
		icon = SDL_CreateRGBSurfaceFrom(imgd->getData(), w, h, 32, pitch, rmask, gmask, bmask, amask);
	}

	if (!icon)
		return false;

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);

	imgd->retain();
	if (currentMode.icon)
		currentMode.icon->release();
	currentMode.icon = imgd;

	return true;
}

love::image::ImageData *Window::getIcon()
{
	return currentMode.icon;
}

void Window::swapBuffers()
{
	SDL_GL_SwapBuffers();
}

bool Window::hasFocus() const
{
	return (SDL_GetAppState() & SDL_APPINPUTFOCUS) != 0;
}

bool Window::hasMouseFocus() const
{
	return (SDL_GetAppState() & SDL_APPMOUSEFOCUS) != 0;
}

bool Window::isVisible() const
{
	return (SDL_GetAppState() & SDL_APPACTIVE) != 0;
}

void Window::setMouseVisible(bool visible)
{
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

bool Window::getMouseVisible() const
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) ? true : false;
}

love::window::Window *Window::getSingleton()
{
	if (!singleton)
		singleton = new Window();
	else
		singleton->retain();

	return singleton;
}

const char *Window::getName() const
{
	return "love.window.sdl";
}

} // sdl
} // window
} // love
