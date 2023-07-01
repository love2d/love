/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

class Window final : public love::window::Window
{
public:

	Window();
	~Window();

	void setGraphics(graphics::Graphics *graphics) override;

	bool setWindow(int width = 800, int height = 600, WindowSettings *settings = nullptr) override;
	void getWindow(int &width, int &height, WindowSettings &settings) override;

	void close() override;

	bool setFullscreen(bool fullscreen, FullscreenType fstype) override;
	bool setFullscreen(bool fullscreen) override;

	bool onSizeChanged(int width, int height) override;

	int getDisplayCount() const override;

	const char *getDisplayName(int displayindex) const override;

	DisplayOrientation getDisplayOrientation(int displayindex) const override;

	std::vector<WindowSize> getFullscreenSizes(int displayindex) const override;

	void getDesktopDimensions(int displayindex, int &width, int &height) const override;

	void setPosition(int x, int y, int displayindex) override;
	void getPosition(int &x, int &y, int &displayindex) override;

	Rect getSafeArea() const override;

	bool isOpen() const override;

	void setWindowTitle(const std::string &title) override;
	const std::string &getWindowTitle() const override;

	bool setIcon(love::image::ImageData *imgd) override;
	love::image::ImageData *getIcon() override;

	void setVSync(int vsync) override;
	int getVSync() const override;

	void setDisplaySleepEnabled(bool enable) override;
	bool isDisplaySleepEnabled() const override;

	void minimize() override;
	void maximize() override;
	void restore() override;

	bool isMaximized() const override;
	bool isMinimized() const override;

	void swapBuffers() override;

	bool hasFocus() const override;
	bool hasMouseFocus() const override;

	bool isVisible() const override;

	void setMouseGrab(bool grab) override;
	bool isMouseGrabbed() const override;

	int getWidth() const override;
	int getHeight() const override;
	int getPixelWidth() const override;
	int getPixelHeight() const override;

	void clampPositionInWindow(double *wx, double *wy) const override;

	void windowToPixelCoords(double *x, double *y) const override;
	void pixelToWindowCoords(double *x, double *y) const override;

	void windowToDPICoords(double *x, double *y) const override;
	void DPIToWindowCoords(double *x, double *y) const override;

	double getDPIScale() const override;
	double getNativeDPIScale() const override;

	double toPixels(double x) const override;
	void toPixels(double wx, double wy, double &px, double &py) const override;
	double fromPixels(double x) const override;
	void fromPixels(double px, double py, double &wx, double &wy) const override;

	const void *getHandle() const override;

	bool showMessageBox(const std::string &title, const std::string &message, MessageBoxType type, bool attachtowindow) override;
	int showMessageBox(const MessageBoxData &data) override;

	void requestAttention(bool continuous) override;

	const char *getName() const override;

private:

	void close(bool allowExceptions);

	struct ContextAttribs
	{
		int versionMajor;
		int versionMinor;
		bool gles;
		bool debug;
	};

	void setGLFramebufferAttributes(int msaa, bool sRGB, bool stencil, int depth);
	void setGLContextAttributes(const ContextAttribs &attribs);
	bool checkGLVersion(const ContextAttribs &attribs, std::string &outversion);
	std::vector<ContextAttribs> getContextAttribsList() const;
	bool createWindowAndContext(int x, int y, int w, int h, Uint32 windowflags, int msaa, bool stencil, int depth);

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
	ContextAttribs contextAttribs;

	StrongRef<graphics::Graphics> graphics;

}; // Window

} // sdl
} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
