--[[
Copyright (c) 2006-2010 LOVE Development Team

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
	return cwd .. love.path.normalslashes(p)

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

	local abs_arg0 = love.path.getfull(love.arg.getLow(arg))
	love.filesystem.init(abs_arg0)

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
		love.filesystem = nil
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
		if love.graphics.checkMode(c.screen.width, c.screen.height, c.screen.fullscreen) then
			assert(love.graphics.setMode(c.screen.width, c.screen.height, c.screen.fullscreen, c.screen.vsync, c.screen.fsaa), "Could not set screen mode")
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

	love.load = function()

		major, minor, rev = string.match(love._version_string, "(%d)%.(%d)%.(%d)")
		if not major then major = 0 end
		if not minor then minor = 0 end
		if not rev then rev = 0 end

		names = {
			"knoll1",
			"love",
			"planet",
			"star1",
		}

		local decode = function(file)
			return love.graphics.newImage(love.image.newImageData(file))
		end

		images = {}

		for i,v in pairs(names) do
			images[v] = decode(love["_"..v.."_png"])
		end
	
		planet = {
			x = 400,
			y = 300,
			w = 128,
			h = 128,
			img = images["planet"]
		}
	
		logo = {
			x = 400,
			y = 300,
			w = 256,
			h = 64,
			r = 0,
			img = images["love"]
		}

		love.graphics.setBackgroundColor(18, 18, 18)
		star1 = images["star1"]
		knoll1 = images["knoll1"]
		
		layers = {}
		knolls = create_knolls(10)
	
		-- Add star layers.
		table.insert(layers, create_star_layer(100, 0.5, 0.5))
		table.insert(layers, create_star_layer(70, 0.7, 0.7))
		table.insert(layers, create_star_layer(50, 1, 1))
	
		math.randomseed(os.time())
	end

	function create_star(scale, speed)
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

	function create_star_layer(num_stars, scale, speed)
		
		local layer = {}
	
		for i = 1,num_stars do
			table.insert(layer, create_star(scale, speed))
		end
	
		return layer
	end

	function update_star(dt, s)
		s.t = s.t + 0.1 * s.speed * dt
	
		while s.t > 1 do
			s.t = s.t - 1
		end
	end

	function draw_star(s)

		local x = s.x0 + s.x * s.t
		local y = s.y
		local r = s.r0 + s.rv * s.t
		local sx = s.scale
		local sy = s.scale

		love.graphics.setColor(255*s.scale, 255*s.scale, 255, 255*s.scale)
		love.graphics.draw(star1, x, y, r, sx, sy, 16, 16)
	end

	function update_star_layer(dt, layer)
		for k,v in ipairs(layer) do
			update_star(dt, v)
		end
	end

	function draw_star_layer(layer)
		for k,v in ipairs(layer) do
			draw_star(v)
		end
	end

	function create_knolls(n)
		local t = {}
		for i = 1,n do
			table.insert(t, create_knoll((math.pi * 2 / n) * i))
		end
		return t
	end

	function create_knoll(p)
		return {
			p = p,
			t0 = math.random() * 10,
			t = 0
		}
	end

	function update_knoll(dt, k)
		k.t = k.t + dt
	end

	function draw_knoll(k)
		local x = planet.x
		local y = planet.y
		local a = k.p + k.t
		local h = 225 + 20 * (1 + math.sin(k.t0 + k.t))
		love.graphics.draw(knoll1, x, y, a, .75, .75, 64, h)
	end

	function update_logo(dt)
		logo.r = logo.r + dt
		if logo.r > 360 then logo.r = logo.r - 360 end
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
		update_logo(dt)
	end

	function love.keypressed(k)
		if k == "r" then
			local main = love.filesystem.load("main.lua")
			main()
			love.load()
		end
	end

	love.conf = function(t)
		t.title = "LOVE " .. love._version_string .. " (" .. love._version_codename .. "): The Final Frontier"
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
	love.graphics.setBackgroundColor(89, 157, 220)
	local font = love.graphics.newFont(love._vera_ttf, 14)
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
