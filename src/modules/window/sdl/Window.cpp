/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#ifdef LOVE_GRAPHICS_VULKAN
#	include "graphics/vulkan/Graphics.h"
#	include "graphics/vulkan/Vulkan.h"
#endif
#include "Window.h"
#include "filesystem/Filesystem.h"

#ifdef LOVE_ANDROID
#include "common/android.h"
#endif

#ifdef LOVE_IOS
#include "common/ios.h"
#endif

// C++
#include <iostream>
#include <vector>
#include <algorithm>

// C
#include <cstdio>

#ifdef LOVE_GRAPHICS_VULKAN
#include <SDL3/SDL_vulkan.h>
#endif

#if defined(LOVE_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <VersionHelpers.h>
#elif defined(LOVE_MACOS)
#include "common/macos.h"
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

namespace love
{
namespace window
{

// See src/modules/window/Window.cpp.
void setHighDPIAllowedImplementation(bool enable)
{
	LOVE_UNUSED(enable);
}

namespace sdl
{

Window::Window()
	: love::window::Window("love.window.sdl")
	, open(false)
	, mouseGrabbed(false)
	, window(nullptr)
	, glcontext(nullptr)
#ifdef LOVE_GRAPHICS_METAL
	, metalView(nullptr)
#endif
	, displayedWindowError(false)
	, contextAttribs()
{
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		throw love::Exception("Could not initialize SDL video subsystem (%s)", SDL_GetError());

	// Make sure the screensaver doesn't activate by default.
	setDisplaySleepEnabled(false);

#ifdef LOVE_WINDOWS
	// Turned off by default, because it (ironically) causes stuttering issues
	// on some setups. More investigation is needed before enabling it.
	canUseDwmFlush = SDL_GetHintBoolean("LOVE_GRAPHICS_VSYNC_DWM", false);
#endif

	dialogEventId = SDL_RegisterEvents(1);
}

Window::~Window()
{
	close(false);
	graphics.set(nullptr);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

void Window::setGraphics(graphics::Graphics *graphics)
{
	this->graphics.set(graphics);
}

void Window::setGLFramebufferAttributes(bool sRGB)
{
	// Set GL window / framebuffer attributes.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

	// Always use 24/8 depth/stencil.
	// Changing this after initial window creation would need the context to be
	// destroyed and recreated, which we really don't want.
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// Backbuffer MSAA is handled by the love.graphics implementation.
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, sRGB ? 1 : 0);

#if defined(LOVE_WINDOWS)
	// Avoid the Microsoft OpenGL 1.1 software renderer on Windows. Apparently
	// older Intel drivers like to use it as a fallback when requesting some
	// unsupported framebuffer attribute values, rather than properly failing.
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif
}

void Window::setGLContextAttributes(const ContextAttribs &attribs)
{
	int profilemask = 0;
	int contextflags = 0;

	if (attribs.gles)
		profilemask = SDL_GL_CONTEXT_PROFILE_ES;
	else if (attribs.versionMajor * 10 + attribs.versionMinor >= 32)
		profilemask |= SDL_GL_CONTEXT_PROFILE_CORE;
	else if (attribs.debug)
		profilemask = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;

	if (attribs.debug)
		contextflags |= SDL_GL_CONTEXT_DEBUG_FLAG;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, attribs.versionMajor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, attribs.versionMinor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profilemask);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextflags);
}

bool Window::checkGLVersion(const ContextAttribs &attribs, std::string &outversion)
{
	typedef unsigned char GLubyte;
	typedef unsigned int GLenum;
	typedef const GLubyte *(APIENTRY *glGetStringPtr)(GLenum name);
	const GLenum GL_VENDOR_ENUM   = 0x1F00;
	const GLenum GL_RENDERER_ENUM = 0x1F01;
	const GLenum GL_VERSION_ENUM  = 0x1F02;

	// We don't have OpenGL headers or an automatic OpenGL function loader in
	// this module, so we have to get the glGetString function pointer ourselves.
	glGetStringPtr glGetStringFunc = (glGetStringPtr) SDL_GL_GetProcAddress("glGetString");
	if (!glGetStringFunc)
		return false;

	const char *glversion = (const char *) glGetStringFunc(GL_VERSION_ENUM);
	if (!glversion)
		return false;

	outversion = glversion;

	const char *glrenderer = (const char *) glGetStringFunc(GL_RENDERER_ENUM);
	if (glrenderer)
		outversion += " - " + std::string(glrenderer);

	const char *glvendor = (const char *) glGetStringFunc(GL_VENDOR_ENUM);
	if (glvendor)
		outversion += " (" + std::string(glvendor) + ")";

	int glmajor = 0;
	int glminor = 0;

	// glGetString(GL_VERSION) returns a string with the format "major.minor",
	// or "OpenGL ES major.minor" in GLES contexts.
	const char *format = "%d.%d";
	if (attribs.gles)
		format = "OpenGL ES %d.%d";

	if (sscanf(glversion, format, &glmajor, &glminor) != 2)
		return false;

	if (glmajor < attribs.versionMajor
		|| (glmajor == attribs.versionMajor && glminor < attribs.versionMinor))
		return false;

	return true;
}

std::vector<Window::ContextAttribs> Window::getContextAttribsList() const
{
	// If we already have a set of context attributes that we know work, just
	// return that. love.graphics doesn't really support switching GL versions
	// after the first initialization.
	if (contextAttribs.versionMajor > 0)
		return std::vector<ContextAttribs>{contextAttribs};

	bool preferGLES = false;

#ifdef LOVE_GRAPHICS_USE_OPENGLES
	preferGLES = true;
#endif

	const char *curdriver = SDL_GetCurrentVideoDriver();
	const char *glesdrivers[] = {"RPI", "Android", "uikit", "winrt", "emscripten"};

	// We always want to try OpenGL ES first on certain video backends.
	for (const char *glesdriver : glesdrivers)
	{
		if (curdriver && strstr(curdriver, glesdriver) == curdriver)
		{
			preferGLES = true;
			break;
		}
	}

	const char *gleshint = SDL_GetHint("LOVE_GRAPHICS_USE_OPENGLES");
	if (gleshint != nullptr)
		preferGLES = (gleshint != nullptr && gleshint[0] != '0');

	// Do we want a debug context?
	bool debug = love::graphics::isDebugEnabled();

	const char *preferGL3hint = SDL_GetHint("LOVE_GRAPHICS_USE_GL3");
	bool preferGL3 = (preferGL3hint != nullptr && preferGL3hint[0] != '0');

	std::vector<ContextAttribs> glcontexts =
	{
		{4, 3, false, debug},
		{3, 3, false, debug},
	};

	std::vector<ContextAttribs> glescontexts =
	{
		{3, 2, true, debug},
		{3, 0, true, debug},
	};

	if (preferGL3)
	{
		std::swap(glcontexts[0], glcontexts[1]);
		std::swap(glescontexts[0], glescontexts[1]);
	}

	std::vector<ContextAttribs> attribslist;

	if (preferGLES)
	{
		attribslist.insert(attribslist.end(), glescontexts.begin(), glescontexts.end());
		attribslist.insert(attribslist.end(), glcontexts.begin(), glcontexts.end());
	}
	else
	{
		attribslist.insert(attribslist.end(), glcontexts.begin(), glcontexts.end());
		attribslist.insert(attribslist.end(), glescontexts.begin(), glescontexts.end());
	}

	return attribslist;
}

bool Window::createWindowAndContext(int x, int y, int w, int h, Uint32 windowflags, graphics::Renderer renderer)
{
	bool needsglcontext = (windowflags & SDL_WINDOW_OPENGL) != 0;
#ifdef LOVE_GRAPHICS_METAL
	bool needsmetalview = (windowflags & SDL_WINDOW_METAL) != 0;
#endif

	std::string windowerror;
	std::string contexterror;
	std::string glversion;

	// Unfortunately some OpenGL context settings are part of the internal
	// window state in the Windows and Linux SDL backends, so we have to
	// recreate the window when we want to change those settings...
	// Also, apparently some Intel drivers on Windows give back a Microsoft
	// OpenGL 1.1 software renderer context when high MSAA values are requested!

	const auto create = [&](const ContextAttribs *attribs) -> bool
	{
		if (glcontext)
		{
			SDL_GL_DestroyContext(glcontext);
			glcontext = nullptr;
		}

#ifdef LOVE_GRAPHICS_METAL
		if (metalView)
		{
			SDL_Metal_DestroyView(metalView);
			metalView = nullptr;
		}
#endif

		if (window)
		{
			SDL_DestroyWindow(window);
			SDL_FlushEvents(SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST);
			window = nullptr;
		}

		window = SDL_CreateWindow(title.c_str(), w, h, windowflags);

		if (!window)
		{
			windowerror = std::string(SDL_GetError());
			return false;
		}

		SDL_SetWindowPosition(window, x, y);

		if (attribs != nullptr && renderer == love::graphics::Renderer::RENDERER_OPENGL)
		{
#ifdef LOVE_MACOS
			love::macos::setWindowSRGBColorSpace(window);
#endif

			glcontext = SDL_GL_CreateContext(window);

			if (!glcontext)
				contexterror = std::string(SDL_GetError());

			// Make sure the context's version is at least what we requested.
			if (glcontext && !checkGLVersion(*attribs, glversion))
			{
				SDL_GL_DestroyContext(glcontext);
				glcontext = nullptr;
			}

			if (!glcontext)
			{
				SDL_DestroyWindow(window);
				window = nullptr;
				return false;
			}
		}

		return true;
	};

	if (renderer == graphics::RENDERER_OPENGL)
	{
		std::vector<ContextAttribs> attribslist = getContextAttribsList();

		// Try each context profile in order.
		for (ContextAttribs attribs : attribslist)
		{
			bool curSRGB = love::graphics::isGammaCorrect();

			setGLFramebufferAttributes(curSRGB);
			setGLContextAttributes(attribs);

			windowerror.clear();
			contexterror.clear();

			create(&attribs);

			if (!window && curSRGB)
			{
				// The sRGB setting could have caused the failure.
				setGLFramebufferAttributes(false);
				if (create(&attribs))
					curSRGB = false;
			}

			if (window && glcontext)
			{
				// Store the successful context attributes so we can re-use them in
				// subsequent calls to createWindowAndContext.
				contextAttribs = attribs;
				love::graphics::setGammaCorrect(curSRGB);
				break;
			}
		}
	}
#ifdef LOVE_GRAPHICS_METAL
	else if (renderer == graphics::RENDERER_METAL)
	{
		if (create(nullptr) && window != nullptr)
			metalView = SDL_Metal_CreateView(window);

		if (metalView == nullptr && window != nullptr)
		{
			contexterror = SDL_GetError();
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}
#endif
	else
	{
		create(nullptr);
	}

	bool failed = window == nullptr;
	failed |= (needsglcontext && !glcontext);
#ifdef LOVE_GRAPHICS_METAL
	failed |= (needsmetalview && !metalView);
#endif

	if (failed)
	{
		std::string title = "Unable to create renderer";
		std::string message = "This program requires a graphics card and video drivers which support OpenGL 3.3 or OpenGL ES 3.0.";

		if (!glversion.empty())
			message += "\n\nDetected OpenGL version:\n" + glversion;
		else if (!contexterror.empty())
			message += "\n\nRenderer context creation error: " + contexterror;
		else if (!windowerror.empty())
			message += "\n\nSDL window creation error: " + windowerror;

		std::cerr << title << std::endl << message << std::endl;

		// Display a message box with the error, but only once.
		if (!displayedWindowError)
		{
			showMessageBox(title, message, MESSAGEBOX_ERROR, false);
			displayedWindowError = true;
		}

		close();
		return false;
	}

	open = true;
	return true;
}

struct SDLDisplayIDs
{
	SDLDisplayIDs()
	{
		ids = SDL_GetDisplays(&count);
	}

	~SDLDisplayIDs()
	{
		if (ids)
			SDL_free(ids);
	}

	int count = 0;
	SDL_DisplayID *ids = nullptr;
};

static SDL_DisplayID GetSDLDisplayIDForIndex(int displayindex)
{
	SDLDisplayIDs displayids;
	if (displayindex < 0 || displayindex >= displayids.count)
		return (SDL_DisplayID) 0;
	return displayids.ids[displayindex];
}

bool Window::setWindow(int width, int height, WindowSettings *settings)
{
	if (!graphics.get())
		graphics.set(Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS));

	if (graphics.get() && graphics->isRenderTargetActive())
		throw love::Exception("love.window.setMode cannot be called while a render target is active in love.graphics.");

	auto renderer = graphics != nullptr ? graphics->getRenderer() : graphics::RENDERER_NONE;

	if (isOpen())
		updateSettings(this->settings, false);

	WindowSettings f;

	if (settings)
		f = *settings;

	f.minwidth = std::max(f.minwidth, 1);
	f.minheight = std::max(f.minheight, 1);

	SDLDisplayIDs displays;
	int displaycount = displays.count;

	f.displayindex = std::min(std::max(f.displayindex, 0), displaycount - 1);

	// Use the desktop resolution if a width or height of 0 is specified.
	if (width == 0 || height == 0)
	{
		const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(displays.ids[f.displayindex]);
		width = mode->w;
		height = mode->h;
	}

	// On Android, disable fullscreen first on window creation so it's
	// possible to change the orientation by specifying portait width and
	// height, otherwise SDL will pick the current orientation dimensions when
	// fullscreen flag is set. Don't worry, we'll set it back later when user
	// also requested fullscreen after the window is created.
	// See https://github.com/love2d/love-android/issues/196
#ifdef LOVE_ANDROID
	bool fullscreen = f.fullscreen;

	f.fullscreen = false;
	f.fstype = FULLSCREEN_DESKTOP;
#endif

	int x = f.x;
	int y = f.y;

	if (f.useposition)
	{
		// The position needs to be in the global coordinate space.
		SDL_Rect displaybounds = {};
		SDL_GetDisplayBounds(displays.ids[f.displayindex], &displaybounds);
		x += displaybounds.x;
		y += displaybounds.y;
	}
	else
	{
		if (f.centered)
			x = y = SDL_WINDOWPOS_CENTERED_DISPLAY(displays.ids[f.displayindex]);
		else
			x = y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(displays.ids[f.displayindex]);
	}

	Uint32 sdlflags = 0;
	SDL_DisplayMode fsmode = {};

	if (f.fullscreen)
	{
		sdlflags |= SDL_WINDOW_FULLSCREEN;

		if (f.fstype == FULLSCREEN_EXCLUSIVE)
		{
			SDL_DisplayID display = displays.ids[f.displayindex];
			if (!SDL_GetClosestFullscreenDisplayMode(display, width, height, 0, isHighDPIAllowed(), &fsmode))
			{
				// GetClosestDisplayMode will fail if we request a size larger
				// than the largest available display mode, so we'll try to use
				// the largest (first) mode in that case.
				int modecount = 0;
				SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(display, &modecount);
				if (modecount > 0)
					fsmode = *modes[0];
				SDL_free(modes);
				if (fsmode.w == 0 || fsmode.h == 0)
					return false;
			}
		}
	}

	bool needsetmode = false;

	if (renderer != windowRenderer && isOpen())
		close();

	if (isOpen())
	{
		if (fsmode.w > 0 && fsmode.h > 0)
			SDL_SetWindowFullscreenMode(window, &fsmode);
		else
			SDL_SetWindowFullscreenMode(window, nullptr);

		if (SDL_SetWindowFullscreen(window, (sdlflags & SDL_WINDOW_FULLSCREEN) != 0) && renderer == graphics::RENDERER_OPENGL)
			SDL_GL_MakeCurrent(window, glcontext);

		// TODO: should we make this conditional, to avoid love.resize events when the size doesn't change?
		SDL_SetWindowSize(window, width, height);

		if (this->settings.resizable != f.resizable)
			SDL_SetWindowResizable(window, f.resizable);

		if (this->settings.borderless != f.borderless)
			SDL_SetWindowBordered(window, !f.borderless);
	}
	else
	{
		if (renderer == graphics::RENDERER_OPENGL)
			sdlflags |= SDL_WINDOW_OPENGL;
	#ifdef LOVE_GRAPHICS_METAL
		if (renderer == graphics::RENDERER_METAL)
			sdlflags |= SDL_WINDOW_METAL;
	#endif

		if (renderer == graphics::RENDERER_VULKAN)
			sdlflags |= SDL_WINDOW_VULKAN;

		if (f.resizable)
			 sdlflags |= SDL_WINDOW_RESIZABLE;

		if (f.borderless)
			 sdlflags |= SDL_WINDOW_BORDERLESS;

		// Note: this flag is ignored on Windows.
		if (isHighDPIAllowed())
			sdlflags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

		Uint32 createflags = sdlflags & (~SDL_WINDOW_FULLSCREEN);

		if (!createWindowAndContext(x, y, width, height, createflags, renderer))
			return false;

		if (f.fullscreen)
		{
			if (fsmode.w > 0 && fsmode.h > 0)
				SDL_SetWindowFullscreenMode(window, &fsmode);
			else
				SDL_SetWindowFullscreenMode(window, nullptr);
			SDL_SetWindowFullscreen(window, true);
		}

		needsetmode = true;
	}

	windowRenderer = renderer;

	// Make sure the window keeps any previously set icon.
	setIcon(icon.get());

	// Make sure the mouse keeps its previous grab setting.
	setMouseGrab(mouseGrabbed);

	// Enforce minimum window dimensions.
	SDL_SetWindowMinimumSize(window, f.minwidth, f.minheight);

	if (this->settings.displayindex != f.displayindex || f.useposition || f.centered)
		SDL_SetWindowPosition(window, x, y);

	SDL_RaiseWindow(window);

	setVSync(f.vsync);

	updateSettings(f, false);

	if (graphics.get())
	{
		double scaledw, scaledh;
		fromPixels((double) pixelWidth, (double) pixelHeight, scaledw, scaledh);

		if (needsetmode)
		{
			void *context = nullptr;
			if (renderer == graphics::RENDERER_OPENGL)
				context = (void *) glcontext;
#ifdef LOVE_GRAPHICS_METAL
			if (renderer == graphics::RENDERER_METAL && metalView)
				context = (void *) SDL_Metal_GetLayer(metalView);
#endif

			// TODO: try/catch
			graphics->setMode(context, (int) scaledw, (int) scaledh, pixelWidth, pixelHeight, f.stencil, f.depth, f.msaa);
		}
		else
		{
			graphics->backbufferChanged((int) scaledw, (int) scaledh, pixelWidth, pixelHeight, f.stencil, f.depth, f.msaa);
		}

		this->settings.msaa = graphics->getBackbufferMSAA();
	}

	// Set fullscreen when user requested it before.
	// See above for explanation.
#ifdef LOVE_ANDROID
	setFullscreen(fullscreen);
	love::android::setImmersive(fullscreen);
#endif

	SDL_SyncWindow(window);

	return true;
}

bool Window::onSizeChanged(int width, int height)
{
	if (!window)
		return false;

	SDL_GetWindowSize(window, &windowWidth, &windowHeight);

	if (!SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight))
	{
		pixelWidth = width;
		pixelHeight = height;
	}

