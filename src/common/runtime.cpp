/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "config.h"
#include "runtime.h"

// LOVE
#include "Module.h"
#include "Object.h"
#include "Reference.h"
#include "StringMap.h"

// C++
#include <algorithm>
#include <iostream>
#include <cstdio>

namespace love
{

/**
 * Called when an object is collected. The object is released
 * once in this function, possibly deleting it.
 **/
static int w__gc(lua_State *L)
{
	Proxy *p = (Proxy *) lua_touserdata(L, 1);
	Object *object = (Object *) p->data;

	object->release();

	return 0;
}

static int w__tostring(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}

static int w__typeOf(lua_State *L)
{
	Proxy *p = (Proxy *)lua_touserdata(L, 1);
	Type t = luax_type(L, 2);
	luax_pushboolean(L, p->flags[t]);
	return 1;
}

static int w__eq(lua_State *L)
{
	Proxy *p1 = (Proxy *)lua_touserdata(L, 1);
	Proxy *p2 = (Proxy *)lua_touserdata(L, 2);
	luax_pushboolean(L, p1->data == p2->data);
	return 1;
}

Reference *luax_refif(lua_State *L, int type)
{
	Reference *r = 0;

	// Create a reference only if the test succeeds.
	if (lua_type(L, -1) == type)
		r = new Reference(L);
	else // Pop the value even if it fails (but also if it succeeds).
		lua_pop(L, 1);

	return r;
}

void luax_printstack(lua_State *L)
{
	for (int i = 1; i<=lua_gettop(L); i++)
	{
		std::cout << i << " - " << luaL_typename(L, i) << std::endl;
	}
}

bool luax_toboolean(lua_State *L, int idx)
{
	return (lua_toboolean(L, idx) != 0);
}

void luax_pushboolean(lua_State *L, bool b)
{
	lua_pushboolean(L, b ? 1 : 0);
}

bool luax_optboolean(lua_State *L, int idx, bool b)
{
	if (lua_isboolean(L, idx) == 1)
		return (lua_toboolean(L, idx) == 1 ? true : false);
	return b;
}

std::string luax_tostring(lua_State *L, int idx)
{
	size_t len;
	const char *str = lua_tolstring(L, idx, &len);
	return std::string(str, len);
}

std::string luax_checkstring(lua_State *L, int idx)
{
	size_t len;
	const char *str = luaL_checklstring(L, idx, &len);
	return std::string(str, len);
}

void luax_pushstring(lua_State *L, const std::string &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

bool luax_boolflag(lua_State *L, int table_index, const char *key, bool defaultValue)
{
	lua_getfield(L, table_index, key);

	bool retval;
	if (lua_isnoneornil(L, -1))
		retval = defaultValue;
	else
		retval = lua_toboolean(L, -1);

	lua_pop(L, 1);
	return retval;
}

int luax_intflag(lua_State *L, int table_index, const char *key, int defaultValue)
{
	lua_getfield(L, table_index, key);

	int retval;
	if (!lua_isnumber(L, -1))
		retval = defaultValue;
	else
		retval = (int) lua_tointeger(L, -1);

	lua_pop(L, 1);
	return retval;
}

int luax_assert_argc(lua_State *L, int min)
{
	int argc = lua_gettop(L);
	if (argc < min)
		return luaL_error(L, "Incorrect number of arguments. Got [%d], expected at least [%d]", argc, min);
	return 0;
}

int luax_assert_argc(lua_State *L, int min, int max)
{
	int argc = lua_gettop(L);
	if (argc < min || argc > max)
		return luaL_error(L, "Incorrect number of arguments. Got [%d], expected [%d-%d]", argc, min, max);
	return 0;
}

int luax_assert_function(lua_State *L, int idx)
{
	if (!lua_isfunction(L, idx))
		return luaL_error(L, "Argument must be of type \"function\".");
	return 0;
}

int luax_assert_nilerror(lua_State *L, int idx)
{
	if (lua_isnoneornil(L, idx))
	{
		if (lua_isstring(L, idx + 1))
			return luaL_error(L, lua_tostring(L, idx + 1));
		else
			return luaL_error(L, "assertion failed!");
	}
	return 0;
}

void luax_setfuncs(lua_State *L, const luaL_Reg *l)
{
	if (l == 0)
		return;

	for (; l->name != 0; l++)
	{
		lua_pushcfunction(L, l->func);
		lua_setfield(L, -2, l->name);
	}
}

int luax_register_module(lua_State *L, const WrappedModule &m)
{
	// Put a reference to the C++ module in Lua.
	luax_insistregistry(L, REGISTRY_MODULES);

	Proxy *p = (Proxy *)lua_newuserdata(L, sizeof(Proxy));
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
	if (m.functions != 0)
		luax_setfuncs(L, m.functions);

	// Register types.
	if (m.types != 0)
		for (const lua_CFunction *t = m.types; *t != 0; t++)
			(*t)(L);

	lua_pushvalue(L, -1);
	lua_setfield(L, -3, m.name); // love.graphics = table
	lua_remove(L, -2); // love

	// Register module instance
	Module::registerInstance(m.module);

	return 1;
}

int luax_preload(lua_State *L, lua_CFunction f, const char *name)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, f);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
	return 0;
}

