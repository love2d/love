/**
 * Copyright (c) 2006-2025 LOVE Development Team
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

#ifndef LOVE_WINDOW_WINDOW_H
#define LOVE_WINDOW_WINDOW_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"
#include "common/math.h"
#include "common/Optional.h"
#include "image/ImageData.h"

// C++
#include <string>
#include <vector>

namespace love
{

namespace graphics
{
class Graphics;
}

namespace window
{

// Applied when the window is first created.
void setHighDPIAllowed(bool enable);
bool isHighDPIAllowed();

// Forward-declared so it can be used in the class methods. We can't define the
// whole thing here because it uses the Window::Type enum.
struct WindowSettings;

class Window : public Module
{
public:

	typedef void (*FileDialogCallback)(void *context, const std::vector<std::string> &files, const char *filtername, const char *err);

	// Different window settings.
	enum Setting
	{
		SETTING_FULLSCREEN,
		SETTING_FULLSCREEN_TYPE,
		SETTING_VSYNC,
		SETTING_MSAA,
		SETTING_STENCIL,
		SETTING_DEPTH,
		SETTING_RESIZABLE,
		SETTING_MIN_WIDTH,
		SETTING_MIN_HEIGHT,
		SETTING_BORDERLESS,
		SETTING_CENTERED,
		SETTING_DISPLAYINDEX,
		SETTING_DISPLAY, // Deprecated
		SETTING_HIGHDPI, // Deprecated
		SETTING_USE_DPISCALE,
		SETTING_HIDEHOMEINDICATOR,
		SETTING_REFRESHRATE,
		SETTING_X,
		SETTING_Y,
		SETTING_MAX_ENUM
	};

	enum FullscreenType
	{
		FULLSCREEN_EXCLUSIVE,
		FULLSCREEN_DESKTOP,
		FULLSCREEN_MAX_ENUM
	};

	enum MessageBoxType
	{
		MESSAGEBOX_ERROR,
		MESSAGEBOX_WARNING,
		MESSAGEBOX_INFO,
		MESSAGEBOX_MAX_ENUM
	};

	enum FileDialogType
	{
		FILEDIALOG_OPENFILE,
		FILEDIALOG_OPENFOLDER,
		FILEDIALOG_SAVEFILE,
		FILEDIALOG_MAX_ENUM
	};

	enum DisplayOrientation
	{
		ORIENTATION_UNKNOWN,
		ORIENTATION_LANDSCAPE,
		ORIENTATION_LANDSCAPE_FLIPPED,
		ORIENTATION_PORTRAIT,
		ORIENTATION_PORTRAIT_FLIPPED,
		ORIENTATION_MAX_ENUM
	};

	struct WindowSize
	{
		int width;
		int height;

		bool operator == (const WindowSize &w) const
		{
			return w.width == width && w.height == height;
		}
	};

	struct MessageBoxData
	{
		MessageBoxType type;

		std::string title;
		std::string message;

		std::vector<std::string> buttons;
		int enterButtonIndex;
		int escapeButtonIndex;

		bool attachToWindow;
	};

	struct FileDialogFilter
	{
		std::string name;
		std::string pattern;
	};

	struct FileDialogData
	{
		FileDialogType type;
		std::string title;
		std::string acceptLabel;
		std::string cancelLabel;
		std::string defaultName;
		std::vector<FileDialogFilter> filters;
		bool multiSelect;
		bool attachToWindow;
	};

	virtual ~Window();

	virtual void setGraphics(graphics::Graphics *graphics) = 0;

	virtual bool setWindow(int width = 800, int height = 600, WindowSettings *settings = nullptr) = 0;
	virtual void getWindow(int &width, int &height, WindowSettings &settings) = 0;

	virtual void close() = 0;

	virtual bool setFullscreen(bool fullscreen, FullscreenType fstype) = 0;
	virtual bool setFullscreen(bool fullscreen) = 0;

	virtual bool onSizeChanged(int width, int height) = 0;

	virtual int getDisplayCount() const = 0;

	virtual const char *getDisplayName(int displayindex) const = 0;

	virtual DisplayOrientation getDisplayOrientation(int displayindex) const = 0;

	virtual std::vector<WindowSize> getFullscreenSizes(int displayindex) const = 0;

	virtual void getDesktopDimensions(int displayindex, int &width, int &height) const = 0;

	virtual void setPosition(int x, int y, int displayindex) = 0;
	virtual void getPosition(int &x, int &y, int &displayindex) = 0;

	virtual Rect getSafeArea() const = 0;

	virtual bool isOpen() const = 0;

	virtual void setWindowTitle(const std::string &title) = 0;
	virtual const std::string &getWindowTitle() const = 0;

	virtual bool setIcon(love::image::ImageData *imgd) = 0;
	virtual love::image::ImageData *getIcon() = 0;

	virtual void setVSync(int vsync) = 0;
	virtual int getVSync() const = 0;

	virtual void setDisplaySleepEnabled(bool enable) = 0;
	virtual bool isDisplaySleepEnabled() const = 0;

	virtual void minimize() = 0;
	virtual void maximize() = 0;
	virtual void restore() = 0;
	virtual void focus() = 0;

	virtual bool isMaximized() const = 0;
	virtual bool isMinimized() const = 0;

	// default no-op implementation
	virtual void swapBuffers();

	virtual bool hasFocus() const = 0;
	virtual bool hasMouseFocus() const = 0;

	virtual bool isVisible() const = 0;
	virtual bool isOccluded() const = 0;

	virtual void setMouseGrab(bool grab) = 0;
	virtual bool isMouseGrabbed() const = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual int getPixelWidth() const = 0;
	virtual int getPixelHeight() const = 0;

	virtual void clampPositionInWindow(double *wx, double *wy) const = 0;

	// Note: window-space coordinates are not necessarily the same as
	// density-independent units (which toPixels and fromPixels use.)
	virtual void windowToPixelCoords(double *x, double *y) const = 0;
	virtual void pixelToWindowCoords(double *x, double *y) const = 0;

	virtual void windowToDPICoords(double *x, double *y) const = 0;
	virtual void DPIToWindowCoords(double *x, double *y) const = 0;

	virtual double getDPIScale() const = 0;
	virtual double getNativeDPIScale() const = 0;

	virtual double toPixels(double x) const = 0;
	virtual void toPixels(double wx, double wy, double &px, double &py) const = 0;
	virtual double fromPixels(double x) const = 0;
	virtual void fromPixels(double px, double py, double &wx, double &wy) const = 0;

	virtual void *getHandle() const = 0;

	virtual bool showMessageBox(const std::string &title, const std::string &message, MessageBoxType type, bool attachtowindow) = 0;
	virtual int showMessageBox(const MessageBoxData &data) = 0;

	virtual void showFileDialog(const FileDialogData &data, FileDialogCallback callback, void *context) = 0;

	virtual void requestAttention(bool continuous) = 0;

	STRINGMAP_CLASS_DECLARE(Setting);
	STRINGMAP_CLASS_DECLARE(FullscreenType);
	STRINGMAP_CLASS_DECLARE(MessageBoxType);
	STRINGMAP_CLASS_DECLARE(FileDialogType);
	STRINGMAP_CLASS_DECLARE(DisplayOrientation);

protected:

	Window(const char *name);

}; // Window

struct WindowSettings
{
	bool fullscreen = false;
	Window::FullscreenType fstype = Window::FULLSCREEN_DESKTOP;
	int vsync = 1;
	int msaa = 0;
	bool stencil = true;
	bool depth = false;
	bool resizable = false;
	int minwidth = 1;
	int minheight = 1;
	bool borderless = false;
	bool centered = true;
	int displayindex = 0;
	bool usedpiscale = true;
	char *hidehomeindicator = "0";
	double refreshrate = 0.0;
	bool useposition = false;
	int x = 0;
	int y = 0;
};

} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
