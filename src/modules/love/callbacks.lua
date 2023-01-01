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

local love = require("love")

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
		mousepressed = function (x,y,b,t,c)
			if love.mousepressed then return love.mousepressed(x,y,b,t,c) end
		end,
		mousereleased = function (x,y,b,t,c)
			if love.mousereleased then return love.mousereleased(x,y,b,t,c) end
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
			if love.lowmemory then love.lowmemory() end
			collectgarbage()
			collectgarbage()
		end,
		displayrotated = function (display, orient)
			if love.displayrotated then return love.displayrotated(display, orient) end
		end,
	}, {
		__index = function(self, name)
			error("Unknown event: " .. name)
		end,
	})

end

-----------------------------------------------------------
-- Default callbacks.
-----------------------------------------------------------

function love.run()
	if love.load then love.load(love.arg.parseGameArguments(arg), arg) end

	-- We don't want the first frame's dt to include time taken by love.load.
	if love.timer then love.timer.step() end

	local dt = 0

	-- Main loop time.
	return function()
		-- Process events.
		if love.event then
			love.event.pump()
			for name, a,b,c,d,e,f in love.event.poll() do
				if name == "quit" then
					if not love.quit or not love.quit() then
						return a or 0
					end
				end
				love.handlers[name](a,b,c,d,e,f)
			end
		end

		-- Update dt, as we'll be passing it to update
		if love.timer then dt = love.timer.step() end

		-- Call update and draw
		if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled

		if love.graphics and love.graphics.isActive() then
			love.graphics.origin()
			love.graphics.clear(love.graphics.getBackgroundColor())

			if love.draw then love.draw() end

			love.graphics.present()
		end

		if love.timer then love.timer.sleep(0.001) end
	end

end

local debug, print, tostring, error = debug, print, tostring, error

function love.threaderror(t, err)
	error("Thread error ("..tostring(t)..")\n\n"..err, 0)
end

local utf8 = require("utf8")

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
		if love.mouse.isCursorSupported() then
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
	local font = love.graphics.setNewFont(14)

	love.graphics.setColor(1, 1, 1)

	local trace = debug.traceback()

	love.graphics.origin()

	local sanitizedmsg = {}
	for char in msg:gmatch(utf8.charpattern) do
		table.insert(sanitizedmsg, char)
	end
	sanitizedmsg = table.concat(sanitizedmsg)

	local err = {}

	table.insert(err, "Error\n")
	table.insert(err, sanitizedmsg)

	if #sanitizedmsg ~= #msg then
		table.insert(err, "Invalid UTF-8 string in error message.")
	end

	table.insert(err, "\n")

	for l in trace:gmatch("(.-)\n") do
		if not l:match("boot.lua") then
			l = l:gsub("stack traceback:", "Traceback\n")
			table.insert(err, l)
		end
	end

	local p = table.concat(err, "\n")

	p = p:gsub("\t", "")
	p = p:gsub("%[string \"(.-)\"%]", "%1")

	local function draw()
		if not love.graphics.isActive() then return end
		local pos = 70
		love.graphics.clear(89/255, 157/255, 220/255)
		love.graphics.printf(p, pos, pos, love.graphics.getWidth() - pos)
		love.graphics.present()
	end

	local fullErrorText = p
	local function copyToClipboard()
		if not love.system then return end
		love.system.setClipboardText(fullErrorText)
		p = p .. "\nCopied to clipboard!"
	end

	if love.system then
		p = p .. "\n\nPress Ctrl+C or tap to copy this error"
	end

	return function()
		love.event.pump()

		for e, a, b, c in love.event.poll() do
			if e == "quit" then
				return 1
			elseif e == "keypressed" and a == "escape" then
				return 1
			elseif e == "keypressed" and a == "c" and love.keyboard.isDown("lctrl", "rctrl") then
				copyToClipboard()
			elseif e == "touchpressed" then
				local name = love.window.getTitle()
				if #name == 0 or name == "Untitled" then name = "Game" end
				local buttons = {"OK", "Cancel"}
				if love.system then
					buttons[3] = "Copy to clipboard"
				end
				local pressed = love.window.showMessageBox("Quit "..name.."?", "", buttons)
				if pressed == 1 then
					return 1
				elseif pressed == 3 then
					copyToClipboard()
				end
			end
		end

		draw()

		if love.timer then
			love.timer.sleep(0.1)
		end
	end

end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