int luax_register_type(lua_State *L, const char *tname, const luaL_Reg *f)
{
	// Verify that this type name has a matching Type ID and type name mapping.
	love::Type ltype;
	if (!love::getType(tname, ltype))
		printf("Missing type entry for type name: %s\n", tname);

	// Get the place for storing and re-using instantiated love types.
	luax_getregistry(L, REGISTRY_TYPES);

	// Create registry._lovetypes if it doesn't exist yet.
	if (!lua_istable(L, -1))
	{
		lua_newtable(L);
		lua_replace(L, -2);

		// Create a metatable.
		lua_newtable(L);

		// metatable.__mode = "v". Weak userdata values.
		lua_pushliteral(L, "v");
		lua_setfield(L, -2, "__mode");

		// setmetatable(newtable, metatable)
		lua_setmetatable(L, -2);

		// registry._lovetypes = newtable
		lua_setfield(L, LUA_REGISTRYINDEX, "_lovetypes");
	}
	else
		lua_pop(L, 1);

	luaL_newmetatable(L, tname);

	// m.__index = m
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// setup gc
	lua_pushcfunction(L, w__gc);
	lua_setfield(L, -2, "__gc");

	// Add equality
	lua_pushcfunction(L, w__eq);
	lua_setfield(L, -2, "__eq");

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

	if (f != 0)
		luax_setfuncs(L, f);

	lua_pop(L, 1); // Pops metatable.
	return 0;
}

int luax_table_insert(lua_State *L, int tindex, int vindex, int pos)
{
	if (tindex < 0)
		tindex = lua_gettop(L)+1+tindex;
	if (vindex < 0)
		vindex = lua_gettop(L)+1+vindex;
	if (pos == -1)
	{
		lua_pushvalue(L, vindex);
		lua_rawseti(L, tindex, lua_objlen(L, tindex)+1);
		return 0;
	}
	else if (pos < 0)
		pos = lua_objlen(L, tindex)+1+pos;
	for (int i = lua_objlen(L, tindex)+1; i > pos; i--)
	{
		lua_rawgeti(L, tindex, i-1);
		lua_rawseti(L, tindex, i);
	}
	lua_pushvalue(L, vindex);
	lua_rawseti(L, tindex, pos);
	return 0;
}

int luax_register_searcher(lua_State *L, lua_CFunction f, int pos)
{
	// Add the package loader to the package.loaders table.
	lua_getglobal(L, "package");

	if (lua_isnil(L, -1))
		return luaL_error(L, "Can't register searcher: package table does not exist.");

	lua_getfield(L, -1, "loaders");

	// Lua 5.2 renamed package.loaders to package.searchers.
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		lua_getfield(L, -1, "searchers");
	}

	if (lua_isnil(L, -1))
		return luaL_error(L, "Can't register searcher: package.loaders table does not exist.");

	lua_pushcfunction(L, f);
	luax_table_insert(L, -2, -1, pos);
	lua_pop(L, 3);
	return 0;
}

