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

local table_concat = table.concat
local ipairs = ipairs
local pcall = pcall
local graphics = love.graphics

function graphics.newVideo(file, settings)
	settings = settings == nil and {} or settings
	if type(settings) ~= "table" then error("bad argument #2 to newVideo (expected table)", 2) end

	local video = love.graphics._newVideo(file, settings.dpiscale)
	local source, success

	if settings.audio ~= false and love.audio then
		success, source = pcall(love.audio.newSource, video:getStream():getFilename(), "stream", "file")
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

function graphics.stencil(func, action, value, keepvalues)
	love.markDeprecated(2, "love.graphics.stencil", "function", "replaced", "love.graphics.setStencilMode or setStencilState")

	if not keepvalues then
		graphics.clear(false, true, false)
	end

	if value == nil then value = 1 end

	local action2, mode2, value2, readmask2, writemask2 = graphics.getStencilState()
	local mr, mg, mb, ma = graphics.getColorMask()

	graphics.setStencilState(action, "always", value)
	graphics.setColorMask(false)

	local success, err = pcall(func)

	graphics.setStencilState(action2, mode2, value2, readmask2, writemask2)
	graphics.setColorMask(mr, mg, mb, ma)

	if not success then
		error(err, 2)
	end
end

function graphics.setStencilTest(mode, value)
	love.markDeprecated(2, "love.graphics.setStencilTest", "function", "replaced", "love.graphics.setStencilMode or setStencilState")

	if mode ~= nil then
		graphics.setStencilState("keep", mode, value)
	else
		graphics.setStencilState()
	end
end

function graphics.getStencilTest()
	love.markDeprecated(2, "love.graphics.getStencilTest", "function", "replaced", "love.graphics.getStencilMode or getStencilState")

	local action, mode, value = graphics.getStencilState()
	return mode, value
end

function graphics._transformGLSLErrorMessages(message)
	local shadertype = message:match("Cannot compile (%a+) shader code")
	local compiling = shadertype ~= nil
	if not shadertype then
		shadertype = message:match("Error validating (%a+) shader")
	end
	if not shadertype then return message end
	local lines = {}
	local prefix = compiling and "Cannot compile " or "Error validating "
	lines[#lines+1] = prefix..shadertype.." shader code:"
	for l in message:gmatch("[^\n]+") do
		-- nvidia: 0(<linenumber>) : error/warning [NUMBER]: <error message>
		local linenumber, what, message = l:match("^0%((%d+)%)%s*:%s*(%w+)[^:]+:%s*(.+)$")
		if not linenumber then
			-- AMD: ERROR 0:<linenumber>: error/warning(#[NUMBER]) [ERRORNAME]: <errormessage>
			linenumber, what, message = l:match("^%w+: 0:(%d+):%s*(%w+)%([^%)]+%)%s*(.+)$")
		end
		if not linenumber then
			-- macOS (?): ERROR: 0:<linenumber>: <errormessage>
			what, linenumber, message = l:match("^(%w+): %d+:(%d+): (.+)$")
		end
		if not linenumber and l:match("^ERROR:") then
			what = l
		end
		if linenumber and what and message then
			lines[#lines+1] = ("Line %d: %s: %s"):format(linenumber, what, message)
		elseif what then
			lines[#lines+1] = what
		end
	end
	-- did not match any known error messages
	if #lines == 1 then return message end
	return table_concat(lines, "\n")
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
