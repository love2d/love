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

-- Make sure love exists.
local love = require("love")

-- Used for setup:
love.path = {}
love.arg = {}

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
	p = love.path.normalslashes(p)

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
		keypressed = function (b,s,r)
			if love.keypressed then return love.keypressed(b,s,r) end
		end,
		keyreleased = function (b,s)
			if love.keyreleased then return love.keyreleased(b,s) end
		end,
		textinput = function (t)
			if love.textinput then return love.textinput(t) end
		end,
		textedited = function (t,s,l)
			if love.textedited then return love.textedited(t,s,l) end
		end,
		mousemoved = function (x,y,dx,dy,t)
			if love.mousemoved then return love.mousemoved(x,y,dx,dy,t) end
		end,
		mousepressed = function (x,y,b,t)
			if love.mousepressed then return love.mousepressed(x,y,b,t) end
		end,
		mousereleased = function (x,y,b,t)
			if love.mousereleased then return love.mousereleased(x,y,b,t) end
		end,
		wheelmoved = function (x,y)
			if love.wheelmoved then return love.wheelmoved(x,y) end
		end,
		touchpressed = function (id,x,y,dx,dy,p)
			if love.touchpressed then return love.touchpressed(id,x,y,dx,dy,p) end
		end,
		touchreleased = function (id,x,y,dx,dy,p)
			if love.touchreleased then return love.touchreleased(id,x,y,dx,dy,p) end
		end,
		touchmoved = function (id,x,y,dx,dy,p)
			if love.touchmoved then return love.touchmoved(id,x,y,dx,dy,p) end
		end,
		joystickpressed = function (j,b)
			if love.joystickpressed then return love.joystickpressed(j,b) end
		end,
		joystickreleased = function (j,b)
			if love.joystickreleased then return love.joystickreleased(j,b) end
		end,
		joystickaxis = function (j,a,v)
			if love.joystickaxis then return love.joystickaxis(j,a,v) end
		end,
		joystickhat = function (j,h,v)
			if love.joystickhat then return love.joystickhat(j,h,v) end
		end,
		gamepadpressed = function (j,b)
			if love.gamepadpressed then return love.gamepadpressed(j,b) end
		end,
		gamepadreleased = function (j,b)
			if love.gamepadreleased then return love.gamepadreleased(j,b) end
		end,
		gamepadaxis = function (j,a,v)
			if love.gamepadaxis then return love.gamepadaxis(j,a,v) end
		end,
		joystickadded = function (j)
			if love.joystickadded then return love.joystickadded(j) end
		end,
		joystickremoved = function (j)
			if love.joystickremoved then return love.joystickremoved(j) end
		end,
		focus = function (f)
			if love.focus then return love.focus(f) end
		end,
		mousefocus = function (f)
			if love.mousefocus then return love.mousefocus(f) end
		end,
		visible = function (v)
			if love.visible then return love.visible(v) end
		end,
		quit = function ()
			return
		end,
		threaderror = function (t, err)
			if love.threaderror then return love.threaderror(t, err) end
		end,
		resize = function (w, h)
			if love.resize then return love.resize(w, h) end
		end,
		filedropped = function (f)
			if love.filedropped then return love.filedropped(f) end
		end,
		directorydropped = function (dir)
			if love.directorydropped then return love.directorydropped(dir) end
		end,
		lowmemory = function ()
			collectgarbage()
			if love.lowmemory then return love.lowmemory() end
		end,
	}, {
		__index = function(self, name)
			error("Unknown event: " .. name)
		end,
	})

end

local function uridecode(s)
	return s:gsub("%%%x%x", function(str)
		return string.char(tonumber(str:sub(2), 16))
	end)
end

local no_game_code = false

