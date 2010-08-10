/**
* Copyright (c) 2006-2010 LOVE Development Team
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

		// Filesystem.
		FILESYSTEM_FILE_ID,
		FILESYSTEM_FILE_DATA_ID,

		// Font
		FONT_FONT_DATA_ID,
		FONT_GLYPH_DATA_ID,
		FONT_RASTERIZER_ID,

		// Graphics
		GRAPHICS_DRAWABLE_ID,
		GRAPHICS_IMAGE_ID,
		GRAPHICS_QUAD_ID,
		GRAPHICS_GLYPH_ID,
		GRAPHICS_FONT_ID,
		GRAPHICS_PARTICLE_SYSTEM_ID,
		GRAPHICS_SPRITE_BATCH_ID,
		GRAPHICS_VERTEX_BUFFER_ID,

		// Image
		IMAGE_IMAGE_DATA_ID,
		IMAGE_ENCODED_IMAGE_DATA_ID,

		// Audio
		AUDIO_SOURCE_ID,

		// Sound
		SOUND_SOUND_DATA_ID,
		SOUND_DECODER_ID,

		// Physics
		PHYSICS_WORLD_ID,
		PHYSICS_CONTACT_ID,
		PHYSICS_BODY_ID,
		PHYSICS_SHAPE_ID,
		PHYSICS_CIRCLE_SHAPE_ID,
		PHYSICS_POLYGON_SHAPE_ID,
		PHYSICS_JOINT_ID,
		PHYSICS_MOUSE_JOINT_ID,
		PHYSICS_DISTANCE_JOINT_ID,
		PHYSICS_PRISMATIC_JOINT_ID,
		PHYSICS_REVOLUTE_JOINT_ID,
		PHYSICS_PULLEY_JOINT_ID,
		PHYSICS_GEAR_JOINT_ID,

		// Thread
		THREAD_THREAD_ID,

		// The modules themselves. Only add abstracted modules here.
		MODULE_FILESYSTEM_ID,
		MODULE_IMAGE_ID,
		MODULE_SOUND_ID,

		// Count the number of bits needed.
		TYPE_MAX_ENUM
	};

	typedef std::bitset<TYPE_MAX_ENUM> bits;

	const bits INVALID_T = bits(1) << INVALID_ID;

	const bits OBJECT_T = bits(1) << OBJECT_ID;
	const bits DATA_T = (bits(1) << DATA_ID) | OBJECT_T;
	const bits MODULE_T = (bits(1) << MODULE_ID) | OBJECT_T;

	// Filesystem.
	const bits FILESYSTEM_FILE_T = (bits(1) << FILESYSTEM_FILE_ID) | OBJECT_T;
	const bits FILESYSTEM_FILE_DATA_T = (bits(1) << FILESYSTEM_FILE_DATA_ID) | DATA_T;

	const bits FONT_FONT_DATA_T = (bits(1) << FONT_FONT_DATA_ID) | DATA_T;
	const bits FONT_GLYPH_DATA_T = (bits(1) << FONT_GLYPH_DATA_ID) | DATA_T;
	const bits FONT_RASTERIZER_T = (bits(1) << FONT_RASTERIZER_ID) | OBJECT_T;

	// Graphics.
	const bits GRAPHICS_DRAWABLE_T = (bits(1) << GRAPHICS_DRAWABLE_ID) | OBJECT_T;
	const bits GRAPHICS_IMAGE_T = (bits(1) << GRAPHICS_IMAGE_ID) | GRAPHICS_DRAWABLE_T;
	const bits GRAPHICS_QUAD_T = (bits(1) << GRAPHICS_QUAD_ID) | OBJECT_T;
	const bits GRAPHICS_GLYPH_T = (bits(1) << GRAPHICS_GLYPH_ID) | GRAPHICS_DRAWABLE_T;
	const bits GRAPHICS_FONT_T = (bits(1) << GRAPHICS_FONT_ID) | OBJECT_T;
	const bits GRAPHICS_PARTICLE_SYSTEM_T = (bits(1) << GRAPHICS_PARTICLE_SYSTEM_ID) | GRAPHICS_DRAWABLE_T;
	const bits GRAPHICS_SPRITE_BATCH_T = (bits(1) << GRAPHICS_SPRITE_BATCH_ID) | GRAPHICS_DRAWABLE_T;
	const bits GRAPHICS_VERTEX_BUFFER_T = (bits(1) << GRAPHICS_VERTEX_BUFFER_ID) | GRAPHICS_DRAWABLE_T;

	// Image.
	const bits IMAGE_IMAGE_DATA_T = (bits(1) << IMAGE_IMAGE_DATA_ID) | DATA_T;
	const bits IMAGE_ENCODED_IMAGE_DATA_T = (bits(1) << IMAGE_ENCODED_IMAGE_DATA_ID) | DATA_T;

	// Audio.
	const bits AUDIO_SOURCE_T = (bits(1) << AUDIO_SOURCE_ID) | OBJECT_T;

	// Sound.
	const bits SOUND_SOUND_DATA_T = (bits(1) << SOUND_SOUND_DATA_ID) | DATA_T;
	const bits SOUND_DECODER_T = bits(1) << SOUND_DECODER_ID;

	// Physics.
	const bits PHYSICS_WORLD_T = (bits(1) << PHYSICS_WORLD_ID) | OBJECT_T;
	const bits PHYSICS_CONTACT_T = (bits(1) << PHYSICS_CONTACT_ID) | OBJECT_T;
	const bits PHYSICS_BODY_T = (bits(1) << PHYSICS_BODY_ID) | OBJECT_T;
	const bits PHYSICS_SHAPE_T = (bits(1) << PHYSICS_SHAPE_ID) | OBJECT_T;
	const bits PHYSICS_CIRCLE_SHAPE_T = (bits(1) << PHYSICS_CIRCLE_SHAPE_ID) | PHYSICS_SHAPE_T;
	const bits PHYSICS_POLYGON_SHAPE_T = (bits(1) << PHYSICS_POLYGON_SHAPE_ID) | PHYSICS_SHAPE_T;
	const bits PHYSICS_JOINT_T = (bits(1) << PHYSICS_JOINT_ID) | OBJECT_T;
	const bits PHYSICS_MOUSE_JOINT_T = (bits(1) << PHYSICS_MOUSE_JOINT_ID) | PHYSICS_JOINT_T;
	const bits PHYSICS_DISTANCE_JOINT_T = (bits(1) << PHYSICS_DISTANCE_JOINT_ID) | PHYSICS_JOINT_T;
	const bits PHYSICS_PRISMATIC_JOINT_T = (bits(1) << PHYSICS_PRISMATIC_JOINT_ID) | PHYSICS_JOINT_T;
	const bits PHYSICS_REVOLUTE_JOINT_T = (bits(1) << PHYSICS_REVOLUTE_JOINT_ID) | PHYSICS_JOINT_T;
	const bits PHYSICS_PULLEY_JOINT_T = (bits(1) << PHYSICS_PULLEY_JOINT_ID) | PHYSICS_JOINT_T;
	const bits PHYSICS_GEAR_JOINT_T = (bits(1) << PHYSICS_GEAR_JOINT_ID) | PHYSICS_JOINT_T;

	// Thread.
	const bits THREAD_THREAD_T = (bits(1) << THREAD_THREAD_ID) | OBJECT_T;

	// Modules.
	const bits MODULE_FILESYSTEM_T = (bits(1) << MODULE_FILESYSTEM_ID) | MODULE_T;
	const bits MODULE_IMAGE_T = (bits(1) << MODULE_IMAGE_ID) | MODULE_T;
	const bits MODULE_SOUND_T = (bits(1) << MODULE_SOUND_ID) | MODULE_T;

	bool getType(const char * in, Type & out);
	bool getType(Type in, const char *& out);

} // love

#endif // LOVE_TYPES_H
