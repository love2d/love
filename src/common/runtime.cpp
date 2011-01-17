/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "runtime.h"

// LOVE
#include "Module.h"
#include "Object.h"
#include "Reference.h"
#include "StringMap.h"

// STD
#include <iostream>

// SDL
#include <SDL_mutex.h>
#include <SDL_thread.h>

namespace love
{
	static SDL_mutex *gcmutex = 0;
	void *_gcmutex = 0;
	unsigned int _gcthread = 0;
	/**
	* Called when an object is collected. The object is released
	* once in this function, possibly deleting it.
	**/
	static int w__gc(lua_State * L)
	{
		if (!gcmutex)
		{
			gcmutex = SDL_CreateMutex();
			_gcmutex = (void*) gcmutex;
		}
		Proxy * p = (Proxy *)lua_touserdata(L, 1);
		Object * t = (Object *)p->data;
		if(p->own)
		{
			SDL_mutexP(gcmutex);
			_gcthread = (unsigned int) SDL_ThreadID();
			t->release();
			SDL_mutexV(gcmutex);
		}
		return 0;
	}

	static int w__tostring(lua_State * L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		return 1;
	}

	static int w__typeOf(lua_State * L)
	{
		Proxy * p = (Proxy *)lua_touserdata(L, 1);
		Type t = luax_type(L, 2);
		luax_pushboolean(L, p->flags[t]);
		return 1;
	}

	Reference * luax_refif(lua_State * L, int type)
	{
		Reference * r = 0;

		// Create a reference only if the test succeeds.
		if(lua_type(L, -1) == type)
			r = new Reference(L);
		else // Pop the value even if it fails (but also if it succeeds).
			lua_pop(L, 1);

		return r;
	}

	void luax_printstack(lua_State * L)
	{
		for(int i = 1;i<=lua_gettop(L);i++)
		{
			std::cout << i << " - " << luaL_typename(L, i) << std::endl;
		}
	}

	bool luax_toboolean(lua_State * L, int idx)
	{
		return (lua_toboolean(L, idx) != 0);
	}

	void luax_pushboolean(lua_State * L, bool b)
	{
		lua_pushboolean(L, b ? 1 : 0);
	}

	bool luax_optboolean(lua_State * L, int idx, bool b)
	{
		if(lua_isboolean(L, idx) == 1)
			return (lua_toboolean(L, idx) == 1 ? true : false);
		return b;
	}

	int luax_assert_argc(lua_State * L, int min)
	{
		int argc = lua_gettop(L);
		if( argc < min )
			return luaL_error(L, "Incorrect number of arguments. Got [%d], expected at least [%d]", argc, min);
		return 0;
	}

	int luax_assert_argc(lua_State * L, int min, int max)
	{
		int argc = lua_gettop(L);
		if( argc < min || argc > max)
			return luaL_error(L, "Incorrect number of arguments. Got [%d], expected [%d-%d]", argc, min, max);
		return 0;
	}

	int luax_assert_function(lua_State * L, int n)
	{
		if(!lua_isfunction(L, n))
			return luaL_error(L, "Argument must be of type \"function\".");
		return 0;
	}

	int luax_register_module(lua_State * L, const WrappedModule & m)
	{
		// Put a reference to the C++ module in Lua.
		luax_getregistry(L, REGISTRY_MODULES);

		Proxy * p = (Proxy *)lua_newuserdata(L, sizeof(Proxy));
		p->own = true;
		p->data = m.module;
		p->flags = m.flags;

		luaL_newmetatable(L, m.module->getName());
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, w__gc);
		lua_setfield(L, -2, "__gc");

		lua_setmetatable(L, -2);
		lua_setfield(L, -2, m.name); // _modules[name] = proxy
		lua_pop(L, 1);

		// Gets the love table.
		luax_insistglobal(L, "love");

		// Create new table for module.
		lua_newtable(L);

		// Register all the functions.
		luaL_register(L, 0, m.functions);

		// Register types.
		if(m.types != 0)
			for(const lua_CFunction * t = m.types; *t != 0; t++)
				(*t)(L);

