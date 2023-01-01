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

#include "Event.h"

#include "filesystem/DroppedFile.h"
#include "filesystem/Filesystem.h"
#include "keyboard/sdl/Keyboard.h"
#include "joystick/JoystickModule.h"
#include "joystick/sdl/Joystick.h"
#include "touch/sdl/Touch.h"
#include "graphics/Graphics.h"
#include "window/Window.h"
#include "common/Exception.h"
#include "audio/Audio.h"
#include "common/config.h"
#include "timer/Timer.h"

#include <cmath>

namespace love
{
namespace event
{
namespace sdl
{

// SDL reports mouse coordinates in the window coordinate system in OS X, but
// we want them in pixel coordinates (may be different with high-DPI enabled.)
static void windowToDPICoords(double *x, double *y)
{
	auto window = Module::getInstance<window::Window>(Module::M_WINDOW);
	if (window)
		window->windowToDPICoords(x, y);
}

#ifndef LOVE_MACOSX
static void normalizedToDPICoords(double *x, double *y)
{
	double w = 1.0, h = 1.0;

	auto window = Module::getInstance<window::Window>(Module::M_WINDOW);
	if (window)
	{
		w = window->getWidth();
		h = window->getHeight();
		window->windowToDPICoords(&w, &h);
	}

	if (x)
		*x = ((*x) * w);
	if (y)
		*y = ((*y) * h);
}
#endif

// SDL's event watch callbacks trigger when the event is actually posted inside
// SDL, unlike with SDL_PollEvents. This is useful for some events which require
// handling inside the function which triggered them on some backends.
static int SDLCALL watchAppEvents(void * /*udata*/, SDL_Event *event)
{
	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);

	switch (event->type)
	{
	// On iOS, calling any OpenGL ES function after the function which triggers
	// SDL_APP_DIDENTERBACKGROUND is called will kill the app, so we handle it
	// with an event watch callback, which will be called inside that function.
	case SDL_APP_DIDENTERBACKGROUND:
	case SDL_APP_WILLENTERFOREGROUND:
		if (gfx)
			gfx->setActive(event->type == SDL_APP_WILLENTERFOREGROUND);
		break;
	default:
		break;
	}

	return 1;
}

const char *Event::getName() const
{
	return "love.event.sdl";
}

Event::Event()
{
	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
		throw love::Exception("Could not initialize SDL events subsystem (%s)", SDL_GetError());

	SDL_AddEventWatch(watchAppEvents, this);
}

Event::~Event()
{
	SDL_DelEventWatch(watchAppEvents, this);
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void Event::pump()
{
	exceptionIfInRenderPass("love.event.pump");

	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		Message *msg = convert(e);
		if (msg)
		{
			push(msg);
			msg->release();
		}
	}
}

Message *Event::wait()
{
	exceptionIfInRenderPass("love.event.wait");

	SDL_Event e;

	if (SDL_WaitEvent(&e) != 1)
		return nullptr;

	return convert(e);
}

void Event::clear()
{
	exceptionIfInRenderPass("love.event.clear");

	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		// Do nothing with 'e' ...
	}

	love::event::Event::clear();
}

void Event::exceptionIfInRenderPass(const char *name)
{
	// Some core OS graphics functionality (e.g. swap buffers on some platforms)
	// happens inside SDL_PumpEvents - which is called by SDL_PollEvent and
	// friends. It's probably a bad idea to call those functions while a Canvas
	// is active.
	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isCanvasActive())
		throw love::Exception("%s cannot be called while a Canvas is active in love.graphics.", name);
}

