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
#include "wrap_Sensor.h"
#include "sdl/Sensor.h"

namespace love
{
namespace sensor
{

#define instance() (Module::getInstance<Sensor>(Module::M_SENSOR))

inline Sensor::SensorType luax_checksensortype(lua_State *L, int i)
{
	const char *sensorType = luaL_checkstring(L, i);
	Sensor::SensorType type = Sensor::SENSOR_MAX_ENUM;

	if (!Sensor::getConstant(sensorType, type))
		luax_enumerror(L, "sensor mode", Sensor::getConstants(type), sensorType);

	return type;
}

static int w_hasSensor(lua_State *L)
{
	Sensor::SensorType type = luax_checksensortype(L, 1);

	lua_pushboolean(L, instance()->hasSensor(type));
	return 1;
}

static int w_isEnabled(lua_State *L)
{
	Sensor::SensorType type = luax_checksensortype(L, 1);

	lua_pushboolean(L, instance()->isEnabled(type));
	return 1;
}

static int w_setEnabled(lua_State *L)
{
	Sensor::SensorType type = luax_checksensortype(L, 1);
	bool enabled = luax_checkboolean(L, 2);

	luax_catchexcept(L, [&](){ instance()->setEnabled(type, enabled); });
	return 0;
}

static int w_getData(lua_State *L)
{
	Sensor::SensorType type = luax_checksensortype(L, 1);

	std::vector<float> data;
	luax_catchexcept(L, [&](){ data = instance()->getData(type); });

	for (float f: data)
		lua_pushnumber(L, f);

	return (int) data.size();
}

static int w_getName(lua_State *L)
{
	Sensor::SensorType type = luax_checksensortype(L, 1);
	const char *name = nullptr;

	luax_catchexcept(L, [&](){ name = instance()->getSensorName(type); });
	lua_pushstring(L, name);
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "hasSensor", w_hasSensor },
	{ "isEnabled", w_isEnabled },
	{ "setEnabled", w_setEnabled },
	{ "getData", w_getData },
	{ "getName", w_getName },
	{ nullptr, nullptr }
};

extern "C" int luaopen_love_sensor(lua_State * L)
{
	Sensor *instance = instance();
	if (instance == nullptr)
		luax_catchexcept(L, [&]() { instance = new love::sensor::sdl::Sensor(); });
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "sensor";
	w.type = &Module::type;
	w.functions = functions;
	w.types = nullptr;

	return luax_register_module(L, w);
}

} // sensor
} // love
