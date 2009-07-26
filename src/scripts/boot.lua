
-- Make sure love table exists.
if not love then love = {} end

-- Used for setup:
love.path = {}

function love.insmod(name, provides)
	if love.__mod[provides] and love.__mod[provides][name] then
		love.__mod[provides][name].open()
		print("Opened " .. provides .. " module " .. name .. ".")
	end
end

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

end

-- Standard callback handlers.
love.handlers = {
	[love.event_keypressed] = function (b, u)
		if love.keypressed then love.keypressed(b, u) end
	end,
	[love.event_keyreleased] = function (b)
		if love.keyreleased then love.keyreleased(b) end
	end,
	[love.event_mousepressed] = function (x,y,b)
		if love.mousepressed then love.mousepressed(x,y,b) end
	end,
	[love.event_mousereleased] = function (x,y,b)
		if love.mousereleased then love.mousereleased(x,y,b) end
	end,
	[love.event_joystickpressed] = function (j,b)
		if love.joystickpressed then love.joystickpressed(j,b) end
	end,
	[love.event_joystickreleased] = function (j,b)
		if love.joystickreleased then love.joystickreleased(j,b) end
	end,
	[love.event_quit] = function ()
		return
	end,
}

function love.init()

	if love.__args then
		for i,v in pairs(love.__args) do
			print(i,v)
		end
	end

	love.filesystem = require("love.filesystem")
	love.event = require("love.event")
	love.keyboard = require("love.keyboard")
	love.mouse = require("love.mouse")
	love.timer = require("love.timer")
	love.joystick = require("love.joystick")
	love.image = require("love.image")
	love.graphics = require("love.graphics")
	love.audio = require("love.audio")
	love.physics = require("love.physics")
	love.sound = require("love.sound")
	love.native = require("love.native")

	if love.__args[1] and love.__args[1] ~= "" then
		love.filesystem.setIdentity("love2")
		love.filesystem.setSource(love.path.getfull(love.__args[1]))
		require("main.lua")
	end

	love.run()
end

function love.run()

	-- CONFIG BEGINS

	if love.graphics.checkMode(800, 600, false) then
		love.graphics.setMode(800, 600, false, false)
	end

	-- CONFIG ENDS

	if love.load then love.load() end

	-- Main loop time.
	while true do
		love.timer.step()
		if love.update then love.update(love.timer.getDelta()) end
		love.graphics.clear()
		if love.draw then love.draw() end

		-- Process events.
		for e,a,b,c in love.event.poll() do
			if e == love.event_quit then return end
			love.handlers[e](a,b,c)
		end

		--love.timer.sleep(10)
		love.graphics.present()

	end

end

-----------------------------------------------------------
-- Default screen.
-----------------------------------------------------------

function love.defaultscreen()

	-- Main loop goes here.

end

-----------------------------------------------------------
-- Error screen.
-----------------------------------------------------------

function love.errorscreen()
	
	-- Main loop goes here.

end

-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

local result = xpcall(love.init,
	function (msg)
		print(msg, debug.traceback())
	end)


print("Done.")