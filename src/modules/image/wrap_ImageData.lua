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

local ImageData_mt, ffifuncspointer_str = ...
local ImageData = ImageData_mt.__index

local tonumber, assert, error = tonumber, assert, error
local type, pcall = type, pcall
local floor = math.floor
local min, max = math.min, math.max

local function inside(x, y, w, h)
	return x >= 0 and x < w and y >= 0 and y < h
end

local function clamp01(x)
	return min(max(x, 0), 1)
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

local bitstatus, bit = pcall(require, "bit")
if not bitstatus then return end

pcall(ffi.cdef, [[
typedef struct Proxy Proxy;
typedef uint16_t float16;
typedef uint16_t float11;
typedef uint16_t float10;

typedef struct FFI_ImageData
{
	void (*lockMutex)(Proxy *p);
	void (*unlockMutex)(Proxy *p);

	float (*float16to32)(float16 f);
	float16 (*float32to16)(float f);

	float (*float11to32)(float11 f);
	float11 (*float32to11)(float f);

	float (*float10to32)(float10 f);
	float10 (*float32to10)(float f);
} FFI_ImageData;

struct ImageData_Pixel_R8 { uint8_t r; };
struct ImageData_Pixel_RG8 { uint8_t r, g; };
struct ImageData_Pixel_RGBA8 { uint8_t r, g, b, a; };

struct ImageData_Pixel_R16 { uint16_t r; };
struct ImageData_Pixel_RG16 { uint16_t r, g; };
struct ImageData_Pixel_RGBA16 { uint16_t r, g, b, a; };

struct ImageData_Pixel_R16F { float16 r; };
struct ImageData_Pixel_RG16F { float16 r, g; };
struct ImageData_Pixel_RGBA16F { float16 r, g, b, a; };

struct ImageData_Pixel_R32F { float r; };
struct ImageData_Pixel_RG32F { float r, g; };
struct ImageData_Pixel_RGBA32F { float r, g, b, a; };

struct ImageData_Pixel_RGBA4 { uint16_t rgba; };
struct ImageData_Pixel_RGB5A1 { uint16_t rgba; };
struct ImageData_Pixel_RGB565 { uint16_t rgb; };
struct ImageData_Pixel_RGB10A2 { uint32_t rgba; };
struct ImageData_Pixel_RG11B10F { uint32_t rgb; };
]])

local ffifuncs = ffi.cast("FFI_ImageData **", ffifuncspointer_str)[0]