	if (graphics.get())
	{
		double scaledw, scaledh;
		fromPixels((double) pixelWidth, (double) pixelHeight, scaledw, scaledh);
		graphics->backbufferChanged((int) scaledw, (int) scaledh, pixelWidth, pixelHeight);
	}

	return true;
}

void Window::updateSettings(const WindowSettings &newsettings, bool updateGraphicsViewport)
{
	SDL_SyncWindow(window);

	Uint32 wflags = SDL_GetWindowFlags(window);

	// Set the new display mode as the current display mode.
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);

	pixelWidth = windowWidth;
	pixelHeight = windowHeight;

	SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight);

	if (((wflags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN) && SDL_GetWindowFullscreenMode(window) == nullptr)
	{
		settings.fullscreen = true;
		settings.fstype = FULLSCREEN_DESKTOP;
	}
	else if ((wflags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
	{
		settings.fullscreen = true;
		settings.fstype = FULLSCREEN_EXCLUSIVE;
	}
	else
	{
		settings.fullscreen = false;
		settings.fstype = newsettings.fstype;
	}

#ifdef LOVE_ANDROID
	settings.fullscreen = love::android::getImmersive();
#endif

	// SDL_GetWindowMinimumSize gives back 0,0 sometimes...
	settings.minwidth = newsettings.minwidth;
	settings.minheight = newsettings.minheight;

	settings.resizable = (wflags & SDL_WINDOW_RESIZABLE) != 0;
	settings.borderless = (wflags & SDL_WINDOW_BORDERLESS) != 0;
	settings.centered = newsettings.centered;

	getPosition(settings.x, settings.y, settings.displayindex);

	setHighDPIAllowed((wflags & SDL_WINDOW_HIGH_PIXEL_DENSITY) != 0);

	settings.usedpiscale = newsettings.usedpiscale;

	// Only minimize on focus loss if the window is in exclusive-fullscreen mode
	if (settings.fullscreen && settings.fstype == FULLSCREEN_EXCLUSIVE)
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1");
	else
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	settings.vsync = getVSync();

	settings.stencil = newsettings.stencil;
	settings.depth = newsettings.depth;

	SDLDisplayIDs displayids;
	const SDL_DisplayMode *dmode = SDL_GetCurrentDisplayMode(displayids.ids[settings.displayindex]);

	// May be 0 if the refresh rate can't be determined.
	settings.refreshrate = dmode->refresh_rate;

	// Update the viewport size now instead of waiting for event polling.
	if (updateGraphicsViewport && graphics.get())
	{
		double scaledw, scaledh;
		fromPixels((double) pixelWidth, (double) pixelHeight, scaledw, scaledh);
		graphics->backbufferChanged((int) scaledw, (int) scaledh, pixelWidth, pixelHeight);
	}
}

void Window::getWindow(int &width, int &height, WindowSettings &newsettings)
{
	// The window might have been modified (moved, resized, etc.) by the user.
	if (window)
		updateSettings(settings, true);

	width = windowWidth;
	height = windowHeight;
	newsettings = settings;
}

void Window::close()
{
	close(true);
}

void Window::close(bool allowExceptions)
{
	if (graphics.get())
	{
		if (allowExceptions && graphics->isRenderTargetActive())
			throw love::Exception("love.window.close cannot be called while a render target is active in love.graphics.");

		graphics->unSetMode();
	}

	if (glcontext)
	{
		SDL_GL_DestroyContext(glcontext);
		glcontext = nullptr;
	}

#ifdef LOVE_GRAPHICS_METAL
	if (metalView)
	{
		SDL_Metal_DestroyView(metalView);
		metalView = nullptr;
	}
#endif

	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;

		// The old window may have generated pending events which are no longer
		// relevant. Destroy them all!
		SDL_FlushEvents(SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST);
	}

	open = false;
}

bool Window::setFullscreen(bool fullscreen, FullscreenType fstype)
{
	if (!window)
		return false;

	if (graphics.get() && graphics->isRenderTargetActive())
		throw love::Exception("love.window.setFullscreen cannot be called while a render target is active in love.graphics.");

	WindowSettings newsettings = settings;
	newsettings.fullscreen = fullscreen;
	newsettings.fstype = fstype;

	bool sdlflags = fullscreen;
	if (fullscreen)
	{
		if (fstype == FULLSCREEN_DESKTOP)
			SDL_SetWindowFullscreenMode(window, nullptr);
		else
		{
			SDL_DisplayID displayid = SDL_GetDisplayForWindow(window);
			SDL_DisplayMode mode = {};
			if (SDL_GetClosestFullscreenDisplayMode(displayid, windowWidth, windowHeight, 0, isHighDPIAllowed(), &mode))
				SDL_SetWindowFullscreenMode(window, &mode);
		}
	}

#ifdef LOVE_ANDROID
	love::android::setImmersive(fullscreen);
#endif

	if (SDL_SetWindowFullscreen(window, sdlflags))
	{
		if (glcontext)
			SDL_GL_MakeCurrent(window, glcontext);

		updateSettings(newsettings, true);
		return true;
	}

	return false;
}

bool Window::setFullscreen(bool fullscreen)
{
	return setFullscreen(fullscreen, settings.fstype);
}

int Window::getDisplayCount() const
{
	SDLDisplayIDs displayids;
	return displayids.count;
}

const char *Window::getDisplayName(int displayindex) const
{
	const char *name = SDL_GetDisplayName(GetSDLDisplayIDForIndex(displayindex));

	if (name == nullptr)
		throw love::Exception("Invalid display index: %d", displayindex + 1);

	return name;
}

Window::DisplayOrientation Window::getDisplayOrientation(int displayindex) const
{
	switch (SDL_GetCurrentDisplayOrientation(GetSDLDisplayIDForIndex(displayindex)))
	{
		case SDL_ORIENTATION_UNKNOWN: return ORIENTATION_UNKNOWN;
		case SDL_ORIENTATION_LANDSCAPE: return ORIENTATION_LANDSCAPE;
		case SDL_ORIENTATION_LANDSCAPE_FLIPPED: return ORIENTATION_LANDSCAPE_FLIPPED;
		case SDL_ORIENTATION_PORTRAIT: return ORIENTATION_PORTRAIT;
		case SDL_ORIENTATION_PORTRAIT_FLIPPED: return ORIENTATION_PORTRAIT_FLIPPED;
	}

	return ORIENTATION_UNKNOWN;
}

std::vector<Window::WindowSize> Window::getFullscreenSizes(int displayindex) const
{
	std::vector<WindowSize> sizes;

	int count = 0;
	SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(GetSDLDisplayIDForIndex(displayindex), &count);

	for (int i = 0; i < count; i++)
	{
		// TODO: other mode properties?
		WindowSize w = {modes[i]->w, modes[i]->h};

		// SDL2's display mode list has multiple entries for modes of the same
		// size with different bits per pixel, so we need to filter those out.
		if (std::find(sizes.begin(), sizes.end(), w) == sizes.end())
			sizes.push_back(w);
	}

	SDL_free(modes);

	return sizes;
}

void Window::getDesktopDimensions(int displayindex, int &width, int &height) const
{
	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(GetSDLDisplayIDForIndex(displayindex));
	if (mode != nullptr)
	{
		// TODO: other properties?
		width = mode->w;
		height = mode->h;
	}
	else
	{
		width = 0;
		height = 0;
	}
}

void Window::setPosition(int x, int y, int displayindex)
{
	if (!window)
		return;

	SDLDisplayIDs displayids;

	displayindex = std::min(std::max(displayindex, 0), displayids.count - 1);

	SDL_Rect displaybounds = {};
	SDL_GetDisplayBounds(displayids.ids[displayindex], &displaybounds);

	// The position needs to be in the global coordinate space.
	x += displaybounds.x;
	y += displaybounds.y;

	SDL_SetWindowPosition(window, x, y);
	SDL_SyncWindow(window);

	settings.useposition = true;
}

void Window::getPosition(int &x, int &y, int &displayindex)
{
	if (!window)
	{
		x = y =  0;
		displayindex = 0;
		return;
	}

	SDL_DisplayID displayid = SDL_GetDisplayForWindow(window);
	SDLDisplayIDs displayids;
	displayindex = 0;
	for (int i = 0; i < displayids.count; i++)
	{
		if (displayids.ids[i] == displayid)
		{
			displayindex = i;
			break;
		}
	}

	SDL_GetWindowPosition(window, &x, &y);

	// In SDL <= 2.0.3, fullscreen windows are always reported as 0,0. In every
	// other case we need to convert the position from global coordinates to the
	// monitor's coordinate space.
	if (x != 0 || y != 0)
	{
		SDL_Rect displaybounds = {};
		SDL_GetDisplayBounds(displayid, &displaybounds);

		x -= displaybounds.x;
		y -= displaybounds.y;
	}
}

Rect Window::getSafeArea() const
{
#if defined(LOVE_IOS)
	if (window != nullptr)
		return love::ios::getSafeArea(window);
#elif defined(LOVE_ANDROID)
	if (window != nullptr)
	{
		int top, left, bottom, right;

		if (love::android::getSafeArea(top, left, bottom, right))
		{
			// DisplayCutout API returns safe area in pixels
			// and is affected by display orientation.
			double safeLeft, safeTop, safeWidth, safeHeight;
			fromPixels(left, top, safeLeft, safeTop);
			fromPixels(pixelWidth - left - right, pixelHeight - top - bottom, safeWidth, safeHeight);
			return {(int) safeLeft, (int) safeTop, (int) safeWidth, (int) safeHeight};
		}
	}
#endif

	double dw, dh;
	fromPixels(pixelWidth, pixelHeight, dw, dh);
	return {0, 0, (int) dw, (int) dh};
}

bool Window::isOpen() const
{
	return open;
}

void Window::setWindowTitle(const std::string &title)
{
	this->title = title;

	if (window)
		SDL_SetWindowTitle(window, title.c_str());
}

const std::string &Window::getWindowTitle() const
{
	return title;
}

bool Window::setIcon(love::image::ImageData *imgd)
{
	if (!imgd)
		return false;

	if (imgd->getFormat() != PIXELFORMAT_RGBA8_UNORM)
		throw love::Exception("setIcon only accepts 32-bit RGBA images.");

	icon.set(imgd);

	if (!window)
		return false;

	int w = imgd->getWidth();
	int h = imgd->getHeight();
	int bytesperpixel = (int) getPixelFormatBlockSize(imgd->getFormat());
	int pitch = w * bytesperpixel;

	SDL_Surface *sdlicon = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ABGR8888, imgd->getData(), pitch);

	if (!sdlicon)
		return false;

	SDL_SetWindowIcon(window, sdlicon);
	SDL_DestroySurface(sdlicon);

	return true;
}

