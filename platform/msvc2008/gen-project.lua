-- Generates some MSVC2008 project files.

require("lfs")

math.randomseed(os.time())

function scan(path)
	local files = {}
	local dirs = {}
	for f in lfs.dir(path) do
		if f ~= "." and f ~= ".."  then
			local a = lfs.attributes(path.."/"..f)
			if a and a.mode == "directory" then
				table.insert(dirs, f)
			else
				table.insert(files, f)
			end
		end
	end
	return files, dirs
end

function ignore(path)
	
	if string.find(path, "libtcc") then
		if string.find(path, "libtcc%.c") then
			return false
		else
			print("NO", path)
			return true
		end
	end
	
	return false
end

function traverse(path, p, t)
	
	if not p then p = path end
	if not t then t = {} end
	
	local f, d = scan(path)
	
	for i,v in ipairs(f) do
		if not ignore(path.."/"..v) then
			table.insert(t, path.."/"..v)
		end
	end
	
	for i,v in ipairs(d) do
		t[v] = traverse(path.."/"..v, v, {})
	end
	
	return t
end

function winslash(s)
	return string.gsub(s, "/", "\\")
end

function addfolder(name, t, out)
	-- Build the data.
	for i,v in pairs(t) do
		if type(v) == "table" then
			table.insert(out, "\n<Filter Name=\""..i.."\">")
			addfolder(i, v, out)
			table.insert(out, "</Filter>\n")
		else
			table.insert(out, "<File RelativePath=\""..winslash(v).."\">")
			if string.find(v, "%.cpp") then
				table.insert(out, "<FileConfiguration Name=\"Debug|Win32\">")
				local u1,u2,obj = string.find(v, "(/modules/.*/)")
				table.insert(out, "<Tool Name=\"VCCLCompilerTool\" ObjectFile=\"$(IntDir)"..winslash(obj).."\" />")
				table.insert(out, "</FileConfiguration>")
				table.insert(out, "<FileConfiguration Name=\"Release|Win32\">")
				table.insert(out, "<Tool Name=\"VCCLCompilerTool\" ObjectFile=\"$(IntDir)"..winslash(obj).."\" />")
				table.insert(out, "</FileConfiguration>")
			end
			table.insert(out, "</File>")
		end
	end
end

function contains(haystack, needle)
	for i,v in ipairs(haystack) do
		if v == needle then return true end
	end
	return false
end

guid = {}
guid.chars = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'}

function guid.gen()
	local g = {"{"}
	for i=1,8 do table.insert(g, guid.chars[math.random(1,16)]) end
	table.insert(g, "-")
	for j=1,3 do
		for i=1,4 do table.insert(g, guid.chars[math.random(1,16)]) end
		table.insert(g, "-")
	end
	for i=1,12 do table.insert(g, guid.chars[math.random(1,16)]) end
	table.insert(g, "}")
	return table.concat(g)
end

filter = {}

function filter.guid(data)
	local g = guid.gen()
	print(" GUID: " .. g)
	return string.gsub(data, "{GUID}", g)
end

function filter.name(data, module)
	print(" Name: " .. module)
	return string.gsub(data, "{MODULE}", module)
end

function filter.includes(data, module)

	local incstr = "include;../../src;../../src/modules;"
	local sdlstr = "include/SDL"

	local sdlmod = {"audio", "event", "graphics", "joystick", "keyboard", "mouse", "timer"}
	
	if contains(sdlmod, module) then
		incstr = incstr .. sdlstr
	end

	print(" Include: " .. incstr)
	return string.gsub(data, "{INCLUDES}", incstr)
end

function filter.defines(data, module)

	local unique = string.upper(module) .."_EXPORTS"
	local release = "WIN32;NDEBUG;_WINDOWS;_USRDLL;".. unique
	local dbg = "WIN32;_DEBUG;_WINDOWS;_USRDLL;".. unique
	
	print(" Defines:")
	print("  Debug: "..dbg)
	print("  Release: "..release)
	
	local data = string.gsub(data, "{DEFINES_RELEASE}", release)
	
	return string.gsub(data, "{DEFINES_DEBUG}", dbg)
end

function filter.libdir(data, module)
	local dir = "lib"
	print(" Libdir: " .. dir)
	return string.gsub(data, "{LIBDIR}", dir)
end

libconf = {
	audio = "lua5.1.lib SDL.lib openal32.lib",
	event = "lua5.1.lib SDL.lib",
	filesystem = "lua5.1.lib physfs.lib",
	font = "lua5.1.lib freetype.lib",
	graphics = "lua5.1.lib SDL.lib freetype.lib opengl32.lib",
	image = "lua5.1.lib DevIL.lib",
	joystick = "lua5.1.lib SDL.lib",
	keyboard = "lua5.1.lib SDL.lib",
	mouse = "lua5.1.lib SDL.lib",
	native = "lua5.1.lib",
	physics = "lua5.1.lib",
	sound = "lua5.1.lib libmodplug.lib libmpg123.lib libogg.lib libvorbis.lib libvorbisfile.lib libFLAC_static.lib libFLAC++_static.lib",
	timer = "lua5.1.lib SDL.lib",
}

function filter.libs(data, module)
	print(" Libs: " .. libconf[module])
	return string.gsub(data, "{LIBS}", libconf[module])
end

filter.filters = {
	filter.name,
	filter.guid,
	filter.includes,
	filter.defines,
	filter.libdir,
	filter.libs,
}

function update(module, t)

	print("Updating "..module.." ...")

	local infile = "master.vct"
	local outfile = module..".vcproj"

	-- Get the template.
	local f = io.open(infile, "r")
	if not f then print("Warning, file "..infile.." not found."); return end
	local data = f:read("*a")
	io.close(f)
	
	for i,v in ipairs(filter.filters) do
		data = v(data, module)
	end
	
	local out = {}
	
	table.insert(out, "<!-- Generated stuff begins -->")
	table.insert(out, "<Files>")
	
	-- Build the data.
	addfolder(module, t.modules[module], out)
	
	table.insert(out, "</Files>")
	table.insert(out, "<!-- Generated stuff ends -->")
	
	-- Insert into file.
	data = string.gsub(data, "{FILES}", table.concat(out, "\n"))
	
	-- Write the file.
	f = io.open(outfile, "w")
	f:write(data)
	io.close(f)
end

t = traverse("../../src")

print("")
print("------------------")
print("")

function printr(t, tab)
	if not tab then tab = 0 end
	for i,v in pairs(t) do
		if type(v) == "table" then
			print(string.rep("\t", tab)..i.." => {")
			printr(v, tab+1)
			print(string.rep("\t", tab).."}")
		else
			print(string.rep("\t", tab)..i.." => " .. v)
		end
	end
end

--printr(t)

local mods = {
	"audio",
	"event",
	"filesystem",
	"font",
	"graphics",
	"image",
	"joystick",
	"keyboard",
	"mouse",
	"native",
	"physics",
	"sound",
	"timer",
}

for i,v in ipairs(mods) do
	update(v, t)
end
