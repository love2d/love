/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_TYPES_H
#define LOVE_TYPES_H

// STD
#include <bitset>

namespace love
{

enum Type
{
	INVALID_ID = 0,
	// Cross-module types.
	OBJECT_ID,
	DATA_ID,
	MODULE_ID,
	STREAM_ID,

	// Filesystem.
	FILESYSTEM_FILE_ID,
	FILESYSTEM_DROPPED_FILE_ID,
	FILESYSTEM_FILE_DATA_ID,

	// Font
	FONT_GLYPH_DATA_ID,
	FONT_RASTERIZER_ID,

	// Graphics
	GRAPHICS_DRAWABLE_ID,
	GRAPHICS_TEXTURE_ID,
	GRAPHICS_IMAGE_ID,
	GRAPHICS_QUAD_ID,
	GRAPHICS_FONT_ID,
	GRAPHICS_PARTICLE_SYSTEM_ID,
	GRAPHICS_SPRITE_BATCH_ID,
	GRAPHICS_CANVAS_ID,
	GRAPHICS_SHADER_ID,
	GRAPHICS_MESH_ID,
	GRAPHICS_TEXT_ID,
	GRAPHICS_VIDEO_ID,

	// Image
	IMAGE_IMAGE_DATA_ID,
	IMAGE_COMPRESSED_IMAGE_DATA_ID,

	// Joystick
	JOYSTICK_JOYSTICK_ID,

	// Math
	MATH_RANDOM_GENERATOR_ID,
	MATH_BEZIER_CURVE_ID,
	MATH_COMPRESSED_DATA_ID,

	// Audio
	AUDIO_SOURCE_ID,

	// Sound
	SOUND_SOUND_DATA_ID,
	SOUND_DECODER_ID,

	// Mouse
	MOUSE_CURSOR_ID,

	// Physics
	PHYSICS_WORLD_ID,
	PHYSICS_CONTACT_ID,
	PHYSICS_BODY_ID,
	PHYSICS_FIXTURE_ID,
	PHYSICS_SHAPE_ID,
	PHYSICS_CIRCLE_SHAPE_ID,
	PHYSICS_POLYGON_SHAPE_ID,
	PHYSICS_EDGE_SHAPE_ID,
	PHYSICS_CHAIN_SHAPE_ID,
	PHYSICS_JOINT_ID,
	PHYSICS_MOUSE_JOINT_ID,
	PHYSICS_DISTANCE_JOINT_ID,
	PHYSICS_PRISMATIC_JOINT_ID,
	PHYSICS_REVOLUTE_JOINT_ID,
	PHYSICS_PULLEY_JOINT_ID,
	PHYSICS_GEAR_JOINT_ID,
	PHYSICS_FRICTION_JOINT_ID,
	PHYSICS_WELD_JOINT_ID,
	PHYSICS_ROPE_JOINT_ID,
	PHYSICS_WHEEL_JOINT_ID,
	PHYSICS_MOTOR_JOINT_ID,

	// Thread
	THREAD_THREAD_ID,
	THREAD_CHANNEL_ID,

	// Video
	VIDEO_VIDEO_STREAM_ID,

	// The modules themselves. Only add abstracted modules here.
	MODULE_FILESYSTEM_ID,
	MODULE_GRAPHICS_ID,
	MODULE_IMAGE_ID,
	MODULE_SOUND_ID,

	// Count the number of bits needed.
	TYPE_MAX_ENUM
};

typedef std::bitset<TYPE_MAX_ENUM> TypeBits;

/**
 * Array of length TYPE_MAX_ENUM containing the flags for each love Type.
 **/
extern const TypeBits *typeFlags;

void addTypeName(Type type, const char *name);
bool getTypeName(const char *in, Type &out);
bool getTypeName(Type in, const char *&out);

} // love

#endif // LOVE_TYPES_H