local conversions = {
	r8 = {
		pointer = ffi.typeof("struct ImageData_Pixel_R8 *"),
		tolua = function(self)
			return tonumber(self.r) / 255, 0, 0, 1
		end,
		fromlua = function(self, r)
			self.r = (clamp01(r) * 255) + 0.5
		end,
	},
	rg8 = {
		pointer = ffi.typeof("struct ImageData_Pixel_RG8 *"),
		tolua = function(self)
			return tonumber(self.r) / 255, tonumber(self.g) / 255, 0, 1
		end,
		fromlua = function(self, r, g)
			self.r = (clamp01(r) * 255) + 0.5
			self.g = (clamp01(g) * 255) + 0.5
		end,
	},
	rgba8 = {
		pointer = ffi.typeof("struct ImageData_Pixel_RGBA8 *"),
		tolua = function(self)
			return tonumber(self.r) / 255, tonumber(self.g) / 255, tonumber(self.b) / 255, tonumber(self.a) / 255
		end,
		fromlua = function(self, r, g, b, a)
			self.r = (clamp01(r) * 255) + 0.5
			self.g = (clamp01(g) * 255) + 0.5
			self.b = (clamp01(b) * 255) + 0.5
			self.a = a == nil and 255 or (clamp01(a) * 255) + 0.5
		end,
	},
	r16 = {
		pointer = ffi.typeof("struct ImageData_Pixel_R16 *"),
		tolua = function(self)
			return tonumber(self.r) / 65535, 0, 0, 1
		end,
		fromlua = function(self, r)
			self.r = (clamp01(r) * 65535) + 0.5
		end,
	},
	rg16 = {
		pointer = ffi.typeof("struct ImageData_Pixel_RG16 *"),
		tolua = function(self)
			return tonumber(self.r) / 65535, tonumber(self.g) / 65535, 0, 1
		end,
		fromlua = function(self, r, g)
			self.r = (clamp01(r) * 65535) + 0.5
			self.g = (clamp01(g) * 65535) + 0.5
		end,
	},
	rgba16 = {
		pointer = ffi.typeof("struct ImageData_Pixel_RGBA16 *"),
		tolua = function(self)
			return tonumber(self.r) / 65535, tonumber(self.g) / 65535, tonumber(self.b) / 65535, tonumber(self.a) / 65535
		end,
		fromlua = function(self, r, g, b, a)
			self.r = (clamp01(r) * 65535) + 0.5
			self.g = (clamp01(g) * 65535) + 0.5
			self.b = (clamp01(b) * 65535) + 0.5
			self.a = a == nil and 65535 or (clamp01(a) * 65535) + 0.5
		end,
	},
	r16f = {
		pointer = ffi.typeof("struct ImageData_Pixel_R16F *"),
		tolua = function(self)
			return tonumber(ffifuncs.float16to32(self.r)), 0, 0, 1
		end,
		fromlua = function(self, r)
			self.r = ffifuncs.float32to16(r)
		end,
	},
	rg16f = {
		pointer = ffi.typeof("struct ImageData_Pixel_RG16F *"),
		tolua = function(self)
			return tonumber(ffifuncs.float16to32(self.r)), tonumber(ffifuncs.float16to32(self.g)), 0, 1
		end,
		fromlua = function(self, r, g, b, a)
			self.r = ffifuncs.float32to16(r)
			self.g = ffifuncs.float32to16(g)
		end,
	},
	rgba16f = {
		pointer = ffi.typeof("struct ImageData_Pixel_RGBA16F *"),
		tolua = function(self)
			return tonumber(ffifuncs.float16to32(self.r)),
			       tonumber(ffifuncs.float16to32(self.g)),
			       tonumber(ffifuncs.float16to32(self.b)),
			       tonumber(ffifuncs.float16to32(self.a))
		end,
		fromlua = function(self, r, g, b, a)
			self.r = ffifuncs.float32to16(r)
			self.g = ffifuncs.float32to16(g)
			self.b = ffifuncs.float32to16(b)
			self.a = ffifuncs.float32to16(a == nil and 1.0 or a)
		end,
	},
	r32f = {
		pointer = ffi.typeof("struct ImageData_Pixel_R32F *"),
		tolua = function(self)
			return tonumber(self.r), 0, 0, 1
		end,
		fromlua = function(self, r, g, b, a)
			self.r = r
		end,
	},
	rg32f = {
		pointer = ffi.typeof("struct ImageData_Pixel_RG32F *"),
		tolua = function(self)
			return tonumber(self.r), tonumber(self.g), 0, 1
		end,
		fromlua = function(self, r, g, b, a)
			self.r = r
			self.g = g
		end,
	},
	rgba32f = {
		pointer = ffi.typeof("struct ImageData_Pixel_RGBA32F *"),
		tolua = function(self)
			return tonumber(self.r), tonumber(self.g), tonumber(self.b), tonumber(self.a)
		end,
		fromlua = function(self, r, g, b, a)
			self.r = r
			self.g = g
			self.b = b
			self.a = a == nil and 1.0 or a
		end,
	},
	rgba4 = {
		-- LSB->MSB: [a, b, g, r]
		pointer = ffi.typeof("struct ImageData_Pixel_RGBA4 *"),
		tolua = function(self)
			local rgba = self.rgba
			local a = tonumber(bit.band(rgba, 0xF)) / 0xF
			local b = tonumber(bit.band(bit.rshift(rgba, 4), 0xF)) / 0xF
			local g = tonumber(bit.band(bit.rshift(rgba, 8), 0xF)) / 0xF
			local r = tonumber(bit.rshift(rgba, 12)) / 0xF
			return r, g, b, a
		end,
		fromlua = function(self, r, g, b, a)
			-- bit functions round internally.
			r = clamp01(r) * 0xF
			g = clamp01(g) * 0xF
			b = clamp01(b) * 0xF
			a = a == nil and 0xF or clamp01(a) * 0xF
			self.rgba = bit.bor(bit.lshift(r, 12), bit.lshift(g, 8), bit.lshift(b, 4), a)
		end,
	},
	rgb5a1 = {
		-- LSB->MSB: [a, b, g, r]
		pointer = ffi.typeof("struct ImageData_Pixel_RGB5A1 *"),
		tolua = function(self)
			local rgba = self.rgba
			local r = tonumber(bit.band(bit.rshift(rgba, 11), 0x1F)) / 0x1F
			local g = tonumber(bit.band(bit.rshift(rgba,  6), 0x1F)) / 0x1F
			local b = tonumber(bit.band(bit.rshift(rgba,  1), 0x1F)) / 0x1F
			local a = tonumber(bit.band(rgba, 0x1))
			return r, g, b, a
		end,
		fromlua = function(self, r, g, b, a)
			-- bit functions round internally.
			r = clamp01(r) * 0x1F
			g = clamp01(g) * 0x1F
			b = clamp01(b) * 0x1F
			a = a == nil and 1 or clamp01(a)
			self.rgba = bit.bor(bit.lshift(r, 11), bit.lshift(g, 6), bit.lshift(b, 1), a)
		end,
	},
	rgb565 = {
		-- LSB->MSB: [b, g, r]
		pointer = ffi.typeof("struct ImageData_Pixel_RGB565 *"),
		tolua = function(self)
			local rgb = self.rgb
			local r = bit.band(bit.rshift(rgb, 11), 0x1F) / 0x1F
			local g = bit.band(bit.rshift(rgb, 5), 0x3F) / 0x3F
			local b = bit.band(rgb, 0x1F) / 0x1F
			return r, g, b, 1
		end,
		fromlua = function(self, r, g, b)
			-- bit functions round internally.
			r = clamp01(r) * 0x1F
			g = clamp01(g) * 0x3F
			b = clamp01(b) * 0x1F
			self.rgb = bit.bor(bit.lshift(r, 11), bit.lshift(g, 5), b)
		end,
	},
	rgb10a2 = {
		-- LSB->MSB: [r, g, b, a]
		pointer = ffi.typeof("struct ImageData_Pixel_RGB10A2 *"),
		tolua = function(self)
			local rgba = self.rgba
			local r = tonumber(bit.band(rgba, 0x3FF)) / 0x3FF
			local g = tonumber(bit.band(bit.rshift(rgba, 10), 0x3FF)) / 0x3FF
			local b = tonumber(bit.band(bit.rshift(rgba, 20), 0x3FF)) / 0x3FF
			local a = tonumber(bit.rshift(rgba, 30)) / 0x3
			return r, g, b, a
		end,
		fromlua = function(self, r, g, b, a)
			-- bit functions round internally.
			r = clamp01(r) * 0x3FF
			g = clamp01(g) * 0x3FF
			b = clamp01(b) * 0x3FF
			a = a == nil and 0x3 or clamp01(a) * 0x3
			self.rgba = bit.bor(r, bit.lshift(g, 10), bit.lshift(b, 20), bit.lshift(a, 30))
		end,
	},
	rg11b10f = {
		-- LSB->MSB: [r, g, b]
		pointer = ffi.typeof("struct ImageData_Pixel_RG11B10F *"),
		tolua = function(self)
			local rgb = self.rgb
			local r = tonumber(ffifuncs.float11to32(bit.band(rgb, 0x7FF)))
			local g = tonumber(ffifuncs.float11to32(bit.band(bit.rshift(rgb, 11), 0x7FF)))
			local b = tonumber(ffifuncs.float10to32(bit.band(bit.rshift(rgb, 22), 0x3FF)))
			return r, g, b, 1
		end,
		fromlua = function(self, r, g, b, a)
			self.rgb = bit.bor(
				ffifuncs.float32to11(r),
				bit.lshift(ffifuncs.float32to11(g), 11),
				bit.lshift(ffifuncs.float32to10(b), 22)
			)
		end,
	},
}

