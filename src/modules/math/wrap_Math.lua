R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2015 LOVE Development Team

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

local math, ffifuncspointer = ...

if type(jit) ~= "table" or not jit.status() then
	-- LuaJIT's FFI is *much* slower than LOVE's regular methods when the JIT
	-- compiler is disabled.
	return
end

local status, ffi = pcall(require, "ffi")
if not status then return end

local type, tonumber = type, tonumber

-- Matches the struct declaration in wrap_Math.cpp.
pcall(ffi.cdef, [[
typedef struct FFI_Math
{
	float (*noise1)(float x);
	float (*noise2)(float x, float y);
	float (*noise3)(float x, float y, float z);
	float (*noise4)(float x, float y, float z, float w);

	float (*gammaToLinear)(float c);
	float (*linearToGamma)(float c);
} FFI_Math;
]])

local ffifuncs = ffi.cast("FFI_Math *", ffifuncspointer)


-- Overwrite some regular love.math functions with FFI implementations.

function math.noise(x, y, z, w)
	if w ~= nil then
		return tonumber(ffifuncs.noise4(x, y, z, w))
	elseif z ~= nil then
		return tonumber(ffifuncs.noise3(x, y, z))
	elseif y ~= nil then
		return tonumber(ffifuncs.noise2(x, y))
	elseif x ~= nil then
		return tonumber(ffifuncs.noise1(x))
	end
end

local function gammaToLinear(c)
	if c ~= nil then
		return tonumber(ffifuncs.gammaToLinear(c / 255)) * 255
	end
	return c
end

function math.gammaToLinear(r, g, b, a)
	if type(r) == "table" then
		local t = r
		return gammaToLinear(t[1]), gammaToLinear(t[2]), gammaToLinear(t[3]), t[4]
	end
	return gammaToLinear(r), gammaToLinear(g), gammaToLinear(b), a
end

local function linearToGamma(c)
	if c ~= nil then
		return tonumber(ffifuncs.linearToGamma(c / 255)) * 255
	end
	return c
end

function math.linearToGamma(r, g, b, a)
	if type(r) == "table" then
		local t = r
		return linearToGamma(t[1]), linearToGamma(t[2]), linearToGamma(t[3]), t[4]
	end
	return linearToGamma(r), linearToGamma(g), linearToGamma(b), a
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
