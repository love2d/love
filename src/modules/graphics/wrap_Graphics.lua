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

function love.graphics.newVideo(file, settings)
	settings = settings == nil and {} or settings
	if type(settings) ~= "table" then error("bad argument #2 to newVideo (expected table)", 2) end

	local video = love.graphics._newVideo(file, settings.dpiscale)
	local source, success

	if settings.audio ~= false and love.audio then
		success, source = pcall(love.audio.newSource, video:getStream():getFilename(), "stream")
	end
	if success then
		video:setSource(source)
	elseif settings.audio == true then
		if love.audio then
			error("Video had no audio track", 2)
		else
			error("love.audio was not loaded", 2)
		end
	else
		video:getStream():setSync()
	end

	return video
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
