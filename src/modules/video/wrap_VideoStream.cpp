/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "wrap_VideoStream.h"

namespace love
{
namespace video
{

VideoStream *luax_checkvideostream(lua_State *L, int idx)
{
	return luax_checktype<VideoStream>(L, idx);
}

int w_VideoStream_setSync(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);

	if (luax_istype(L, 2, love::audio::Source::type))
	{
		auto src = luax_totype<love::audio::Source>(L, 2);
		auto sync = new VideoStream::SourceSync(src);
		stream->setSync(sync);
		sync->release();
	}
	else if (luax_istype(L, 2, VideoStream::type))
	{
		auto other = luax_totype<VideoStream>(L, 2);
		stream->setSync(other->getSync());
	}
	else if (lua_isnoneornil(L, 2))
	{
		auto newSync = new VideoStream::DeltaSync();
		newSync->copyState(stream->getSync());
		stream->setSync(newSync);
		newSync->release();
	}
	else
		return luax_typerror(L, 2, "Source or VideoStream or nil");

	return 0;
}

int w_VideoStream_getFilename(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	luax_pushstring(L, stream->getFilename());
	return 1;
}

int w_VideoStream_play(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	stream->play();
	return 0;
}

int w_VideoStream_pause(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	stream->pause();
	return 0;
}

int w_VideoStream_seek(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	double offset = luaL_checknumber(L, 2);
	stream->seek(offset);
	return 0;
}

int w_VideoStream_rewind(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	stream->seek(0);
	return 0;
}

int w_VideoStream_tell(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	lua_pushnumber(L, stream->tell());
	return 1;
}

int w_VideoStream_isPlaying(lua_State *L)
{
	auto stream = luax_checkvideostream(L, 1);
	luax_pushboolean(L, stream->isPlaying());
	return 1;
}

static const luaL_Reg videostream_functions[] =
{
	{ "setSync", w_VideoStream_setSync },
	{ "getFilename", w_VideoStream_getFilename },
	{ "play", w_VideoStream_play },
	{ "pause", w_VideoStream_pause },
	{ "seek", w_VideoStream_seek },
	{ "rewind", w_VideoStream_rewind },
	{ "tell", w_VideoStream_tell },
	{ "isPlaying", w_VideoStream_isPlaying },
	{ 0, 0 }
};

int luaopen_videostream(lua_State *L)
{
	return luax_register_type(L, &VideoStream::type, videostream_functions, nullptr);
}

} // video
} // love
