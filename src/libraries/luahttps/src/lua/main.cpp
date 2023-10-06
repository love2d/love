#include <algorithm>
#include <set>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include "../common/HTTPS.h"
#include "../common/config.h"

static std::string validMethod[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "PATCH"};

static int str_toupper(char c)
{
	unsigned char uc = (unsigned char) c;
	return toupper(uc);
}

static std::string w_checkstring(lua_State *L, int idx)
{
	size_t len;
	const char *str = luaL_checklstring(L, idx, &len);
	return std::string(str, len);
}

static void w_pushstring(lua_State *L, const std::string &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

static void w_readheaders(lua_State *L, int idx, HTTPSClient::header_map &headers)
{
	if (idx < 0)
		idx += lua_gettop(L) + 1;

	lua_pushnil(L);
	while (lua_next(L, idx))
	{
		auto header = w_checkstring(L, -2);
		headers[header] = w_checkstring(L, -1);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

static std::string w_optmethod(lua_State *L, int idx, const std::string &defaultMethod)
{
	std::string *const validMethodEnd = validMethod + sizeof(validMethod) / sizeof(std::string);

	if (lua_isnoneornil(L, idx))
		return defaultMethod;

	std::string str = w_checkstring(L, idx);
	std::transform(str.begin(), str.end(), str.begin(), str_toupper);

	if (std::find(validMethod, validMethodEnd, str) == validMethodEnd)
		luaL_argerror(L, idx, "expected one of \"get\", \"head\", \"post\", \"put\", \"delete\", or \"patch\"");

	return str;
}

static int w_request(lua_State *L)
{
	auto url = w_checkstring(L, 1);
	HTTPSClient::Request req(url);

	bool advanced = false;

	if (lua_istable(L, 2))
	{
		advanced = true;

		std::string defaultMethod = "GET";

		lua_getfield(L, 2, "data");
		if (!lua_isnoneornil(L, -1))
		{
			req.postdata = w_checkstring(L, -1);
			req.headers["Content-Type"] = "application/x-www-form-urlencoded";
			defaultMethod = "POST";
		}
		lua_pop(L, 1);

		lua_getfield(L, 2, "method");
		req.method = w_optmethod(L, -1, defaultMethod);
		lua_pop(L, 1);

		lua_getfield(L, 2, "headers");
		if (!lua_isnoneornil(L, -1))
			w_readheaders(L, -1, req.headers);
		lua_pop(L, 1);
	}

	HTTPSClient::Reply reply;

	try
	{
		reply = request(req);
	}
	catch (const std::exception& e)
	{
		std::string errorMessage = e.what();
		lua_pushnil(L);
		lua_pushstring(L, errorMessage.c_str());
		return 2;
	}

	lua_pushinteger(L, reply.responseCode);
	w_pushstring(L, reply.body);

	if (advanced)
	{
		lua_newtable(L);
		for (const auto &header : reply.headers)
		{
			w_pushstring(L, header.first);
			w_pushstring(L, header.second);
			lua_settable(L, -3);
		}
	}

	return advanced ? 3 : 2;
}

extern "C" int HTTPS_DLLEXPORT luaopen_https(lua_State *L)
{
	lua_newtable(L);

	lua_pushcfunction(L, w_request);
	lua_setfield(L, -2, "request");

	return 1;
}