local _getWidth = ImageData.getWidth
local _getHeight = ImageData.getHeight
local _getDimensions = ImageData.getDimensions
local _getFormat = ImageData.getFormat
local _release = ImageData.release

-- Table which holds ImageData objects as keys, and information about the objects
-- as values. Uses weak keys so the ImageData objects can still be GC'd properly.
local objectcache = setmetatable({}, {
	__mode = "k",
	__index = function(self, imagedata)
		local width, height = _getDimensions(imagedata)
		local format = _getFormat(imagedata)
		
		local conv = conversions[format]

		local p = {
			width = width,
			height = height,
			format = format,
			pointer = ffi.cast(conv.pointer, imagedata:getFFIPointer()),
			tolua = conv.tolua,
			fromlua = conv.fromlua,
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
	local tolua = p.tolua
	local fromlua = p.fromlua

	for y=iy, iy+ih-1 do
		for x=ix, ix+iw-1 do
			local pixel = pixels[y*idw+x]
			local r, g, b, a = func(x, y, tolua(pixel))
			fromlua(pixel, r, g, b, a)
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
	local r, g, b, a = p.tolua(pixel)
	ffifuncs.unlockMutex(self)

	return r, g, b, a
end

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

	ffifuncs.lockMutex(self)
	p.fromlua(p.pointer[y * p.width + x], r, g, b, a)
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

function ImageData:getFormat()
	return objectcache[self].format
end

function ImageData:release()
	objectcache[self] = nil
	return _release(self)
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
