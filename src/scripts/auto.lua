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
	local src_file = assert(io.open(src, "rb"))
	local src_data = src_file:read("*a")
	-- remove comments
	src_data = src_data:gsub("%-%-%[%[.-%-%-%]%]", ""):gsub("%-%-.-\n", "")
	-- remove unneeded whitespaces
	src_data = src_data:gsub("^%s+",""):gsub("%s*$",""):gsub("%s%s+", " ")
	local src_len = #src_data
	src_file:close()

	src_data = string.format("const std::string %s = %q;", cpp_name, src_data)

	-- Overwrite the old file only if they are different.
	local dst_file = assert(io.open(dst, "r"))
	dst_data = dst_file:read("*a")
	dst_file:close()

	local header, content, footer = dst_data:match('^(.-)(const.-%b"";)(.-)$')
	assert(header and content and footer, "error parsing header/content/footer")

	if content == src_data then
		print(name .. ": no change")
	else
		local dst_file = assert(io.open(dst, "wb"))
		dst_file:write(header)
		dst_file:write(src_data)
		dst_file:write(footer)
		dst_file:close()

		print(name .. ": updated")
	end

end

if #arg == 0 then
	print("Usage: lua auto.lua <name1> <name2> .. <name3>")
else
	for i, v in ipairs(arg) do
		local ok, err = pcall(auto, v:gsub("%.lua$",""))
		if not ok then
			print("cannot update "..v..": " .. err)
		end
	end
end

