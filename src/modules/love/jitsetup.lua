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

if type(jit) ~= "table" or not jit.status() then
	return
end

-- Double the defaults.
jit.opt.start("maxtrace=2000", "maxrecord=8000")

-- Somewhat arbitrary value. Needs to be higher than the combined sizes below,
-- and higher than the default (512) because that's already too low.
jit.opt.start("maxmcode=16384")

if jit.arch == "arm64" then
	-- https://github.com/LuaJIT/LuaJIT/issues/285
	-- LuaJIT 2.1 on arm64 currently (as of commit b4b2dce) can only use memory
	-- for JIT compilation within a certain short range. Other libraries such as
	-- SDL can take all the usable space in that range and cause attempts at JIT
	-- compilation to both fail and take a long time.
	-- This is a very hacky attempt at a workaround. LuaJIT allocates executable
	-- code in pools. We'll try "reserving" pools before any external code is
	-- executed, by causing JIT compilation via a small loop. We can't easily
	-- tell if JIT compilation succeeded, so we do several successively smaller
	-- pool allocations in case previous ones fail.
	-- This is a really hacky hack and by no means foolproof - there are a lot of
	-- potential situations (especially when threads are used) where previously
	-- executed external code will still take up space that LuaJIT needed for itself.

	jit.opt.start("sizemcode=2048")
	for i=1, 100 do end
	
	jit.opt.start("sizemcode=1024")
	for i=1, 100 do end
	
	jit.opt.start("sizemcode=512")
	for i=1, 100 do end
	
	jit.opt.start("sizemcode=256")
	for i=1, 100 do end
	
	jit.opt.start("sizemcode=128")
	for i=1, 100 do end
else
	-- Somewhat arbitrary value (>= the default).
	jit.opt.start("sizemcode=128")
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
