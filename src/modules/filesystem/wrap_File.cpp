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

#include "wrap_File.h"

#include "common/Data.h"
#include "common/Exception.h"
#include "common/int.h"

namespace love
{
namespace filesystem
{

int luax_ioError(lua_State *L, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	lua_pushnil(L);
	lua_pushvfstring(L, fmt, args);

	va_end(args);
	return 2;
}

File *luax_checkfile(lua_State *L, int idx)
{
	return luax_checktype<File>(L, idx, FILESYSTEM_FILE_ID);
}

int w_File_getSize(lua_State *L)
{
	File *t = luax_checkfile(L, 1);

	int64 size = -1;
	try
	{
		size = t->getSize();
	}
	catch (love::Exception &e)
	{
		return luax_ioError(L, "%s", e.what());
	}

	// Push nil on failure or if size does not fit into a double precision floating-point number.
	if (size == -1)
		return luax_ioError(L, "Could not determine file size.");
	else if (size >= 0x20000000000000LL)
		return luax_ioError(L, "Size is too large.");

	lua_pushnumber(L, (lua_Number) size);
	return 1;
}

int w_File_open(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	const char *str = luaL_checkstring(L, 2);
	File::Mode mode;

	if (!File::getConstant(str, mode))
		return luaL_error(L, "Incorrect file open mode: %s", str);

	try
	{
		luax_pushboolean(L, file->open(mode));
	}
	catch (love::Exception &e)
	{
		return luax_ioError(L, "%s", e.what());
	}

	return 1;
}

int w_File_close(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	luax_pushboolean(L, file->close());
	return 1;
}

int w_File_isOpen(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	luax_pushboolean(L, file->isOpen());
	return 1;
}

int w_File_read(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	StrongRef<Data> d = nullptr;

	int64 size = (int64) luaL_optnumber(L, 2, (lua_Number) File::ALL);

	try
	{
		d.set(file->read(size), Acquire::NORETAIN);
	}
	catch (love::Exception &e)
	{
		return luax_ioError(L, "%s", e.what());
	}

	lua_pushlstring(L, (const char *) d->getData(), d->getSize());
	lua_pushnumber(L, d->getSize());
	return 2;
}

int w_File_write(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	bool result = false;

	if (lua_isstring(L, 2))
	{
		try
		{
			size_t datasize = 0;
			const char *data = lua_tolstring(L, 2, &datasize);

			if (!lua_isnoneornil(L, 3))
				datasize = luaL_checkinteger(L, 3);

			result = file->write(data, datasize);
		}
		catch (love::Exception &e)
		{
			return luax_ioError(L, "%s", e.what());
		}
	}
	else if (luax_istype(L, 2, DATA_ID))
	{
		try
		{
			love::Data *data = luax_totype<love::Data>(L, 2, DATA_ID);
			result = file->write(data, luaL_optinteger(L, 3, data->getSize()));
		}
		catch (love::Exception &e)
		{
			return luax_ioError(L, "%s", e.what());
		}
	}
	else
	{
		return luaL_argerror(L, 2, "string or data expected");
	}

	luax_pushboolean(L, result);
	return 1;
}

int w_File_flush(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	bool success = false;
	try
	{
		success = file->flush();
	}
	catch (love::Exception &e)
	{
		return luax_ioError(L, "%s", e.what());
	}
	luax_pushboolean(L, success);
	return 1;
}

int w_File_isEOF(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	luax_pushboolean(L, file->isEOF());
	return 1;
}

int w_File_tell(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	int64 pos = file->tell();
	// Push nil on failure or if pos does not fit into a double precision floating-point number.
	if (pos == -1)
		return luax_ioError(L, "Invalid position.");
	else if (pos >= 0x20000000000000LL)
		return luax_ioError(L, "Number is too large.");
	else
		lua_pushnumber(L, (lua_Number)pos);
	return 1;
}

int w_File_seek(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	lua_Number pos = luaL_checknumber(L, 2);

	// Push false on negative and precision-problematic numbers.
	// Better fail than seek to an unknown position.
	if (pos < 0.0 || pos >= 9007199254740992.0)
		luax_pushboolean(L, false);
	else
		luax_pushboolean(L, file->seek((uint64)pos));
	return 1;
}

int w_File_lines_i(lua_State *L)
{
	const int bufsize = 1024;
	char buf[bufsize];
	int linesize = 0;
	bool newline = false;

	File *file = luax_checktype<File>(L, lua_upvalueindex(1), FILESYSTEM_FILE_ID);

	// Only accept read mode at this point.
	if (file->getMode() != File::MODE_READ)
		return luaL_error(L, "File needs to stay in read mode.");

	int64 pos = file->tell();
	int64 userpos = -1;

	if (lua_isnoneornil(L, lua_upvalueindex(2)) == 0)
	{
		// User may have changed the file position.
		userpos = pos;
		pos = (int64) lua_tonumber(L, lua_upvalueindex(2));
		if (userpos != pos)
			file->seek(pos);
	}

	while (!newline && !file->isEOF())
	{
		// This 64-bit to 32-bit integer cast should be safe as it never exceeds bufsize.
		int read = (int) file->read(buf, bufsize);
		if (read < 0)
			return luaL_error(L, "Could not read from file.");

		linesize += read;

		for (int i = 0; i < read; i++)
		{
			if (buf[i] == '\n')
			{
				linesize -= read - i;
				newline = true;
				break;
			}
		}
	}

	if (newline || (file->isEOF() && linesize > 0))
	{
		if (linesize < bufsize)
		{
			// We have the line in the buffer on the stack. No 'new' and 'read' needed.
			lua_pushlstring(L, buf, linesize > 0 && buf[linesize - 1] == '\r' ? linesize - 1 : linesize);
			if (userpos < 0)
				file->seek(pos + linesize + 1);
		}
		else
		{
			char *str = 0;
			try
			{
				str = new char[linesize + 1];
			}
			catch(std::bad_alloc &)
			{
				// Can't lua_error (longjmp) in exception handlers.
			}

			if (!str)
				return luaL_error(L, "Out of memory.");

			file->seek(pos);

			// Read the \n anyway and save us a call to seek.
			if (file->read(str, linesize + 1) == -1)
			{
				delete [] str;
				return luaL_error(L, "Could not read from file.");
			}

			lua_pushlstring(L, str, str[linesize - 1] == '\r' ? linesize - 1 : linesize);
			delete [] str;
		}

		if (userpos >= 0)
		{
			// Save new position in upvalue.
			lua_pushnumber(L, (lua_Number)(pos + linesize + 1));
			lua_replace(L, lua_upvalueindex(2));
			file->seek(userpos);
		}

		return 1;
	}

	// EOF reached.
	if (userpos >= 0 && luax_toboolean(L, lua_upvalueindex(3)))
		file->seek(userpos);
	else
		file->close();

	return 0;
}

int w_File_lines(lua_State *L)
{
	File *file = luax_checkfile(L, 1);

	lua_pushnumber(L, 0); // File position.
	luax_pushboolean(L, file->getMode() != File::MODE_CLOSED); // Save current file mode.

	if (file->getMode() != File::MODE_READ)
	{
		if (file->getMode() != File::MODE_CLOSED)
			file->close();

		bool success = false;
		luax_catchexcept(L, [&](){ success = file->open(File::MODE_READ); });

		if (!success)
			return luaL_error(L, "Could not open file.");
	}

	lua_pushcclosure(L, w_File_lines_i, 3);
	return 1;
}

int w_File_setBuffer(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	const char *str = luaL_checkstring(L, 2);
	int64 size = (int64) luaL_optnumber(L, 3, 0.0);

	File::BufferMode bufmode;
	if (!File::getConstant(str, bufmode))
		return luaL_error(L, "Incorrect file buffer mode: %s", str);

	bool success = false;
	try
	{
		success = file->setBuffer(bufmode, size);
	}
	catch (love::Exception &e)
	{
		return luax_ioError(L, "%s", e.what());
	}

	luax_pushboolean(L, success);
	return 1;
}

int w_File_getBuffer(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	int64 size = 0;
	File::BufferMode bufmode = file->getBuffer(size);
	const char *str = 0;

	if (!File::getConstant(bufmode, str))
		return luax_ioError(L, "Unknown file buffer mode.");

	lua_pushstring(L, str);
	lua_pushnumber(L, (lua_Number) size);
	return 2;
}

int w_File_getMode(lua_State *L)
{
	File *file = luax_checkfile(L, 1);

	File::Mode mode = file->getMode();
	const char *str = 0;

	if (!File::getConstant(mode, str))
		return luax_ioError(L, "Unknown file mode.");

	lua_pushstring(L, str);
	return 1;
}

int w_File_getFilename(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	luax_pushstring(L, file->getFilename());
	return 1;
}

int w_File_getExtension(lua_State *L)
{
	File *file = luax_checkfile(L, 1);
	luax_pushstring(L, file->getExtension());
	return 1;
}

const luaL_Reg w_File_functions[] =
{
	{ "getSize", w_File_getSize },
	{ "open", w_File_open },
	{ "close", w_File_close },
	{ "isOpen", w_File_isOpen },
	{ "read", w_File_read },
	{ "write", w_File_write },
	{ "flush", w_File_flush },
	{ "isEOF", w_File_isEOF },
	{ "tell", w_File_tell },
	{ "seek", w_File_seek },
	{ "lines", w_File_lines },
	{ "setBuffer", w_File_setBuffer },
	{ "getBuffer", w_File_getBuffer },
	{ "getMode", w_File_getMode },
	{ "getFilename", w_File_getFilename },
	{ "getExtension", w_File_getExtension },
	{ 0, 0 }
};

extern "C" int luaopen_file(lua_State *L)
{
	return luax_register_type(L, FILESYSTEM_FILE_ID, "File", w_File_functions, nullptr);
}

} // filesystem
} // love
