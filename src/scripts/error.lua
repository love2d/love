---------------------------------------------
-- LOVE Error handling code.
---------------------------------------------

-- Colors. (Only one for now)
c = 
{
	{ name = "#0072ff", r = 0, g = 114, b = 255 }
}

-- This will be displayed at any time.
display =
{
	message = "",
	title = "ERROR"
}

-- Helper variables.
ww, wh = love.graphics.getWidth(), love.graphics.getHeight()


-- This saves the graphics state when entering 
-- error mode.
function save_state()
	-- Save the graphics state.
	state = {}
	state.color = love.graphics.getColor()
	state.background_color = love.graphics.getBackgroundColor()
	state.font = love.graphics.getFont()
	state.color_mode = love.graphics.getColorMode()
	state.blend_mode = love.graphics.getBlendMode()
	state.scissor = { love.graphics.getScissor() }

	-- Set new values.
	love.graphics.setFont(font)
	love.graphics.setColor(255, 255, 255)
	love.graphics.setBackgroundColor(c[1].r, c[1].g, c[1].b)
	love.graphics.setColorMode(love.color_normal)
	love.graphics.setBlendMode(love.blend_normal)
	love.graphics.setScissor()
end

-- This restores the graphics state when 
-- exiting error mode.
function restore_state()
	if state then
		if state.color then love.graphics.setColor(state.color) end
		if state.background_color then love.graphics.setBackgroundColor(state.background_color) end
		if state.font then love.graphics.setFont(state.font) end
		if state.color_mode then love.graphics.setColorMode(state.color_mode) end
		if state.blend_mode then love.graphics.setBlendMode(state.blend_mode) end
		if state.scissor[1] then love.graphics.setScissor(state.scissor[1], state.scissor[2], state.scissor[3], state.scissor[4]) end
	end
end

function load()

	-- Create buttons.
	b = 
	{
		{ 
			x = 20, y = wh-42, 
			w = 150, h = 24,
			label = "(Q)uit", 
			key = love.key_q,
			action = love.system.exit, 
			hover = false
		}, 
		{ 
			x = 171, y = wh-42, 
			w = 150, h = 24,
			label = "(R)estart",
			key = love.key_r, 
			action = game_restart, 
			hover = false
		}, 
		{ 
			x = 322, y = wh-42, 
			w = 150, h = 24,
			label = "(C)ontinue", 
			key = love.key_c, 
			action = game_continue, 
			hover = false
		}
	}
	font = love.graphics.newFont(love.default_font, 14)
	logo = love.graphics.newImage(love.default_logo_128)
	logo:setCenter(128, 64)
end

function update(dt)
	local mx, my = love.mouse.getX(), love.mouse.getY()
	for i=1,#b do
		if mx > b[i].x and mx < b[i].x+b[i].w and my > b[i].y and my < b[i].y+b[i].h and not love.mouse.isDown(love.mouse_left) then
			b[i].hover = true
		else
			b[i].hover = false
		end
	end
end

function message(msg, tag)

	if tag == love.tag_command and msg == "savestate" then
		save_state()
	end
	
	if tag == love.tag_incompatible then
		display.title = "Incompatible game!"
		display.message = msg	
	end

	if tag == love.tag_error then
		display.title = "ERROR!"
		msg = string.gsub(msg, "stack traceback", "\nStack traceback");
		msg = string.gsub(msg, "%[string \"", "");
		msg = string.gsub(msg, "\"%]:", ":");
		msg = string.gsub(msg, "\t", "   ");
		display.message = msg	
	end
	
	if tag == love.tag_warning then
		display.title = "Warning!"
		display.message = msg
	end
end

function draw()
	love.graphics.setBackgroundColor(c[1].r, c[1].g, c[1].b)
	love.graphics.setColor(255, 255, 255)
	love.graphics.draw(display.title, 50, 50)
	love.graphics.drawf(display.message, 50, 100, ww-100)
	love.graphics.draw( logo, love.graphics.getWidth(), love.graphics.getHeight())
	
	-- Draw buttons.
	for i=1,#b do
		if not b[i].hover then love.graphics.setColor(255, 255, 255, 128) else love.graphics.setColor(255, 255, 255, 200) end
		love.graphics.quad(love.draw_fill, 
			b[i].x, b[i].y, 
			b[i].x, b[i].y+b[i].h, 
			b[i].x+b[i].w, b[i].y+b[i].h, 
			b[i].x+b[i].w, b[i].y)
		if not b[i].hover then love.graphics.setColor(255, 255, 255) else love.graphics.setColor(0, 114, 255) end
		love.graphics.draw(b[i].label, b[i].x + b[i].w/2 - font:getWidth(b[i].label)/2, b[i].y + b[i].h/2 + 5)
	end
end

function keypressed(key)
	for i=1,#b do
		if b[i].key == key then
			b[i].action()
		end
	end
end

function mousereleased(x, y, button)
	for i=1,#b do
		if button == love.mouse_left and x > b[i].x and x < b[i].x+b[i].w and y > b[i].y and y < b[i].y+b[i].h then
			b[i].action()
		end
	end
end

function game_continue()
	restore_state()
	love.system.resume()
	display.message = ""
end
	
function game_restart()
	restore_state()
	love.system.resume()
	love.system.restart() -- Main game is current.
	display.message = ""
end