Message *Event::convert(const SDL_Event &e)
{
	Message *msg = nullptr;

	std::vector<Variant> vargs;
	vargs.reserve(4);

	love::filesystem::Filesystem *filesystem = nullptr;

	love::keyboard::Keyboard::Key key = love::keyboard::Keyboard::KEY_UNKNOWN;
	love::keyboard::Keyboard::Scancode scancode = love::keyboard::Keyboard::SCANCODE_UNKNOWN;

	const char *txt;
	const char *txt2;
	std::map<SDL_Keycode, love::keyboard::Keyboard::Key>::const_iterator keyit;

#ifndef LOVE_MACOSX
	love::touch::sdl::Touch *touchmodule = nullptr;
	love::touch::Touch::TouchInfo touchinfo;
#endif

#ifdef LOVE_LINUX
	static bool touchNormalizationBug = false;
#endif

	switch (e.type)
	{
	case SDL_KEYDOWN:
		if (e.key.repeat)
		{
			auto kb = Module::getInstance<love::keyboard::Keyboard>(Module::M_KEYBOARD);
			if (kb && !kb->hasKeyRepeat())
				break;
		}

		keyit = keys.find(e.key.keysym.sym);
		if (keyit != keys.end())
			key = keyit->second;

		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";

		love::keyboard::sdl::Keyboard::getConstant(e.key.keysym.scancode, scancode);
		if (!love::keyboard::Keyboard::getConstant(scancode, txt2))
			txt2 = "unknown";

		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back(txt2, strlen(txt2));
		vargs.emplace_back(e.key.repeat != 0);
		msg = new Message("keypressed", vargs);
		break;
	case SDL_KEYUP:
		keyit = keys.find(e.key.keysym.sym);
		if (keyit != keys.end())
			key = keyit->second;

		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";

		love::keyboard::sdl::Keyboard::getConstant(e.key.keysym.scancode, scancode);
		if (!love::keyboard::Keyboard::getConstant(scancode, txt2))
			txt2 = "unknown";

		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back(txt2, strlen(txt2));
		msg = new Message("keyreleased", vargs);
		break;
	case SDL_TEXTINPUT:
		txt = e.text.text;
		vargs.emplace_back(txt, strlen(txt));
		msg = new Message("textinput", vargs);
		break;
	case SDL_TEXTEDITING:
		txt = e.edit.text;
		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back((double) e.edit.start);
		vargs.emplace_back((double) e.edit.length);
		msg = new Message("textedited", vargs);
		break;
	case SDL_MOUSEMOTION:
		{
			double x = (double) e.motion.x;
			double y = (double) e.motion.y;
			double xrel = (double) e.motion.xrel;
			double yrel = (double) e.motion.yrel;
			windowToDPICoords(&x, &y);
			windowToDPICoords(&xrel, &yrel);
			vargs.emplace_back(x);
			vargs.emplace_back(y);
			vargs.emplace_back(xrel);
			vargs.emplace_back(yrel);
			vargs.emplace_back(e.motion.which == SDL_TOUCH_MOUSEID);
			msg = new Message("mousemoved", vargs);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			// SDL uses button 3 for the right mouse button, but we use button 2
			int button = e.button.button;
			switch (button)
			{
			case SDL_BUTTON_RIGHT:
				button = 2;
				break;
			case SDL_BUTTON_MIDDLE:
				button = 3;
				break;
			}

			double px = (double) e.button.x;
			double py = (double) e.button.y;
			windowToDPICoords(&px, &py);
			vargs.emplace_back(px);
			vargs.emplace_back(py);
			vargs.emplace_back((double) button);
			vargs.emplace_back(e.button.which == SDL_TOUCH_MOUSEID);
			vargs.emplace_back((double) e.button.clicks);

			bool down = e.type == SDL_MOUSEBUTTONDOWN;
			msg = new Message(down ? "mousepressed" : "mousereleased", vargs);
		}
		break;
	case SDL_MOUSEWHEEL:
		vargs.emplace_back((double) e.wheel.x);
		vargs.emplace_back((double) e.wheel.y);
		msg = new Message("wheelmoved", vargs);
		break;
	case SDL_FINGERDOWN:
	case SDL_FINGERUP:
	case SDL_FINGERMOTION:
		// Touch events are disabled in OS X because we only actually want touch
		// screen events, but most touch devices in OS X aren't touch screens
		// (and SDL doesn't differentiate.) Non-screen touch devices like Mac
		// trackpads won't give touch coords in the window's coordinate-space.
#ifndef LOVE_MACOSX
		touchinfo.id = (int64) e.tfinger.fingerId;
		touchinfo.x = e.tfinger.x;
		touchinfo.y = e.tfinger.y;
		touchinfo.dx = e.tfinger.dx;
		touchinfo.dy = e.tfinger.dy;
		touchinfo.pressure = e.tfinger.pressure;

#ifdef LOVE_LINUX
		// FIXME: hacky workaround for SDL not normalizing touch coordinates in
		// its X11 backend: https://bugzilla.libsdl.org/show_bug.cgi?id=2307
		if (touchNormalizationBug || fabs(touchinfo.x) >= 1.5 || fabs(touchinfo.y) >= 1.5 || fabs(touchinfo.dx) >= 1.5 || fabs(touchinfo.dy) >= 1.5)
		{
			touchNormalizationBug = true;
			windowToDPICoords(&touchinfo.x, &touchinfo.y);
			windowToDPICoords(&touchinfo.dx, &touchinfo.dy);
		}
		else
#endif
		{
			// SDL's coords are normalized to [0, 1], but we want screen coords.
			normalizedToDPICoords(&touchinfo.x, &touchinfo.y);
			normalizedToDPICoords(&touchinfo.dx, &touchinfo.dy);
		}

		// We need to update the love.touch.sdl internal state from here.
		touchmodule = (touch::sdl::Touch *) Module::getInstance("love.touch.sdl");
		if (touchmodule)
			touchmodule->onEvent(e.type, touchinfo);

		// This is a bit hackish and we lose the higher 32 bits of the id on
		// 32-bit systems, but SDL only ever gives id's that at most use as many
		// bits as can fit in a pointer (for now.)
		// We use lightuserdata instead of a lua_Number (double) because doubles
		// can't represent all possible id values on 64-bit systems.
		vargs.emplace_back((void *) (intptr_t) touchinfo.id);
		vargs.emplace_back(touchinfo.x);
		vargs.emplace_back(touchinfo.y);
		vargs.emplace_back(touchinfo.dx);
		vargs.emplace_back(touchinfo.dy);
		vargs.emplace_back(touchinfo.pressure);

		if (e.type == SDL_FINGERDOWN)
			txt = "touchpressed";
		else if (e.type == SDL_FINGERUP)
			txt = "touchreleased";
		else
			txt = "touchmoved";
		msg = new Message(txt, vargs);
#endif
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYDEVICEADDED:
	case SDL_JOYDEVICEREMOVED:
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERAXISMOTION:
		msg = convertJoystickEvent(e);
		break;
	case SDL_WINDOWEVENT:
		msg = convertWindowEvent(e);
		break;
#if SDL_VERSION_ATLEAST(2, 0, 9)
	case SDL_DISPLAYEVENT:
		if (e.display.event == SDL_DISPLAYEVENT_ORIENTATION)
		{
			auto orientation = window::Window::ORIENTATION_UNKNOWN;
			switch ((SDL_DisplayOrientation) e.display.data1)
			{
			case SDL_ORIENTATION_UNKNOWN:
			default:
				orientation = window::Window::ORIENTATION_UNKNOWN;
				break;
			case SDL_ORIENTATION_LANDSCAPE:
				orientation = window::Window::ORIENTATION_LANDSCAPE;
				break;
			case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
				orientation = window::Window::ORIENTATION_LANDSCAPE_FLIPPED;
				break;
			case SDL_ORIENTATION_PORTRAIT:
				orientation = window::Window::ORIENTATION_PORTRAIT;
				break;
			case SDL_ORIENTATION_PORTRAIT_FLIPPED:
				orientation = window::Window::ORIENTATION_PORTRAIT_FLIPPED;
				break;
			}

			if (!window::Window::getConstant(orientation, txt))
				txt = "unknown";

			vargs.emplace_back((double)(e.display.display + 1));
			vargs.emplace_back(txt, strlen(txt));

			msg = new Message("displayrotated", vargs);
		}
		break;
#endif
	case SDL_DROPFILE:
		filesystem = Module::getInstance<filesystem::Filesystem>(Module::M_FILESYSTEM);
		if (filesystem != nullptr)
		{
			// Allow mounting any dropped path, so zips or dirs can be mounted.
			filesystem->allowMountingForPath(e.drop.file);

			if (filesystem->isRealDirectory(e.drop.file))
			{
				vargs.emplace_back(e.drop.file, strlen(e.drop.file));
				msg = new Message("directorydropped", vargs);
			}
			else
			{
				auto *file = new love::filesystem::DroppedFile(e.drop.file);
				vargs.emplace_back(&love::filesystem::DroppedFile::type, file);
				msg = new Message("filedropped", vargs);
				file->release();
			}
		}
		SDL_free(e.drop.file);
		break;
	case SDL_QUIT:
	case SDL_APP_TERMINATING:
		msg = new Message("quit");
		break;
	case SDL_APP_LOWMEMORY:
		msg = new Message("lowmemory");
		break;
	default:
		break;
	}

	return msg;
}

