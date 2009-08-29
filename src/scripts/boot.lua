
-- Make sure love table exists.
if not love then love = {} end

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
	return cwd .. love.path.normalslashes(p)

end

-- Returns the leaf of a full path.
function love.path.leaf(p)

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

function love.createhandlers()

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

end

-- This can't be overriden. 
function love.boot()

	-- This is absolutely needed.
	require("love")
	require("love.filesystem")

	if arg and arg[1] then
		love.filesystem.init(love.path.getfull(love.arg.getLow(arg)))
		if not pcall(love.filesystem.setSource, love.path.getfull(arg[1])) then
			love.nogame()
		end
	else
		love.filesystem = nil
		love.nogame() -- Who needs a game? Got one embedded right here!
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
			font = true,
			signal = true,
		},
	}

	-- If config file exists, load it and allow it to update config table.
	if not love.conf and love.filesystem and love.filesystem.exists("conf.lua") then
		require("conf.lua")
	end
	
	-- Yes, conf.lua might not exist, but there are other ways of making
	-- love.conf appear, so we should check for it anyway.
	if love.conf then
		love.conf(c)
	end
	
	-- Gets desired modules.
	for k,v in pairs(c.modules) do
		if v then
			require("love." .. k)
		end
	end
	
	if love.keyboard then
		love.createhandlers()
	end
	
	-- Setup screen here.
	if c.screen and c.modules.graphics then 
		if love.graphics.checkMode(c.screen.width, c.screen.height, c.screen.fullscreen) then
			love.graphics.setMode(c.screen.width, c.screen.height, c.screen.fullscreen, c.screen.vsync, c.screen.fsaa)
		end
		love.graphics.setCaption(c.title)
	end
	
	if love.filesystem and love.filesystem.exists("main.lua") then require("main.lua") end
	
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

