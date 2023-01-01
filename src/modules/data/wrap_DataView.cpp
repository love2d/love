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

#include "wrap_DataView.h"
#include "wrap_Data.h"

namespace love
{
namespace data
{

DataView *luax_checkdataview(lua_State *L, int idx)
{
	return luax_checktype<DataView>(L, idx);
}

int w_DataView_clone(lua_State *L)
{
	DataView *t = luax_checkdataview(L, 1);
	DataView *c = nullptr;
	luax_catchexcept(L, [&](){ c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

static const luaL_Reg w_DataView_functions[] =
{
	{ "clone", w_DataView_clone },
	{ 0, 0 }
};

int luaopen_dataview(lua_State *L)
{
	luax_register_type(L, &DataView::type, w_Data_functions, w_DataView_functions, nullptr);
	love::data::luax_rundatawrapper(L, DataView::type);
	return 0;
}

} // data
} // love
