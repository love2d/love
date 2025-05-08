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

#include "Event.h"

#include "common/int.h"
#include "filesystem/Filesystem.h"
#include "keyboard/sdl/Keyboard.h"
#include "joystick/JoystickModule.h"
#include "touch/sdl/Touch.h"
#include "graphics/Graphics.h"
#include "window/Window.h"
#include "common/Exception.h"
#include "audio/Audio.h"
#include "common/config.h"
#include "timer/Timer.h"
#include "sensor/sdl/Sensor.h"

#include <cmath>

#include "joystick/sdl/Joystick.h"
#include "window/sdl/Window.h"

namespace love
{
namespace event
{
namespace sdl
{

// SDL reports mouse coordinates in the window coordinate system in OS X, but
// we want them in pixel coordinates (may be different with high-DPI enabled.)
static void windowToDPICoords(love::window::Window *window, double *x, double *y)
{
	if (window)
		window->windowToDPICoords(x, y);
}

static void clampToWindow(love::window::Window *window, double *x, double *y)
{
	if (window)
		window->clampPositionInWindow(x, y);
}

static void normalizedToDPICoords(love::window::Window *window, double *x, double *y)
{
	double w = 1.0, h = 1.0;

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

// SDL's event watch callbacks trigger when the event is actually posted inside
// SDL, unlike with SDL_PollEvents. This is useful for some events which require
// handling inside the function which triggered them on some backends.
// Note: this may run on non-main threads on some platforms (Android?)
static bool SDLCALL watchAppEvents(void *udata, SDL_Event *event)
{
	auto eventModule = (Event *)udata;

	switch (event->type)
	{
	case SDL_EVENT_DID_ENTER_BACKGROUND:
	case SDL_EVENT_WILL_ENTER_FOREGROUND:
		// On iOS, calling any OpenGL ES function after the function which triggers
		// SDL_APP_DIDENTERBACKGROUND is called will kill the app, so we handle it
		// with an event watch callback, which will be called inside that function.
		{
			auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
			if (gfx && SDL_IsMainThread())
				gfx->setActive(event->type == SDL_EVENT_WILL_ENTER_FOREGROUND);
		}
		break;
	case SDL_EVENT_WINDOW_EXPOSED:
		if (eventModule != nullptr && SDL_IsMainThread() && eventModule->allowModalDraws())
			eventModule->modalDraw();
		break;
	default:
		break;
	}

	return true;
}

Event::Event()
	: love::event::Event("love.event.sdl")
{
	if (!SDL_InitSubSystem(SDL_INIT_EVENTS))
		throw love::Exception("Could not initialize SDL events subsystem (%s)", SDL_GetError());

	SDL_AddEventWatch(watchAppEvents, this);
}

Event::~Event()
{
	SDL_RemoveEventWatch(watchAppEvents, this);
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void Event::pump(float waitTimeout)
{
	exceptionIfInRenderPass("love.event.pump");

	bool shouldPoll = false;

	if (insideEventPump)
	{
		// Don't pump if we're inside the event pump already, but do allow
		// polling what's in the SDL queue.
		shouldPoll = true;
	}
	else
	{
		int waitTimeoutMS = 0;
		if (std::isinf(waitTimeout) || waitTimeout < 0.0f)
			waitTimeoutMS = -1; // Wait forever.
		else if (waitTimeout > 0.0f)
			waitTimeoutMS = (int)std::min<int64>(LOVE_INT32_MAX, 1000LL * waitTimeout);

		// Wait for the first event, if requested. WaitEvent also calls PumpEvents.
		SDL_Event e = {};
		insideEventPump = true;
		bool success = false;
		try
		{
			success = SDL_WaitEventTimeout(&e, waitTimeoutMS);
		}
		catch (std::exception &)
		{
			insideEventPump = false;
			throw;
		}
		insideEventPump = false;

		if (success)
		{
			StrongRef<Message> msg(convert(e), Acquire::NORETAIN);
			if (msg)
				push(msg);

			// Fetch any extra events that came in during WaitEvent.
			shouldPoll = true;
		}

		// For exceptions generated inside a modal draw callback, propagate them
		// outside of OS event processing instead of inside.
		if (!deferredExceptionMessage.empty())
		{
			std::string exceptionstr = deferredExceptionMessage;
			deferredExceptionMessage.clear();
			throw love::Exception("%s", exceptionstr.c_str());
		}
	}

	if (shouldPoll)
	{
		SDL_Event e;
		while (SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) > 0)
		{
			StrongRef<Message> msg(convert(e), Acquire::NORETAIN);
			if (msg)
				push(msg);
		}
	}
}

Message *Event::wait()
{
	exceptionIfInRenderPass("love.event.wait");

	SDL_Event e;

	if (!SDL_WaitEvent(&e))
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

bool Event::allowModalDraws() const
{
	return insideEventPump;
}

void Event::exceptionIfInRenderPass(const char *name)
{
	// Some core OS graphics functionality (e.g. swap buffers on some platforms)
	// happens inside SDL_PumpEvents - which is called by SDL_PollEvent and
	// friends. It's probably a bad idea to call those functions while a RT
	// is active.
	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isRenderTargetActive())
		throw love::Exception("%s cannot be called while a render target is active in love.graphics.", name);
}

Message *Event::convert(const SDL_Event &e)
{
	Message *msg = nullptr;

	std::vector<Variant> vargs;
	vargs.reserve(4);

	love::filesystem::Filesystem *filesystem = nullptr;
	love::sensor::Sensor *sensorInstance = nullptr;
	love::window::Window *win = Module::getInstance<window::Window>(Module::M_WINDOW);

	love::keyboard::Keyboard::Key key = love::keyboard::Keyboard::KEY_UNKNOWN;
	love::keyboard::Keyboard::Scancode scancode = love::keyboard::Keyboard::SCANCODE_UNKNOWN;

	const char *txt;
	const char *txt2;

	love::touch::sdl::Touch *touchmodule = nullptr;
	love::touch::Touch::TouchInfo touchinfo = {};

	if (win)
	{
		// Dubious cast, but it's not like having an SDL event backend
		// with a non-SDL window backend will be a thing.
		auto sdlwin = dynamic_cast<love::window::sdl::Window *>(win);
		if (sdlwin != nullptr)
			sdlwin->handleSDLEvent(e);
	}

	switch (e.type)
	{
	case SDL_EVENT_KEY_DOWN:
		if (e.key.repeat)
		{
			auto kb = Module::getInstance<love::keyboard::Keyboard>(Module::M_KEYBOARD);
			if (kb && !kb->hasKeyRepeat())
				break;
		}

		love::keyboard::sdl::Keyboard::getConstant(e.key.key, key);
		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";

		love::keyboard::sdl::Keyboard::getConstant(e.key.scancode, scancode);
		if (!love::keyboard::Keyboard::getConstant(scancode, txt2))
			txt2 = "unknown";

		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back(txt2, strlen(txt2));
		vargs.emplace_back(e.key.repeat != 0);
		msg = new Message("keypressed", vargs);
		break;
	case SDL_EVENT_KEY_UP:
		love::keyboard::sdl::Keyboard::getConstant(e.key.key, key);
		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";

		love::keyboard::sdl::Keyboard::getConstant(e.key.scancode, scancode);
		if (!love::keyboard::Keyboard::getConstant(scancode, txt2))
			txt2 = "unknown";

		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back(txt2, strlen(txt2));
		msg = new Message("keyreleased", vargs);
		break;
	case SDL_EVENT_TEXT_INPUT:
		txt = e.text.text;
		vargs.emplace_back(txt, strlen(txt));
		msg = new Message("textinput", vargs);
		break;
	case SDL_EVENT_TEXT_EDITING:
		txt = e.edit.text;
		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back((double) e.edit.start);
		vargs.emplace_back((double) e.edit.length);
		msg = new Message("textedited", vargs);
		break;
	case SDL_EVENT_MOUSE_MOTION:
		{
			double x = (double) e.motion.x;
			double y = (double) e.motion.y;
			double xrel = (double) e.motion.xrel;
			double yrel = (double) e.motion.yrel;

			// SDL reports mouse coordinates outside the window bounds when click-and-
			// dragging. For compatibility we clamp instead since user code may not be
			// able to handle out-of-bounds coordinates. SDL has a hint to turn off
			// auto capture, but it doesn't report the mouse's position at the edge of
			// the window if the mouse moves fast enough when it's off.
			clampToWindow(win, &x, &y);
			windowToDPICoords(win, &x, &y);
			windowToDPICoords(win, &xrel, &yrel);

			vargs.emplace_back(x);
			vargs.emplace_back(y);
			vargs.emplace_back(xrel);
			vargs.emplace_back(yrel);
			vargs.emplace_back(e.motion.which == SDL_TOUCH_MOUSEID);
			msg = new Message("mousemoved", vargs);
		}
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
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

			clampToWindow(win, &px, &py);
			windowToDPICoords(win, &px, &py);

			vargs.emplace_back(px);
			vargs.emplace_back(py);
			vargs.emplace_back((double) button);
			vargs.emplace_back(e.button.which == SDL_TOUCH_MOUSEID);
			vargs.emplace_back((double) e.button.clicks);

			bool down = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
			msg = new Message(down ? "mousepressed" : "mousereleased", vargs);
		}
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		vargs.emplace_back((double) e.wheel.x);
		vargs.emplace_back((double) e.wheel.y);

		txt = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? "flipped" : "standard";
		vargs.emplace_back(txt, strlen(txt));

		msg = new Message("wheelmoved", vargs);
		break;
	case SDL_EVENT_FINGER_DOWN:
	case SDL_EVENT_FINGER_UP:
	case SDL_EVENT_FINGER_MOTION:
		touchinfo.id = (int64)e.tfinger.fingerID;
		touchinfo.x = e.tfinger.x;
		touchinfo.y = e.tfinger.y;
		touchinfo.dx = e.tfinger.dx;
		touchinfo.dy = e.tfinger.dy;
		touchinfo.pressure = e.tfinger.pressure;
		touchinfo.deviceType = love::touch::sdl::Touch::getDeviceType(SDL_GetTouchDeviceType(e.tfinger.touchID));
		touchinfo.mouse = e.tfinger.touchID == SDL_MOUSE_TOUCHID;

		// SDL's coords are normalized to [0, 1], but we want screen coords for direct touches.
		if (touchinfo.deviceType == love::touch::Touch::DEVICE_TOUCHSCREEN)
		{
			normalizedToDPICoords(win, &touchinfo.x, &touchinfo.y);
			normalizedToDPICoords(win, &touchinfo.dx, &touchinfo.dy);
		}

		// We need to update the love.touch.sdl internal state from here.
		touchmodule = (touch::sdl::Touch *) Module::getInstance("love.touch.sdl");
		if (touchmodule)
			touchmodule->onEvent(e.type, touchinfo);

		if (!love::touch::Touch::getConstant(touchinfo.deviceType, txt))
			txt = "unknown";

		// This is a bit hackish and we lose the higher 32 bits of the id on
		// 32-bit systems, but SDL only ever gives id's that at most use as many
		// bits as can fit in a pointer (for now.)
		// We use lightuserdata instead of a lua_Number (double) because doubles
		// can't represent all possible id values on 64-bit systems.
		vargs.emplace_back((void *)(intptr_t)touchinfo.id);
		vargs.emplace_back(touchinfo.x);
		vargs.emplace_back(touchinfo.y);
		vargs.emplace_back(touchinfo.dx);
		vargs.emplace_back(touchinfo.dy);
		vargs.emplace_back(touchinfo.pressure);
		vargs.emplace_back(txt, strlen(txt));
		vargs.emplace_back(touchinfo.mouse);

		if (e.type == SDL_EVENT_FINGER_DOWN)
			txt = "touchpressed";
		else if (e.type == SDL_EVENT_FINGER_UP || e.type == SDL_EVENT_FINGER_CANCELED)
			txt = "touchreleased";
		else
			txt = "touchmoved";
		msg = new Message(txt, vargs);
		break;
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
	case SDL_EVENT_JOYSTICK_HAT_MOTION:
	case SDL_EVENT_JOYSTICK_ADDED:
	case SDL_EVENT_JOYSTICK_REMOVED:
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
	case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
		msg = convertJoystickEvent(e);
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
	case SDL_EVENT_WINDOW_MOUSE_ENTER:
	case SDL_EVENT_WINDOW_MOUSE_LEAVE:
	case SDL_EVENT_WINDOW_SHOWN:
	case SDL_EVENT_WINDOW_HIDDEN:
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
	case SDL_EVENT_WINDOW_MINIMIZED:
	case SDL_EVENT_WINDOW_RESTORED:
	case SDL_EVENT_WINDOW_EXPOSED:
	case SDL_EVENT_WINDOW_OCCLUDED:
		msg = convertWindowEvent(e, win);
		break;
	case SDL_EVENT_DISPLAY_ORIENTATION:
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

			int count = 0;
			int displayindex = 0;
			SDL_DisplayID *displays = SDL_GetDisplays(&count);
			for (int i = 0; i < count; i++)
			{
				if (displays[i] == e.display.displayID)
				{
					displayindex = i;
					break;
				}
			}
			SDL_free(displays);
			vargs.emplace_back((double)(displayindex + 1));
			vargs.emplace_back(txt, strlen(txt));

			msg = new Message("displayrotated", vargs);
		}
		break;
	case SDL_EVENT_DROP_BEGIN:
		msg = new Message("dropbegan", vargs);
		break;
	case SDL_EVENT_DROP_COMPLETE:
		{
			double x = e.drop.x;
			double y = e.drop.y;
			windowToDPICoords(win, &x, &y);
			vargs.emplace_back(x);
			vargs.emplace_back(y);
			msg = new Message("dropcompleted", vargs);
		}
		break;
	case SDL_EVENT_DROP_POSITION:
		{
			double x = e.drop.x;
			double y = e.drop.y;
			windowToDPICoords(win, &x, &y);
			vargs.emplace_back(x);
			vargs.emplace_back(y);
			msg = new Message("dropmoved", vargs);
		}
		break;
	case SDL_EVENT_DROP_FILE:
		filesystem = Module::getInstance<filesystem::Filesystem>(Module::M_FILESYSTEM);
		if (filesystem != nullptr)
		{
			const char *filepath = e.drop.data;
			// Allow mounting any dropped path, so zips or dirs can be mounted.
			filesystem->allowMountingForPath(filepath);

			double x = e.drop.x;
			double y = e.drop.y;
			windowToDPICoords(win, &x, &y);

			if (filesystem->isRealDirectory(filepath))
			{
				vargs.emplace_back(filepath, strlen(filepath));
				vargs.emplace_back(x);
				vargs.emplace_back(y);
				msg = new Message("directorydropped", vargs);
			}
			else
			{
				auto *file = filesystem->openNativeFile(filepath, love::filesystem::File::MODE_CLOSED);
				vargs.emplace_back(&love::filesystem::File::type, file);
				vargs.emplace_back(x);
				vargs.emplace_back(y);
				msg = new Message("filedropped", vargs);
				file->release();
			}
		}
		break;
	case SDL_EVENT_QUIT:
	case SDL_EVENT_TERMINATING:
		msg = new Message("quit");
		break;
	case SDL_EVENT_LOW_MEMORY:
		msg = new Message("lowmemory");
		break;
	case SDL_EVENT_LOCALE_CHANGED:
		msg = new Message("localechanged");
		break;
	case SDL_EVENT_SENSOR_UPDATE:
		sensorInstance = Module::getInstance<sensor::Sensor>(M_SENSOR);
		if (sensorInstance)
		{
			std::vector<void*> sensors = sensorInstance->getHandles();

			for (void *s: sensors)
			{
				SDL_Sensor *sensor = (SDL_Sensor *) s;
				SDL_SensorID id = SDL_GetSensorID(sensor);

				if (e.sensor.which == id)
				{
					// Found sensor
					const char *sensorType;
					auto sdltype = SDL_GetSensorType(sensor);
					if (!sensor::Sensor::getConstant(sensor::sdl::Sensor::convert(sdltype), sensorType))
						sensorType = "unknown";

					vargs.emplace_back(sensorType, strlen(sensorType));
					// Both accelerometer and gyroscope only pass up to 3 values.
					// https://github.com/libsdl-org/SDL/blob/SDL2/include/SDL_sensor.h#L81-L127
					vargs.emplace_back(e.sensor.data[0]);
					vargs.emplace_back(e.sensor.data[1]);
					vargs.emplace_back(e.sensor.data[2]);
					msg = new Message("sensorupdated", vargs);

					break;
				}
			}
		}
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
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		stick = joymodule->getJoystickFromID(e.jbutton.which);
		if (!stick)
			break;

		vargs.emplace_back(joysticktype, stick);
		vargs.emplace_back((double)(e.jbutton.button+1));
		msg = new Message((e.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN) ?
						  "joystickpressed" : "joystickreleased",
						  vargs);
		break;
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
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
	case SDL_EVENT_JOYSTICK_HAT_MOTION:
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
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			const auto &b = e.gbutton;
			if (!joystick::sdl::Joystick::getConstant((SDL_GamepadButton) b.button, padbutton))
				break;

			if (!joystick::Joystick::getConstant(padbutton, txt))
				break;

			stick = joymodule->getJoystickFromID(b.which);
			if (!stick)
				break;

			vargs.emplace_back(joysticktype, stick);
			vargs.emplace_back(txt, strlen(txt));
			msg = new Message(e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ?
							  "gamepadpressed" : "gamepadreleased", vargs);
		}
		break;
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		if (joystick::sdl::Joystick::getConstant((SDL_GamepadAxis) e.gaxis.axis, padaxis))
		{
			if (!joystick::Joystick::getConstant(padaxis, txt))
				break;

			const auto &a = e.gaxis;
			stick = joymodule->getJoystickFromID(a.which);
			if (!stick)
				break;

			vargs.emplace_back(joysticktype, stick);
			vargs.emplace_back(txt, strlen(txt));
			float value = joystick::Joystick::clampval(a.value / 32768.0f);
			vargs.emplace_back((double) value);
			msg = new Message("gamepadaxis", vargs);
		}
		break;
	case SDL_EVENT_JOYSTICK_ADDED:
		// jdevice.which is the joystick device index.
		stick = joymodule->addJoystick(e.jdevice.which);
		if (stick)
		{
			vargs.emplace_back(joysticktype, stick);
			msg = new Message("joystickadded", vargs);
		}
		break;
	case SDL_EVENT_JOYSTICK_REMOVED:
		// jdevice.which is the joystick instance ID now.
		stick = joymodule->getJoystickFromID(e.jdevice.which);
		if (stick)
		{
			joymodule->removeJoystick(stick);
			vargs.emplace_back(joysticktype, stick);
			msg = new Message("joystickremoved", vargs);
		}
		break;
#if defined(LOVE_ENABLE_SENSOR)
	case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
		{
			const auto &sens = e.gsensor;
			stick = joymodule->getJoystickFromID(sens.which);
			if (stick)
			{
				using Sensor = love::sensor::Sensor;

				const char *sensorName;
				Sensor::SensorType sensorType = love::sensor::sdl::Sensor::convert((SDL_SensorType) sens.sensor);
				if (!Sensor::getConstant(sensorType, sensorName))
					sensorName = "unknown";

				vargs.emplace_back(joysticktype, stick);
				vargs.emplace_back(sensorName, strlen(sensorName));
				vargs.emplace_back(sens.data[0]);
				vargs.emplace_back(sens.data[1]);
				vargs.emplace_back(sens.data[2]);
				msg = new Message("joysticksensorupdated", vargs);
			}
		}
		break;
#endif // defined(LOVE_ENABLE_SENSOR)
	default:
		break;
	}