love::image::ImageData *Window::getIcon()
{
	return icon.get();
}

void Window::setVSync(int vsync)
{
	if (glcontext != nullptr)
	{
		SDL_GL_SetSwapInterval(vsync);

		// Check if adaptive vsync was requested but not supported, and fall
		// back to regular vsync if so.
		if (vsync == -1)
		{
			int actualvsync = 0;
			SDL_GL_GetSwapInterval(&actualvsync);
			if (actualvsync != -1)
				SDL_GL_SetSwapInterval(1);
		}
	}

#ifdef LOVE_GRAPHICS_VULKAN
	if (windowRenderer == love::graphics::RENDERER_VULKAN)
	{
		auto vgfx = dynamic_cast<love::graphics::vulkan::Graphics*>(graphics.get());
		vgfx->setVsync(vsync);
	}
#endif

#if defined(LOVE_GRAPHICS_METAL) && defined(LOVE_MACOS)
	if (metalView != nullptr)
	{
		void *metallayer = SDL_Metal_GetLayer(metalView);
		love::macos::setMetalLayerVSync(metallayer, vsync != 0);
	}
#endif
}

int Window::getVSync() const
{
	if (glcontext != nullptr)
	{
		int interval = 0;
		SDL_GL_GetSwapInterval(&interval);
		return interval;
	}

#if defined(LOVE_GRAPHICS_METAL)
	if (metalView != nullptr)
	{
#ifdef LOVE_MACOS
		void *metallayer = SDL_Metal_GetLayer(metalView);
		return love::macos::getMetalLayerVSync(metallayer) ? 1 : 0;
#else
		return 1;
#endif
	}
#endif

#ifdef LOVE_GRAPHICS_VULKAN
	if (windowRenderer == love::graphics::RENDERER_VULKAN)
	{
		auto vgfx = dynamic_cast<love::graphics::vulkan::Graphics*>(graphics.get());
		return vgfx->getVsync();
	}
#endif

	return 0;
}

