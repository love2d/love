function b64(name)
	local i = io.popen("base64 " .. name)
	local encoded = i:read("*a")
	i:close()

	local output = ("local %s =\n%q"):format(name:gsub("%.", "_"), encoded)
	return output
end

for i, v in ipairs(arg) do
	print(b64(v))
end