Message *Event::convertJoystickEvent(const SDL_Event &e) const
{
	auto joymodule = Module::getInstance<joystick::JoystickModule>(Module::M_JOYSTICK);
	if (!joymodule)
		return nullptr;

	Message *msg = nullptr;

	std::vector<Variant> vargs;
	vargs.reserve(4);

	love::Type *joysticktype = &love::joystick::Joystick::type;
	love::joystick::Joystick *stick = nullptr;
	love::joystick::Joystick::Hat hat;
	love::joystick::Joystick::GamepadButton padbutton;
	love::joystick::Joystick::GamepadAxis padaxis;
	const char *txt;

	switch (e.type)
	{
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		stick = joymodule->getJoystickFromID(e.jbutton.which);
		if (!stick)
			break;

		vargs.emplace_back(joysticktype, stick);
		vargs.emplace_back((double)(e.jbutton.button+1));
		msg = new Message((e.type == SDL_JOYBUTTONDOWN) ?
						  "joystickpressed" : "joystickreleased",
						  vargs);
		break;
	case SDL_JOYAXISMOTION:
		{
			stick = joymodule->getJoystickFromID(e.jaxis.which);
			if (!stick)
				break;

			vargs.emplace_back(joysticktype, stick);
			vargs.emplace_back((double)(e.jaxis.axis+1));
			float value = joystick::Joystick::clampval(e.jaxis.value / 32768.0f);
			vargs.emplace_back((double) value);
			msg = new Message("joystickaxis", vargs);
		}
		break;
	case SDL_JOYHATMOTION:
		if (!joystick::sdl::Joystick::getConstant(e.jhat.value, hat) || !joystick::Joystick::getConstant(hat, txt))
			break;

		stick = joymodule->getJoystickFromID(e.jhat.which);
		if (!stick)
			break;

		vargs.emplace_back(joysticktype, stick);
		vargs.emplace_back((double)(e.jhat.hat+1));
		vargs.emplace_back(txt, strlen(txt));
		msg = new Message("joystickhat", vargs);
		break;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		if (!joystick::sdl::Joystick::getConstant((SDL_GameControllerButton) e.cbutton.button, padbutton))
			break;

		if (!joystick::Joystick::getConstant(padbutton, txt))
			break;

		stick = joymodule->getJoystickFromID(e.cbutton.which);
		if (!stick)
			break;

		vargs.emplace_back(joysticktype, stick);
		vargs.emplace_back(txt, strlen(txt));
		msg = new Message(e.type == SDL_CONTROLLERBUTTONDOWN ?
						  "gamepadpressed" : "gamepadreleased", vargs);
		break;
	case SDL_CONTROLLERAXISMOTION:
		if (joystick::sdl::Joystick::getConstant((SDL_GameControllerAxis) e.caxis.axis, padaxis))
		{
			if (!joystick::Joystick::getConstant(padaxis, txt))
				break;

			stick = joymodule->getJoystickFromID(e.caxis.which);
			if (!stick)
				break;

			vargs.emplace_back(joysticktype, stick);
			vargs.emplace_back(txt, strlen(txt));
			float value = joystick::Joystick::clampval(e.caxis.value / 32768.0f);
			vargs.emplace_back((double) value);
			msg = new Message("gamepadaxis", vargs);
		}
		break;
	case SDL_JOYDEVICEADDED:
		// jdevice.which is the joystick device index.
		stick = joymodule->addJoystick(e.jdevice.which);
		if (stick)
		{
			vargs.emplace_back(joysticktype, stick);
			msg = new Message("joystickadded", vargs);
		}
		break;
	case SDL_JOYDEVICEREMOVED:
		// jdevice.which is the joystick instance ID now.
		stick = joymodule->getJoystickFromID(e.jdevice.which);
		if (stick)
		{
			joymodule->removeJoystick(stick);
			vargs.emplace_back(joysticktype, stick);
			msg = new Message("joystickremoved", vargs);
		}
		break;
	default:
		break;
	}

	return msg;
}

