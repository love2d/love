R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2023 LOVE Development Team

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
--]]

local RandomGenerator_mt, ffifuncspointer_str = ...
local RandomGenerator = RandomGenerator_mt.__index

local type, tonumber, error = type, tonumber, error
local floor = math.floor

local _random = RandomGenerator._random

local function getrandom(r, l, u)
	if u ~= nil then
		if type(r) ~= "number" then error("bad argument #1 to 'random' (number expected)", 2) end
		if type(l) ~= "number" then error("bad argument #2 to 'random' (number expected)", 2) end
		return floor(r * (u - l + 1)) + l
	elseif l ~= nil then
		if type(l) ~= "number" then error("bad argument #1 to 'random' (number expected)", 2) end
		return floor(r * l) + 1
	else
		return r
	end
end

function RandomGenerator:random(l, u)
	local r = _random(self)
	return getrandom(r, l, u)
end

if type(jit) ~= "table" or not jit.status() then
	-- LuaJIT's FFI is *much* slower than LOVE's regular methods when the JIT
	-- compiler is disabled.
	return
end

local status, ffi = pcall(require, "ffi")
if not status then return end

pcall(ffi.cdef, [[
typedef struct Proxy Proxy;

typedef struct FFI_RandomGenerator
{
	double (*random)(Proxy *p);
	double (*randomNormal)(Proxy *p, double stddev, double mean);
} FFI_RandomGenerator;
]])

local ffifuncs = ffi.cast("FFI_RandomGenerator **", ffifuncspointer_str)[0]


-- Overwrite some regular love.math functions with FFI implementations.

function RandomGenerator:random(l, u)
	-- TODO: This should ideally be handled inside ffifuncs.random
	if self == nil then error("bad argument #1 to 'random' (RandomGenerator expected, got no value)", 2) end
	local r = tonumber(ffifuncs.random(self))
	return getrandom(r, l, u)
end

function RandomGenerator:randomNormal(stddev, mean)
	-- TODO: This should ideally be handled inside ffifuncs.randomNormal
	if self == nil then error("bad argument #1 to 'randomNormal' (RandomGenerator expected, got no value)", 2) end

	stddev = stddev == nil and 1 or stddev
	mean = mean == nil and 0 or mean

	if type(stddev) ~= "number" then error("bad argument #1 to 'randomNormal' (number expected)", 2) end
	if type(mean) ~= "number" then error("bad argument #2 to 'randomNormal' (number expected)", 2) end

	return tonumber(ffifuncs.randomNormal(self, stddev, mean))
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