void luax_rawnewtype(lua_State *L, const char *name, bits flags, love::Object *object)
{
	Proxy *u = (Proxy *)lua_newuserdata(L, sizeof(Proxy));

	object->retain();

	u->data = (void *) object;
	u->flags = flags;

	luaL_newmetatable(L, name);
	lua_setmetatable(L, -2);
}

void luax_pushtype(lua_State *L, const char *name, bits flags, love::Object *object)
{
	if (object == nullptr)
	{
		lua_pushnil(L);
		return;
	}

	// Fetch the registry table of instantiated types.
	luax_getregistry(L, REGISTRY_TYPES);

	// The table might not exist - it should be insisted in luax_register_type.
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return luax_rawnewtype(L, name, flags, object);
	}

	// Get the value of lovetypes[object] on the stack.
	lua_pushlightuserdata(L, (void *) object);
	lua_gettable(L, -2);

	// If the Proxy userdata isn't in the instantiated types table yet, add it.
	if (lua_type(L, -1) != LUA_TUSERDATA)
	{
		lua_pop(L, 1);

		luax_rawnewtype(L, name, flags, object);

		lua_pushlightuserdata(L, (void *) object);
		lua_pushvalue(L, -2);

		// lovetypes[object] = Proxy.
		lua_settable(L, -4);
	}

	// Remove the lovetypes table from the stack.
	lua_remove(L, -2);

	// Keep the Proxy userdata on the stack.
}

bool luax_istype(lua_State *L, int idx, love::bits type)
{
	if (lua_type(L, idx) != LUA_TUSERDATA)
		return false;

	return ((((Proxy *)lua_touserdata(L, idx))->flags & type) == type);
}

int luax_getfunction(lua_State *L, const char *mod, const char *fn)
{
	lua_getglobal(L, "love");
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find global love!");
	lua_getfield(L, -1, mod);
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s!", mod);
	lua_getfield(L, -1, fn);
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s.%s!", mod, fn);

	lua_remove(L, -2); // remove mod
	lua_remove(L, -2); // remove fn
	return 0;
}

int luax_convobj(lua_State *L, int idx, const char *mod, const char *fn)
{
	// Convert to absolute index if necessary.
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
		idx += lua_gettop(L) + 1;

	// Convert string to a file.
	luax_getfunction(L, mod, fn);
	lua_pushvalue(L, idx); // The initial argument.
	lua_call(L, 1, 2); // Call the function, one arg, one return value (plus optional errstring.)
	luax_assert_nilerror(L, -2); // Make sure the function returned something.
	lua_pop(L, 1); // Pop the second return value now that we don't need it.
	lua_replace(L, idx); // Replace the initial argument with the new object.
	return 0;
}

int luax_convobj(lua_State *L, int idxs[], int n, const char *mod, const char *fn)
{
	luax_getfunction(L, mod, fn);
	for (int i = 0; i < n; i++)
	{
		lua_pushvalue(L, idxs[i]); // The arguments.
	}
	lua_call(L, n, 2); // Call the function, n args, one return value (plus optional errstring.)
	luax_assert_nilerror(L, -2); // Make sure the function returned something.
	lua_pop(L, 1); // Pop the second return value now that we don't need it.
	lua_replace(L, idxs[0]); // Replace the initial argument with the new object.
	return 0;
}

int luax_pconvobj(lua_State *L, int idx, const char *mod, const char *fn)
{
	// Convert string to a file.
	luax_getfunction(L, mod, fn);
	lua_pushvalue(L, idx); // The initial argument.
	int ret = lua_pcall(L, 1, 1, 0); // Call the function, one arg, one return value.
	if (ret == 0)
		lua_replace(L, idx); // Replace the initial argument with the new object.
	return ret;
}