function love.nogame()

	love.load = function()

		love.graphics.setBackgroundColor(0x84, 0xca, 0xff)
		
		names = {
			"wheel_major",
			"wheel_minor",
			"wheel_revision",
			"belt_tooth",
			"belt_track",
			"turret_body",
			"turret_cannon",
			"star",
			"knoll01",
			"knoll02",
			"knoll03",
			"knoll04",
			"tree01",
			"bubble",
			"love",
		}
		
		local decode = function(file)
			return love.graphics.newImage(love.image.newImageData(file))
		end
		
		images = {}
		
		for i,v in pairs(names) do
			images[v] = decode(love["_"..v.."_png"])
		end

		pools = {
			{
				images.knoll01,
				images.knoll02,
			},
			{
				images.knoll03,
				images.knoll04,
			},
		}

		List = {}
		List.__index = List

		List.new = function(self)
			local o = {
				head = nil,
			}
			setmetatable(o, List)
			return o
		end

		List.update = function(self, dt)
			local n = self.head
			while n do
				n:update(dt)
				n = n.next
			end
		end

		List.draw = function(self)
			local n = self.head
			while n do
				n:draw()
				n = n.next
			end
		end

		Node = {}
		Node.__index = Node

		Node.new = function(self, object)
			local o = {
				next = nil, 
			}
			setmetatable(o, List)
			return o
		end

		Node.insert = function(self, list)
			local h = list.head
			list.head = self
			self.next = h
		end

		Node.remove = function(self)
			parent.next = self.next
		end

		Object = Node:new()
		Object.__index = Object
		setmetatable(Object, Node)

		Object.new = function(self)
			local o = {
				image = nil,
				x = 0, 
				y = 0,
				dx = -400,
				dy = 0,
				scale = 1,
				r = 0,
				duration = 30,
				passed = 0,
				t = 0,
				alpha = 255
			}
			setmetatable(o, Object)
			return o
		end

		Object.update = function(self, dt)
			self.passed = self.passed + dt
			while self.passed > self.duration do
				self.passed = self.passed - self.duration
			end
			self.t = self.passed/self.duration
		end

		Object.draw = function(self)
			if self.image then
				local x = self.x + self.dx*self.t
				local y = self.y + self.dy*self.t
				local r = self.r*self.t
				love.graphics.setColor(255, 255, 255, self.alpha)
				love.graphics.draw(self.image, x, y, r, self.scale)
				love.graphics.setColor(255, 255, 255, 255)
			end
		end

		Tree = Object:new()
		Tree.__index = Tree
		setmetatable(Tree, Object)

		Tree.new = function(self)
			local o = {}
			o.image = images.tree01
			o.x = 800 + math.random(0, 800)
			o.y = 300 + math.random(0, 40)
			o.xt = -200;
			o.dx = o.xt - o.x
			o.speed = 100
			o.duration = -o.dx/o.speed
			setmetatable(o, Tree)
			return o
		end

		Star = Object:new()
		Star.__index = Star
		setmetatable(Star, Object)

		Star.new = function(self, speed, scale)
			local o = {}
			o.image = images.star
			o.x = 800 + math.random(0, 800)
			o.y = -200 + math.random(0, 300)
			o.xt = -50;
			o.dy = 400
			o.dx = o.xt - o.x
			o.speed = speed
			o.scale = scale
			o.duration = -o.dx/o.speed
			o.r = math.pi * 5
			o.alpha = 100 + math.random(155)
			setmetatable(o, Star)
			return o
		end

		Knoll = Object:new()
		Knoll.__index = Knoll
		setmetatable(Knoll, Object)

		Knoll.new = function(self, pool, var, speed)
			local o = {}
			o.image = pools[pool][math.random(1, #pools[pool])]
			o.x = 800 + math.random(0, 800)
			o.y = 300 + var - math.random(0, var*2)
			o.xt = -200;
			o.dx = o.xt - o.x
			o.speed = speed
			o.duration = -o.dx/o.speed
			setmetatable(o, Star)
			return o
		end

		Belt = Object:new()
		Belt.__index = Belt
		setmetatable(Belt, Object)

		Belt.new = function(self, n)

			local o = {}
			
			o.r = 30
			o.d = o.r*2
			o.half_c = math.pi*o.r
			o.c = 2*o.half_c
			o.x = 200
			o.y = 300
			o.th = 1
			o.ta = 1
			o.w = o.th*o.half_c
			o.total = o.th*2+o.ta*2
			o.teeth = {}

			for i=0,n-1 do
				local b = { x = 0, y = 0, t = (o.total/n)*i }
				table.insert(o.teeth, b)
			end
			
			setmetatable(o, Belt)
			return o
		end

		Belt.update = function(self, dt)
			for i,b in ipairs(self.teeth) do
				b.t = b.t + dt
				
				if b.t < self.th then
					local t = b.t
					b.x = self.x + self.w * (t/self.th)
					b.y = self.y
				elseif b.t < self.th + self.ta then 
					local t = (self.th + self.ta - b.t)
					b.x = self.x + self.w + math.cos(-math.pi*t + math.pi/2)*self.r
					b.y = self.y + self.r + math.sin(-math.pi*t + math.pi/2)*self.r
				elseif b.t < self.th*2 + self.ta then
					local t = (b.t - self.th*2 + self.ta)/self.th
					b.x = self.x + self.w * (2-t)
					b.y = self.y + self.d
				elseif b.t < self.total then 
					local t = (self.th*2 + self.ta - b.t)
					b.x = self.x + math.cos(-math.pi*t + math.pi/2)*self.r
					b.y = self.y + self.r + math.sin(-math.pi*t + math.pi/2)*self.r
				else
					b.t = b.t - self.total
				end
			end
		end

		Belt.draw = function(self)
			for i,b in ipairs(self.teeth) do
				love.graphics.draw(images.belt_tooth, b.x, b.y)
			end
		end

		Tank = Object:new()
		Tank.__index = Tank
		setmetatable(Tank, Object)

		Tank.new = function(self)
			local o = {}
			o.x = 200
			o.y = 490
			o.i = 49
			o.belt = Belt:new(30)
			o.belt.x = o.x-7
			o.belt.y = o.y-37
			o.angle = 0
			setmetatable(o, Tank)
			return o
		end

		Tank.update = function(self, dt)
			self.angle = self.angle + dt * math.pi/2
			self.belt:update(dt)
		end


		Tank.draw = function(self)
			love.graphics.draw(images.turret_cannon, self.x+30, self.y-80)
			love.graphics.draw(images.turret_body, self.x-12, self.y-110)
			love.graphics.draw(images.belt_track, self.belt.x-74, self.belt.y-28)
			love.graphics.draw(images.wheel_major, self.x, self.y, self.angle, 1, 1, 32, 32)
			love.graphics.draw(images.wheel_minor, self.x+self.i, self.y, self.angle, 1, 1, 32, 32)
			love.graphics.draw(images.wheel_revision, self.x+self.i*2, self.y, self.angle, 1, 1, 32, 32)
			self.belt:draw()
		end

		Bubble = Object:new()
		Bubble.__index = Bubble
		setmetatable(Bubble, Object)

		 Bubble.new = function(self)
			local o = {}
			o.x = 240
			o.y = 190
			o.angle = 0
			setmetatable(o, Bubble)
			return o
		end

		Bubble.update = function(self, dt)
			self.angle = self.angle + dt*5
		end

		Bubble.draw = function(self)
			local yv = math.sin(self.angle)*5
			love.graphics.draw(images.bubble, self.x, self.y+yv)
			love.graphics.draw(images.love, self.x+8, self.y+yv+95)
		end

		timers = {}

		Timer = {}
		Timer.__index = Timer

		Timer.spawn = function(self, tick, f)
			local o = {
				passed = 0,
				tick = tick,
				f = f
			}
			setmetatable(o, Timer)
			table.insert(timers, o)
		end

		Timer.update = function(self, dt)
			self.passed = self.passed + dt
			while self.passed > self.tick do
				self.passed = self.passed - self.tick
				self.f()
			end
		end

		lists = {
			b = List:new(), 
			f = List:new()
		}
		
		do
			local t = Bubble:new()
			t:insert(lists.f)
		end
		
		do
			local t = Tank:new()
			t:insert(lists.f)
		end

		for i=1,3 do
			local t = Tree:new(50, 300)
			t:insert(lists.b)
		end
		

		for i=1,2 do
			local t = Knoll:new(1, 50, 100)
			t:insert(lists.b)
		end

		for i=1,40 do
			local t = Star:new(100, 1)
			t:insert(lists.b)
		end

		for i=1,5 do
			local t = Knoll:new(2, 100, 50)
			t:insert(lists.b)
		end
		
		for i,v in pairs(lists) do
			v:update(10)
		end
		
	end



	love.update = function(dt)

		for i,v in ipairs(timers) do v:update(dt) end


		for i,v in pairs(lists) do
			v:update(dt)
		end
		
		love.timer.sleep(10)
	end


	love.draw = function()

		lists.b:draw()
		
		-- Ground
		love.graphics.setColor(146, 201, 87)
		love.graphics.rectangle(love.draw_fill, 0, 530, 800, 70)
		love.graphics.setColor(205, 227, 161)
		love.graphics.rectangle(love.draw_fill, 0, 520, 800, 10)
		love.graphics.setColor(255, 255, 255)
		
		lists.f:draw()
		
	end
	
	love.conf = function(t)
		t.title = "*Tank* you for using LOVE " .. love._version_string .. " (" .. love._version_codename .. ")"
		t.modules.audio = false
		t.modules.sound = false
		t.modules.physics = false
		t.modules.joystick = false
		t.modules.native = false
		t.modules.font = false
	end
	
end

-----------------------------------------------------------
-- Error screen.
-----------------------------------------------------------

function love.errhand(msg)

	if not love.graphics or not love.event then
		return error_printer(msg)
	end
	
	-- Load.
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
	
	love.graphics.printf(p, 70, 70, love.graphics.getWidth() - 70)	
	
	love.graphics.present()
		
	while true do
		e, a, b, c = love.event.wait()
		
		if e == love.event_quit then return end
		if e == love.event_keypressed and a == love.key_escape then
			return
		end

	end
	
end


-----------------------------------------------------------
-- The root of all calls.
-----------------------------------------------------------

function error_printer(msg)
	print("boot", msg, debug.traceback())
end

result = xpcall(love.boot, error_printer)
if not result then return end
result = xpcall(love.init, love.errhand)
if not result then return end
result = xpcall(love.run, love.errhand)
if not result then return end

print("Done.")
