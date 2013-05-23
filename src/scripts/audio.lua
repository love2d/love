--[[
Copyright (c) 2006-2013 LOVE Development Team

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

function love.audio.newSource(a, b)
	if type(a) == "string" then
		local err
		a, err = love.filesystem.newFileData(a)
		if not a then
			error(err, 2)
		end
	end
	if type(a) == "userdata" then
		if a:typeOf("File") or a:typeOf("FileData") then
			a = love.sound.newDecoder(a)
		end
		
		if a:typeOf("Decoder") then
			if b == "static" then
				a = love.sound.newSoundData(a)
			end
			return love.audio.newSource1(a)
		end
		if a:typeOf("SoundData") then
			return love.audio.newSource1(a)
		end
	end
	error("No matching function overload", 2)
end