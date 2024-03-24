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
#include "Sensor.h"

// SDL
#include <SDL.h>
#include <SDL_sensor.h>

namespace love
{
namespace sensor
{
namespace sdl
{

Sensor::Sensor()
	: love::sensor::Sensor("love.sensor.sdl")
	, sensors()
{
	if (SDL_InitSubSystem(SDL_INIT_SENSOR) < 0)
		throw love::Exception("Could not initialize SDL sensor subsystem (%s)", SDL_GetError());
}

Sensor::~Sensor()
{
	SDL_QuitSubSystem(SDL_INIT_SENSOR);
}

bool Sensor::hasSensor(SensorType type)
{
#if SDL_VERSION_ATLEAST(3, 0, 0)
	int count = 0;
	SDL_SensorID *sensorIDs = SDL_GetSensors(&count);
	for (int i = 0; i < count; i++)
	{
		if (convert(SDL_GetSensorInstanceType(sensorIDs[i])) == type)
		{
			SDL_free(sensorIDs);
			return true;
		}
	}
	SDL_free(sensorIDs);
#else
	for (int i = 0; i < SDL_NumSensors(); i++)
	{
		if (convert(SDL_SensorGetDeviceType(i)) == type)
			return true;
	}
#endif

	return false;
}

bool Sensor::isEnabled(SensorType type)
{
	return sensors[type]; // nullptr is default
}

void Sensor::setEnabled(SensorType type, bool enable)
{
	if (sensors[type] && !enable)
	{
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_CloseSensor(sensors[type]);
#else
		SDL_SensorClose(sensors[type]);
#endif
		sensors[type] = nullptr;
	}
	else if (sensors[type] == nullptr && enable)
	{
#if SDL_VERSION_ATLEAST(3, 0, 0)
		int count = 0;
		SDL_SensorID *sensorIDs = SDL_GetSensors(&count);
		for (int i = 0; i < count; i++)
		{
			if (convert(SDL_GetSensorInstanceType(sensorIDs[i])) == type)
			{
				SDL_Sensor *sensorHandle = SDL_OpenSensor(sensorIDs[i]);

				if (sensorHandle == nullptr)
				{
					SDL_free(sensorIDs);

					const char *name = nullptr;
					getConstant(type, name);
					throw love::Exception("Could not open \"%s\" SDL sensor (%s)", name, SDL_GetError());
				}

				sensors[type] = sensorHandle;
			}
		}
		SDL_free(sensorIDs);
#else
		for (int i = 0; i < SDL_NumSensors(); i++)
		{
			if (convert(SDL_SensorGetDeviceType(i)) == type)
			{
				SDL_Sensor *sensorHandle = SDL_SensorOpen(i);

				if (sensorHandle == nullptr)
				{
					const char *name = nullptr;
					getConstant(type, name);
					throw love::Exception("Could not open \"%s\" SDL sensor (%s)", name, SDL_GetError());
				}

				sensors[type] = sensorHandle;
			}
		}
#endif
	}
}

std::vector<float> Sensor::getData(SensorType type)
{
	if (sensors[type] == nullptr)
	{
		const char *name = nullptr;
		getConstant(type, name);

		throw love::Exception("\"%s\" sensor is not enabled", name);
	}

	std::vector<float> values(3);

#if SDL_VERSION_ATLEAST(3, 0, 0)
	if (SDL_GetSensorData(sensors[type], values.data(), (int) values.size()) != 0)
#else
	if (SDL_SensorGetData(sensors[type], values.data(), (int) values.size()) != 0)
#endif
	{
		const char *name = nullptr;
		getConstant(type, name);

		throw love::Exception("Could not get \"%s\" SDL sensor data (%s)", name, SDL_GetError());
	}

	return values;
}

std::vector<void*> Sensor::getHandles()
{
	std::vector<void*> nativeSensor;

	for (std::pair<SensorType, SDL_Sensor*> data : sensors)
	{
		if (data.second)
			nativeSensor.push_back(data.second);
	}

	return nativeSensor;
}

const char *Sensor::getSensorName(SensorType type)
{
	if (sensors[type] == nullptr)
	{
		const char *name = nullptr;
		getConstant(type, name);

		throw love::Exception("\"%s\" sensor is not enabled", name);
	}

#if SDL_VERSION_ATLEAST(3, 0, 0)
	return SDL_GetSensorName(sensors[type]);
#else
	return SDL_SensorGetName(sensors[type]);
#endif
}

Sensor::SensorType Sensor::convert(SDL_SensorType type)
{
	switch (type)
	{
		case SDL_SENSOR_ACCEL:
			return SENSOR_ACCELEROMETER;
		case SDL_SENSOR_GYRO:
			return SENSOR_GYROSCOPE;
		default:
			return SENSOR_MAX_ENUM;
	}
}

SDL_SensorType Sensor::convert(Sensor::SensorType type)
{
	switch (type)
	{
		case SENSOR_ACCELEROMETER:
			return SDL_SENSOR_ACCEL;
		case SENSOR_GYROSCOPE:
			return SDL_SENSOR_GYRO;
		default:
			return SDL_SENSOR_UNKNOWN;
	}
}

}
}
}
