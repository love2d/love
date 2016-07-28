-- Usage:
-- lua auto.lua <name1> <name2> .. <nameN>
--
-- Example:
-- lua auto.lua boot graphics

local max_width = 18
local pattern = [[
/**
 * Copyright (c) 2006-2016 LOVE Development Team
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

namespace love
{

// [%s]
const unsigned char %s[] = 
{
%s
}; // [%s]
} // love
]]
--formatting parameters:
-- - input file name
-- - c variable name
-- - array contents
-- - input file name


function auto(name)
	--the input file name
	local src = name .. ".lua"
	--and the output one
	local dst = name .. ".lua.h"
	--the name of the variable
	local cpp_name = name .. "_lua"

	--do a minimal code check
	--(syntax errors, really)
	loadfile(src)
	--no error catching? no
	--we have the main loop doing that for us

	--what character is this on this line?
	local counter = 0

	local function tohex(c)
		counter = counter + 1
		--if we've reached the maximum width (or 0)
		--then we'll carry on and add a newline
		if counter % max_width == 0 then
			return ("\n\t0x%02x, "):format(c:byte())
		end
		--otherwise we just use the hex of the current byte
		return ("0x%02x, "):format(c:byte())
	end

	--let's open the input file
	local src_file = io.open(src, "rb")
	--create an output string
	local out_data = ""
	--go through the input file line-by-line
	for line in src_file:lines() do
		--if the line is non-empty
		if #line > 0 then
			--set the counter to -1
			--this will start a new line (see tohex)
			counter = -1
			--append the output to what we had, plus a newline character (0x0a is newline)
			out_data = ("%s%s0x0a,"):format(out_data, line:gsub("\r", ""):gsub(".", tohex))
		end
	end
	--close our input
	src_file:close()

	--open, write and close the output
	out_file = io.open(dst, "wb")
	--see pattern above
	out_file:write(pattern:format(src, cpp_name, out_data, src))
	out_file:close()

	--tell the world we succeeded!
	print(name .. ": Success")
end

--usage
if #arg == 0 then
	return print("Usage: lua auto.lua <name1> <name2> .. <name3>")
end

--the 'main' procedure
for i, v in ipairs(arg) do
	--run the auto function for every argument
	--but do it with pcall, to catch errors
	local ok, err = true
	v = v:gsub("^scripts/", "")
	if v:match("/") then
		ok, err = false, "not in scripts directory"
	else
		v = v:gsub("%.lua$", "") -- normalize input
		ok, err = pcall(auto, v)
	end
	if not ok then
		--inform people we've failed
		print(v .. ": " .. err)
	end
end

