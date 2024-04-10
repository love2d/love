function love.conf(t)
  print("love.conf")
  t.console = true
  t.window.name = 'love.test'
  t.window.width = 360
  t.window.height = 240
  t.window.resizable = true
  t.window.depth = true
  t.window.stencil = true
end

-- custom crash message here to catch anything that might occur with modules 
-- loading before we hit main.lua
local function error_printer(msg, layer)
  print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end
function love.errorhandler(msg)
  msg = tostring(msg)
  error_printer(msg, 2)
end