void Window::setDisplaySleepEnabled(bool enable)
{
	if (enable)
		SDL_EnableScreenSaver();
	else
		SDL_DisableScreenSaver();
}

bool Window::isDisplaySleepEnabled() const
{
	return SDL_ScreenSaverEnabled();
}

void Window::minimize()
{
	if (window != nullptr)
	{
		SDL_MinimizeWindow(window);
		updateSettings(settings, true);
	}
}

void Window::maximize()
{
	if (window != nullptr)
	{
		SDL_MaximizeWindow(window);
		updateSettings(settings, true);
	}
}

void Window::restore()
{
	if (window != nullptr)
	{
		SDL_RestoreWindow(window);
		updateSettings(settings, true);
	}
}

void Window::focus()
{
	if (window != nullptr)
	{
		SDL_RaiseWindow(window);
		updateSettings(settings, true);
	}
}

bool Window::isMaximized() const
{
	return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED);
}

bool Window::isMinimized() const
{
	return window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED);
}

void Window::swapBuffers()
{
	if (glcontext)
	{
#ifdef LOVE_WINDOWS
		bool useDwmFlush = false;
		int swapInterval = getVSync();

		// https://github.com/love2d/love/issues/1628
		// VSync can interact badly with Windows desktop composition (DWM) in windowed mode. DwmFlush can be used instead
		// of vsync, but it's much less flexible so we're very conservative here with where it's used:
		// - It won't work with exclusive or desktop fullscreen.
		// - DWM refreshes don't always match the refresh rate of the monitor the window is in (or the requested swap
		//   interval), so we only use it when they do match.
		// - The user may force GL vsync, and DwmFlush shouldn't be used together with GL vsync.
		if (canUseDwmFlush && !settings.fullscreen && swapInterval == 1)
		{
			// Desktop composition is always enabled in Windows 8+. But DwmIsCompositionEnabled won't always return true...
			// (see DwmIsCompositionEnabled docs).
			BOOL compositionEnabled = IsWindows8OrGreater();
			if (compositionEnabled || (SUCCEEDED(DwmIsCompositionEnabled(&compositionEnabled)) && compositionEnabled))
			{
				DWM_TIMING_INFO info = {};
				info.cbSize = sizeof(DWM_TIMING_INFO);
				double dwmRefreshRate = 0;
				if (SUCCEEDED(DwmGetCompositionTimingInfo(nullptr, &info)))
					dwmRefreshRate = (double)info.rateRefresh.uiNumerator / (double)info.rateRefresh.uiDenominator;

				SDL_DisplayMode dmode = {};
				SDL_DisplayID display = SDL_GetDisplayForWindow(window);
				const SDL_DisplayMode* modePtr = SDL_GetCurrentDisplayMode(display);
				if (modePtr)
					dmode = *modePtr;

				if (dmode.refresh_rate > 0 && dwmRefreshRate > 0 && (fabs(dmode.refresh_rate - dwmRefreshRate) < 2))
				{
					SDL_GL_SetSwapInterval(0);
					int interval = 0;
					if (SDL_GL_GetSwapInterval(&interval) == 0 && interval == 0)
						useDwmFlush = true;
					else
						SDL_GL_SetSwapInterval(swapInterval);
				}
			}
		}
#endif

		SDL_GL_SwapWindow(window);

#ifdef LOVE_WINDOWS
		if (useDwmFlush)
		{
			DwmFlush();
			SDL_GL_SetSwapInterval(swapInterval);
		}
#endif
	}
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
	return window && (SDL_GetWindowFlags(window) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) == 0;
}

