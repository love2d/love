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

local ImageData_mt, ffifuncspointer = ...
local ImageData = ImageData_mt.__index

local tonumber, assert, error = tonumber, assert, error
local type, pcall = type, pcall
local floor = math.floor

local function inside(x, y, w, h)
	return x >= 0 and x < w and y >= 0 and y < h
end

-- Implement thread-safe ImageData:mapPixel regardless of whether the FFI is
-- used or not.
function ImageData:mapPixel(func, ix, iy, iw, ih)
	local idw, idh = self:getDimensions()

	ix = ix or 0
	iy = iy or 0
	iw = iw or idw
	ih = ih or idh

	if type(ix) ~= "number" then error("bad argument #2 to ImageData:mapPixel (expected number)", 2) end
	if type(iy) ~= "number" then error("bad argument #3 to ImageData:mapPixel (expected number)", 2) end
	if type(iw) ~= "number" then error("bad argument #4 to ImageData:mapPixel (expected number)", 2) end
	if type(ih) ~= "number" then error("bad argument #5 to ImageData:mapPixel (expected number)", 2) end

	if type(func) ~= "function" then error("bad argument #1 to ImageData:mapPixel (expected function)", 2) end
	if not (inside(ix, iy, idw, idh) and inside(ix+iw-1, iy+ih-1, idw, idh)) then error("Invalid rectangle dimensions", 2) end

	-- performAtomic and mapPixelUnsafe have Lua-C API and FFI versions.
	self:_performAtomic(self._mapPixelUnsafe, self, func, ix, iy, iw, ih)
end


-- Everything below this point is efficient FFI replacements for existing
-- ImageData functionality.

if type(jit) ~= "table" or not jit.status() then
	-- LuaJIT's FFI is *much* slower than LOVE's regular methods when the JIT
	-- compiler is disabled.
	return
end

local status, ffi = pcall(require, "ffi")
if not status then return end

pcall(ffi.cdef, [[
typedef struct Proxy Proxy;

typedef struct FFI_ImageData
{
	void (*lockMutex)(Proxy *p);
	void (*unlockMutex)(Proxy *p);
} FFI_ImageData;

typedef struct ImageData_Pixel
{
	uint8_t r, g, b, a;
} ImageData_Pixel;
]])

local ffifuncs = ffi.cast("FFI_ImageData *", ffifuncspointer)

local pixelpointer = ffi.typeof("ImageData_Pixel *")

local _getWidth = ImageData.getWidth
local _getHeight = ImageData.getHeight
local _getDimensions = ImageData.getDimensions

-- Table which holds ImageData objects as keys, and information about the objects
-- as values. Uses weak keys so the ImageData objects can still be GC'd properly.
local objectcache = setmetatable({}, {
	__mode = "k",
	__index = function(self, imagedata)
		local width, height = _getDimensions(imagedata)
		local pointer = ffi.cast(pixelpointer, imagedata:getPointer())

		local p = {
			width = width,
			height = height,
			pointer = pointer,
		}

		self[imagedata] = p
		return p
	end,
})


-- Overwrite existing functions with new FFI versions.

function ImageData:_performAtomic(...)
	ffifuncs.lockMutex(self)
	local success, err = pcall(...)
	ffifuncs.unlockMutex(self)

	if not success then
		error(err, 3)
	end
end

function ImageData:_mapPixelUnsafe(func, ix, iy, iw, ih)
	local p = objectcache[self]
	local idw, idh = p.width, p.height

	ix = floor(ix)
	iy = floor(iy)
	iw = floor(iw)
	ih = floor(ih)

	local pixels = p.pointer

	for y=iy, iy+ih-1 do
		for x=ix, ix+iw-1 do
			local p = pixels[y*idw+x]
			local r, g, b, a = func(x, y, tonumber(p.r), tonumber(p.g), tonumber(p.b), tonumber(p.a))
			pixels[y*idw+x].r = r
			pixels[y*idw+x].g = g
			pixels[y*idw+x].b = b
			pixels[y*idw+x].a = a == nil and 255 or a
		end
	end
end

function ImageData:getPixel(x, y)
	if type(x) ~= "number" then error("bad argument #1 to ImageData:getPixel (expected number)", 2) end
	if type(y) ~= "number" then error("bad argument #2 to ImageData:getPixel (expected number)", 2) end

	x = floor(x)
	y = floor(y)

	local p = objectcache[self]
	if not inside(x, y, p.width, p.height) then error("Attempt to get out-of-range pixel!", 2) end

	ffifuncs.lockMutex(self)
	local pixel = p.pointer[y * p.width + x]
	local r, g, b, a = tonumber(pixel.r), tonumber(pixel.g), tonumber(pixel.b), tonumber(pixel.a)
	ffifuncs.unlockMutex(self)

	return r, g, b, a
end

local temppixel = ffi.new("ImageData_Pixel")

function ImageData:setPixel(x, y, r, g, b, a)
	if type(x) ~= "number" then error("bad argument #1 to ImageData:setPixel (expected number)", 2) end
	if type(y) ~= "number" then error("bad argument #2 to ImageData:setPixel (expected number)", 2) end

	x = floor(x)
	y = floor(y)

	if type(r) == "table" then
		local t = r
		r, g, b, a = t[1], t[2], t[3], t[4]
	end

	if type(r) ~= "number" then error("bad red color component argument to ImageData:setPixel (expected number)", 2) end
	if type(g) ~= "number" then error("bad green color component argument to ImageData:setPixel (expected number)", 2) end
	if type(b) ~= "number" then error("bad blue color component argument to ImageData:setPixel (expected number)", 2) end
	if a ~= nil and type(a) ~= "number" then error("bad alpha color component argument to ImageData:setPixel (expected number)", 2) end

	local p = objectcache[self]
	if not inside(x, y, p.width, p.height) then error("Attempt to set out-of-range pixel!", 2) end

	temppixel.r = r
	temppixel.g = g
	temppixel.b = b
	temppixel.a = a == nil and 255 or a

	ffifuncs.lockMutex(self)
	p.pointer[y * p.width + x] = temppixel
	ffifuncs.unlockMutex(self)
end

function ImageData:getWidth()
	return objectcache[self].width
end

function ImageData:getHeight()
	return objectcache[self].height
end

function ImageData:getDimensions()
	local p = objectcache[self]
	return p.width, p.height
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
