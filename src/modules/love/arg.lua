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

local love = require("love")

-- Used for setup:
love.path = {}
love.arg = {}

-- Replace any \ with /.
function love.path.normalslashes(p)
	return p:gsub("\\", "/")
end

-- Makes sure there is a slash at the end
-- of a path.
function love.path.endslash(p)
	if p:sub(-1) ~= "/" then
		return p .. "/"
	else
		return p
	end
end

-- Checks whether a path is absolute or not.
function love.path.abs(p)

	local tmp = love.path.normalslashes(p)

	-- Path is absolute if it starts with a "/".
	if tmp:find("/") == 1 then
		return true
	end

	-- Path is absolute if it starts with a
	-- letter followed by a colon.
	if tmp:find("%a:") == 1 then
		return true
	end

	-- Relative.
	return false

end

-- Converts any path into a full path.
function love.path.getFull(p)

	if love.path.abs(p) then
		return love.path.normalslashes(p)
	end

	local cwd = love.filesystem.getWorkingDirectory()
	cwd = love.path.normalslashes(cwd)
	cwd = love.path.endslash(cwd)

	-- Construct a full path.
	local full = cwd .. love.path.normalslashes(p)

	-- Remove trailing /., if applicable
	return full:match("(.-)/%.$") or full
end

-- Returns the leaf of a full path.
function love.path.leaf(p)
	p = love.path.normalslashes(p)

	local a = 1
	local last = p

	while a do
		a = p:find("/", a+1)

		if a then
			last = p:sub(a+1)
		end
	end

	return last
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
	return a[m], m
end

love.arg.options = {
	console = { a = 0 },
	fused = {a = 0 },
	game = { a = 1 }
}

love.arg.optionIndices = {}

function love.arg.parseOption(m, i)
	m.set = true

	if m.a > 0 then
		m.arg = {}
		for j=i,i+m.a-1 do
			love.arg.optionIndices[j] = true
			table.insert(m.arg, arg[j])
		end
	end

	return m.a
end

function love.arg.parseOptions()

	local game
	local argc = #arg

	local i = 1
	while i <= argc do
		-- Look for options.
		local m = arg[i]:match("^%-%-(.*)")

		if m and m ~= "" and love.arg.options[m] and not love.arg.options[m].set then
			love.arg.optionIndices[i] = true
			i = i + love.arg.parseOption(love.arg.options[m], i+1)
		elseif m == "" then -- handle '--' as an option
			love.arg.optionIndices[i] = true
			if not game then -- handle '--' followed by game name
				game = i + 1
			end
			break
		elseif not game then
			game = i
		end
		i = i + 1
	end

	if not love.arg.options.game.set then
		love.arg.parseOption(love.arg.options.game, game or 0)
	end
end

-- Returns the arguments that are passed to your game via love.load()
-- arguments that were parsed as options are skipped.
function love.arg.parseGameArguments(a)
	local out = {}

	local _, lowindex = love.arg.getLow(a)

	local o = lowindex
	for i=lowindex, #a do
		if not love.arg.optionIndices[i] then
			out[o] = a[i]
			o = o + 1
		end
	end

	return out
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