bool Window::isOccluded() const
{
	return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_OCCLUDED) != 0;
}

void Window::setMouseGrab(bool grab)
{
	mouseGrabbed = grab;
	if (window)
		SDL_SetWindowMouseGrab(window, grab);
}

bool Window::isMouseGrabbed() const
{
	if (window)
		return SDL_GetWindowMouseGrab(window);
	else
		return mouseGrabbed;
}

int Window::getWidth() const
{
	return windowWidth;
}

int Window::getHeight() const
{
	return windowHeight;
}

int Window::getPixelWidth() const
{
	return pixelWidth;
}

int Window::getPixelHeight() const
{
	return pixelHeight;
}

void Window::clampPositionInWindow(double *wx, double *wy) const
{
	if (wx != nullptr)
		*wx = std::min(std::max(0.0, *wx), (double) getWidth() - 1);
	if (wy != nullptr)
		*wy = std::min(std::max(0.0, *wy), (double) getHeight() - 1);
}

void Window::windowToPixelCoords(double *x, double *y) const
{
	if (x != nullptr)
		*x = (*x) * ((double) pixelWidth / (double) windowWidth);
	if (y != nullptr)
		*y = (*y) * ((double) pixelHeight / (double) windowHeight);
}

void Window::pixelToWindowCoords(double *x, double *y) const
{
	if (x != nullptr)
		*x = (*x) * ((double) windowWidth / (double) pixelWidth);
	if (y != nullptr)
		*y = (*y) * ((double) windowHeight / (double) pixelHeight);
}

