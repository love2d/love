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

#ifndef LOVE_SENSOR_SDL_SENSOR_H
#define LOVE_SENSOR_SDL_SENSOR_H

// LOVE
#include "sensor/Sensor.h"

// SDL
#include <SDL3/SDL_sensor.h>

// std
#include <map>

namespace love
{
namespace sensor
{
namespace sdl
{

class Sensor : public love::sensor::Sensor
{
public:
	Sensor();
	~Sensor() override;

	bool hasSensor(SensorType type) override;
	bool isEnabled(SensorType type) override;
	void setEnabled(SensorType type, bool enable) override;
	std::vector<float> getData(SensorType type) override;
	std::vector<void*> getHandles() override;
	const char *getSensorName(SensorType type) override;

	static SensorType convert(SDL_SensorType type);
	static SDL_SensorType convert(SensorType type);

private:
	std::map<SensorType, SDL_Sensor*> sensors;

}; // Sensor

} // sdl
} // sensor
} // love

#endif
