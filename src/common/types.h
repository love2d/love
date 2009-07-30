/**
* Copyright (c) 2006-2009 LOVE Development Team
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
	enum
	{
		// Cross-module types.
		LOVE_OBJECT_ID = 0,
		LOVE_DATA_ID,

		// Filesystem.
		LOVE_FILESYSTEM_FILE_ID,
		LOVE_FILESYSTEM_FILE_DATA_ID,

		// Font
		LOVE_FONT_GLYPH_DATA_ID,
		LOVE_FONT_RASTERIZER_ID,

		// Graphics
		LOVE_GRAPHICS_DRAWABLE_ID,
		LOVE_GRAPHICS_IMAGE_ID,
		LOVE_GRAPHICS_FRAME_ID,
		LOVE_GRAPHICS_GLYPH_ID,
		LOVE_GRAPHICS_ANIMATION_ID,
		LOVE_GRAPHICS_COLOR_ID,
		LOVE_GRAPHICS_FONT_ID,
		LOVE_GRAPHICS_PARTICLE_SYSTEM_ID,
		LOVE_GRAPHICS_SPRITE_BATCH_ID,
		LOVE_GRAPHICS_VERTEX_BUFFER_ID,

		// Image
		LOVE_IMAGE_IMAGE_DATA_ID,

		// Audio
		LOVE_AUDIO_AUDIBLE_ID,
		LOVE_AUDIO_SOUND_ID,
		LOVE_AUDIO_MUSIC_ID,
		LOVE_AUDIO_SOURCE_ID,

		// Sound
		LOVE_SOUND_SOUND_DATA_ID,
		LOVE_SOUND_DECODER_ID,

		// Physics
		LOVE_PHYSICS_WORLD_ID,
		LOVE_PHYSICS_CONTACT_ID,
		LOVE_PHYSICS_BODY_ID,
		LOVE_PHYSICS_SHAPE_ID,
		LOVE_PHYSICS_CIRCLE_SHAPE_ID,
		LOVE_PHYSICS_POLYGON_SHAPE_ID,
		LOVE_PHYSICS_JOINT_ID,
		LOVE_PHYSICS_MOUSE_JOINT_ID,
		LOVE_PHYSICS_DISTANCE_JOINT_ID,
		LOVE_PHYSICS_PRISMATIC_JOINT_ID,
		LOVE_PHYSICS_REVOLUTE_JOINT_ID,
		LOVE_PHYSICS_PULLEY_JOINT_ID,
		LOVE_PHYSICS_GEAR_JOINT_ID,

		// Modules.
		LOVE_PHYSFS_ID, 
		LOVE_OPENGL_ID, 
		LOVE_DIRECT3D_ID,
		LOVE_BOX2D_ID,
		LOVE_DEVIL_ID,
		LOVE_SDLSOUND_ID,
		LOVE_OPENAL_ID,

		// Count the number of bits needed.
		LOVE_BIT_SIZE
	};

	typedef std::bitset<LOVE_BIT_SIZE> bits;

	const bits LOVE_OBJECT_BITS = bits(1) << LOVE_OBJECT_ID;
	const bits LOVE_DATA_BITS = (bits(1) << LOVE_DATA_ID) | LOVE_OBJECT_BITS;

	// Filesystem.
	const bits LOVE_FILESYSTEM_FILE_BITS = (bits(1) << LOVE_FILESYSTEM_FILE_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_FILESYSTEM_FILE_DATA_BITS = (bits(1) << LOVE_FILESYSTEM_FILE_DATA_ID) | LOVE_DATA_BITS;

	const bits LOVE_FONT_GLYPH_DATA_BITS = (bits(1) << LOVE_FONT_GLYPH_DATA_ID) | LOVE_DATA_BITS;
	const bits LOVE_FONT_RASTERIZER_BITS = (bits(1) << LOVE_FONT_RASTERIZER_ID) | LOVE_OBJECT_BITS;

	// Graphics.
	const bits LOVE_GRAPHICS_DRAWABLE_BITS = (bits(1) << LOVE_GRAPHICS_DRAWABLE_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_GRAPHICS_IMAGE_BITS = (bits(1) << LOVE_GRAPHICS_IMAGE_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;
	const bits LOVE_GRAPHICS_FRAME_BITS = (bits(1) << LOVE_GRAPHICS_FRAME_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_GRAPHICS_GLYPH_BITS = (bits(1) << LOVE_GRAPHICS_GLYPH_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;
	const bits LOVE_GRAPHICS_ANIMATION_BITS = (bits(1) << LOVE_GRAPHICS_ANIMATION_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;
	const bits LOVE_GRAPHICS_COLOR_BITS = (bits(1) << LOVE_GRAPHICS_COLOR_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_GRAPHICS_FONT_BITS = (bits(1) << LOVE_GRAPHICS_FONT_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_GRAPHICS_PARTICLE_SYSTEM_BITS = (bits(1) << LOVE_GRAPHICS_PARTICLE_SYSTEM_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;
	const bits LOVE_GRAPHICS_SPRITE_BATCH_BITS = (bits(1) << LOVE_GRAPHICS_SPRITE_BATCH_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;
	const bits LOVE_GRAPHICS_VERTEX_BUFFER_BITS = (bits(1) << LOVE_GRAPHICS_VERTEX_BUFFER_ID) | LOVE_GRAPHICS_DRAWABLE_BITS;

	// Image.
	const bits LOVE_IMAGE_IMAGE_DATA_BITS = (bits(1) << LOVE_IMAGE_IMAGE_DATA_ID) | LOVE_DATA_BITS;
	
	// Audio.
	const bits LOVE_AUDIO_AUDIBLE_BITS = (bits(1) << LOVE_AUDIO_AUDIBLE_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_AUDIO_SOUND_BITS = (bits(1) << LOVE_AUDIO_SOUND_ID) | LOVE_AUDIO_AUDIBLE_BITS;
	const bits LOVE_AUDIO_MUSIC_BITS = (bits(1) << LOVE_AUDIO_MUSIC_ID) | LOVE_AUDIO_AUDIBLE_BITS;
	const bits LOVE_AUDIO_SOURCE_BITS = (bits(1) << LOVE_AUDIO_SOURCE_ID) | LOVE_OBJECT_BITS;

	// Sound.
	const bits LOVE_SOUND_SOUND_DATA_BITS = (bits(1) << LOVE_SOUND_SOUND_DATA_ID) | LOVE_DATA_BITS;
	const bits LOVE_SOUND_DECODER_BITS = bits(1) << LOVE_SOUND_DECODER_ID;

	// Physics.
	const bits LOVE_PHYSICS_WORLD_BITS = (bits(1) << LOVE_PHYSICS_WORLD_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_PHYSICS_CONTACT_BITS = (bits(1) << LOVE_PHYSICS_CONTACT_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_PHYSICS_BODY_BITS = (bits(1) << LOVE_PHYSICS_BODY_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_PHYSICS_SHAPE_BITS = (bits(1) << LOVE_PHYSICS_SHAPE_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_PHYSICS_CIRCLE_SHAPE_BITS = (bits(1) << LOVE_PHYSICS_CIRCLE_SHAPE_ID) | LOVE_PHYSICS_SHAPE_BITS;
	const bits LOVE_PHYSICS_POLYGON_SHAPE_BITS = (bits(1) << LOVE_PHYSICS_POLYGON_SHAPE_ID) | LOVE_PHYSICS_SHAPE_BITS;
	const bits LOVE_PHYSICS_JOINT_BITS = (bits(1) << LOVE_PHYSICS_JOINT_ID) | LOVE_OBJECT_BITS;
	const bits LOVE_PHYSICS_MOUSE_JOINT_BITS = (bits(1) << LOVE_PHYSICS_MOUSE_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;
	const bits LOVE_PHYSICS_DISTANCE_JOINT_BITS = (bits(1) << LOVE_PHYSICS_DISTANCE_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;
	const bits LOVE_PHYSICS_PRISMATIC_JOINT_BITS = (bits(1) << LOVE_PHYSICS_PRISMATIC_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;
	const bits LOVE_PHYSICS_REVOLUTE_JOINT_BITS = (bits(1) << LOVE_PHYSICS_REVOLUTE_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;
	const bits LOVE_PHYSICS_PULLEY_JOINT_BITS = (bits(1) << LOVE_PHYSICS_PULLEY_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;
	const bits LOVE_PHYSICS_GEAR_JOINT_BITS = (bits(1) << LOVE_PHYSICS_GEAR_JOINT_ID) | LOVE_PHYSICS_JOINT_BITS;

	// Modules.
	const bits LOVE_PHYSFS_BITS = bits(1) << LOVE_PHYSFS_ID;
	const bits LOVE_OPENGL_BITS = bits(1) << LOVE_OPENGL_ID;
	const bits LOVE_DIRECT3D_BITS = bits(1) << LOVE_DIRECT3D_ID;
	const bits LOVE_BOX2D_BITS = bits(1) << LOVE_BOX2D_ID;
	const bits LOVE_DEVIL_BITS = bits(1) << LOVE_DEVIL_ID;
	const bits LOVE_SDLSOUND_BITS = bits(1) << LOVE_SDLSOUND_ID;
	const bits LOVE_OPENAL_BITS = bits(1) << LOVE_OPENAL_ID;

	// Wraps all userdata pointers.
	struct userdata
	{
		bits flags; // Holds type information.
		void * data;
		bool own; // True if Lua should delete on GC.
	};

} // love

#endif // LOVE_TYPES_H
