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

// LOVE
#include "filesystem/wrap_Filesystem.h"

#include "theora/Video.h"
#include "wrap_Video.h"
#include "wrap_VideoStream.h"

namespace love
{
namespace video
{

#define instance() (Module::getInstance<Video>(Module::M_VIDEO))

int w_newVideoStream(lua_State *L)
{
	love::filesystem::File *file = love::filesystem::luax_getfile(L, 1);

	VideoStream *stream = nullptr;
	luax_catchexcept(L, [&]() {
		// Can't check if open for reading
		if (!file->isOpen() && !file->open(love::filesystem::File::MODE_READ))
			luaL_error(L, "File is not open and cannot be opened");

		stream = instance()->newVideoStream(file);
	});

	luax_pushtype(L, VIDEO_VIDEO_STREAM_ID, stream);
	stream->release();
	return 1;
}

static const lua_CFunction types[] =
{
	luaopen_videostream,
	0
};

static const luaL_Reg functions[] =
{
	{ "newVideoStream", w_newVideoStream },
	{ 0, 0 }
};

extern "C" int luaopen_love_video(lua_State *L)
{
	Video *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::video::theora::Video(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "video";
	w.type = MODULE_ID;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // video
} // love