Message *Event::convertWindowEvent(const SDL_Event &e)
{
	Message *msg = nullptr;

	std::vector<Variant> vargs;
	vargs.reserve(4);

	window::Window *win = nullptr;
	graphics::Graphics *gfx = nullptr;

	if (e.type != SDL_WINDOWEVENT)
		return nullptr;

	switch (e.window.event)
	{
	case SDL_WINDOWEVENT_FOCUS_GAINED:
	case SDL_WINDOWEVENT_FOCUS_LOST:
		vargs.emplace_back(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED);
		msg = new Message("focus", vargs);
		break;
	case SDL_WINDOWEVENT_ENTER:
	case SDL_WINDOWEVENT_LEAVE:
		vargs.emplace_back(e.window.event == SDL_WINDOWEVENT_ENTER);
		msg = new Message("mousefocus", vargs);
		break;
	case SDL_WINDOWEVENT_SHOWN:
	case SDL_WINDOWEVENT_HIDDEN:
		vargs.emplace_back(e.window.event == SDL_WINDOWEVENT_SHOWN);
		msg = new Message("visible", vargs);
		break;
	case SDL_WINDOWEVENT_RESIZED:
		{
			double width  = e.window.data1;
			double height = e.window.data2;

			gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
			win = Module::getInstance<window::Window>(Module::M_WINDOW);

			// WINDOWEVENT_SIZE_CHANGED will always occur before RESIZED.
			// The size values in the Window aren't necessarily the same as the
			// graphics size, which is what we want to output.
			if (gfx)
			{
				width  = gfx->getWidth();
				height = gfx->getHeight();
			}
			else if (win)
			{
				width  = win->getWidth();
				height = win->getHeight();
				windowToDPICoords(&width, &height);
			}

			vargs.emplace_back(width);
			vargs.emplace_back(height);
			msg = new Message("resize", vargs);
		}
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		win = Module::getInstance<window::Window>(Module::M_WINDOW);
		if (win)
			win->onSizeChanged(e.window.data1, e.window.data2);
		break;
	case SDL_WINDOWEVENT_MINIMIZED:
	case SDL_WINDOWEVENT_RESTORED:
#ifdef LOVE_ANDROID
		if (auto audio = Module::getInstance<audio::Audio>(Module::M_AUDIO))
		{
			if (e.window.event == SDL_WINDOWEVENT_MINIMIZED)
				audio->pauseContext();
			else if (e.window.event == SDL_WINDOWEVENT_RESTORED)
				audio->resumeContext();
		}
#endif
		break;
	}

	return msg;
}