void Window::windowToDPICoords(double *x, double *y) const
{
	double px = x != nullptr ? *x : 0.0;
	double py = y != nullptr ? *y : 0.0;

	windowToPixelCoords(&px, &py);

	double dpix = 0.0;
	double dpiy = 0.0;

	fromPixels(px, py, dpix, dpiy);

	if (x != nullptr)
		*x = dpix;
	if (y != nullptr)
		*y = dpiy;
}

void Window::DPIToWindowCoords(double *x, double *y) const
{
	double dpix = x != nullptr ? *x : 0.0;
	double dpiy = y != nullptr ? *y : 0.0;

	double px = 0.0;
	double py = 0.0;

	toPixels(dpix, dpiy, px, py);
	pixelToWindowCoords(&px, &py);

	if (x != nullptr)
		*x = px;
	if (y != nullptr)
		*y = py;
}

double Window::getDPIScale() const
{
	return settings.usedpiscale ? getNativeDPIScale() : 1.0;
}

double Window::getNativeDPIScale() const
{
#ifdef LOVE_ANDROID
	return love::android::getScreenScale();
#else
	return window != nullptr ? SDL_GetWindowDisplayScale(window) : 1.0;
#endif
}

double Window::toPixels(double x) const
{
	return x * getDPIScale();
}

