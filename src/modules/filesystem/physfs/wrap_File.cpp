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

#include "wrap_File.h"

namespace love
{
namespace filesystem
{
namespace physfs
{
	File * luax_checkfile(lua_State * L, int idx)
	{
		return luax_checktype<File>(L, idx, "File", LOVE_FILESYSTEM_FILE_BITS);
	}

	int _wrap_File_getSize(lua_State * L)
	{
		File * t = luax_checkfile(L, 1);
		lua_pushinteger(L, t->getSize());
		return 1;
	}

	int _wrap_File_open(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		int mode = luaL_optint(L, 2, File::READ);
		lua_pushboolean(L, file->open((File::Mode)mode) ? 1 : 0);
		return 1;
	}

	int _wrap_File_close(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		lua_pushboolean(L, file->close() ? 1 : 0);
		return 1;
	}

	int _wrap_File_read(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		Data * d = file->read(luaL_optint(L, 2, file->getSize()));
		lua_pushlstring(L, (const char*) d->getData(), d->getSize());
		lua_pushnumber(L, d->getSize());
		d->release();
		return 2;
	}

	int _wrap_File_write(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		bool result;
		if ( file->getMode() == File::CLOSED )
			return luaL_error(L, "File is not open.");
		if ( lua_isstring(L, 2) )
			result = file->write(lua_tostring(L, 2), luaL_optint(L, 3, lua_objlen(L, 2)));
		else
			return luaL_error(L, "String expected.");
		lua_pushboolean(L, result);
		return 1;
	}

	int _wrap_File_eof(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		lua_pushboolean(L, file->eof() ? 1 : 0);
		return 1;
	}

	int _wrap_File_tell(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		lua_pushinteger(L, file->tell());
		return 1;
	}

	int _wrap_File_seek(lua_State * L)
	{
		File * file = luax_checkfile(L, 1);
		int pos = luaL_checkinteger(L, 2);
		lua_pushboolean(L, file->seek(pos) ? 1 : 0);
		return 1;
	}

	//yes, the following two are copy-pasted and slightly edited

	int _wrap_File_lines(lua_State * L)
	{
		File * file;

		if(luax_istype(L, 1, LOVE_FILESYSTEM_FILE_BITS))
		{
			file = luax_checktype<File>(L, 1, "File", LOVE_FILESYSTEM_FILE_BITS);
			lua_pushboolean(L, 0); // 0 = do not close.
		}
		else
			return luaL_error(L, "Expected file handle.");

		// Reset the file position.
		if(!file->seek(0))
		return luaL_error(L, "File does not appear to be open.\n");

		lua_pushcclosure(L, lines_i, 2);
		return 1;
	}

	int lines_i(lua_State * L)
	{
		// We're using a 1k buffer.
		const static int bufsize = 1024;
		static char buf[bufsize];

		File * file = luax_checktype<File>(L, lua_upvalueindex(1), "File", LOVE_FILESYSTEM_FILE_BITS);
		int close = (int)lua_tointeger(L, lua_upvalueindex(2));

		// Find the next newline.
		// pos must be at the start of the line we're trying to find.
		int pos = file->tell();
		int newline = -1;
		int totalread = 0;

		while(!file->eof())
		{
			int current = file->tell();
			int read = file->read(buf, bufsize);
			totalread += read;

			if(read < 0)
				return luaL_error(L, "Readline failed!");

			for(int i = 0;i<read;i++)
			{
				if(buf[i] == '\n')
				{
					newline = current+i;
					break;
				}
			}

			if(newline > 0)
				break;
		}

		// Special case for the last "line".
		if(newline <= 0 && file->eof() && totalread > 0)
			newline = pos + totalread;

		// We've got a newline.
		if(newline > 0)
		{
			// Ok, we've got a line.
			int linesize = (newline-pos);

			// Allocate memory for the string.
			char * str = new char[linesize];

			// Read it.
			file->seek(pos);
			if(file->read(str, linesize) == -1)
			return luaL_error(L, "Read error.");

			if(str[linesize-1]=='\r')
			linesize -= 1;

			lua_pushlstring(L, str, linesize);

			// Free the memory. Lua has a copy now.
			delete[] str;

			// Set the beginning of the next line.
			if(!file->eof())
				file->seek(newline+1);

			return 1;
		}

		if(close)
		{
			file->close();
			file->release();
		}

		// else: (newline <= 0)
		return 0;
	}

	const luaL_Reg wrap_File_functions[] = {
		{ "getSize", _wrap_File_getSize },
		{ "open", _wrap_File_open },
		{ "close", _wrap_File_close },
		{ "read", _wrap_File_read },
		{ "write", _wrap_File_write },
		{ "eof", _wrap_File_eof },
		{ "tell", _wrap_File_tell },
		{ "seek", _wrap_File_seek },
		{ "lines", _wrap_File_lines },
		{ 0, 0 }
	};

	int wrap_File_open(lua_State * L)
	{
		luax_register_type(L, "File", wrap_File_functions);
		return 0;
	}
	
} // physfs
} // filesystem
} // love