	return msg;
}

Message *Event::convertWindowEvent(const SDL_Event &e, love::window::Window *win)
{
	Message *msg = nullptr;

	std::vector<Variant> vargs;
	vargs.reserve(4);

	graphics::Graphics *gfx = nullptr;

	auto event = e.type;

	switch (event)
	{
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		vargs.emplace_back(event == SDL_EVENT_WINDOW_FOCUS_GAINED);
		msg = new Message("focus", vargs);
		break;
	case SDL_EVENT_WINDOW_MOUSE_ENTER:
	case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		vargs.emplace_back(event == SDL_EVENT_WINDOW_MOUSE_ENTER);
		msg = new Message("mousefocus", vargs);
		break;
	case SDL_EVENT_WINDOW_SHOWN:
	case SDL_EVENT_WINDOW_HIDDEN:
	case SDL_EVENT_WINDOW_MINIMIZED:
	case SDL_EVENT_WINDOW_RESTORED:
#ifdef LOVE_ANDROID
		if (auto audio = Module::getInstance<audio::Audio>(Module::M_AUDIO))
		{
			if (event == SDL_EVENT_WINDOW_MINIMIZED)
				audio->pauseContext();
			else if (event == SDL_EVENT_WINDOW_RESTORED)
				audio->resumeContext();
		}
#endif
		// WINDOW_RESTORED can also happen when going from maximized -> unmaximized,
		// but there isn't a nice way to avoid sending our event in that situation.
		vargs.emplace_back(event == SDL_EVENT_WINDOW_SHOWN || event == SDL_EVENT_WINDOW_RESTORED);
		msg = new Message("visible", vargs);
		break;
	case SDL_EVENT_WINDOW_EXPOSED:
		msg = new Message("exposed");
		break;
	case SDL_EVENT_WINDOW_OCCLUDED:
		msg = new Message("occluded");
		break;
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		{
			double width = e.window.data1;
			double height = e.window.data2;

			gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
			if (win)
				win->onSizeChanged(e.window.data1, e.window.data2);

			// The size values in the Window aren't necessarily the same as the
			// graphics size, which is what we want to output.
			if (gfx)
			{
				width = gfx->getWidth();
				height = gfx->getHeight();
			}
			else if (win)
			{
				width = win->getWidth();
				height = win->getHeight();
				windowToDPICoords(win, &width, &height);
			}

			vargs.emplace_back(width);
			vargs.emplace_back(height);
			msg = new Message("resize", vargs);
		}
		break;
	}

	return msg;
}

} // sdl
} // event
} // love