std::map<SDL_Keycode, love::keyboard::Keyboard::Key> Event::createKeyMap()
{
	using love::keyboard::Keyboard;

	std::map<SDL_Keycode, Keyboard::Key> k;

	k[SDLK_UNKNOWN] = Keyboard::KEY_UNKNOWN;

	k[SDLK_RETURN] = Keyboard::KEY_RETURN;
	k[SDLK_ESCAPE] = Keyboard::KEY_ESCAPE;
	k[SDLK_BACKSPACE] = Keyboard::KEY_BACKSPACE;
	k[SDLK_TAB] = Keyboard::KEY_TAB;
	k[SDLK_SPACE] = Keyboard::KEY_SPACE;
	k[SDLK_EXCLAIM] = Keyboard::KEY_EXCLAIM;
	k[SDLK_QUOTEDBL] = Keyboard::KEY_QUOTEDBL;
	k[SDLK_HASH] = Keyboard::KEY_HASH;
	k[SDLK_PERCENT] = Keyboard::KEY_PERCENT;
	k[SDLK_DOLLAR] = Keyboard::KEY_DOLLAR;
	k[SDLK_AMPERSAND] = Keyboard::KEY_AMPERSAND;
	k[SDLK_QUOTE] = Keyboard::KEY_QUOTE;
	k[SDLK_LEFTPAREN] = Keyboard::KEY_LEFTPAREN;
	k[SDLK_RIGHTPAREN] = Keyboard::KEY_RIGHTPAREN;
	k[SDLK_ASTERISK] = Keyboard::KEY_ASTERISK;
	k[SDLK_PLUS] = Keyboard::KEY_PLUS;
	k[SDLK_COMMA] = Keyboard::KEY_COMMA;
	k[SDLK_MINUS] = Keyboard::KEY_MINUS;
	k[SDLK_PERIOD] = Keyboard::KEY_PERIOD;
	k[SDLK_SLASH] = Keyboard::KEY_SLASH;
	k[SDLK_0] = Keyboard::KEY_0;
	k[SDLK_1] = Keyboard::KEY_1;
	k[SDLK_2] = Keyboard::KEY_2;
	k[SDLK_3] = Keyboard::KEY_3;
	k[SDLK_4] = Keyboard::KEY_4;
	k[SDLK_5] = Keyboard::KEY_5;
	k[SDLK_6] = Keyboard::KEY_6;
	k[SDLK_7] = Keyboard::KEY_7;
	k[SDLK_8] = Keyboard::KEY_8;
	k[SDLK_9] = Keyboard::KEY_9;
	k[SDLK_COLON] = Keyboard::KEY_COLON;
	k[SDLK_SEMICOLON] = Keyboard::KEY_SEMICOLON;
	k[SDLK_LESS] = Keyboard::KEY_LESS;
	k[SDLK_EQUALS] = Keyboard::KEY_EQUALS;
	k[SDLK_GREATER] = Keyboard::KEY_GREATER;
	k[SDLK_QUESTION] = Keyboard::KEY_QUESTION;
	k[SDLK_AT] = Keyboard::KEY_AT;

	k[SDLK_LEFTBRACKET] = Keyboard::KEY_LEFTBRACKET;
	k[SDLK_BACKSLASH] = Keyboard::KEY_BACKSLASH;
	k[SDLK_RIGHTBRACKET] = Keyboard::KEY_RIGHTBRACKET;
	k[SDLK_CARET] = Keyboard::KEY_CARET;
	k[SDLK_UNDERSCORE] = Keyboard::KEY_UNDERSCORE;
	k[SDLK_BACKQUOTE] = Keyboard::KEY_BACKQUOTE;
	k[SDLK_a] = Keyboard::KEY_A;
	k[SDLK_b] = Keyboard::KEY_B;
	k[SDLK_c] = Keyboard::KEY_C;
	k[SDLK_d] = Keyboard::KEY_D;
	k[SDLK_e] = Keyboard::KEY_E;
	k[SDLK_f] = Keyboard::KEY_F;
	k[SDLK_g] = Keyboard::KEY_G;
	k[SDLK_h] = Keyboard::KEY_H;
	k[SDLK_i] = Keyboard::KEY_I;
	k[SDLK_j] = Keyboard::KEY_J;
	k[SDLK_k] = Keyboard::KEY_K;
	k[SDLK_l] = Keyboard::KEY_L;
	k[SDLK_m] = Keyboard::KEY_M;
	k[SDLK_n] = Keyboard::KEY_N;
	k[SDLK_o] = Keyboard::KEY_O;
	k[SDLK_p] = Keyboard::KEY_P;
	k[SDLK_q] = Keyboard::KEY_Q;
	k[SDLK_r] = Keyboard::KEY_R;
	k[SDLK_s] = Keyboard::KEY_S;
	k[SDLK_t] = Keyboard::KEY_T;
	k[SDLK_u] = Keyboard::KEY_U;
	k[SDLK_v] = Keyboard::KEY_V;
	k[SDLK_w] = Keyboard::KEY_W;
	k[SDLK_x] = Keyboard::KEY_X;
	k[SDLK_y] = Keyboard::KEY_Y;
	k[SDLK_z] = Keyboard::KEY_Z;

	k[SDLK_CAPSLOCK] = Keyboard::KEY_CAPSLOCK;

	k[SDLK_F1] = Keyboard::KEY_F1;
	k[SDLK_F2] = Keyboard::KEY_F2;
	k[SDLK_F3] = Keyboard::KEY_F3;
	k[SDLK_F4] = Keyboard::KEY_F4;
	k[SDLK_F5] = Keyboard::KEY_F5;
	k[SDLK_F6] = Keyboard::KEY_F6;
	k[SDLK_F7] = Keyboard::KEY_F7;
	k[SDLK_F8] = Keyboard::KEY_F8;
	k[SDLK_F9] = Keyboard::KEY_F9;
	k[SDLK_F10] = Keyboard::KEY_F10;
	k[SDLK_F11] = Keyboard::KEY_F11;
	k[SDLK_F12] = Keyboard::KEY_F12;

	k[SDLK_PRINTSCREEN] = Keyboard::KEY_PRINTSCREEN;
	k[SDLK_SCROLLLOCK] = Keyboard::KEY_SCROLLLOCK;
	k[SDLK_PAUSE] = Keyboard::KEY_PAUSE;
	k[SDLK_INSERT] = Keyboard::KEY_INSERT;
	k[SDLK_HOME] = Keyboard::KEY_HOME;
	k[SDLK_PAGEUP] = Keyboard::KEY_PAGEUP;
	k[SDLK_DELETE] = Keyboard::KEY_DELETE;
	k[SDLK_END] = Keyboard::KEY_END;
	k[SDLK_PAGEDOWN] = Keyboard::KEY_PAGEDOWN;
	k[SDLK_RIGHT] = Keyboard::KEY_RIGHT;
	k[SDLK_LEFT] = Keyboard::KEY_LEFT;
	k[SDLK_DOWN] = Keyboard::KEY_DOWN;
	k[SDLK_UP] = Keyboard::KEY_UP;

	k[SDLK_NUMLOCKCLEAR] = Keyboard::KEY_NUMLOCKCLEAR;
	k[SDLK_KP_DIVIDE] = Keyboard::KEY_KP_DIVIDE;
	k[SDLK_KP_MULTIPLY] = Keyboard::KEY_KP_MULTIPLY;
	k[SDLK_KP_MINUS] = Keyboard::KEY_KP_MINUS;
	k[SDLK_KP_PLUS] = Keyboard::KEY_KP_PLUS;
	k[SDLK_KP_ENTER] = Keyboard::KEY_KP_ENTER;
	k[SDLK_KP_0] = Keyboard::KEY_KP_0;
	k[SDLK_KP_1] = Keyboard::KEY_KP_1;
	k[SDLK_KP_2] = Keyboard::KEY_KP_2;
	k[SDLK_KP_3] = Keyboard::KEY_KP_3;
	k[SDLK_KP_4] = Keyboard::KEY_KP_4;
	k[SDLK_KP_5] = Keyboard::KEY_KP_5;
	k[SDLK_KP_6] = Keyboard::KEY_KP_6;
	k[SDLK_KP_7] = Keyboard::KEY_KP_7;
	k[SDLK_KP_8] = Keyboard::KEY_KP_8;
	k[SDLK_KP_9] = Keyboard::KEY_KP_9;
	k[SDLK_KP_PERIOD] = Keyboard::KEY_KP_PERIOD;
	k[SDLK_KP_COMMA] = Keyboard::KEY_KP_COMMA;
	k[SDLK_KP_EQUALS] = Keyboard::KEY_KP_EQUALS;

	k[SDLK_APPLICATION] = Keyboard::KEY_APPLICATION;
	k[SDLK_POWER] = Keyboard::KEY_POWER;
	k[SDLK_F13] = Keyboard::KEY_F13;
	k[SDLK_F14] = Keyboard::KEY_F14;
	k[SDLK_F15] = Keyboard::KEY_F15;
	k[SDLK_F16] = Keyboard::KEY_F16;
	k[SDLK_F17] = Keyboard::KEY_F17;
	k[SDLK_F18] = Keyboard::KEY_F18;
	k[SDLK_F19] = Keyboard::KEY_F19;
	k[SDLK_F20] = Keyboard::KEY_F20;
	k[SDLK_F21] = Keyboard::KEY_F21;
	k[SDLK_F22] = Keyboard::KEY_F22;
	k[SDLK_F23] = Keyboard::KEY_F23;
	k[SDLK_F24] = Keyboard::KEY_F24;
	k[SDLK_EXECUTE] = Keyboard::KEY_EXECUTE;
	k[SDLK_HELP] = Keyboard::KEY_HELP;
	k[SDLK_MENU] = Keyboard::KEY_MENU;
	k[SDLK_SELECT] = Keyboard::KEY_SELECT;
	k[SDLK_STOP] = Keyboard::KEY_STOP;
	k[SDLK_AGAIN] = Keyboard::KEY_AGAIN;
	k[SDLK_UNDO] = Keyboard::KEY_UNDO;
	k[SDLK_CUT] = Keyboard::KEY_CUT;
	k[SDLK_COPY] = Keyboard::KEY_COPY;
	k[SDLK_PASTE] = Keyboard::KEY_PASTE;
	k[SDLK_FIND] = Keyboard::KEY_FIND;
	k[SDLK_MUTE] = Keyboard::KEY_MUTE;
	k[SDLK_VOLUMEUP] = Keyboard::KEY_VOLUMEUP;
	k[SDLK_VOLUMEDOWN] = Keyboard::KEY_VOLUMEDOWN;

	k[SDLK_ALTERASE] = Keyboard::KEY_ALTERASE;
	k[SDLK_SYSREQ] = Keyboard::KEY_SYSREQ;
	k[SDLK_CANCEL] = Keyboard::KEY_CANCEL;
	k[SDLK_CLEAR] = Keyboard::KEY_CLEAR;
	k[SDLK_PRIOR] = Keyboard::KEY_PRIOR;
	k[SDLK_RETURN2] = Keyboard::KEY_RETURN2;
	k[SDLK_SEPARATOR] = Keyboard::KEY_SEPARATOR;
	k[SDLK_OUT] = Keyboard::KEY_OUT;
	k[SDLK_OPER] = Keyboard::KEY_OPER;
	k[SDLK_CLEARAGAIN] = Keyboard::KEY_CLEARAGAIN;

	k[SDLK_THOUSANDSSEPARATOR] = Keyboard::KEY_THOUSANDSSEPARATOR;
	k[SDLK_DECIMALSEPARATOR] = Keyboard::KEY_DECIMALSEPARATOR;
	k[SDLK_CURRENCYUNIT] = Keyboard::KEY_CURRENCYUNIT;
	k[SDLK_CURRENCYSUBUNIT] = Keyboard::KEY_CURRENCYSUBUNIT;

	k[SDLK_LCTRL] = Keyboard::KEY_LCTRL;
	k[SDLK_LSHIFT] = Keyboard::KEY_LSHIFT;
	k[SDLK_LALT] = Keyboard::KEY_LALT;
	k[SDLK_LGUI] = Keyboard::KEY_LGUI;
	k[SDLK_RCTRL] = Keyboard::KEY_RCTRL;
	k[SDLK_RSHIFT] = Keyboard::KEY_RSHIFT;
	k[SDLK_RALT] = Keyboard::KEY_RALT;
	k[SDLK_RGUI] = Keyboard::KEY_RGUI;

	k[SDLK_MODE] = Keyboard::KEY_MODE;

	k[SDLK_AUDIONEXT] = Keyboard::KEY_AUDIONEXT;
	k[SDLK_AUDIOPREV] = Keyboard::KEY_AUDIOPREV;
	k[SDLK_AUDIOSTOP] = Keyboard::KEY_AUDIOSTOP;
	k[SDLK_AUDIOPLAY] = Keyboard::KEY_AUDIOPLAY;
	k[SDLK_AUDIOMUTE] = Keyboard::KEY_AUDIOMUTE;
	k[SDLK_MEDIASELECT] = Keyboard::KEY_MEDIASELECT;
	k[SDLK_WWW] = Keyboard::KEY_WWW;
	k[SDLK_MAIL] = Keyboard::KEY_MAIL;
	k[SDLK_CALCULATOR] = Keyboard::KEY_CALCULATOR;
	k[SDLK_COMPUTER] = Keyboard::KEY_COMPUTER;
	k[SDLK_AC_SEARCH] = Keyboard::KEY_APP_SEARCH;
	k[SDLK_AC_HOME] = Keyboard::KEY_APP_HOME;
	k[SDLK_AC_BACK] = Keyboard::KEY_APP_BACK;
	k[SDLK_AC_FORWARD] = Keyboard::KEY_APP_FORWARD;
	k[SDLK_AC_STOP] = Keyboard::KEY_APP_STOP;
	k[SDLK_AC_REFRESH] = Keyboard::KEY_APP_REFRESH;
	k[SDLK_AC_BOOKMARKS] = Keyboard::KEY_APP_BOOKMARKS;

	k[SDLK_BRIGHTNESSDOWN] = Keyboard::KEY_BRIGHTNESSDOWN;
	k[SDLK_BRIGHTNESSUP] = Keyboard::KEY_BRIGHTNESSUP;
	k[SDLK_DISPLAYSWITCH] = Keyboard::KEY_DISPLAYSWITCH;
	k[SDLK_KBDILLUMTOGGLE] = Keyboard::KEY_KBDILLUMTOGGLE;
	k[SDLK_KBDILLUMDOWN] = Keyboard::KEY_KBDILLUMDOWN;
	k[SDLK_KBDILLUMUP] = Keyboard::KEY_KBDILLUMUP;
	k[SDLK_EJECT] = Keyboard::KEY_EJECT;
	k[SDLK_SLEEP] = Keyboard::KEY_SLEEP;

#ifdef LOVE_ANDROID
	k[SDLK_AC_BACK] = Keyboard::KEY_ESCAPE;
#endif

	return k;
}

std::map<SDL_Keycode, love::keyboard::Keyboard::Key> Event::keys = Event::createKeyMap();

} // sdl
} // event
} // love
