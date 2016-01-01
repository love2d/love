R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2016 LOVE Development Team

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

local Video_mt = ...
local Video = Video_mt.__index

function Video:setSource(source)
	self:_setSource(source)
	self:getStream():setSync(source)
end

function Video:play()
	return self:getStream():play()
end

function Video:pause()
	return self:getStream():pause()
end

function Video:seek(offset)
	return self:getStream():seek(offset)
end

function Video:rewind()
	return self:getStream():rewind()
end

function Video:tell()
	return self:getStream():tell()
end

function Video:isPlaying()
	return self:getStream():isPlaying()
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