void Window::toPixels(double wx, double wy, double &px, double &py) const
{
	double scale = getDPIScale();
	px = wx * scale;
	py = wy * scale;
}

double Window::fromPixels(double x) const
{
	return x / getDPIScale();
}

void Window::fromPixels(double px, double py, double &wx, double &wy) const
{
	double scale = getDPIScale();
	wx = px / scale;
	wy = py / scale;
}

void *Window::getHandle() const
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

bool Window::showMessageBox(const std::string &title, const std::string &message, MessageBoxType type, bool attachtowindow)
{
	SDL_MessageBoxFlags flags = convertMessageBoxType(type);
	SDL_Window *sdlwindow = attachtowindow ? window : nullptr;

	return SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), sdlwindow);
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

	for (int i = 0; i < (int) data.buttons.size(); i++)
	{
		SDL_MessageBoxButtonData sdlbutton = {};

		sdlbutton.buttonID = i;
		sdlbutton.text = data.buttons[i].c_str();

		if (i == data.enterButtonIndex)
			sdlbutton.flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;

		if (i == data.escapeButtonIndex)
			sdlbutton.flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;

		sdlbuttons.push_back(sdlbutton);
	}

	sdldata.buttons = &sdlbuttons[0];

	int pressedbutton = -2;
	SDL_ShowMessageBox(&sdldata, &pressedbutton);

	return pressedbutton;
}

