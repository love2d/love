-- Usage:
-- lua auto.lua <name1> <name2> .. <nameN>
--
-- Example:
-- lua auto.lua boot graphics

max_width = 15

function auto(name)

	local src = name .. ".lua"
	local dst = name .. ".lua.h"
	local cpp_name = name .. "_lua"

	-- Read source Lua file
	local src_file = io.open(src, "rb")
	local src_data = src_file:read("*a")
 	local src_len = #src_data
	src_file:close()

	local lines = {}
	local line = {}
	table.insert(lines, line)

	for i=1,src_len do
		table.insert(line, string.byte(src_data, i))
		if math.mod(i, max_width) == 0 then
			line = {}
			table.insert(lines, line)
		end
	end

	local src_output = "const unsigned char "..cpp_name.."[] = \n{\n"

	for i,line in ipairs(lines) do
		local concat = {}
		for j,b in ipairs(line) do
			table.insert(concat, string.format("0x%02X,", b))
		end
		src_output = src_output .. "\t" .. table.concat(concat, "").."\n"
	end

	src_output = src_output .. "};"

	local include = true

	-- Read dst
	local dst_lines = {}
	for line in io.lines(dst) do

		if line == "// ["..src.."]" then
			include = false
			table.insert(dst_lines, line)
			table.insert(dst_lines, src_output)
		end

		if line == "// [/"..src.."]" then
			include = true
		end

		if include then
			table.insert(dst_lines, line)
		end
	end

	local tmp_data = table.concat(dst_lines, "\n")

	-- Overwrite the old file only if they are different.
	local dst_file = io.open(dst, "rb")
	dst_data = dst_file:read("*a")
	dst_file:close()

	if tmp_data == dst_data then
		print(name .. ": no change")
	else

		local dst_file = io.open(dst, "wb")
		dst_file:write(tmp_data)
		dst_file:close()

		print(name .. ": updated")
	end

end

if #arg == 0 then
	print("Usage: lua auto.lua <name1> <name2> .. <name3>")
else
	for i, v in ipairs(arg) do
		auto(v)
	end
end

