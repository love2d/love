
-- Make sure love table exists.
if not love then love = {} end

-- Used for setup:
love.path = {}

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

-- This can't be overriden. 
function love.boot()

	print("boot")

	-- This is absolutely needed. 
	require("love.filesystem")

	-- Prints the arguments passes to the app.
	if love.__args then
		for i,v in pairs(love.__args) do
			print(i,v)
		end
	end	
	
	-- Sets the source for the game.
	if love.__args[1] and love.__args[1] ~= "" then
		love.filesystem.setSource(love.path.getfull(love.__args[1]))
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
			native = true,
		},
	}

	-- If config file exists, load it and allow it to update config table.
	if love.filesystem.exists("conf.lua") then
		require("conf.lua")
		love.conf(c)
	end
	
	-- Gets desired modules.
	for k,v in pairs(c.modules) do
		if v then
			require("love." .. k)
		end
	end
	
	
	-- Setup screen here.
	if c.screen and c.modules.graphics then 
		if love.graphics.checkMode(c.screen.width, c.screen.height, c.screen.fullscreen) then
			love.graphics.setMode(c.screen.width, c.screen.height, c.screen.fullscreen, c.screen.vsync, c.screen.fsaa)
		end
		love.graphics.setCaption(c.title)
	end
	
	if love.filesystem.exists("main.lua") then require("main.lua") end
	
end

function love.run()

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

function error_printer(msg)
	print("boot", msg, debug.traceback())
end

result = xpcall(love.boot, error_printer)
result = xpcall(love.init, error_printer)
result = xpcall(love.run, error_printer)

print("Done.")