R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2024 LOVE Development Team

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
local main_file = "main.lua"

-- This can't be overridden.
function love.boot()

	-- This is absolutely needed.
	require("love.filesystem")

	love.rawGameArguments = arg

	local arg0 = love.arg.getLow(love.rawGameArguments)
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
	love.arg.parseOptions(love.rawGameArguments)

	-- parseGameArguments can only be called after parseOptions.
	love.parsedGameArguments = love.arg.parseGameArguments(love.rawGameArguments)

	local o = love.arg.options

	local is_fused_game = can_has_game or love.arg.options.fused.set

	love.filesystem.setFused(is_fused_game)

	love.setDeprecationOutput(not love.filesystem.isFused())

	main_file = "main.lua"
	local custom_main_file = false

	local identity = ""
	if not can_has_game and o.game.set and o.game.arg[1] then
		local nouri = o.game.arg[1]
		local full_source = nouri

		-- Ignore "content://" uri as it's used to open a file-descriptor
		-- directly for Android.
		if nouri:sub(1, 10) ~= "content://" then
			if nouri:sub(1, 7) == "file://" then
				nouri = uridecode(nouri:sub(8))
			end

			full_source = love.path.getFull(nouri)
			local source_leaf = love.path.leaf(full_source)

			if source_leaf:match("%.lua$") then
				main_file = source_leaf
				custom_main_file = true
				full_source = love.path.getFull(full_source:sub(1, -(#source_leaf + 1)))
			end
		end

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
	local realdir = love.filesystem.getRealDirectory(main_file)
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

	if can_has_game and not (love.filesystem.getInfo(main_file) or (not custom_main_file and love.filesystem.getInfo("conf.lua"))) then
		no_game_code = true
	end

	if not can_has_game then
        -- when editing this message, change it at love.cpp too
        print([[LOVE is an *awesome* framework you can use to make 2D games in Lua
https://love2d.org

usage:
    love --version                  prints LOVE version and quits
    love --help                     prints this message and quits
    love path/to/gamedir            runs the game from the given directory which contains a main.lua file
    love path/to/packagedgame.love  runs the packaged game from the provided .love file
    love path/to/file.lua           runs the game from the given .lua file
]]);
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
			displayindex = 1,
			vsync = 1,
			msaa = 0,
			borderless = false,
			resizable = false,
			centered = true,
			usedpiscale = true,
		},
		graphics = {
			gammacorrect = false,
			lowpower = false,
			renderers = nil,
			excluderenderers = nil,
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
			sensor = true,
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
		gammacorrect = nil, -- Moved to t.graphics.
		highdpi = false,
		renderers = nil, -- Moved to t.graphics.
		excluderenderers = nil, -- Moved to t.graphics.
		trackpadtouch = false,
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

	if love._setGammaCorrect then
		local gammacorrect = false
		if type(c.graphics) == "table" then
			gammacorrect = c.graphics.gammacorrect
		end
		if c.gammacorrect ~= nil then
			love.markDeprecated(2, "t.gammacorrect in love.conf", "field", "replaced", "t.graphics.gammacorrect")
			gammacorrect = c.gammacorrect
		end
		love._setGammaCorrect(gammacorrect)
	end

	if love._setLowPowerPreferred and type(c.graphics) == "table" then
		love._setLowPowerPreferred(c.graphics.lowpower)
	end

	if love._setRenderers then
		local renderers = love._getDefaultRenderers()
		if type(c.renderers) == "table" then
			love.markDeprecated(2, "t.renderers in love.conf", "field", "replaced", "t.graphics.renderers")
			renderers = c.renderers
		end
		if type(c.graphics) == "table" and type(c.graphics.renderers) == "table" then
			renderers = c.graphics.renderers
		end
		if love.arg.options.renderers.set then
			local renderersstr = love.arg.options.renderers.arg[1]
			renderers = {}
			for r in renderersstr:gmatch("[^,]+") do
				table.insert(renderers, r)
			end
		end

		local excluderenderers = nil
		if type(c.excluderenderers) == "table" then
			love.markDeprecated(2, "t.excluderenderers in love.conf", "field", "replaced", "t.graphics.excluderenderers")
			excluderenderers = c.excluderenderers
		end
		if type(c.graphics) == "table" and type(c.graphics.excluderenderers) == "table" then
			excluderenderers = c.graphics.excluderenderers
		end
		if love.arg.options.excluderenderers.set then
			local excludestr = love.arg.options.excluderenderers.arg[1]
			excluderenderers = {}
			for r in excludestr:gmatch("[^,]+") do
				table.insert(excluderenderers, r)
			end
		end

		if type(excluderenderers) == "table" then
			for i,v in ipairs(excluderenderers) do
				for j=#renderers, 1, -1 do
					if renderers[j] == v then
						table.remove(renderers, j)
						break
					end
				end
			end
		end

		love._setRenderers(renderers)
	end

	if love._setHighDPIAllowed then
		love._setHighDPIAllowed(c.highdpi)
	end

	if love._setTrackpadTouch then
		love._setTrackpadTouch(c.trackpadtouch)
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
		"sensor",
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
		if c.window.icon then
			assert(love.image, "If an icon is set in love.conf, love.image must be loaded.")
			love.window.setIcon(love.image.newImageData(c.window.icon))
		end

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
			displayindex = c.window.displayindex,
			display = c.window.display, -- deprecated
			highdpi = c.window.highdpi, -- deprecated
			usedpiscale = c.window.usedpiscale,
			x = c.window.x,
			y = c.window.y,
		}), "Could not set window mode")
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
		if love.filesystem.getInfo(main_file) then
			require(main_file:gsub("%.lua$", ""))
		end
	end

	if no_game_code then
		local opts = love.arg.options
		local gamepath = opts.game.set and opts.game.arg[1] or ""
		local gamestr = gamepath == "" and "" or " at "..gamepath
		error("No code to run"..gamestr.."\nYour game might be packaged incorrectly.\nMake sure "..main_file.." is at the top level of the zip or folder.")
	elseif invalid_game_path then
		error("Cannot load game at path '" .. invalid_game_path .. "'.\nMake sure a folder exists at the specified path.")
	end
end

local xpcall = xpcall
local coroutine_yield = coroutine.yield
local print, debug, tostring = print, debug, tostring

local function error_printer(msg, layer)
	print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

return function()
	local func
	local setModalDrawFunc = false
	local inerror = false

	local function deferErrhand(...)
		local errhand = love.errorhandler or love.errhand
		local handler = (not inerror and errhand) or error_printer
		inerror = true
		func = handler(...)
		inerror = false
	end

	local function earlyinit()
		func = nil

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
			setModalDrawFunc = true
		elseif inerror then -- Error in error handler
			print("Error: " .. tostring(main))
		end
	end

	func = earlyinit
	local prevFunc = nil

	while func do
		if setModalDrawFunc and love.event and func ~= prevFunc then
			prevFunc = func
			love.event._setDefaultModalDrawCallback(func)
		end
		local _, retval, restartvalue = xpcall(func, deferErrhand)
		if retval then return retval, restartvalue end
		coroutine_yield()
	end

	return 1
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
