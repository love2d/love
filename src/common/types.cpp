/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "types.h"
#include "StringMap.h"

namespace love
{

static const TypeBits *createTypeFlags()
{
	static TypeBits b[TYPE_MAX_ENUM];
	TypeBits one = TypeBits(1);

	b[INVALID_ID] = one << INVALID_ID;

	b[OBJECT_ID] = one << OBJECT_ID;
	b[DATA_ID] = (one << DATA_ID) | b[OBJECT_ID];
	b[MODULE_ID] = (one << MODULE_ID) | b[OBJECT_ID];

	// Filesystem.
	b[FILESYSTEM_FILE_ID] = (one << FILESYSTEM_FILE_ID) | b[OBJECT_ID];
	b[FILESYSTEM_DROPPED_FILE_ID] = (one << FILESYSTEM_DROPPED_FILE_ID) | b[FILESYSTEM_FILE_ID];
	b[FILESYSTEM_FILE_DATA_ID] = (one << FILESYSTEM_FILE_DATA_ID) | b[DATA_ID];

	b[FONT_GLYPH_DATA_ID] = (one << FONT_GLYPH_DATA_ID) | b[DATA_ID];
	b[FONT_RASTERIZER_ID] = (one << FONT_RASTERIZER_ID) | b[OBJECT_ID];

	// Graphics.
	b[GRAPHICS_DRAWABLE_ID] = (one << GRAPHICS_DRAWABLE_ID) | b[OBJECT_ID];
	b[GRAPHICS_TEXTURE_ID] = (one << GRAPHICS_TEXTURE_ID) | b[GRAPHICS_DRAWABLE_ID];
	b[GRAPHICS_IMAGE_ID] = (one << GRAPHICS_IMAGE_ID) | b[GRAPHICS_TEXTURE_ID];
	b[GRAPHICS_QUAD_ID] = (one << GRAPHICS_QUAD_ID) | b[OBJECT_ID];
	b[GRAPHICS_FONT_ID] = (one << GRAPHICS_FONT_ID) | b[OBJECT_ID];
	b[GRAPHICS_PARTICLE_SYSTEM_ID] = (one << GRAPHICS_PARTICLE_SYSTEM_ID) | b[GRAPHICS_DRAWABLE_ID];
	b[GRAPHICS_SPRITE_BATCH_ID] = (one << GRAPHICS_SPRITE_BATCH_ID) | b[GRAPHICS_DRAWABLE_ID];
	b[GRAPHICS_CANVAS_ID] = (one << GRAPHICS_CANVAS_ID) | b[GRAPHICS_TEXTURE_ID];
	b[GRAPHICS_SHADER_ID] = (one << GRAPHICS_SHADER_ID) | b[OBJECT_ID];
	b[GRAPHICS_MESH_ID] = (one << GRAPHICS_MESH_ID) | b[GRAPHICS_DRAWABLE_ID];
	b[GRAPHICS_TEXT_ID] = (one << GRAPHICS_TEXT_ID) | b[GRAPHICS_DRAWABLE_ID];

	// Image.
	b[IMAGE_IMAGE_DATA_ID] = (one << IMAGE_IMAGE_DATA_ID) | b[DATA_ID];
	b[IMAGE_COMPRESSED_DATA_ID] = (one << IMAGE_COMPRESSED_DATA_ID) | b[DATA_ID];

	// Joystick.
	b[JOYSTICK_JOYSTICK_ID] = (one << JOYSTICK_JOYSTICK_ID) | b[OBJECT_ID];

	// Math.
	b[MATH_RANDOM_GENERATOR_ID] = (one << MATH_RANDOM_GENERATOR_ID) | b[OBJECT_ID];
	b[MATH_BEZIER_CURVE_ID] = (one << MATH_BEZIER_CURVE_ID) | b[OBJECT_ID];

	// Audio.
	b[AUDIO_SOURCE_ID] = (one << AUDIO_SOURCE_ID) | b[OBJECT_ID];

	// Sound.
	b[SOUND_SOUND_DATA_ID] = (one << SOUND_SOUND_DATA_ID) | b[DATA_ID];
	b[SOUND_DECODER_ID] = one << SOUND_DECODER_ID;

	// Mouse.
	b[MOUSE_CURSOR_ID] = (one << MOUSE_CURSOR_ID) | b[OBJECT_ID];

	// Physics.
	b[PHYSICS_WORLD_ID] = (one << PHYSICS_WORLD_ID) | b[OBJECT_ID];
	b[PHYSICS_CONTACT_ID] = (one << PHYSICS_CONTACT_ID) | b[OBJECT_ID];
	b[PHYSICS_BODY_ID] = (one << PHYSICS_BODY_ID) | b[OBJECT_ID];
	b[PHYSICS_FIXTURE_ID] = (one << PHYSICS_FIXTURE_ID) | b[OBJECT_ID];
	b[PHYSICS_SHAPE_ID] = (one << PHYSICS_SHAPE_ID) | b[OBJECT_ID];
	b[PHYSICS_CIRCLE_SHAPE_ID] = (one << PHYSICS_CIRCLE_SHAPE_ID) | b[PHYSICS_SHAPE_ID];
	b[PHYSICS_POLYGON_SHAPE_ID] = (one << PHYSICS_POLYGON_SHAPE_ID) | b[PHYSICS_SHAPE_ID];
	b[PHYSICS_EDGE_SHAPE_ID] = (one << PHYSICS_EDGE_SHAPE_ID) | b[PHYSICS_SHAPE_ID];
	b[PHYSICS_CHAIN_SHAPE_ID] = (one << PHYSICS_CHAIN_SHAPE_ID) | b[PHYSICS_SHAPE_ID];
	b[PHYSICS_JOINT_ID] = (one << PHYSICS_JOINT_ID) | b[OBJECT_ID];
	b[PHYSICS_MOUSE_JOINT_ID] = (one << PHYSICS_MOUSE_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_DISTANCE_JOINT_ID] = (one << PHYSICS_DISTANCE_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_PRISMATIC_JOINT_ID] = (one << PHYSICS_PRISMATIC_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_REVOLUTE_JOINT_ID] = (one << PHYSICS_REVOLUTE_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_PULLEY_JOINT_ID] = (one << PHYSICS_PULLEY_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_GEAR_JOINT_ID] = (one << PHYSICS_GEAR_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_FRICTION_JOINT_ID] = (one << PHYSICS_FRICTION_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_WELD_JOINT_ID] = (one << PHYSICS_WELD_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_ROPE_JOINT_ID] = (one << PHYSICS_ROPE_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_WHEEL_JOINT_ID] = (one << PHYSICS_WHEEL_JOINT_ID) | b[PHYSICS_JOINT_ID];
	b[PHYSICS_MOTOR_JOINT_ID] = (one << PHYSICS_MOTOR_JOINT_ID) | b[PHYSICS_JOINT_ID];

	// Thread.
	b[THREAD_THREAD_ID] = (one << THREAD_THREAD_ID) | b[OBJECT_ID];
	b[THREAD_CHANNEL_ID] = (one << THREAD_CHANNEL_ID) | b[OBJECT_ID];

	// Modules.
	b[MODULE_FILESYSTEM_ID] = (one << MODULE_FILESYSTEM_ID) | b[MODULE_ID];
	b[MODULE_GRAPHICS_ID] = (one << MODULE_GRAPHICS_ID) | b[MODULE_ID];
	b[MODULE_IMAGE_ID] = (one << MODULE_IMAGE_ID) | b[MODULE_ID];
	b[MODULE_SOUND_ID] = (one << MODULE_SOUND_ID) | b[MODULE_ID];

	return b;
}

const TypeBits *typeFlags = createTypeFlags();

StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[] =
{
	{"Invalid", INVALID_ID},

	{"Object", OBJECT_ID},
	{"Data", DATA_ID},
	{"Module", MODULE_ID},

	// Filesystem
	{"File", FILESYSTEM_FILE_ID},
	{"DroppedFile", FILESYSTEM_DROPPED_FILE_ID},
	{"FileData", FILESYSTEM_FILE_DATA_ID},

	// Font
	{"GlyphData", FONT_GLYPH_DATA_ID},
	{"Rasterizer", FONT_RASTERIZER_ID},

	// Graphics
	{"Drawable", GRAPHICS_DRAWABLE_ID},
	{"Texture", GRAPHICS_TEXTURE_ID},
	{"Image", GRAPHICS_IMAGE_ID},
	{"Quad", GRAPHICS_QUAD_ID},
	{"Font", GRAPHICS_FONT_ID},
	{"ParticleSystem", GRAPHICS_PARTICLE_SYSTEM_ID},
	{"SpriteBatch", GRAPHICS_SPRITE_BATCH_ID},
	{"Canvas", GRAPHICS_CANVAS_ID},
	{"Shader", GRAPHICS_SHADER_ID},
	{"Mesh", GRAPHICS_MESH_ID},
	{"Text", GRAPHICS_TEXT_ID},

	// Image
	{"ImageData", IMAGE_IMAGE_DATA_ID},
	{"CompressedData", IMAGE_COMPRESSED_DATA_ID},

	// Joystick
	{"Joystick", JOYSTICK_JOYSTICK_ID},

	// Math
	{"RandomGenerator", MATH_RANDOM_GENERATOR_ID},
	{"BezierCurve", MATH_BEZIER_CURVE_ID},

	// Audio
	{"Source", AUDIO_SOURCE_ID},

	// Sound
	{"SoundData", SOUND_SOUND_DATA_ID},
	{"Decoder", SOUND_DECODER_ID},

	// Mouse
	{"Cursor", MOUSE_CURSOR_ID},

	// Physics
	{"World", PHYSICS_WORLD_ID},
	{"Contact", PHYSICS_CONTACT_ID},
	{"Body", PHYSICS_BODY_ID},
	{"Fixture", PHYSICS_FIXTURE_ID},
	{"Shape", PHYSICS_SHAPE_ID},
	{"CircleShape", PHYSICS_CIRCLE_SHAPE_ID},
	{"PolygonShape", PHYSICS_POLYGON_SHAPE_ID},
	{"EdgeShape", PHYSICS_EDGE_SHAPE_ID},
	{"ChainShape", PHYSICS_CHAIN_SHAPE_ID},
	{"Joint", PHYSICS_JOINT_ID},
	{"MouseJoint", PHYSICS_MOUSE_JOINT_ID},
	{"DistanceJoint", PHYSICS_DISTANCE_JOINT_ID},
	{"PrismaticJoint", PHYSICS_PRISMATIC_JOINT_ID},
	{"RevoluteJoint", PHYSICS_REVOLUTE_JOINT_ID},
	{"PulleyJoint", PHYSICS_PULLEY_JOINT_ID},
	{"GearJoint", PHYSICS_GEAR_JOINT_ID},
	{"FrictionJoint", PHYSICS_FRICTION_JOINT_ID},
	{"WeldJoint", PHYSICS_WELD_JOINT_ID},
	{"RopeJoint", PHYSICS_ROPE_JOINT_ID},
	{"WheelJoint", PHYSICS_WHEEL_JOINT_ID},
	{"MotorJoint", PHYSICS_MOTOR_JOINT_ID},

	// Thread
	{"Thread", THREAD_THREAD_ID},
	{"Channel", THREAD_CHANNEL_ID},

	// The modules themselves. Only add abstracted modules here.
	{"filesystem", MODULE_FILESYSTEM_ID},
	{"graphics", MODULE_GRAPHICS_ID},
	{"image", MODULE_IMAGE_ID},
	{"sound", MODULE_SOUND_ID},
};

StringMap<Type, TYPE_MAX_ENUM> types(typeEntries, sizeof(typeEntries));

static_assert((sizeof(typeEntries) / sizeof(typeEntries[0])) == TYPE_MAX_ENUM, "Type name array size doesn't match the total number of type IDs!");

bool getType(const char *in, love::Type &out)
{
	return types.find(in, out);
}

bool getType(love::Type in, const char *&out)
{
	return types.find(in, out);
}

} // love
