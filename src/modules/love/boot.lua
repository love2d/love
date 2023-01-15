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

-- Make sure love exists.
local love = require("love")

-- Essential code boot/init.
require("love.arg")
require("love.callbacks")

local function uridecode(s)
	return s:gsub("%%%x%x", function(str)
		return string.char(tonumber(str:sub(2), 16))
	end)
end

local no_game_code = false
local invalid_game_path = nil

-- This can't be overridden.
function love.boot()

	-- This is absolutely needed.
	require("love.filesystem")

	local arg0 = love.arg.getLow(arg)
	love.filesystem.init(arg0)

	local exepath = love.filesystem.getExecutablePath()
	if #exepath == 0 then
		-- This shouldn't happen, but just in case we'll fall back to arg0.
		exepath = arg0
	end

	no_game_code = false
	invalid_game_path = nil

	-- Is this one of those fancy "fused" games?
	local can_has_game = pcall(love.filesystem.setSource, exepath)

	-- It's a fused game, don't parse --game argument
	if can_has_game then
		love.arg.options.game.set = true
	end

	-- Parse options now that we know which options we're looking for.
	love.arg.parseOptions()

	local o = love.arg.options

	local is_fused_game = can_has_game or love.arg.options.fused.set

	love.filesystem.setFused(is_fused_game)

	love.setDeprecationOutput(not love.filesystem.isFused())

	local identity = ""
	if not can_has_game and o.game.set and o.game.arg[1] then
		local nouri = o.game.arg[1]

		if nouri:sub(1, 7) == "file://" then
			nouri = uridecode(nouri:sub(8))
		end

		local full_source = love.path.getFull(nouri)
		can_has_game = pcall(love.filesystem.setSource, full_source)

		if not can_has_game then
			invalid_game_path = full_source
		end

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

	if can_has_game and not (love.filesystem.getInfo("main.lua") or love.filesystem.getInfo("conf.lua")) then
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
			vsync = 1,
			msaa = 0,
			borderless = false,
			resizable = false,
			centered = true,
			highdpi = false,
			usedpiscale = true,
		},
		modules = {
			data = true,
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
		audio = {
			mixwithsystem = true, -- Only relevant for Android / iOS.
			mic = false, -- Only relevant for Android.
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
	if (not love.conf) and love.filesystem and love.filesystem.getInfo("conf.lua") then
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

	if love._setAudioMixWithSystem then
		if c.audio and c.audio.mixwithsystem ~= nil then
			love._setAudioMixWithSystem(c.audio.mixwithsystem)
		end
	end

	if love._requestRecordingPermission then
		love._requestRecordingPermission(c.audio and c.audio.mic)
	end

	-- Gets desired modules.
	for k,v in ipairs{
		"data",
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
		love.window.setTitle(c.window.title or c.title)
		assert(love.window.setMode(c.window.width, c.window.height,
		{
			fullscreen = c.window.fullscreen,
			fullscreentype = c.window.fullscreentype,
			vsync = c.window.vsync,
			msaa = c.window.msaa,
			stencil = c.window.stencil,
			depth = c.window.depth,
			resizable = c.window.resizable,
			minwidth = c.window.minwidth,
			minheight = c.window.minheight,
			borderless = c.window.borderless,
			centered = c.window.centered,
			display = c.window.display,
			highdpi = c.window.highdpi,
			usedpiscale = c.window.usedpiscale,
			x = c.window.x,
			y = c.window.y,
		}), "Could not set window mode")
		if c.window.icon then
			assert(love.image, "If an icon is set in love.conf, love.image must be loaded!")
			love.window.setIcon(love.image.newImageData(c.window.icon))
		end
	end

	-- The first couple event pumps on some systems (e.g. macOS) can take a
	-- while. We'd rather hit that slowdown here than in event processing
	-- within the first frames.
	if love.event then
		for i = 1, 2 do love.event.pump() end
	end

	-- Our first timestep, because window creation can take some time
	if love.timer then
		love.timer.step()
	end

	if love.filesystem then
		love.filesystem._setAndroidSaveExternal(c.externalstorage)
		love.filesystem.setIdentity(c.identity or love.filesystem.getIdentity(), c.appendidentity)
		if love.filesystem.getInfo("main.lua") then
			require("main")
		end
	end

	if no_game_code then
		error("No code to run\nYour game might be packaged incorrectly.\nMake sure main.lua is at the top level of the zip.")
	elseif invalid_game_path then
		error("Cannot load game at path '" .. invalid_game_path .. "'.\nMake sure a folder exists at the specified path.")
	end
end

local print, debug, tostring = print, debug, tostring

local function error_printer(msg, layer)
	print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

return function()
	local func
	local inerror = false

	local function deferErrhand(...)
		local errhand = love.errorhandler or love.errhand
		local handler = (not inerror and errhand) or error_printer
		inerror = true
		func = handler(...)
	end

	local function earlyinit()
		-- If love.boot fails, return 1 and finish immediately
		local result = xpcall(love.boot, error_printer)
		if not result then return 1 end

		-- If love.init or love.run fails, don't return a value,
		-- as we want the error handler to take over
		result = xpcall(love.init, deferErrhand)
		if not result then return end

		-- NOTE: We can't assign to func directly, as we'd
		-- overwrite the result of deferErrhand with nil on error
		local main
		result, main = xpcall(love.run, deferErrhand)
		if result then
			func = main
		end
	end

	func = earlyinit

	while func do
		local _, retval = xpcall(func, deferErrhand)
		if retval then return retval end
		coroutine.yield()
	end

	return 1
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
