R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2015 LOVE Development Team

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

local event, listeners = ...

local type, select, error = type, select, error
local pairs, ipairs = pairs, ipairs
local table_insert, table_remove = table.insert, table.remove
local max = math.max

local function addListenerForEvent(event, func)
	listeners[event] = listeners[event] or {}

	-- Don't add a function to the event's array more than once.
	for i,v in ipairs(listeners[event]) do
		if v == func then
			return
		end
	end

	table_insert(listeners[event], func)
end

function event.addListener(func, ...)
	if type(func) ~= "function" then
		error("Invalid argument #1 to addListener (expected function)", 2)
	end

	local t = (...)
	if type(t) == "table" then
		for i, name in ipairs(t) do
			if type(name) ~= "string" then
				error("Invalid table element #"..i.." in argument #2 to love.event.addListener (expected string)", 2)
			end

			addListenerForEvent(name, func)
		end
	else
		local n = select("#", ...)
		for i = 1, max(n, 1) do
			local name = select(i, ...)

			if type(name) ~= "string" then
				error("Invalid argument #"..(i+1).." to addListener (expected string)", 2)
			end

			addListenerForEvent(name, func)
		end
	end

	return func
end

function event.removeListener(func)
	if type(func) ~= "function" then
		error("Invalid argument #1 to removeListener (expected function)", 2)
	end

	local success = false

	-- Remove the listener function from all event names it was subscribed to.
	for eventname, eventlisteners in pairs(listeners) do
		for i=#eventlisteners, 1, -1 do
			if eventlisteners[i] == func then
				table_remove(eventlisteners, i)
				success = true
				break
			end
		end
	end

	return success
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
