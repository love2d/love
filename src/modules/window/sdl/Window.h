/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

	bool setWindow(int width = 800, int height = 600, WindowSettings *settings = nullptr);
	void getWindow(int &width, int &height, WindowSettings &settings);

	void close();

	bool setFullscreen(bool fullscreen, FullscreenType fstype);
	bool setFullscreen(bool fullscreen);

	bool onSizeChanged(int width, int height);

	int getDisplayCount() const;

	const char *getDisplayName(int displayindex) const;

	std::vector<WindowSize> getFullscreenSizes(int displayindex) const;

	void getDesktopDimensions(int displayindex, int &width, int &height) const;

	void setPosition(int x, int y, int displayindex);
	void getPosition(int &x, int &y, int &displayindex);

	bool isOpen() const;

	void setWindowTitle(const std::string &title);
	const std::string &getWindowTitle() const;

	bool setIcon(love::image::ImageData *imgd);
	love::image::ImageData *getIcon();

	void setDisplaySleepEnabled(bool enable);
	bool isDisplaySleepEnabled() const;

	void minimize();
	void maximize();

	bool isMaximized() const;

	void swapBuffers();

	bool hasFocus() const;
	bool hasMouseFocus() const;

	bool isVisible() const;

	void setMouseGrab(bool grab);
	bool isMouseGrabbed() const;

	void getPixelDimensions(int &w, int &h) const;
	void windowToPixelCoords(double *x, double *y) const;
	void pixelToWindowCoords(double *x, double *y) const;

	double getPixelScale() const;

	double toPixels(double x) const;
	void toPixels(double wx, double wy, double &px, double &py) const;
	double fromPixels(double x) const;
	void fromPixels(double px, double py, double &wx, double &wy) const;

	const void *getHandle() const;

	bool showMessageBox(const std::string &title, const std::string &message, MessageBoxType type, bool attachtowindow);
	int showMessageBox(const MessageBoxData &data);

	void requestAttention(bool continuous);

	const char *getName() const;

private:

	struct ContextAttribs
	{
		int versionMajor;
		int versionMinor;
		bool gles;
		bool debug;
	};

	void setGLFramebufferAttributes(int msaa, bool sRGB);
	void setGLContextAttributes(const ContextAttribs &attribs);
	bool checkGLVersion(const ContextAttribs &attribs, std::string &outversion);
	bool createWindowAndContext(int x, int y, int w, int h, Uint32 windowflags, int msaa);

	// Update the saved window settings based on the window's actual state.
	void updateSettings(const WindowSettings &newsettings, bool updateGraphicsViewport);

	SDL_MessageBoxFlags convertMessageBoxType(MessageBoxType type) const;

	std::string title;

	int windowWidth  = 800;
	int windowHeight = 600;
	int pixelWidth   = 800;
	int pixelHeight  = 600;
	WindowSettings settings;
	StrongRef<love::image::ImageData> icon;

	bool open;

	bool mouseGrabbed;

	SDL_Window *window;
	SDL_GLContext context;

	bool displayedWindowError;
	bool hasSDL203orEarlier;

}; // Window

} // sdl
} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