-- This can't be overriden.
function love.boot()

	-- This is absolutely needed.
	require("love.filesystem")

	love.arg.parse_options()

	local o = love.arg.options

	local arg0 = love.arg.getLow(arg)
	love.filesystem.init(arg0)

	local exepath = love.filesystem.getExecutablePath()
	if #exepath == 0 then
		-- This shouldn't happen, but just in case we'll fall back to arg0.
		exepath = arg0
	end

	-- Is this one of those fancy "fused" games?
	local can_has_game = pcall(love.filesystem.setSource, exepath)
	local is_fused_game = can_has_game or love.arg.options.fused.set

	love.filesystem.setFused(is_fused_game)

	local identity = ""
	if not can_has_game and o.game.set and o.game.arg[1] then
		local nouri = o.game.arg[1]

		if nouri:sub(1, 7) == "file://" then
			nouri = uridecode(nouri:sub(8))
		end

		local full_source =  love.path.getfull(nouri)
		can_has_game = pcall(love.filesystem.setSource, full_source)

		-- Use the name of the source .love as the identity for now.
		identity = love.path.leaf(full_source)
	else
		-- Use the name of the exe as the identity for now.
		identity = love.path.leaf(exepath)
	end

	-- Try to use the archive containing main.lua as the identity name. It
	-- might not be available, in which case the fallbacks above are used.
	local realdir = love.filesystem.getRealDirectory("main.lua")
	if realdir then
		identity = love.path.leaf(realdir)
	end

	identity = identity:gsub("^([%.]+)", "") -- strip leading "."'s
	identity = identity:gsub("%.([^%.]+)$", "") -- strip extension
	identity = identity:gsub("%.", "_") -- replace remaining "."'s with "_"
	identity = #identity > 0 and identity or "lovegame"

	-- When conf.lua is initially loaded, the main source should be checked
	-- before the save directory (the identity should be appended.)
	pcall(love.filesystem.setIdentity, identity, true)

	if can_has_game and not (love.filesystem.isFile("main.lua") or love.filesystem.isFile("conf.lua")) then
		no_game_code = true
	end

	if not can_has_game then
		local nogame = require("love.nogame")
		nogame()
	end
end

function love.init()

	-- Create default configuration settings.
	-- NOTE: Adding a new module to the modules list
	-- will NOT make it load, see below.
	local c = {
		title = "Untitled",
		version = love._version,
		window = {
			width = 800,
			height = 600,
			x = nil,
			y = nil,
			minwidth = 1,
			minheight = 1,
			fullscreen = false,
			fullscreentype = "desktop",
			display = 1,
			vsync = true,
			msaa = 0,
			borderless = false,
			resizable = false,
			centered = true,
			highdpi = false,
		},
		modules = {
			event = true,
			keyboard = true,
			mouse = true,
			timer = true,
			joystick = true,
			touch = true,
			image = true,
			graphics = true,
			audio = true,
			math = true,
			physics = true,
			sound = true,
			system = true,
			font = true,
			thread = true,
			window = true,
			video = true,
		},
		console = false, -- Only relevant for windows.
		identity = false,
		appendidentity = false,
		externalstorage = false, -- Only relevant for Android.
		accelerometerjoystick = true, -- Only relevant for Android / iOS.
		gammacorrect = false,
	}

	-- Console hack, part 1.
	local openedconsole = false
	if love.arg.options.console.set and love._openConsole then
		love._openConsole()
		openedconsole = true
	end

	-- If config file exists, load it and allow it to update config table.
	local confok, conferr
	if (not love.conf) and love.filesystem and love.filesystem.isFile("conf.lua") then
		confok, conferr = pcall(require, "conf")
	end

	-- Yes, conf.lua might not exist, but there are other ways of making
	-- love.conf appear, so we should check for it anyway.
	if love.conf then
		confok, conferr = pcall(love.conf, c)
		-- If love.conf errors, we'll trigger the error after loading modules so
		-- the error message can be displayed in the window.
	end

	-- Console hack, part 2.
	if c.console and love._openConsole and not openedconsole then
		love._openConsole()
	end

	-- Hack for disabling accelerometer-as-joystick on Android / iOS.
	if love._setAccelerometerAsJoystick then
		love._setAccelerometerAsJoystick(c.accelerometerjoystick)
	end

	if love._setGammaCorrect then
		love._setGammaCorrect(c.gammacorrect)
	end

	-- Gets desired modules.
	for k,v in ipairs{
		"thread",
		"timer",
		"event",
		"keyboard",
		"joystick",
		"mouse",
		"touch",
		"sound",
		"system",
		"audio",
		"image",
		"video",
		"font",
		"window",
		"graphics",
		"math",
		"physics",
	} do
		if c.modules[v] then
			require("love." .. v)
		end
	end

	if love.event then
		love.createhandlers()
	end

	-- Check the version
	c.version = tostring(c.version)
	if not love.isVersionCompatible(c.version) then
		local major, minor, revision = c.version:match("^(%d+)%.(%d+)%.(%d+)$")
		if (not major or not minor or not revision) or (major ~= love._version_major and minor ~= love._version_minor) then
			local msg = ("This game indicates it was made for version '%s' of LOVE.\n"..
				"It may not be compatible with the running version (%s)."):format(c.version, love._version)

			print(msg)

			if love.window then
				love.window.showMessageBox("Compatibility Warning", msg, "warning")
			end
		end
	end

	if not confok and conferr then
		error(conferr)
	end

	-- Setup window here.
	if c.window and c.modules.window then
		assert(love.window.setMode(c.window.width, c.window.height,
		{
			fullscreen = c.window.fullscreen,
			fullscreentype = c.window.fullscreentype,
			vsync = c.window.vsync,
			msaa = c.window.msaa,
			resizable = c.window.resizable,
			minwidth = c.window.minwidth,
			minheight = c.window.minheight,
			borderless = c.window.borderless,
			centered = c.window.centered,
			display = c.window.display,
			highdpi = c.window.highdpi,
			x = c.window.x,
			y = c.window.y,
		}), "Could not set window mode")
		love.window.setTitle(c.window.title or c.title)
		if c.window.icon then
			assert(love.image, "If an icon is set in love.conf, love.image must be loaded!")
			love.window.setIcon(love.image.newImageData(c.window.icon))
		end
	end

	-- Our first timestep, because window creation can take some time
	if love.timer then
		love.timer.step()
	end

	if love.filesystem then
		love.filesystem._setAndroidSaveExternal(c.externalstorage)
		love.filesystem.setIdentity(c.identity or love.filesystem.getIdentity(), c.appendidentity)
		if love.filesystem.isFile("main.lua") then
			require("main")
		end
	end

	if no_game_code then
		error("No code to run\nYour game might be packaged incorrectly\nMake sure main.lua is at the top level of the zip")
	end
