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

#ifndef LOVE_SENSOR_H
#define LOVE_SENSOR_H

 // LOVE
#include "common/Module.h"
#include "common/StringMap.h"

namespace love
{
namespace sensor
{

class Sensor: public Module
{
public:

	enum SensorType
	{
		SENSOR_ACCELEROMETER,
		SENSOR_GYROSCOPE,
		SENSOR_MAX_ENUM
	};

	virtual ~Sensor() {}

	/**
	 * Check the availability of the sensor.
	 **/
	virtual bool hasSensor(SensorType type) = 0;

	/**
	 * Check if the sensor is enabled.
	 **/
	virtual bool isEnabled(SensorType type) = 0;

	/**
	 * Enable or disable a sensor.
	 **/
	virtual void setEnabled(SensorType type, bool enabled) = 0;

	/**
	 * Get data from sensor.
	 **/
	virtual std::vector<float> getData(SensorType type) = 0;

	/**
	 * Get backend-dependent handle of sensor.
	 **/
	virtual std::vector<void *> getHandles() = 0;

	virtual const char *getSensorName(SensorType type) = 0;

	STRINGMAP_CLASS_DECLARE(SensorType);

protected:

	Sensor(const char *name);

}; // Sensor

} // sensor
} // love

#endif