int luax_pconvobj(lua_State *L, int idxs[], int n, const char *mod, const char *fn)
{
	luax_getfunction(L, mod, fn);
	for (int i = 0; i < n; i++)
	{
		lua_pushvalue(L, idxs[i]); // The arguments.
	}
	int ret = lua_pcall(L, n, 1, 0); // Call the function, n args, one return value.
	if (ret == 0)
		lua_replace(L, idxs[0]); // Replace the initial argument with the new object.
	return ret;
}

int luax_insist(lua_State *L, int idx, const char *k)
{
	// Convert to absolute index if necessary.
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
		idx += lua_gettop(L) + 1;

	lua_getfield(L, idx, k);

	// Create if necessary.
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1); // Pop the non-table.
		lua_newtable(L);
		lua_pushvalue(L, -1); // Duplicate the table to leave on top.
		lua_setfield(L, idx, k); // lua_stack[idx][k] = lua_stack[-1] (table)
	}

	return 1;
}

int luax_insistglobal(lua_State *L, const char *k)
{
	lua_getglobal(L, k);

	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1); // Pop the non-table.
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, k);
	}

	return 1;
}

int luax_insistlove(lua_State *L, const char *k)
{
	luax_insistglobal(L, "love");
	luax_insist(L, -1, k);

	// The love table should be replaced with the top stack
	// item. Only the reqested table should remain on the stack.
	lua_replace(L, -2);

	return 1;
}

int luax_getlove(lua_State *L, const char *k)
{
	lua_getglobal(L, "love");

	if (!lua_isnil(L, -1))
	{
		lua_getfield(L, -1, k);
		lua_replace(L, -2);
	}

	return 1;
}

int luax_insistregistry(lua_State *L, Registry r)
{
	switch (r)
	{
	case REGISTRY_GC:
		return luax_insistlove(L, "_gc");
	case REGISTRY_MODULES:
		return luax_insistlove(L, "_modules");
	case REGISTRY_TYPES:
		return luax_insist(L, LUA_REGISTRYINDEX, "_lovetypes");
	default:
		return luaL_error(L, "Attempted to use invalid registry.");
	}
}

int luax_getregistry(lua_State *L, Registry r)
{
	switch (r)
	{
	case REGISTRY_GC:
		return luax_getlove(L, "_gc");
	case REGISTRY_MODULES:
		return luax_getlove(L, "_modules");
	case REGISTRY_TYPES:
		lua_getfield(L, LUA_REGISTRYINDEX, "_lovetypes");
		return 1;
	default:
		return luaL_error(L, "Attempted to use invalid registry.");
	}
}

extern "C" int luax_typerror(lua_State *L, int narg, const char *tname)
{
	int argtype = lua_type(L, narg);
	const char *argtname = 0;

	// We want to use the love type name for userdata, if possible.
	if (argtype == LUA_TUSERDATA && luaL_getmetafield(L, narg, "__tostring") != 0)
	{
		lua_pushvalue(L, narg);
		if (lua_pcall(L, 1, 1, 0) == 0 && lua_type(L, -1) == LUA_TSTRING)
		{
			argtname = lua_tostring(L, -1);

			// Non-love userdata might have a tostring metamethod which doesn't
			// describe its type, so we only use __tostring for love types.
			love::Type t;
			if (!love::getType(argtname, t))
				argtname = 0;
		}
	}

	if (argtname == 0)
		argtname = lua_typename(L, argtype);

	const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, argtname);
	return luaL_argerror(L, narg, msg);
}

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

bool getType(const char *in, love::Type &out)
{
	return types.find(in, out);
}

bool getType(love::Type in, const char *&out)
{
	return types.find(in, out);
}

Type luax_type(lua_State *L, int idx)
{
	Type t = INVALID_ID;
	types.find(luaL_checkstring(L, idx), t);
	return t;
}

} // love
