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

-- Make sure love exists.
local love = require("love")

function love.nogame()

	-- 30log.lua begins
	local function require_30log()
	local assert, pairs, type, tostring, setmetatable = assert, pairs, type, tostring, setmetatable
	local baseMt, _instances, _classes, _class = {}, setmetatable({},{__mode='k'}), setmetatable({},{__mode='k'})
	local function assert_class(class, method) assert(_classes[class], ('Wrong method call. Expected class:%s.'):format(method)) end
	local function deep_copy(t, dest, aType) t = t or {}; local r = dest or {}
	  for k,v in pairs(t) do
	    if aType and type(v)==aType then r[k] = v elseif not aType then
	      if type(v) == 'table' and k ~= "__index" then r[k] = deep_copy(v) else r[k] = v end
	    end
	  end; return r
	end
	local function instantiate(self,...)
	  assert_class(self, 'new(...) or class(...)'); local instance = {class = self}; _instances[instance] = tostring(instance); setmetatable(instance,self)
	  if self.init then if type(self.init) == 'table' then deep_copy(self.init, instance) else self.init(instance, ...) end; end; return instance
	end
	local function extend(self, name, extra_params)
	  assert_class(self, 'extend(...)'); local heir = {}; _classes[heir] = tostring(heir); deep_copy(extra_params, deep_copy(self, heir));
	  heir.name, heir.__index, heir.super = extra_params and extra_params.name or name, heir, self; return setmetatable(heir,self)
	end
	baseMt = { __call = function (self,...) return self:new(...) end, __tostring = function(self,...)
	  if _instances[self] then return ("instance of '%s' (%s)"):format(rawget(self.class,'name') or '?', _instances[self]) end
	  return _classes[self] and ("class '%s' (%s)"):format(rawget(self,'name') or '?',_classes[self]) or self
	end}; _classes[baseMt] = tostring(baseMt); setmetatable(baseMt, {__tostring = baseMt.__tostring})
	local class = {isClass = function(class, ofsuper) local isclass = not not _classes[class]; if ofsuper then return isclass and (class.super == ofsuper) end; return isclass end, isInstance = function(instance, ofclass) 
	    local isinstance = not not _instances[instance]; if ofclass then return isinstance and (instance.class == ofclass) end; return isinstance end}; _class = function(name, attr)
	  local c = deep_copy(attr); c.mixins=setmetatable({},{__mode='k'}); _classes[c] = tostring(c); c.name, c.__tostring, c.__call = name or c.name, baseMt.__tostring, baseMt.__call
	  c.include = function(self,mixin) assert_class(self, 'include(mixin)'); self.mixins[mixin] = true; return deep_copy(mixin, self, 'function') end
	  c.new, c.extend, c.__index, c.includes = instantiate, extend, c, function(self,mixin) assert_class(self,'includes(mixin)') return not not (self.mixins[mixin] or (self.super and self.super:includes(mixin))) end
	  c.extends = function(self, class) assert_class(self, 'extends(class)') local super = self; repeat super = super.super until (super == class or super == nil); return class and (super == class) end
	    return setmetatable(c, baseMt) end; class._DESCRIPTION = '30 lines library for object orientation in Lua'; class._VERSION = '30log v1.0.0'; class._URL = 'http://github.com/Yonaba/30log'; class._LICENSE = 'MIT LICENSE <http://www.opensource.org/licenses/mit-license.php>'
	return setmetatable(class,{__call = function(_,...) return _class(...) end })
	end
	-- 30log.lua ends

	local mosaic_png =
	"iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAAGXRFWHRTb2Z0d2FyZQBBZG9i\
	ZSBJbWFnZVJlYWR5ccllPAAAA2ZpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tl\
	dCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1l\
	dGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUu\
	My1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1NjoyNyAgICAgICAgIj4gPHJkZjpS\
	REYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgt\
	bnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wTU09Imh0dHA6\
	Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iIHhtbG5zOnN0UmVmPSJodHRwOi8vbnMuYWRv\
	YmUuY29tL3hhcC8xLjAvc1R5cGUvUmVzb3VyY2VSZWYjIiB4bWxuczp4bXA9Imh0dHA6Ly9u\
	cy5hZG9iZS5jb20veGFwLzEuMC8iIHhtcE1NOk9yaWdpbmFsRG9jdW1lbnRJRD0ieG1wLmRp\
	ZDo2NDY3ODU2OTk0NkJFNTExQTg3RkQ3MTNCOTc2N0UzNyIgeG1wTU06RG9jdW1lbnRJRD0i\
	eG1wLmRpZDo5ODkwN0RFRTg3RUExMUU1QTRBMTk2NDJEQUYyRkUwNyIgeG1wTU06SW5zdGFu\
	Y2VJRD0ieG1wLmlpZDo5ODkwN0RFRDg3RUExMUU1QTRBMTk2NDJEQUYyRkUwNyIgeG1wOkNy\
	ZWF0b3JUb29sPSJBZG9iZSBQaG90b3Nob3AgQ1M2IChXaW5kb3dzKSI+IDx4bXBNTTpEZXJp\
	dmVkRnJvbSBzdFJlZjppbnN0YW5jZUlEPSJ4bXAuaWlkOjBGQTIzQUI1RTc4N0U1MTFBNDUy\
	ODc2NjFDRjM3OTIwIiBzdFJlZjpkb2N1bWVudElEPSJ4bXAuZGlkOjY0Njc4NTY5OTQ2QkU1\
	MTFBODdGRDcxM0I5NzY3RTM3Ii8+IDwvcmRmOkRlc2NyaXB0aW9uPiA8L3JkZjpSREY+IDwv\
	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\
	"
	local toast_back_png =
	"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\
	RU5ErkJggg==\
	"
	local toast_eyes_closed_png =
	"iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAB3UlEQVR4nO3cMU4VURhA4X/A\
	2oIFyILA1sZlsABwAWyEVihYjpaWtiY8mqfEECnnmpzvSyaZ5Db/zD2ZyTSzHQ6Hoetk9QCs\
	JYA4AcQJIE4AcQKIE0CcAOIEECeAOAHECSBOAHECiBNAnADiBBAngDgBxAkgTgBxAogTQJwA\
	4gQQJ4A4AcQJIE4AcQKIE0CcAOIEECeAOAHECSBOAHECiHu3eoCSm8vzt9fvv+00yYvdA9i2\
	7c/59cWHv9a+PHzfe5xdHa/348xczczPmXk/M79m5sfMfF4x064B3Fyev9r0mTmdmcPMPO05\
	y0Jfj8d/YcUr4G5mzo7nvzf9dmYeF8yyq7eecKv+2LoigE//WvDb2v1tbnqbz8A4AcQJIE4A\
	cQKIE0CcAOIEECeAOAHECSBOAHECiBNAnADiBBAngDgBxAkgTgBxAogTQJwA4gQQJ4A4AcQJ\
	IE4AcQKIE0CcAOIEECeAOAHECSBOAHECiBNAnADiBBAngDgBxAkgTgBxAogTQJwA4gQQJ4A4\
	AcQJIE4AcQKIE0CcAOIEECeAOAHECSBOAHECiBNAnADiBBAngDgBxAkgTgBxAogTQJwA4gQQ\
	J4A4AcQJIE4AcQKIE0CcAOIEECeAOAHECSDuGds2Io/iw+DXAAAAAElFTkSuQmCC\
	"
	local toast_eyes_open_png =
	"iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAACHUlEQVR4nO3cMU5UYRhG4TPG\
	DegGVCq3YFgACSHU9nYuAkJs6AyFBS0FHTRQuBCtjImbIHQMBSROCMZuPsI5TzU3U8xbnPvP\
	zRSzWC6XxOvF9IDMKgC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsA\
	uQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuZfTA0z2d97+\
	872Dyz9r27GqE2D9XgHfgRvgN/BhcsxaT4CneAcM2AO27l9vAKfAu6kxEyfAa+ArcAF8BhYD\
	GyZtPLh+M7Li3sQzwCl/74Ad7gL4NrBjyhmwu3J9PjUEZgLYeuTaFMAJcAVsA7+Ao8kxE18B\
	P/9z/WytPOecAZ+AQ+B6ag/MnAAfgWPgPXAJfBnYMOapPexOBPAD2Bz43Dxi0b+Fu/VDkFwB\
	yBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBc\
	AcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWA\
	XAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgV\
	gFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWA3C2ytyc6Pw+oOAAAAABJRU5E\
	rkJggg==\
	"
	local toast_front_png =
	"iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAFy0lEQVR4nO2dzW8VVRiHnwJW\
	EZRWo2IBgwooC/xAF1ajCenCoDEmJkYjIa4krtzoxqUbNu5cuTJB/wkTdWvcoAtF1JZWpKVW\
	kbZCW2vVujj3Ovfaj0vb9507576/J5ncljS/M+E893zNzJmuxcVFRFw2tfsERHuRAMGRAMGR\
	AMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMGRAMHZ0u4TMGHkVLvP\
	YGXufrXdZ7AqXdncE9i6kjcBtwN9wE5gN3Ab0Fs7ehp+3gZ0A9tJX4IdLbKvAP8AM8ACMA1M\
	NRyTwAQwXjvGgIvA5RUTKyJGNQVYubJvAA4C+4F9wL3APbXPPmBzGae3BqaA74FBYAg4A3wJ\
	DC/7122QohoCLF/htwKPAYeBQ7VjP9Wr5PUwCZwGPgc+Bb4gtSyJEkVojwDLV/heYAB4AugH\
	7i/xjNrNDPAZ8F7tM1GCCOUK0Fzx3cBTwDPAUWJV+Gp8BLwOzALuEvjPApZ+2/uB48BLwC3u\
	5efHceAA8DRpsOmKXwvQXPE7gNeAE6R+XLTmQyB9/R1bAZ+FoKLye4CTwCjwLqr8tXCMNNNx\
	xV6AovL7gbPA26T5tlgbm4G3ANeFLq+l4IdI05udTvlROAbc7FmArQCFqR8AN5pmx2Q78IJn\
	AR4twF7gYYfcqLwMuHUDHgIccciMzBHgJq9wDwEGHDIj043j/6mdAEUT9ahZpqjzpFewdQvQ\
	TQlz14A8DriMA6wFOEhnXK2rGo+QLoWb4yGAsOc6kgTmWAtwl3GeKHjQI9RaAK38+eFyudxa\
	gF3GeaLApXu1FuBO4zxRcB9gPhOwFsD1wkVw9gBbrUOtBWh1e7XYGLutA60F0HV/X/qsAyVA\
	XlReAJfVKvEfe6wD9XBoXtxhHSgB8qLHOlAC5IUECE6vdaAEyIvKtwAVeNS4o6m8AL8b54lm\
	zFtsdQF5Yb7Ubi3AjHGeaKbLOtBagKvGeaIZ8+cDrAW4Ypwnmqn8GEAtQGZYCzBlnCec0TQw\
	ONYC/GqcJ5yxFmDlnTFFJbEW4DfjPOGMWoDgWAswbpwnmllo/SdrQwLkxax1oLUAY8Z5whlr\
	AeYoYXvTwMxbB9oJUGxnqm7AjznrQI/7AS44ZIqE+bUWDwF+dsgUib+sAz0EuOiQKRLmF9sk\
	QF78aR3oIYCmgn6YX231EGDEIVMkJEBwshBgEi0GeVHxQWCxGLT8ixHFRsmiBQB1A15IgOBk\
	I4C6AB+yEUAtgA/ZCKAWwIesBPjbKTsyk9aBXgIsoG7AA/Obbu0FKNYCvjPPjs08mdwQUucH\
	x+yIpObf+EXSngKcdcyOiMszF54CDDpmR8R8AAi+AmgMYEt2Akygq4KWZCSAZgIeuDx4671N\
	3LfO+ZHIqAUo+MY5PxLZzQIAvnbOj0SWLYC6ADuyFGAMzQSsyKwLKGYCZ9zKiEWWLQBoKmhF\
	tgKcL6GMCGTWBRTocfGNs4jDk8FQjgCjJZTR6aRbwYwvBUM5AvxUQhmdjvnuYHXKEODHEsro\
	dIa8gssQYB7tGbBRPvYK9hWg6LO+ci2nM1kATgNvAie9CtniFfw/PgGeLamsKjNN2kXtAvAL\
	cIk0vZskvW3lau3fJkjNvlvfX6csAd4HBoDnSiqvXcyTnokYJt0W3/g5zHp2+XIY+TdSlgDz\
	wPPACeAdHN6CXSJ/kL6dg6Q7nwdrvw9xLdvjOFfoWulaXCzhZZ8jpxp/2wq8ArwBPOBf+LqY\
	Bs6RvrXnake9kkdZ7Q2pFavgVpQjQJ1mEQAOAS8CR4HDlPciyznS+sR5Un88QlHRw1zL7VeZ\
	VfRKlCtAI0tl2EaS4ACwr3bsIr0vt7d2XL9M0ixpsDRNqtj6oOoyxYBqnLSB5ThpSnqp5fl1\
	SAW3on0CNLJUBn+CVHArqiGAaBt6eXRwJEBwJEBwJEBwJEBwJEBwJEBwJEBwJEBwJEBwJEBw\
	JEBwJEBw/gUUgiKM2bt1ygAAAABJRU5ErkJggg==\
	"
	local toast_mouth_png =
	"iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAACO0lEQVR4nO3cP2oVURhA8RNJ\
	o+ICAiJWVoqFm7BIkTaF4DYsRNyKYGUrpBBSZweChYSkTBqx8w/PIgmYKnmPebzhnvMrh7nw\
	DXNmGG4xW4vFgnjd2fQA2awCkCsAuQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuQKQ\
	KwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkCkCsAuQKQKwC5ApArALkC\
	kCsAuQKQKwC57U0PsC7vdh+vvPb95+PJ5pi7rRF/FfvfzX8APAEeAveBu5fHriyAH8AZcAJ8\
	Bf6AJ4Jh3wDAPvCB5a7xBHgBnK9lohka+RvgDcsH/gh4Pf0o8zVyAM9WXPd00ilmbuQAfq64\
	7tekU8zcyAEcrLjucNIpZm7kAN6y/FvgCPi0hllma+QAvgEvgdNbnv8F2AX+rm2iGRpyHwCu\
	7QXcA14Be8BzYOfy+G/gOxdP/UcuAgA8ewAwcACw/G6g6cZfGTqA3Gzkb4DcQgHIFYBcAcgV\
	gFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHI\
	FYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwB\
	yBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBcAcgVgFwByBWAXAHIFYBc\
	AcgVgFwByBWAXAHI/QPwhzWTvXo9agAAAABJRU5ErkJggg==\
	"

	local class = require_30log()

	local Toast = class("Toast")

	function Toast:init()
		self:center()

		self.eyes = {}
		self.eyes.closed_t = 0
		self.eyes.blink_t = 2

		self.look = {}
		self.look.target = { x = 0.2,  y = 0.2 }
		self.look.current = { x = 0.2,  y = 0.2 }
		self.look.DURATION = 0.5
		self.look.POINTS = {
			{ x = 0.8, y = 0.8 },
			{ x = 0.1, y = 0.1 },
			{ x = 0.8, y = 0.1 },
			{ x = 0.1, y = 0.8 },
		}
		self.look.point = 0
		self.look.point_t = 1
		self.look.t = 0
	end

	local function easeOut(t, b, c, d)
		t = t / d - 1
		return c * (math.pow(t, 3) + 1) + b
	end

	function Toast:center()
		local ww, wh = love.graphics.getDimensions()
		self.x = math.floor(ww / 2 / 32) * 32 + 16
		self.y = math.floor(wh / 2 / 32) * 32 + 16
	end

	function Toast:get_look_coordinates()
		local t = self.look.t

		local src = self.look.current
		local dst = self.look.target

		local look_x = easeOut(t, src.x, dst.x - src.x, self.look.DURATION)
		local look_y = easeOut(t, src.y, dst.y - src.y, self.look.DURATION)

		return look_x, look_y
	end

	function Toast:update(dt)
		self.look.t = math.min(self.look.t + dt, self.look.DURATION)
		self.eyes.closed_t = math.max(self.eyes.closed_t - dt, 0)
		self.eyes.blink_t = math.max(self.eyes.blink_t - dt, 0)
		self.look.point_t = math.max(self.look.point_t - dt, 0)

		if self.eyes.blink_t == 0 then
			self:blink()
		end

		if self.look.point_t == 0 then
			self:look_at_next_point()
		end

		local look_x, look_y = self:get_look_coordinates()

		self.offset_x = look_x * 4
		self.offset_y = (1 - look_y) * -4
	end

	function Toast:draw()
		local x = self.x
		local y = self.y

		local look_x, look_y = self:get_look_coordinates()

		love.graphics.draw(g_images.toast.back, x, y, self.r, 1, 1, 64, 64)
		love.graphics.draw(g_images.toast.front, x + self.offset_x, y + self.offset_y, self.r, 1, 1, 64, 64)
		love.graphics.draw(self:get_eyes_image(), x + self.offset_x * 2.5, y + self.offset_y * 2.5, self.r, 1, 1, 64, 64)
		love.graphics.draw(g_images.toast.mouth, x + self.offset_x * 2, y + self.offset_y * 2, self.r, 1, 1, 64, 64)
	end

	function Toast:get_eyes_image()
		if self.eyes.closed_t > 0 then
			return g_images.toast.eyes.closed
		end
		return g_images.toast.eyes.open
	end

	function Toast:blink()
		if self.eyes.closed_t > 0 then
			return
		end
		self.eyes.closed_t = 0.1
		self.eyes.blink_t = self.next_blink()
	end

	function Toast:next_blink()
		return 5 + math.random(0, 3)
	end

	function Toast:look_at(tx, ty)
		local look_x, look_y = self:get_look_coordinates()
		self.look.current.x = look_x
		self.look.current.y = look_y

		self.look.t = 0
		self.look.point_t = 3 + math.random(0, 1)

		self.look.target.x = tx
		self.look.target.y = ty
	end

	function Toast:look_at_next_point()
		self.look.point = self.look.point + 1
		if self.look.point > #self.look.POINTS then
			self.look.point = 1
		end
		local point = self.look.POINTS[self.look.point]
		self:look_at(point.x, point.y)
	end

	local Mosaic = class("Mosaic")

	function Mosaic:init()
		local sw, sh = g_images.mosaic:getDimensions()
		local ww, wh = love.graphics.getDimensions()

		local SIZE_X = math.floor(ww / 32 + 2)
		local SIZE_Y = math.floor(wh / 32 + 2)
		local SIZE = SIZE_X * SIZE_Y

		self.batch = love.graphics.newSpriteBatch(g_images.mosaic, SIZE, "dynamic")
		self.pieces = {}
		self.color_t = 1

		local COLORS = {}

		for _,color in ipairs({
			{ 240, 240, 240 }, -- WHITE (ish)
			{ 232, 104, 162}, -- PINK
			{ 69, 155, 168 }, -- BLUE
			{ 67, 93, 119 }, -- DARK BLUE
		}) do
			table.insert(COLORS, color)
			table.insert(COLORS, color)
		end

		-- Insert only once. This way it appears half as often.
		table.insert(COLORS, { 220, 239, 113 }) -- LIME

		self.generator = function()
			return COLORS[math.random(1, #COLORS)]
		end

		local QUADS = {
			love.graphics.newQuad(0, 0, 32, 32, sw, sh),
			love.graphics.newQuad(0, 32, 32, 32, sw, sh),
			love.graphics.newQuad(32, 32, 32, 32, sw, sh),
			love.graphics.newQuad(32, 0, 32, 32, sw, sh),
		}

		local exclude_left = math.floor(ww / 2 / 32)
		local exclude_right = exclude_left + 3
		local exclude_top = math.floor(wh / 2 / 32)
		local exclude_bottom = exclude_top + 3
		local exclude_width = exclude_right - exclude_left + 1
		local exclude_height = exclude_bottom - exclude_top + 1
		local exclude_area = exclude_width * exclude_height

		local EXCLUDE = {}
		for y = exclude_top,exclude_bottom do
			EXCLUDE[y]  = {}
			for x = exclude_left,exclude_right do
				EXCLUDE[y][x] = true
			end
		end

		for y = 1,SIZE_Y do
			for x = 1,SIZE_X do
				if not EXCLUDE[y] or not EXCLUDE[y][x] then
					local piece = {
						x = (x - 1) * 32,
						y = (y - 1) * 32,
						r = math.random(0, 100) / 100 * math.pi,
						rv = 1,
						color = {},
						quad = QUADS[(x + y) % 4 + 1]
					}

					piece.color.prev = self.generator(#self.pieces + 1, 0, 2)
					piece.color.next = piece.color.prev
					table.insert(self.pieces, piece)
				end
			end
		end

		local GLYPHS = {
			N = love.graphics.newQuad(0, 64, 32, 32, sw, sh),
			O = love.graphics.newQuad(32, 64, 32, 32, sw, sh),
			G = love.graphics.newQuad(0, 96, 32, 32, sw, sh),
			A = love.graphics.newQuad(32, 96, 32, 32, sw, sh),
			M = love.graphics.newQuad(64, 96, 32, 32, sw, sh),
			E = love.graphics.newQuad(96, 96, 32, 32, sw, sh),

			U = love.graphics.newQuad(64, 0, 32, 32, sw, sh),
			P = love.graphics.newQuad(96, 0, 32, 32, sw, sh),
			o = love.graphics.newQuad(64, 32, 32, 32, sw, sh),
			S = love.graphics.newQuad(96, 32, 32, 32, sw, sh),
			R = love.graphics.newQuad(64, 64, 32, 32, sw, sh),
			T = love.graphics.newQuad(96, 64, 32, 32, sw, sh),
		}

		local INITIAL_TEXT_COLOR = { 240, 240, 240 }

		local put_text = function(str, offset, x, y)
			local idx = offset + SIZE_X * y + x
			for i = 1, #str do
				local c = str:sub(i, i)
				if c ~= " " then
					local piece = self.pieces[idx + i]
					piece.quad = GLYPHS[c]
					piece.r = 0
					piece.rv = 0
					piece.color.prev = INITIAL_TEXT_COLOR
					piece.color.next = INITIAL_TEXT_COLOR
				end
			end
		end

		local text_center_x = math.floor(ww / 2 / 32)

		local no_game_text_offset = SIZE_X * exclude_bottom - exclude_area
		put_text("No GAME", no_game_text_offset, text_center_x - 2, 1)

		put_text("SUPER TOAST", 0, text_center_x - 4, exclude_top - 3)
	end

	function Mosaic:update(dt)
		self.color_t = math.max(self.color_t - dt, 0)
		local change_color = self.color_t == 0
		if change_color then
			self.color_t = 1
		end
		local gen = self.generator
		for idx,piece in ipairs(self.pieces) do
			piece.r = piece.r + piece.rv * dt
			if change_color and not piece.color.immutable then
				piece.color.prev = piece.color.next
				piece.color.next = gen()
			end
		end
	end

	function Mosaic:draw()
		self.batch:clear()
		love.graphics.setColor(255, 255, 255, 64)
		for idx,piece in ipairs(self.pieces) do
			local ct = 1 - self.color_t
			local c0 = piece.color.prev
			local c1 = piece.color.next
			local r = easeOut(ct, c0[1], c1[1] - c0[1], 1)
			local g = easeOut(ct, c0[2], c1[2] - c0[2], 1)
			local b = easeOut(ct, c0[3], c1[3] - c0[3], 1)

			self.batch:setColor(r, g, b)
			self.batch:add(piece.quad, piece.x, piece.y, piece.r, 1, 1, 16, 16)
		end
		love.graphics.setColor(255, 255, 255, 255)
		love.graphics.draw(self.batch, 0, 0)
	end

	function love.load()
		love.graphics.setBackgroundColor(136, 193, 206)

		local function load_image(file, name)
			return love.graphics.newImage(love.image.newImageData(love.filesystem.newFileData(file, name:gsub("_", "."), "base64")))
		end

		g_images = {}
		g_images.toast = {}
		g_images.toast.back = load_image(toast_back_png, "toast_back.png")
		g_images.toast.front = load_image(toast_front_png, "toast_front.png")
		g_images.toast.eyes = {}
		g_images.toast.eyes.open = load_image(toast_eyes_open_png, "toast_eyes_open.png")
		g_images.toast.eyes.closed = load_image(toast_eyes_closed_png, "toast_eyes_closed.png")
		g_images.toast.mouth = load_image(toast_mouth_png, "toast_mouth.png")
		g_images.mosaic = load_image(mosaic_png, "mosaic.png")

		g_entities = {}
		g_entities.toast = Toast()
		g_entities.mosaic = Mosaic()
	end

	function love.update(dt)
		g_entities.toast:update(dt)
		g_entities.mosaic:update(dt)
	end

	function love.draw()
		love.graphics.setColor(255, 255, 255)
		g_entities.mosaic:draw()
		g_entities.toast:draw()
	end

	function love.resize(w, h)
		g_entities.mosaic = Mosaic()
		g_entities.toast:center()
	end

	function love.keyreleased(key)
		if key == "f" then
			local is_fs = love.window.getFullscreen()
			love.window.setFullscreen(not is_fs)
		end
	end

	function love.mousepressed(x, y, b)
		local tx = x / love.graphics.getWidth()
		local ty = y / love.graphics.getHeight()
		g_entities.toast:look_at(tx, ty)
	end

	function love.conf(t)
		t.title = "L\195\150VE " .. love._version .. " (" .. love._version_codename .. ")"
		t.gammacorrect = true
		t.modules.audio = false
		t.modules.sound = false
		t.modules.physics = false
		t.modules.joystick = false
		t.window.resizable = true
		t.window.highdpi = true
	end
end

return love.nogame
