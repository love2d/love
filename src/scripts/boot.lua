--[[
Copyright (c) 2006-2012 LOVE Development Team

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
	return full:match("(.-)/%.$") or full
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
	fused = {a = 0 },
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
	love.handlers = setmetatable({
		keypressed = function (b, u)
			if love.keypressed then love.keypressed(b, u) end
		end,
		keyreleased = function (b)
			if love.keyreleased then love.keyreleased(b) end
		end,
		mousepressed = function (x,y,b)
			if love.mousepressed then love.mousepressed(x,y,b) end
		end,
		mousereleased = function (x,y,b)
			if love.mousereleased then love.mousereleased(x,y,b) end
		end,
		joystickpressed = function (j,b)
			if love.joystickpressed then love.joystickpressed(j,b) end
		end,
		joystickreleased = function (j,b)
			if love.joystickreleased then love.joystickreleased(j,b) end
		end,
		focus = function (f)
			if love.focus then love.focus(f) end
		end,
		quit = function ()
			return
		end,
	}, {
		__index = function(self, name)
			error("Unknown event: " .. name)
		end,
	})

end

local is_fused_game = false
local no_game_code = false

-- This can't be overriden.
function love.boot()

	-- This is absolutely needed.
	require("love")
	require("love.filesystem")

	love.arg.parse_options()

	local o = love.arg.options

	local arg0 = love.arg.getLow(arg)
	love.filesystem.init(arg0)

	-- Is this one of those fancy "fused" games?
	local can_has_game = pcall(love.filesystem.setSource, arg0)
	is_fused_game = can_has_game
	if love.arg.options.fused.set then
		is_fused_game = true
	end

	if not can_has_game and o.game.set and o.game.arg[1] then
		local nouri = o.game.arg[1]
		if nouri:sub(1, 7) == "file://" then
			nouri = nouri:sub(8)
		end
		local full_source =  love.path.getfull(nouri)
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
	-- NOTE: Adding a new module to the modules list
	-- will NOT make it load, see below.
	local c = {
		title = "Untitled",
		author = "Unnamed",
		version = love._version,
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
		release = false,
	}

	-- If config file exists, load it and allow it to update config table.
	if not love.conf and love.filesystem and love.filesystem.exists("conf.lua") then
		require("conf")
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

	if c.release then
		love._release = {
			title = c.title ~= "Untitled" and c.title or nil,
			author = c.author ~= "Unnamed" and c.author or nil,
			url = c.url
		}
	end

	if love.arg.options.console.set then
		c.console = true
	end

	-- Gets desired modules.
	for k,v in ipairs{
		"thread",
		"timer",
		"event",
		"keyboard",
		"joystick",
		"mouse",
		"sound",
		"audio",
		"image",
		"font",
		"graphics",
		"physics",
	} do
		if c.modules[v] then
			require("love." .. v)
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

	-- Our first timestep, because screen creation can take some time
	if love.timer then
		love.timer.step()
	end

	if love.filesystem then
		love.filesystem.setRelease(c.release and is_fused_game)
		if c.identity then love.filesystem.setIdentity(c.identity) end
		if love.filesystem.exists("main.lua") then require("main") end
	end

	if no_game_code then
		error("No code to run\nYour game might be packaged incorrectly\nMake sure main.lua is at the top level of the zip")
	end

	-- Console hack
	if c.console and love._openConsole then
		love._openConsole()
	end

	-- Check the version
	local compat = false
	c.version = tostring(c.version)
	for i, v in ipairs(love._version_compat) do
		if c.version == v then
			compat = true
			break
		end
	end
	if not compat then
		local major, minor, revision = c.version:match("^(%d+)%.(%d+)%.(%d+)$")
		if (not major or not minor or not revision) or (major ~= love._version_major and minor ~= love._version_minor) then
			local msg = "This game was made for a version that is probably incompatible.\n"..
				"The game might still work, but it is not guaranteed.\n" ..
				"Furthermore, this means one should not judge this game or the engine if not."
			print(msg)
			if love.graphics and love.timer and love.event then
				love.event.pump()
				love.graphics.setBackgroundColor(89, 157, 220)
				love.graphics.clear()
				love.graphics.print(msg, 70, 70)
				love.graphics.present()
				love.graphics.setBackgroundColor(0, 0, 0)
				love.timer.sleep(3)
			end
		end
	end

end

function love.run()

	math.randomseed(os.time())
	math.random() math.random()

	if love.load then love.load(arg) end

	local dt = 0

	-- Main loop time.
	while true do
		-- Process events.
		if love.event then
			love.event.pump()
			for e,a,b,c,d in love.event.poll() do
				if e == "quit" then
					if not love.quit or not love.quit() then
						if love.audio then
							love.audio.stop()
						end
						return
					end
				end
				love.handlers[e](a,b,c,d)
			end
		end

		-- Update dt, as we'll be passing it to update
		if love.timer then
			love.timer.step()
			dt = love.timer.getDelta()
		end

		-- Call update and draw
		if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled
		if love.graphics then
			love.graphics.clear()
			if love.draw then love.draw() end
		end

		if love.timer then love.timer.sleep(0.001) end
		if love.graphics then love.graphics.present() end

	end

end

-----------------------------------------------------------
-- Default screen.
-----------------------------------------------------------

function love.nogame()
	local pig_png =
		"iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ\
		bWFnZVJlYWR5ccllPAAAAyBpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdp\
		bj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6\
		eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMC1jMDYwIDYxLjEz\
		NDc3NywgMjAxMC8wMi8xMi0xNzozMjowMCAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJo\
		dHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlw\
		dGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAv\
		IiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RS\
		ZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpD\
		cmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNSBXaW5kb3dzIiB4bXBNTTpJbnN0YW5jZUlE\
		PSJ4bXAuaWlkOjdCQTYxREYxOUQ5RDExRTBBNjY4RTc1NEVEQTU1MERDIiB4bXBNTTpEb2N1bWVu\
		dElEPSJ4bXAuZGlkOjdCQTYxREYyOUQ5RDExRTBBNjY4RTc1NEVEQTU1MERDIj4gPHhtcE1NOkRl\
		cml2ZWRGcm9tIHN0UmVmOmluc3RhbmNlSUQ9InhtcC5paWQ6N0JBNjFERUY5RDlEMTFFMEE2NjhF\
		NzU0RURBNTUwREMiIHN0UmVmOmRvY3VtZW50SUQ9InhtcC5kaWQ6N0JBNjFERjA5RDlEMTFFMEE2\
		NjhFNzU0RURBNTUwREMiLz4gPC9yZGY6RGVzY3JpcHRpb24+IDwvcmRmOlJERj4gPC94OnhtcG1l\
		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\
		4xkvbY9d6gAAAABJRU5ErkJggg=="
	local heart_png =
		"iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ\
		bWFnZVJlYWR5ccllPAAAAyBpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdp\
		bj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6\
		eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMC1jMDYwIDYxLjEz\
		NDc3NywgMjAxMC8wMi8xMi0xNzozMjowMCAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJo\
		dHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlw\
		dGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAv\
		IiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RS\
		ZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpD\
		cmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNSBXaW5kb3dzIiB4bXBNTTpJbnN0YW5jZUlE\
		PSJ4bXAuaWlkOjk3NUYxN0NGOUQ5RDExRTA5MTgyQjc4MkNEQkY5RkI1IiB4bXBNTTpEb2N1bWVu\
		dElEPSJ4bXAuZGlkOjk3NUYxN0QwOUQ5RDExRTA5MTgyQjc4MkNEQkY5RkI1Ij4gPHhtcE1NOkRl\
		cml2ZWRGcm9tIHN0UmVmOmluc3RhbmNlSUQ9InhtcC5paWQ6OTc1RjE3Q0Q5RDlEMTFFMDkxODJC\
		NzgyQ0RCRjlGQjUiIHN0UmVmOmRvY3VtZW50SUQ9InhtcC5kaWQ6OTc1RjE3Q0U5RDlEMTFFMDkx\
		ODJCNzgyQ0RCRjlGQjUiLz4gPC9yZGY6RGVzY3JpcHRpb24+IDwvcmRmOlJERj4gPC94OnhtcG1l\
		dGE+IDw/eHBhY2tldCBlbmQ9InIiPz7MtgYTAAACd0lEQVR42uyajW2CUBDHH8YBWKBJR8AJChvY\
		AZriBNYJrBOIE9imA6gTyAZlg9p0ATaw99IzfSEgvI9DlLvk5UUUuPu9/71PvePxKPpsA9FzYwAM\
		gAEwAAbAABgAA2AADIABMAAG0EcbmtzkeV7p9Z+nVQhViB8PUNK7j+nBxkF4ZgzVHMp9ydeZfAeU\
		d3hPVnZ/3X6HZ7IhUgQATgZQraEEJT+XDi7AwVQzcB+qjQK0zuTzJ0Xg5ACw1aWjfs1tCTg30wh+\
		XwH0nOVQIlUNpACw5fcNglclKx3MCYJXIYxOSqiLb2CRm75m8AKD+kJwVba2CF6gP+s2RoGlZvCq\
		g/syCHDtFaqxg849hGeNyQDAw2WPHFu20gZVpKbT3OEI90ypgKkDB+9RCScIG8dDfCMFDE0l5shJ\
		2epLgPBdMc5bGaoqowAQOPQzFnTmU6XAVViTydctA2g0Bb9lAGnfAewoAaQdDz6H/N9SAth1HMCK\
		eiq87XLry5UnKQBcaXUVwurcatNlJzi79ta3AoAqSDoGYKHT+tYbIjjf/nQ8NTa1DIIfFS+SbYgo\
		9ojSu7RNTG6yBoCpEF0YwqJqV5g8BQpLT90tMjLpt5kCJyVkqIRDi8HnptInWQsghFHdJoRDm5lK\
		n2wxhMOQVMIbcfDynMH6Hc76gDKDfuFF/O0eO1/qQvBRkx+21gdUqCHBlHDZL2Q49Dox8v0ApV9I\
		HAUf6c72LpYCJSkRYkoEbQXfyumwAYhYVB95O235TgIogJjWKCKVOW8q+04DKMwi5UnOg/g/dMlx\
		imvVd5AAuCXjP0kxAAbAABgAA2AADIABMAAGwAB6ab8CDACjlPepVvE3rwAAAABJRU5ErkJggg=="

	local hearts = {}
	local rings = {}
	
	local cx = 400
	local cy = 300
	
	local pig
	local heart_image
	
	local function add_heart_ring(radius, number, speed)
		local step = math.pi*2/number
		for i = 1,number do
			local heart = {
				radius = radius,
				position = step*i,
				speed = speed,
				opacity = radius/450
			}
			table.insert(hearts, heart)
		end
		table.insert(rings, radius)
	end
	
	local function update_hearts(dt)
		for i,v in ipairs(hearts) do
			v.position = v.position + v.speed*dt*0.6
		end
	end
	
	local function draw_hearts()
		for i,v in ipairs(hearts) do
			local x = math.cos(v.position) * v.radius + cx
			local y = math.sin(v.position) * v.radius + cy
			love.graphics.setColor(255, 255, 255, v.opacity*255)
			love.graphics.draw(heart_image, x, y, v.position+0.4, 1, 1, 32, 32)
		end
	end
	
	local function draw_pig(p)
		love.graphics.setColor(255, 255, 255, 255)
		love.graphics.draw(p.img, p.x, p.y, 0, 1, 1, 128, 128)
	end

	local function loadimage(file, name)
		return love.graphics.newImage(love.image.newImageData(love.filesystem.newFileData(file, name:gsub("_", "."), "base64")))
	end
	
	function love.load()
		math.randomseed(os.time())
		pig = {
			x = cx,
			y = cy,
			img = loadimage(pig_png, "pig.png")
		}
		heart_image = loadimage(heart_png, "heart.png")
	
		add_heart_ring(100, 10, 2.4)
		add_heart_ring(150, 20, 2.2)
		add_heart_ring(200, 25, 2.0)
		add_heart_ring(250, 35, 1.8)
		add_heart_ring(300, 40, 1.6)
		add_heart_ring(350, 50, 1.4)
		add_heart_ring(400, 60, 1.2)
		add_heart_ring(450, 70, 1)
	end
	
	function love.update(dt)
		update_hearts(dt)
	end
	
	function love.draw()
		love.graphics.setBackgroundColor(254, 224, 238)
		draw_hearts()
		draw_pig(pig)
	end
	
	function love.keyreleased(key)
		if key == "escape" then
			love.event.quit()
		end
	end

	function love.conf(t)
		t.title = "L\195\150VE " .. love._version .. " (" .. love._version_codename .. ")"
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
	print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

function love.errhand(msg)
	msg = tostring(msg)

	error_printer(msg, 2)

	if not love.graphics or not love.event or not love.graphics.isCreated() then
		return
	end

	-- Load.
	if love.audio then love.audio.stop() end
	love.graphics.reset()
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

		if e == "quit" then
			return
		end
		if e == "keypressed" and a == "escape" then
			return
		end

		draw()

	end

end

function love.releaseerrhand(msg)
	print("An error has occured, the game has been stopped.")

	if not love.graphics or not love.event or not love.graphics.isCreated() then
		return
	end

	love.graphics.setCanvas()
	love.graphics.setPixelEffect()

	-- Load.
	if love.audio then love.audio.stop() end
	love.graphics.reset()
	love.graphics.setBackgroundColor(89, 157, 220)
	local font = love.graphics.newFont(14)
	love.graphics.setFont(font)

	love.graphics.setColor(255, 255, 255, 255)

	love.graphics.clear()

	local err = {}

	p = string.format("An error has occured that caused %s to stop.\nYou can notify %s about this%s.", love._release.title or "this game", love._release.author or "the author", love._release.url and " at " .. love._release.url or "")

	local function draw()
		love.graphics.clear()
		love.graphics.printf(p, 70, 70, love.graphics.getWidth() - 70)
		love.graphics.present()
	end

	draw()

	local e, a, b, c
	while true do
		e, a, b, c = love.event.wait()

		if e == "quit" then
			return
		end
		if e == "keypressed" and a == "escape" then
			return
		end

		draw()

	end
end


-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

local result = xpcall(love.boot, error_printer)
if not result then return 1 end
local result = xpcall(love.init, love._release and love.releaseerrhand or love.errhand)
if not result then return 1 end
local result, retval = xpcall(love.run, love._release and love.releaseerrhand or love.errhand)
if not result then return 1 end

return tonumber(retval) or 0