// As of a SDL3 prerelease, a lot of SDL file dialog parameters need to persist
// until the callback completes, so we store them here.
// This is also used to retrieve some useful info to pass to love's own callback,
// and to send that along to SDL events (see below).
class FileDialogState : public love::Object
{
public:

	void *context;
	Window::FileDialogCallback callback;
	Uint32 dialogEventId;
	Window::FileDialogData data;
	std::vector<SDL_DialogFileFilter> sdlFilters;
	SDL_PropertiesID props;

	Optional<std::string> err;
	std::vector<std::string> files;
	int filterIndex;
};

static void SDLCALL fileDialogCallbackSDL(void *userdata, const char *const *filelist, int filter)
{
	auto state = (FileDialogState *) userdata;
	if (state == nullptr)
		return;

	auto fs = Module::getInstance<filesystem::Filesystem>(Module::M_FILESYSTEM);

	if (filelist != nullptr)
	{
		// SDL's file list only lasts until the end of the callback, so we copy it.
		for (int i = 0; filelist[i] != nullptr; i++)
			state->files.push_back(filelist[i]);
	}
	else
	{
		state->err.set(SDL_GetError());
	}

	state->filterIndex = filter;

	SDL_DestroyProperties(state->props);

	// The SDL dialog callback isn't guaranteed to be called on the main thread,
	// whereas SDL event polling will happen there. This is needed because Lua states
	// aren't thread safe.
	SDL_Event event = {};
	event.type = state->dialogEventId;
	event.user.data1 = state;

	SDL_PushEvent(&event);
}

void Window::handleSDLEvent(const SDL_Event &event)
{
	if (event.type == dialogEventId)
	{
		// Releases itself when it goes out of scope.
		StrongRef<FileDialogState> state((FileDialogState *) event.user.data1, Acquire::NORETAIN);

		const char *filtername = state->filterIndex >= 0
			? state->data.filters[state->filterIndex].name.c_str()
			: nullptr;

		state->callback(state->context, state->files, filtername, state->err.hasValue ? state->err.value.c_str() : nullptr);
	}
}

void Window::showFileDialog(const FileDialogData &data, FileDialogCallback callback, void *context)
{
	SDL_FileDialogType sdltype = SDL_FILEDIALOG_OPENFILE;
	switch (data.type)
	{
	case FILEDIALOG_OPENFILE:
	default:
		sdltype = SDL_FILEDIALOG_OPENFILE;
		break;
	case FILEDIALOG_OPENFOLDER:
		sdltype = SDL_FILEDIALOG_OPENFOLDER;
		break;
	case FILEDIALOG_SAVEFILE:
		sdltype = SDL_FILEDIALOG_SAVEFILE;
		break;
	}

	auto state = new FileDialogState();
	state->callback = callback;
	state->context = context;
	state->dialogEventId = dialogEventId;
	state->data = data;

	for (const auto &filter : state->data.filters)
	{
		SDL_DialogFileFilter f = {};
		f.name = filter.name.c_str();
		f.pattern = filter.pattern.c_str();
		state->sdlFilters.push_back(f);
	}

	// We destroy this in the dialog callback, since it needs to persist until then (until that's fixed in SDL code).
	state->props = SDL_CreateProperties();

	if (!data.title.empty())
		SDL_SetStringProperty(state->props, SDL_PROP_FILE_DIALOG_TITLE_STRING, state->data.title.c_str());

	if (!data.acceptLabel.empty())
		SDL_SetStringProperty(state->props, SDL_PROP_FILE_DIALOG_ACCEPT_STRING, state->data.acceptLabel.c_str());

	if (!data.cancelLabel.empty())
		SDL_SetStringProperty(state->props, SDL_PROP_FILE_DIALOG_CANCEL_STRING, state->data.cancelLabel.c_str());

	if (!data.defaultName.empty())
		SDL_SetStringProperty(state->props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, state->data.defaultName.c_str());

	if (data.attachToWindow)
		SDL_SetPointerProperty(state->props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, window);

	if (!state->sdlFilters.empty())
	{
		SDL_SetPointerProperty(state->props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, state->sdlFilters.data());
		SDL_SetNumberProperty(state->props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, state->sdlFilters.size());
	}

	SDL_SetBooleanProperty(state->props, SDL_PROP_FILE_DIALOG_MANY_BOOLEAN, data.multiSelect);

	SDL_ShowFileDialogWithProperties(sdltype, fileDialogCallbackSDL, state, state->props);
}

void Window::requestAttention(bool continuous)
{
#if defined(LOVE_WINDOWS) && !defined(LOVE_WINDOWS_UWP)

	if (hasFocus())
		return;

	FLASHWINFO flashinfo = { sizeof(FLASHWINFO) };

	flashinfo.hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	flashinfo.uCount = 1;
	flashinfo.dwFlags = FLASHW_ALL;

	if (continuous)
	{
		flashinfo.uCount = 0;
		flashinfo.dwFlags |= FLASHW_TIMERNOFG;
	}

	FlashWindowEx(&flashinfo);

#elif defined(LOVE_MACOS)

	love::macos::requestAttention(continuous);

#else

	LOVE_UNUSED(continuous);
	
#endif
	
	// TODO: Linux?
}

} // sdl
} // window
} // love
