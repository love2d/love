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

#ifndef LOVE_WINDOW_WINDOW_H
#define LOVE_WINDOW_WINDOW_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"
#include "image/ImageData.h"

// C++
#include <string>
#include <vector>

namespace love
{
namespace window
{

// Forward-declared so it can be used in the class methods. We can't define the
// whole thing here because it uses the Window::Type enum.
struct WindowFlags;

class Window : public Module
{
public:

	enum FullscreenType
	{
		FULLSCREEN_TYPE_NORMAL,
		FULLSCREEN_TYPE_DESKTOP,
		FULLSCREEN_TYPE_MAX_ENUM
	};

	struct WindowSize
	{
		int width;
		int height;
	};

	virtual ~Window();

	virtual bool setWindow(int width = 800, int height = 600, WindowFlags *flags = 0) = 0;
	virtual void getWindow(int &width, int &height, WindowFlags &flags) = 0;

	virtual bool setFullscreen(bool fullscreen, FullscreenType fstype) = 0;
	virtual bool setFullscreen(bool fullscreen) = 0;

	virtual bool onWindowResize(int width, int height) = 0;

	virtual int getDisplayCount() const = 0;

	virtual std::vector<WindowSize> getFullscreenSizes(int displayindex) const = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	virtual void getDesktopDimensions(int displayindex, int &width, int &height) const = 0;

	virtual bool isCreated() const = 0;

	virtual void setWindowTitle(const std::string &title) = 0;
	virtual const std::string &getWindowTitle() const = 0;

	virtual bool setIcon(love::image::ImageData *imgd) = 0;
	virtual love::image::ImageData *getIcon() = 0;

	// default no-op implementation
	virtual void swapBuffers();

	virtual bool hasFocus() const = 0;
	virtual bool hasMouseFocus() const = 0;

	virtual bool isVisible() const = 0;

	virtual void setMouseVisible(bool visible) = 0;
	virtual bool getMouseVisible() const = 0;

	virtual void setMouseGrab(bool grab) = 0;
	virtual bool isMouseGrabbed() const = 0;

	virtual const void *getHandle() const = 0;

	//virtual static Window *createSingleton() = 0;
	//virtual static Window *getSingleton() = 0;
	// No virtual statics, of course, but you are supposed to implement these statics.

	static bool getConstant(const char *in, FullscreenType &out);
	static bool getConstant(FullscreenType in, const char *&out);

protected:

	static Window *singleton;

private:

	static StringMap<FullscreenType, FULLSCREEN_TYPE_MAX_ENUM>::Entry fullscreenTypeEntries[];
	static StringMap<FullscreenType, FULLSCREEN_TYPE_MAX_ENUM> fullscreenTypes;

}; // Window

struct WindowFlags
{
	WindowFlags();
	bool fullscreen; // = false
	Window::FullscreenType fstype; // = FULLSCREEN_TYPE_NORMAL
	bool vsync; // = true
	int fsaa; // = 0
	bool resizable; // = false
	int minwidth; // = 0
	int minheight; // = 0
	bool borderless; // = false
	bool centered; // = true
	int display; // = 0
}; // WindowFlags

} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
