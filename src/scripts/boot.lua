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
		textedit = function (t,s,l)
			if love.textedit then return love.textedit(t,s,l) end
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
		touchpressed = function (id,x,y,dx,dy)
			if love.touchpressed then return love.touchpressed(id,x,y,dx,dy) end
		end,
		touchreleased = function (id,x,y,dx,dy)
			if love.touchreleased then return love.touchreleased(id,x,y,dx,dy) end
		end,
		touchmoved = function (id,x,y,dx,dy)
			if love.touchmoved then return love.touchmoved(id,x,y,dx,dy) end
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

	-- Is this one of those fancy "fused" games?
	local can_has_game = pcall(love.filesystem.setSource, arg0)
	local is_fused_game = can_has_game
	if love.arg.options.fused.set then
		is_fused_game = true
	end

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
		identity = love.path.leaf(arg0)
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
		love.nogame()
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
			srgb = false,
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
		},
		console = false, -- Only relevant for windows.
		identity = false,
		appendidentity = false,
		accelerometerjoystick = true, -- Only relevant for Android / iOS.
	}

	-- Console hack, part 1.
	local openedconsole = false
	if love.arg.options.console.set and love._openConsole then
		love._openConsole()
		openedconsole = true
	end

	-- If config file exists, load it and allow it to update config table.
	if not love.conf and love.filesystem and love.filesystem.isFile("conf.lua") then
		require("conf")
	end

	-- Yes, conf.lua might not exist, but there are other ways of making
	-- love.conf appear, so we should check for it anyway.
	local confok, conferr
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
			srgb = c.window.srgb,
			x = c.window.x,
			y = c.window.y,
		}), "Could not set window mode")
		love.window.setTitle(c.window.title or c.title)
		if c.window.icon then
			assert(love.image, "If an icon is set in love.conf, love.image has to be loaded!")
			love.window.setIcon(love.image.newImageData(c.window.icon))
		end
	end

	-- Our first timestep, because window creation can take some time
	if love.timer then
		love.timer.step()
	end

	if love.filesystem then
		love.filesystem.setIdentity(c.identity or love.filesystem.getIdentity(), c.appendidentity)
		if love.filesystem.isFile("main.lua") then
			require("main")
		end
	end

	if no_game_code then
		error("No code to run\nYour game might be packaged incorrectly\nMake sure main.lua is at the top level of the zip")
	end

	-- Check the version
	c.version = tostring(c.version)
	local compat = love.isVersionCompatible(c.version)
	if not compat then
		local major, minor, revision = c.version:match("^(%d+)%.(%d+)%.(%d+)$")
		if (not major or not minor or not revision) or (major ~= love._version_major and minor ~= love._version_minor) then
			local msg = "This game was made for a different version of LÖVE.\n"..
				"It may not be not be compatible with the running version ("..love._version..")."

			print(msg)

			if love.window then
				love.window.showMessageBox("Compatibility Warning", msg, "warning")
			end
		end
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
			for name, a,b,c,d,e in love.event.poll() do
				if name == "quit" then
					if not love.quit or not love.quit() then
						return
					end
				end
				love.handlers[name](a,b,c,d,e)
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
-- Default screen.
-----------------------------------------------------------