		lua_setfield(L, -2, m.name); // love.graphics = table
		lua_pop(L, 1); // love

		return 0;
	}

	int luax_preload(lua_State * L, lua_CFunction f, const char * name)
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "preload");
		lua_pushcfunction(L, f);
		lua_setfield(L, -2, name);
		lua_pop(L, 2);
		return 0;
	}

	int luax_register_type(lua_State * L, const char * tname, const luaL_Reg * f)
	{
		luaL_newmetatable(L, tname);

		// m.__index = m
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		// setup gc
		lua_pushcfunction(L, w__gc);
		lua_setfield(L, -2, "__gc");

		// Add tostring function.
		lua_pushstring(L, tname);
		lua_pushcclosure(L, w__tostring, 1);
		lua_setfield(L, -2, "__tostring");

		// Add tostring to as type() as well.
		lua_pushstring(L, tname);
		lua_pushcclosure(L, w__tostring, 1);
		lua_setfield(L, -2, "type");

		// Add typeOf
		lua_pushcfunction(L, w__typeOf);
		lua_setfield(L, -2, "typeOf");

		if(f != 0)
			luaL_register(L, 0, f);

		lua_pop(L, 1); // Pops metatable.
		return 0;
	}

	int luax_register_searcher(lua_State * L, lua_CFunction f)
	{
		// Add the package loader to the package.loaders table.
		lua_getglobal(L, "package");

		if(lua_isnil(L, -1))
			return luaL_error(L, "Can't register searcher: package table does not exist.");

		lua_getfield(L, -1, "loaders");

		if(lua_isnil(L, -1))
			return luaL_error(L, "Can't register searcher: package.loaders table does not exist.");

		int len = lua_objlen(L, -1);
		lua_pushinteger(L, len+1);
		lua_pushcfunction(L, f);
		lua_settable(L, -3);
		lua_pop(L, 2);
		return 0;
	}

	void luax_newtype(lua_State * L, const char * name, bits flags, void * data, bool own)
	{
		Proxy * u = (Proxy *)lua_newuserdata(L, sizeof(Proxy));

		u->data = data;
		u->flags = flags;
		u->own = own;

		luaL_newmetatable(L, name);
		lua_setmetatable(L, -2);
	}

	bool luax_istype(lua_State * L, int idx, love::bits type)
	{
		if(lua_isuserdata(L, idx) == 0)
			return false;

		return ((((Proxy *)lua_touserdata(L, idx))->flags & type) == type);
	}

	int luax_getfunction(lua_State * L, const char * mod, const char * fn)
	{
		lua_getglobal(L, "love");
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find global love!");
		lua_getfield(L, -1, mod);
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s!", mod);
		lua_getfield(L, -1, fn);
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s.%s!", mod, fn);

		lua_remove(L, -2); // remove mod
		lua_remove(L, -2); // remove fn
		return 0;
	}

	int luax_convobj(lua_State * L, int idx, const char * mod, const char * fn)
	{
		// Convert string to a file.
		luax_getfunction(L, mod, fn);
		lua_pushvalue(L, idx); // The initial argument.
		lua_call(L, 1, 1); // Call the function, one arg, one return value.
		lua_replace(L, idx); // Replace the initial argument with the new object.
		return 0;
	}

	int luax_convobj(lua_State * L, int idxs[], int n, const char * mod, const char * fn)
	{
		luax_getfunction(L, mod, fn);
		for (int i = 0; i < n; i++) {
			lua_pushvalue(L, idxs[i]); // The arguments.
		}
		lua_call(L, n, 1); // Call the function, n args, one return value.
		lua_replace(L, idxs[0]); // Replace the initial argument with the new object.
		return 0;
	}

	int luax_strtofile(lua_State * L, int idx)
	{
		return luax_convobj(L, idx, "filesystem", "newFile");
	}

	int luax_filetodata(lua_State * L, int idx)
	{
		return luax_convobj(L, idx, "filesystem", "read");
	}

	int luax_insist(lua_State * L, int idx, const char * k)
	{
		lua_getfield(L, idx, k);

		// Create if necessary.
		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1); // Pop the non-table.
			lua_newtable(L);
			lua_pushvalue(L, -1); // Duplicate the table to leave on top.
			lua_setfield(L, -3, k); // k[idx] = table
		}

		return 1;
	}

	int luax_insistglobal(lua_State * L, const char * k)
	{
		lua_getglobal(L, k);

		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1); // Pop the non-table.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setglobal(L, k);
		}

		return 1;
	}

	int luax_insistlove(lua_State * L, const char * k)
	{
		luax_insistglobal(L, "love");
		luax_insist(L, -1, k);

		// The love table should be replaced with the top stack
		// item. Only the reqested table should remain on the stack.
		lua_replace(L, -2);

		return 1;
	}

	int luax_getregistry(lua_State * L, Registry r)
	{
		switch(r)
		{
		case REGISTRY_GC:
			return luax_insistlove(L, "_gc");
		case REGISTRY_MODULES:
			return luax_insistlove(L, "_modules");
		default:
			return luaL_error(L, "Attempted to use invalid registry.");
		}
	}

	StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[] =
	{
		{"Invalid", INVALID_ID},

		{"Object", OBJECT_ID},
		{"Data", DATA_ID},
		{"Module", MODULE_ID},

		// Filesystem
		{"File", FILESYSTEM_FILE_ID},
		{"FileData", FILESYSTEM_FILE_DATA_ID},

		// Font
		{"GlyphData", FONT_GLYPH_DATA_ID},
		{"Rasterizer", FONT_RASTERIZER_ID},

		// Graphics
		{"Drawable", GRAPHICS_DRAWABLE_ID},
		{"Image", GRAPHICS_IMAGE_ID},
		{"Quad", GRAPHICS_QUAD_ID},
		{"Glyph", GRAPHICS_GLYPH_ID},
		{"Font", GRAPHICS_FONT_ID},
		{"ParticleSystem", GRAPHICS_PARTICLE_SYSTEM_ID},
		{"SpriteBatch", GRAPHICS_SPRITE_BATCH_ID},
		{"VertexBuffer", GRAPHICS_VERTEX_BUFFER_ID},

		// Image
		{"ImageData", IMAGE_IMAGE_DATA_ID},
		{"EncodedImageData", IMAGE_ENCODED_IMAGE_DATA_ID},

		// Audio
		{"Source", AUDIO_SOURCE_ID},

		// Sound
		{"SoundData", SOUND_SOUND_DATA_ID},
		{"Decoder", SOUND_DECODER_ID},

		// Physics
		{"World", PHYSICS_WORLD_ID},
		{"Contact", PHYSICS_CONTACT_ID},
		{"Body", PHYSICS_BODY_ID},
		{"Shape", PHYSICS_SHAPE_ID},
		{"CircleShape", PHYSICS_CIRCLE_SHAPE_ID},
		{"PolygonShape", PHYSICS_POLYGON_SHAPE_ID},
		{"Joint", PHYSICS_JOINT_ID},
		{"MouseJoint", PHYSICS_MOUSE_JOINT_ID},
		{"DistanceJoint", PHYSICS_DISTANCE_JOINT_ID},
		{"PrismaticJoint", PHYSICS_PRISMATIC_JOINT_ID},
		{"RevoluteJoint", PHYSICS_REVOLUTE_JOINT_ID},
		{"PulleyJoint", PHYSICS_PULLEY_JOINT_ID},
		{"GearJoint", PHYSICS_GEAR_JOINT_ID},

		// Thread
		{"Thread", THREAD_THREAD_ID},

		// The modules themselves. Only add abstracted modules here.
		{"filesystem", MODULE_FILESYSTEM_ID},
		{"image", MODULE_IMAGE_ID},
		{"sound", MODULE_SOUND_ID},
	};

	StringMap<Type, TYPE_MAX_ENUM> types(typeEntries, sizeof(typeEntries));

	Type luax_type(lua_State * L, int idx)
	{
		Type t = INVALID_ID;
		types.find(luaL_checkstring(L, idx), t);
		return t;
	}
} // love