end

function love.run()

	if love.math then
		love.math.setRandomSeed(os.time())
	end

	if love.load then love.load(arg) end

	-- We don't want the first frame's dt to include time taken by love.load.
	if love.timer then love.timer.step() end

	local dt = 0

	-- Main loop time.
	while true do
		-- Process events.
		if love.event then
			love.event.pump()
			for name, a,b,c,d,e,f in love.event.poll() do
				if name == "quit" then
					if not love.quit or not love.quit() then
						return a
					end
				end
				love.handlers[name](a,b,c,d,e,f)
			end
		end

		-- Update dt, as we'll be passing it to update
		if love.timer then
			love.timer.step()
			dt = love.timer.getDelta()
		end

		-- Call update and draw
		if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled

		if love.graphics and love.graphics.isActive() then
			love.graphics.clear(love.graphics.getBackgroundColor())
			love.graphics.origin()
			if love.draw then love.draw() end
			love.graphics.present()
		end

		if love.timer then love.timer.sleep(0.001) end
	end

end

-----------------------------------------------------------
-- Error screen.
-----------------------------------------------------------

local debug, print = debug, print

local function error_printer(msg, layer)
	print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

function love.errhand(msg)
	msg = tostring(msg)

	error_printer(msg, 2)

	if not love.window or not love.graphics or not love.event then
		return
	end

	if not love.graphics.isCreated() or not love.window.isOpen() then
		local success, status = pcall(love.window.setMode, 800, 600)
		if not success or not status then
			return
		end
	end

	-- Reset state.
	if love.mouse then
		love.mouse.setVisible(true)
		love.mouse.setGrabbed(false)
		love.mouse.setRelativeMode(false)
		if love.mouse.hasCursor() then
			love.mouse.setCursor()
		end
	end
	if love.joystick then
		-- Stop all joystick vibrations.
		for i,v in ipairs(love.joystick.getJoysticks()) do
			v:setVibration()
		end
	end
	if love.audio then love.audio.stop() end
	love.graphics.reset()
	local font = love.graphics.setNewFont(math.floor(love.window.toPixels(14)))

	love.graphics.setBackgroundColor(89, 157, 220)
	love.graphics.setColor(255, 255, 255, 255)

	local trace = debug.traceback()

	love.graphics.clear(love.graphics.getBackgroundColor())
	love.graphics.origin()

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
		local pos = love.window.toPixels(70)
		love.graphics.clear(love.graphics.getBackgroundColor())
		love.graphics.printf(p, pos, pos, love.graphics.getWidth() - pos)
		love.graphics.present()
	end

	while true do
		love.event.pump()

		for e, a, b, c in love.event.poll() do
			if e == "quit" then
				return
			elseif e == "keypressed" and a == "escape" then
				return
			elseif e == "touchpressed" then
				local name = love.window.getTitle()
				if #name == 0 or name == "Untitled" then name = "Game" end
				local buttons = {"OK", "Cancel"}
				local pressed = love.window.showMessageBox("Quit "..name.."?", "", buttons)
				if pressed == 1 then
					return
				end
			end
		end

		draw()

		if love.timer then
			love.timer.sleep(0.1)
		end
	end

end

local function deferErrhand(...)
	local handler = love.errhand or error_printer
	return handler(...)
end


-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

return function()
	local result = xpcall(love.boot, error_printer)
	if not result then return 1 end
	local result = xpcall(love.init, deferErrhand)
	if not result then return 1 end
	local result, retval = xpcall(love.run, deferErrhand)
	if not result then return 1 end

	return tonumber(retval) or 0
end
