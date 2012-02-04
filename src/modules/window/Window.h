/**
* Copyright (c) 2006-2012 LOVE Development Team
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

// STL
#include <string>

// LOVE
#include <common/Module.h>
#include <image/ImageData.h>

namespace love
{
namespace window
{
	class Window : public Module
	{
	protected:
		static Window *singleton;

	public:
		struct WindowSize
		{
			int width;
			int height;
		};

		virtual ~Window();

		virtual bool setWindow(int width = 800, int height = 600, bool fullscreen = false, bool vsync = true, int fsaa = 0) = 0;
		virtual void getWindow(int &width, int &height, bool &fullscreen, bool &vsync, int &fsaa) = 0;

		virtual bool checkWindowSize(int width, int height, bool fullscreen) = 0;
		virtual WindowSize **getFullscreenSizes(int &n) = 0;

		virtual int getWidth() = 0;
		virtual int getHeight() = 0;

		virtual bool isCreated() = 0;

		virtual void setWindowTitle(std::string &title) = 0;
		virtual std::string getWindowTitle() = 0;

		virtual bool setIcon(love::image::ImageData *imgd) = 0;

		// default no-op implementation
		virtual void swapBuffers();

		virtual bool hasFocus() = 0;
		virtual void setMouseVisible(bool visible) = 0;
		virtual bool getMouseVisible() = 0;

		//virtual static Window *getSingleton() = 0;
		// No virtual statics, of course, but you are supposed to implement this static.
	}; // Window
} // window
} // love

#endif // LOVE_WINDOW_WINDOW_H
