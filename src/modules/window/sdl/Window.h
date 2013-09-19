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

#ifndef LOVE_WINDOW_SDL_WINDOW_H
#define LOVE_WINDOW_SDL_WINDOW_H

// LOVE
#include "window/Window.h"

// SDL
#include <SDL.h>

namespace love
{
namespace window
{
namespace sdl
{

class Window : public love::window::Window
{
public:

	Window();
	~Window();

	bool setWindow(int width = 800, int height = 600, WindowFlags *flags = 0);
	void getWindow(int &width, int &height, WindowFlags &flags);

	bool setFullscreen(bool fullscreen, FullscreenType fstype);
	bool setFullscreen(bool fullscreen);

	bool onWindowResize(int width, int height);

	int getDisplayCount() const;

	std::vector<WindowSize> getFullscreenSizes(int displayindex) const;

	int getWidth() const;
	int getHeight() const;

	void getDesktopDimensions(int displayindex, int &width, int &height) const;

	bool isCreated() const;

	void setWindowTitle(const std::string &title);
	const std::string &getWindowTitle() const;

	bool setIcon(love::image::ImageData *imgd);
	love::image::ImageData *getIcon();

	void swapBuffers();

	bool hasFocus() const;
	bool hasMouseFocus() const;

	bool isVisible() const;

	void setMouseVisible(bool visible);
	bool getMouseVisible() const;

	void setMouseGrab(bool grab);
	bool isMouseGrabbed() const;

	const void *getHandle() const;

	static love::window::Window *createSingleton();
	static love::window::Window *getSingleton();

	const char *getName() const;

private:

	bool setContext(int fsaa, bool vsync);
	void setWindowGLAttributes(int fsaa) const;

	// Update the window flags based on the window's actual state.
	void updateWindowFlags(const WindowFlags &newflags);

	std::string windowTitle;

	struct _currentMode
	{
		_currentMode();

		int width;
		int height;
		WindowFlags flags;
		love::image::ImageData *icon;

	} curMode;

	bool created;

	bool mouseGrabbed;

	SDL_Window *window;
	SDL_GLContext context;

}; // Window

} // sdl
} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
