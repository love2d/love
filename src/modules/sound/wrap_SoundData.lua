R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2016 LOVE Development Team

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

local SoundData_mt = ...
local SoundData = SoundData_mt.__index

if type(jit) ~= "table" or not jit.status() then
	-- LuaJIT's FFI is *much* slower than LOVE's regular methods when the JIT
	-- compiler is disabled.
	return
end

local status, ffi = pcall(require, "ffi")
if not status then return end

local tonumber, assert, error = tonumber, assert, error

local float = ffi.typeof("float")
local datatypes = {ffi.typeof("uint8_t *"), ffi.typeof("int16_t *")}

local typemaxvals = {0x7F, 0x7FFF}

local _getBitDepth = SoundData.getBitDepth
local _getSampleCount = SoundData.getSampleCount
local _getSampleRate = SoundData.getSampleRate
local _getChannels = SoundData.getChannels
local _getDuration = SoundData.getDuration

-- Table which holds SoundData objects as keys, and information about the objects
-- as values. Uses weak keys so the SoundData objects can still be GC'd properly.
local objectcache = setmetatable({}, {
	__mode = "k",
	__index = function(self, sounddata)
		local bytedepth = _getBitDepth(sounddata) / 8
		local pointer = ffi.cast(datatypes[bytedepth], sounddata:getPointer())

		local p = {
			bytedepth = bytedepth,
			pointer = pointer,
			size = sounddata:getSize(),
			maxvalue = typemaxvals[bytedepth],
			samplecount = _getSampleCount(sounddata),
			samplerate = _getSampleRate(sounddata),
			channels = _getChannels(sounddata),
			duration = _getDuration(sounddata),
		}

		self[sounddata] = p
		return p
	end,
})


-- Overwrite existing functions with new FFI versions.

function SoundData:getSample(i)
	if type(i) ~= "number" then error("bad argument #1 to SoundData:getSample (expected number)", 2) end

	local p = objectcache[self]

	if not (i >= 0 and i < p.size/p.bytedepth) then
		error("Attempt to get out-of-range sample!", 2)
	end

	if p.bytedepth == 2 then
		-- 16-bit data is stored as signed values internally.
		return tonumber(p.pointer[i]) / p.maxvalue
	else
		-- 8-bit data is stored as unsigned values internally.
		return (tonumber(p.pointer[i]) - 128) / 127
	end
end

function SoundData:setSample(i, sample)
	if type(i) ~= "number" then error("bad argument #1 to SoundData:setSample (expected number)", 2) end
	if type(sample) ~= "number" then error("bad argument #2 to SoundData:setSample (expected number)", 2) end

	local p = objectcache[self]

	if not (i >= 0 and i < p.size/p.bytedepth) then
		error("Attempt to set out-of-range sample!", 2)
	end

	if p.bytedepth == 2 then
		-- 16-bit data is stored as signed values internally.
		p.pointer[i] = sample * p.maxvalue
	else
		-- 8-bit data is stored as unsigned values internally.
		-- The float cast is needed to make values end up the same as in the
		-- C++ version of this method.
		p.pointer[i] = ffi.cast(float, (sample * 127) + 128)
	end
end

function SoundData:getBitDepth()
	return objectcache[self].bytedepth * 8
end

function SoundData:getSampleCount()
	return objectcache[self].samplecount
end

function SoundData:getSampleRate()
	return objectcache[self].samplerate
end

function SoundData:getChannels()
	return objectcache[self].channels
end

function SoundData:getDuration()
	return objectcache[self].duration
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
