--[[
Copyright (c) 2006-2011 LOVE Development Team

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

-- Make sure love table exists.
if not love then love = {} end

-- Used for setup:
love.path = {}
love.arg = {}

-- Unparsed arguments:
argv = {}

-- Replace any \ with /.
function love.path.normalslashes(p)
	return string.gsub(p, "\\", "/")
end

-- Makes sure there is a slash at the end
-- of a path.
function love.path.endslash(p)
	if string.sub(p, string.len(p)-1) ~= "/" then
		return p .. "/"
	else
		return p
	end
end

-- Checks whether a path is absolute or not.
function love.path.abs(p)

	local tmp = love.path.normalslashes(p)

	-- Path is absolute if it starts with a "/".
	if string.find(tmp, "/") == 1 then
		return true
	end

	-- Path is absolute if it starts with a
	-- letter followed by a colon.
	if string.find(tmp, "%a:") == 1 then
		return true
	end

	-- Relative.
	return false

end

-- Converts any path into a full path.
function love.path.getfull(p)

	if love.path.abs(p) then
		return love.path.normalslashes(p)
	end

	local cwd = love.filesystem.getWorkingDirectory()
	cwd = love.path.normalslashes(cwd)
	cwd = love.path.endslash(cwd)

	-- Construct a full path.
	local full = cwd .. love.path.normalslashes(p)

	-- Remove trailing /., if applicable
	return full:match("(.-)/%.") or full
end

-- Returns the leaf of a full path.
function love.path.leaf(p)
	local a = 1
	local last = p

	while a do
		a = string.find(p, "/", a+1)

		if a then
			last = string.sub(p, a+1)
		end
	end

	return last
end

-- Finds the key in the table with the lowest integral index. The lowest
-- will typically the executable, for instance "lua5.1.exe".
function love.arg.getLow(a)
	local m = math.huge
	for k,v in pairs(a) do
		if k < m then
			m = k
		end
	end
	return a[m]
end

love.arg.options = {
	console = { a = 0 },
	game = { a = 1 }
}

function love.arg.parse_option(m, i)
	m.set = true

	if m.a > 0 then
		m.arg = {}
		for j=i,i+m.a-1 do
			table.insert(m.arg, arg[j])
			i = j
		end
	end

	return i
end

function love.arg.parse_options()

	local game
	local argc = #arg

	for i=1,argc do
		-- Look for options.
		local s, e, m = string.find(arg[i], "%-%-(.+)")

		if m and love.arg.options[m] then
			i = love.arg.parse_option(love.arg.options[m], i+1)
		elseif not game then
			game = i
		end
	end

	if not love.arg.options.game.set then
		love.arg.parse_option(love.arg.options.game, game or 0)
	end
end

function love.createhandlers()

	-- Standard callback handlers.
	love.handlers = {
		kp = function (b, u)
			if love.keypressed then love.keypressed(b, u) end
		end,
		kr = function (b)
			if love.keyreleased then love.keyreleased(b) end
		end,
		mp = function (x,y,b)
			if love.mousepressed then love.mousepressed(x,y,b) end
		end,
		mr = function (x,y,b)
			if love.mousereleased then love.mousereleased(x,y,b) end
		end,
		jp = function (j,b)
			if love.joystickpressed then love.joystickpressed(j,b) end
		end,
		jr = function (j,b)
			if love.joystickreleased then love.joystickreleased(j,b) end
		end,
		f = function (f)
			if love.focus then love.focus(f) end
		end,
		q = function ()
			return
		end,
	}

end

-- This can't be overriden.
function love.boot()

	-- This is absolutely needed.
	require("love")
	require("love.filesystem")

	love.arg.parse_options()

	local o = love.arg.options

	--local abs_arg0 = love.path.getfull(love.arg.getLow(arg))
	--love.filesystem.init(abs_arg0)
	love.filesystem.init(love.arg.getLow(arg))

	-- Is this one of those fancy "fused" games?
	local can_has_game = pcall(love.filesystem.setSource, abs_arg0)

	if not can_has_game and o.game.set and o.game.arg[1] then
		local full_source =  love.path.getfull(o.game.arg[1])
		local leaf = love.path.leaf(full_source)
		love.filesystem.setIdentity(leaf)
		can_has_game = pcall(love.filesystem.setSource, full_source)
	end

	if can_has_game and not (love.filesystem.exists("main.lua") or love.filesystem.exists("conf.lua")) then
		no_game_code = true
	end

	if not can_has_game then
		love.nogame()
	end

end

function love.init()

	-- Create default configuration settings.
	local c = {
		title = "Untitled",
		author = "Unnamed",
		version = 0,
		screen = {
			width = 800,
			height = 600,
			fullscreen = false,
			vsync = true,
			fsaa = 0,
		},
		modules = {
			event = true,
			keyboard = true,
			mouse = true,
			timer = true,
			joystick = true,
			image = true,
			graphics = true,
			audio = true,
			physics = true,
			sound = true,
			font = true,
			thread = true,
		},
		console = false, -- Only relevant for windows.
		identity = false,
	}

	-- If config file exists, load it and allow it to update config table.
	if not love.conf and love.filesystem and love.filesystem.exists("conf.lua") then
		require("conf.lua")
	end

	-- Yes, conf.lua might not exist, but there are other ways of making
	-- love.conf appear, so we should check for it anyway.
	if love.conf then
		local ok, err = pcall(love.conf, c)
		if not ok then
			print(err)
			-- continue
		end
	end

	if love.arg.options.console.set then
		c.console = true
	end

	-- Gets desired modules.
	for k,v in pairs(c.modules) do
		if v then
			require("love." .. k)
		end
	end

	if love.event then
		love.createhandlers()
	end

	-- Setup screen here.
	if c.screen and c.modules.graphics then
		if love.graphics.checkMode(c.screen.width, c.screen.height, c.screen.fullscreen) or (c.screen.width == 0 and c.screen.height == 0) then
			assert(love.graphics.setMode(c.screen.width, c.screen.height, c.screen.fullscreen, c.screen.vsync, c.screen.fsaa), "Could not set screen mode")
		else
			error("Could not set screen mode")
		end
		love.graphics.setCaption(c.title)
	end

	if love.filesystem and c.identity then love.filesystem.setIdentity(c.identity) end

	if love.filesystem and love.filesystem.exists("main.lua") then require("main.lua") end

	if no_game_code then
		error("No code to run\nYour game might be packaged incorrectly\nMake sure main.lua is at the top level of the zip")
	end

	-- Console hack
	if c.console and love._openConsole then
		love._openConsole()
	end

end

function love.run()

	if love.load then love.load(arg) end

	local dt = 0

	-- Main loop time.
	while true do
		if love.timer then
			love.timer.step()
			dt = love.timer.getDelta()
		end
		if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled
		if love.graphics then
			love.graphics.clear()
			if love.draw then love.draw() end
		end

		-- Process events.
		if love.event then
			for e,a,b,c in love.event.poll() do
				if e == "q" then
					if not love.quit or not love.quit() then
						if love.audio then
							love.audio.stop()
						end
						return
					end
				end
				love.handlers[e](a,b,c)
			end
		end

		if love.timer then love.timer.sleep(1) end
		if love.graphics then love.graphics.present() end

	end

end

-----------------------------------------------------------
-- Default screen.
-----------------------------------------------------------

function love.nogame()

local knoll1_base64 = [[
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
KZcAAAAASUVORK5CYII=]]

local love_base64 = [[
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
kVm7kiJJ6gAAAABJRU5ErkJggg==]]

local planet_base64 = [[
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]]

local star1_base64 = [[
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAABSElEQVRYhe2W0W3CMBCGv1S8
NyNU6gKwQdmk3SBMAExAmQB1AtoJmk5QGACJEcIE6YMd9bBQfOekSh/yPzmO7f//z3e2s7qu
GRJ3g7KPAkYB/0HApK+FTpfr78d73bzOEThdrshXwNQyXxWB0B04h6I/BzbAM1D2LsBjAxw9
wTkg/8ToPEVADux8++CFfPm+PIUcINPcBd7tFPhWrDkHyr9IwsZ1DDnczptkAcLNm2L4Diga
ETEhqi1oFvPQ3t8lsMBFDrh9NqgiIMgLJTnAEy5nVrQkqSUHHoClYTw491XbgGgZCvfacjsD
78DWt1uhPQcKXEhjOAAz2RErR8tBtBYkFb8Hk4zKh4Y0RcBry7+9aJd6agd1GYYIbsAmOTOw
RSBZQCBij9uKuYUcOr4HBNkLLuvN6OtJVuFKz4xOW9AHBn+UjgJGAT/m/FW9QwCi3AAAAABJ
RU5ErkJggg==]]

	local function create_star(scale, speed)
		return {
			x0 = -100,
			x = 1000,
			y = math.random() * 600,

			r0 = math.random() * math.pi * 2,
			rv = math.random() * math.pi * 4,
			t = math.random(),
			scale = scale,
			speed = speed,
		}
	end

	local function create_star_layer(num_stars, scale, speed)

		local layer = {}

		for i = 1,num_stars do
			table.insert(layer, create_star(scale, speed))
		end

		return layer
	end

	local function update_star(dt, s)
		s.t = s.t + 0.1 * s.speed * dt

		while s.t > 1 do
			s.t = s.t - 1
		end
	end

	local function draw_star(s)

		local x = s.x0 + s.x * s.t
		local y = s.y
		local r = s.r0 + s.rv * s.t
		local sx = s.scale
		local sy = s.scale

		love.graphics.setColor(255*s.scale, 255*s.scale, 255, 255*s.scale)
		love.graphics.draw(star1, x, y, r, sx, sy, 16, 16)
	end

	local function update_star_layer(dt, layer)
		for k,v in ipairs(layer) do
			update_star(dt, v)
		end
	end

	local function draw_star_layer(layer)
		for k,v in ipairs(layer) do
			draw_star(v)
		end
	end

	local function create_knoll(p)
		return {
			p = p,
			t0 = math.random() * 10,
			t = 0
		}
	end

	local function create_knolls(n)
		local t = {}
		for i = 1,n do
			table.insert(t, create_knoll((math.pi * 2 / n) * i))
		end
		return t
	end

	local function update_knoll(dt, k)
		k.t = k.t + dt
	end

	local function draw_knoll(k)
		local x = planet.x
		local y = planet.y
		local a = k.p + k.t
		local h = 225 + 20 * (1 + math.sin(k.t0 + k.t))
		love.graphics.draw(knoll1, x, y, a, .75, .75, 64, h)
	end

	local function update_logo(dt)
		logo.r = logo.r + dt
		if logo.r > 360 then logo.r = logo.r - 360 end
		logo.y = planet.y
	end

	local function update_planet(dt)
		planet.t = planet.t + dt
		planet.y = 300 + math.sin(planet.t)*30
	end

	local function load_image_b64(b64, filename)
		local file_data = love.filesystem.newFileData(b64, filename, "base64")
		local image_data = love.image.newImageData(file_data)
		return love.graphics.newImage(image_data)
	end

	function love.load()

		planet = {
			x = 400,
			y = 300,
			w = 128,
			h = 128,
			t = 0,
			img = load_image_b64(planet_base64, "planet.png")
		}

		logo = {
			x = 400,
			y = 300,
			w = 256,
			h = 64,
			r = 0,
			img = load_image_b64(love_base64, "love.png")
		}

		love.graphics.setBackgroundColor(18, 18, 18)
		star1 = load_image_b64(star1_base64, "star1.png")
		knoll1 = load_image_b64(knoll1_base64, "knoll1.png")

		layers = {}
		knolls = create_knolls(10)

		-- Add star layers.
		table.insert(layers, create_star_layer(100, 0.5, 0.5))
		table.insert(layers, create_star_layer(70, 0.7, 0.7))
		table.insert(layers, create_star_layer(50, 1, 1))

		math.randomseed(os.time())

	end

	function love.draw()
		for k, v in ipairs(layers) do
			draw_star_layer(v)
		end

		for k, v in ipairs(knolls) do
			draw_knoll(v)
		end

		love.graphics.draw(planet.img, planet.x, planet.y, 0, 1, 1, planet.w, planet.h)
		love.graphics.draw(logo.img, logo.x, logo.y, logo.r, 1, 1, logo.w/2, logo.h/2)
	end

	function love.update(dt)
		for k, v in ipairs(layers) do
			update_star_layer(dt, v)
		end
		for k, v in ipairs(knolls) do
			update_knoll(dt, v)
		end
		update_planet(dt)
		update_logo(dt)
	end

	function love.conf(t)
		t.title = "LOVE " .. love._version_string .. " (" .. love._version_codename .. ")"
		t.modules.audio = false
		t.modules.sound = false
		t.modules.physics = false
		t.modules.joystick = false
	end

end

-----------------------------------------------------------
-- Error screen.
-----------------------------------------------------------

local debug = debug

local function error_printer(msg, layer)
	print((debug.traceback("Error: " .. msg, 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

function love.errhand(msg)

	error_printer(msg, 2)

	if not love.graphics or not love.event or not love.graphics.isCreated() then
		return
	end

	love.graphics.setRenderTarget()

	-- Load.
	love.graphics.setScissor()
	love.graphics.setBlendMode("alpha")
	love.graphics.setBackgroundColor(89, 157, 220)
	local font = love.graphics.newFont(14)
	love.graphics.setFont(font)

	love.graphics.setColor(255, 255, 255, 255)

	local trace = debug.traceback()

	love.graphics.clear()

	local err = {}

	table.insert(err, "Error\n")
	table.insert(err, msg.."\n\n")

	for l in string.gmatch(trace, "(.-)\n") do
		if not string.match(l, "boot.lua") then
			l = string.gsub(l, "stack traceback:", "Traceback\n")
			table.insert(err, l)
		end
	end

	local p = table.concat(err, "\n")

	p = string.gsub(p, "\t", "")
	p = string.gsub(p, "%[string \"(.-)\"%]", "%1")

	local function draw()
		love.graphics.clear()
		love.graphics.printf(p, 70, 70, love.graphics.getWidth() - 70)
		love.graphics.present()
	end

	draw()

	local e, a, b, c
	while true do
		e, a, b, c = love.event.wait()

		if e == "q" then
			return
		end
		if e == "kp" and a == "escape" then
			return
		end

		draw()

	end

end


-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

local result = xpcall(love.boot, error_printer)
if not result then return end
local result = xpcall(love.init, love.errhand)
if not result then return end
local result = xpcall(love.run, love.errhand)
if not result then return end