function love.nogame()

	local baby_png =
	"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\
	AElFTkSuQmCC\
	"
	local background_png =
	"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\
	"
	local bubble_png =
	"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\
	"
	local inspector_png =
	"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\
	CPaFrsEAAAAASUVORK5CYII=\
	"
	local text_png =
	"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\
	"

	local function loadimage(file, name)
		return love.graphics.newImage(love.image.newImageData(love.filesystem.newFileData(file, name:gsub("_", "."), "base64")))
	end

	local inspector = {}
	local background = {}
	local bubble = {}
	local text = {}
	local rain = {}
	local g_time = 0
	
	local create_rain

	function love.load()
		love.graphics.setBackgroundColor(137, 194, 218)

		local win_w = love.graphics.getWidth()
		local win_h = love.graphics.getHeight()

		inspector.image = loadimage(inspector_png, "inspector.png")
		inspector.img_w = inspector.image:getWidth()
		inspector.img_h = inspector.image:getHeight()
		inspector.x = win_w * 0.45
		inspector.y = win_h * 0.55

		background.image = loadimage(background_png, "background.png")
		background.img_w = background.image:getWidth()
		background.img_h = background.image:getHeight()
		background.x = 0
		background.y = 0

		bubble.image = loadimage(bubble_png, "bubble.png")
		bubble.img_w = bubble.image:getWidth()
		bubble.img_h = bubble.image:getHeight()
		bubble.x = 140
		bubble.y = -80

		text.image = loadimage(text_png, "text.png")
		text.x = 25
		text.y = 9

		-- Baby Rain
		rain.spacing_x = 110
		rain.spacing_y = 80
		rain.image = loadimage(baby_png, "baby.png")
		rain.img_w = rain.image:getWidth()
		rain.img_h = rain.image:getHeight()
		rain.ox = -rain.img_w / 2
		rain.oy = -rain.img_h / 2
		rain.batch = love.graphics.newSpriteBatch(rain.image, 512)
		rain.t = 0

		create_rain()
	end
	
	function create_rain()
		local batch = rain.batch

		local sx = rain.spacing_x
		local sy = rain.spacing_y
		local ox = rain.ox
		local oy = rain.oy

		local m = 1 / love.window.getPixelScale()
		local batch_w = 2 * math.ceil(m * love.graphics.getWidth() / sx) + 2
		local batch_h = 2 * math.ceil(m * love.graphics.getHeight() / sy) + 2

		batch:clear()

		if batch:getBufferSize() < batch_w * batch_h then
			batch:setBufferSize(batch_w * batch_h)
		end

		for i = 0, batch_h - 1 do
			for j = 0, batch_w - 1 do
				local is_even = (j % 2) == 0
				local offset_y = is_even and 0 or sy / 2
				local x = ox + j * sx
				local y = oy + i * sy + offset_y
				batch:add(x, y)
			end
		end

		batch:flush()
	end

	local function update_rain(t)		
		rain.t = t
	end

	function love.resize(w, h)
		create_rain()
	end

	function love.update(dt)
		g_time = g_time + dt / 2
		local int, frac = math.modf(g_time)
		update_rain(frac)
		inspector.x = love.window.fromPixels(love.graphics.getWidth() * 0.45)
		inspector.y = love.window.fromPixels(love.graphics.getHeight() * 0.55)
	end

	local function draw_grid()
		local y = rain.spacing_y * rain.t

		local small_y = -rain.spacing_y + y / 2
		local big_y = -rain.spacing_y + y

		love.graphics.setBlendMode("subtract")
		love.graphics.setColor(255, 255, 255, 128)
		love.graphics.draw(rain.batch, -rain.spacing_x, small_y, 0, 0.5, 0.5)

		love.graphics.setBlendMode("alpha")
		love.graphics.setColor(255, 255, 255, 255)
		love.graphics.draw(rain.batch, -rain.spacing_x, big_y)
	end

	local function draw_text(x, y)
		local int, frac = math.modf(g_time)
		if frac < 0.5 then
			return
		end
		local tx = x + text.x
		local ty = y + text.y
		love.graphics.draw(text.image, tx, ty, 0, 1, 1, 70, 70)
	end

	local function draw_background(x, y)
		local intensity = (math.sin(math.pi * g_time * 2) + 1) / 2

		local bx = x
		local by = y

		love.graphics.setColor(255, 255, 255, 64 + 16*intensity)
		love.graphics.draw(background.image, bx, by, 0, 0.7, 0.7, 256, 256)
		love.graphics.setColor(255, 255, 255, 32 + 16*intensity)
		love.graphics.draw(background.image, bx, by, 0, 0.65, 0.65, 256, 256)
		love.graphics.setBlendMode("add")
		love.graphics.setColor(255, 255, 255, 16 + 16*intensity)
		love.graphics.draw(background.image, bx, by, 0, 0.6, 0.6, 256, 256)
	end

	local function draw_bubble(x, y)
		local osc = 10 * math.sin(math.pi * g_time)
		local bx = x + bubble.x
		local by = y + bubble.y + osc
		love.graphics.draw(bubble.image, bx, by, 0, 1, 1, 70, 70)

		draw_text(bx, by)
	end

	local function draw_inspector()
		local x, y = inspector.x, inspector.y
		local ox, oy = inspector.img_w / 2, inspector.img_h / 2
		draw_background(x, y)
		love.graphics.setColor(255, 255, 255, 255)
		love.graphics.setBlendMode("alpha")
		love.graphics.draw(inspector.image, x, y, 0, 1, 1, ox, oy)
		draw_bubble(x, y)
	end

	function love.draw()
		love.graphics.setColor(255, 255, 255)

		love.graphics.push()
		love.graphics.scale(love.window.getPixelScale())

		draw_grid()
		draw_inspector()

		love.graphics.pop()
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
		t.window.resizable = true
		t.window.highdpi = true
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

	if not love.graphics.isCreated() or not love.window.isCreated() then
		local success, status = pcall(love.window.setMode, 800, 600)
		if not success or not status then
			return
		end
	end

	-- Reset state.
	if love.mouse then
		love.mouse.setVisible(true)
		love.mouse.setGrabbed(false)
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

	local sRGB = select(3, love.window.getMode()).srgb
	if sRGB and love.math then
		love.graphics.setBackgroundColor(love.math.gammaToLinear(89, 157, 220))
	else
		love.graphics.setBackgroundColor(89, 157, 220)
	end

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
			end
			if e == "keypressed" and a == "escape" then
				return
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
