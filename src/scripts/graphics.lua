--[[
Copyright (c) 2006-2009 LOVE Development Team

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

love.graphics.print = function (...)
	if not love.graphics.getFont() then 
		love.graphics.setFont(love._vera_ttf, 12)
	end
	love.graphics.print1(...)
	love.graphics.print = love.graphics.print1
end

love.graphics.printf = function (...)
	if not love.graphics.getFont() then 
		love.graphics.setFont(love._vera_ttf, 12)
	end
	love.graphics.printf1(...)
	love.graphics.printf = love.graphics.printf1
end

