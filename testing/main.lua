-- load test objs
require('classes.TestSuite')
require('classes.TestModule')
require('classes.TestMethod')

-- create testsuite obj
love.test = TestSuite:new()

-- load test scripts if module is active
-- this is so in future if we have per-module disabling it'll still run
if love ~= nil then require('tests.love') end
if love.audio ~= nil then require('tests.audio') end
if love.data ~= nil then require('tests.data') end
if love.event ~= nil then require('tests.event') end
if love.filesystem ~= nil then require('tests.filesystem') end
if love.font ~= nil then require('tests.font') end
if love.graphics ~= nil then require('tests.graphics') end
if love.image ~= nil then require('tests.image') end
if love.joystick ~= nil then require('tests.joystick') end
if love.keyboard ~= nil then require('tests.keyboard') end
if love.math ~= nil then require('tests.math') end
if love.mouse ~= nil then require('tests.mouse') end
if love.physics ~= nil then require('tests.physics') end
if love.sensor ~= nil then require('tests.sensor') end
if love.sound ~= nil then require('tests.sound') end
if love.system ~= nil then require('tests.system') end
if love.thread ~= nil then require('tests.thread') end
if love.timer ~= nil then require('tests.timer') end
if love.touch ~= nil then require('tests.touch') end
if love.video ~= nil then require('tests.video') end
if love.window ~= nil then require('tests.window') end

-- love.load
-- load given arguments and run the test suite
love.load = function(args)

  -- setup basic img to display
  if love.window ~= nil then
    love.window.updateMode(360, 240, {
      fullscreen = false,
      resizable = true,
      centered = true
    })

    -- set up some graphics to draw if enabled
    if love.graphics ~= nil then
      love.graphics.setDefaultFilter("nearest", "nearest")
      love.graphics.setLineStyle('rough')
      love.graphics.setLineWidth(1)
      Logo = {
        texture = love.graphics.newImage('resources/love.png'),
        img = nil
      }
      Logo.img = love.graphics.newQuad(0, 0, 64, 64, Logo.texture)
      Font = love.graphics.newFont('resources/font.ttf', 8, 'normal')
      TextCommand = 'Loading...'
      TextRun = ''
    end

  end

  -- mount for output later
  if love.filesystem.mountFullPath then
    love.filesystem.mountFullPath(love.filesystem.getSource() .. "/output", "tempoutput", "readwrite")
  end

  -- get all args with any comma lists split out as seperate
  local arglist = {}
  for a=1,#args do
    local splits = UtilStringSplit(args[a], '([^,]+)')
    for s=1,#splits do
      table.insert(arglist, splits[s])
    end
  end

  -- convert args to the cmd to run, modules, method (if any) and disabled
  local testcmd = '--all'
  local module = ''
  local method = ''
  local cmderr = 'Invalid flag used'
  local modules = {
    'audio', 'data', 'event', 'filesystem', 'font', 'graphics', 'image',
    'joystick', 'keyboard', 'love', 'math', 'mouse', 'physics', 'sensor',
    'sound', 'system', 'thread', 'timer', 'touch', 'video', 'window'
  }
  GITHUB_RUNNER = false
  for a=1,#arglist do
    if testcmd == '--method' then
      if module == '' and (arglist[a] == 'love' or love[ arglist[a] ] ~= nil) then 
        module = arglist[a] 
        table.insert(modules, module)
      elseif module ~= '' and love[module] ~= nil and method == '' then
        if love.test[module][arglist[a]] ~= nil then method = arglist[a] end
      end
    end
    if testcmd == '--modules' then
      if (arglist[a] == 'love' or love[ arglist[a] ] ~= nil) and arglist[a] ~= '--isRunner' then 
        table.insert(modules, arglist[a]) 
      end
    end
    if arglist[a] == '--method' then
      testcmd = arglist[a]
      modules = {}
    end
    if arglist[a] == '--modules' then
      testcmd = arglist[a]
      modules = {}
    end
    if arglist[a] == '--isRunner' then
      GITHUB_RUNNER = true
    end
  end

  -- method uses the module + method given
  if testcmd == '--method' then
    local testmodule = TestModule:new(module, method)
    table.insert(love.test.modules, testmodule)
    if module ~= '' and method ~= '' then
      love.test.module = testmodule
      love.test.module:log('grey', '--method "' .. module .. '" "' .. method .. '"')
      love.test.output = 'lovetest_method_' .. module .. '_' .. method
    else
      if method == '' then cmderr = 'No valid method specified' end
      if module == '' then cmderr = 'No valid module specified' end
    end
  end

  -- modules runs all methods for all the modules given
  if testcmd == '--modules' then
    local modulelist = {}
    for m=1,#modules do
      local testmodule = TestModule:new(modules[m])
      table.insert(love.test.modules, testmodule)
      table.insert(modulelist, modules[m])
    end
    if #modulelist > 0 then
      love.test.module = love.test.modules[1]
      love.test.module:log('grey', '--modules "' .. table.concat(modulelist, '" "') .. '"')
      love.test.output = 'lovetest_modules_' .. table.concat(modulelist, '_')
    else
      cmderr = 'No modules specified'
    end
  end

  -- otherwise default runs all methods for all modules
  if arglist[1] == nil or arglist[1] == '' or arglist[1] == '--all' then
    for m=1,#modules do
      local testmodule = TestModule:new(modules[m])
      table.insert(love.test.modules, testmodule)
    end
    love.test.module = love.test.modules[1]
    love.test.module:log('grey', '--all')
    love.test.output = 'lovetest_all'
  end

  if GITHUB_RUNNER then
    love.test.module:log('grey', '--isRunner')
  end

  -- invalid command
  if love.test.module == nil then
    print(cmderr)
    love.event.quit(0)
  else 
    -- start first module
    TextCommand = testcmd
    love.test.module:runTests()
  end

end

-- love.update
-- run test suite logic 
love.update = function(delta)
  love.test:runSuite(delta)
end


-- love.draw
-- draw a little logo to the screen
love.draw = function()
  local lw = (love.graphics.getWidth() - 128) / 2
  local lh = (love.graphics.getHeight() - 128) / 2
  love.graphics.draw(Logo.texture, Logo.img, lw, lh, 0, 2, 2)
  love.graphics.setFont(Font)
  love.graphics.print(TextCommand, 4, 12, 0, 2, 2)
  love.graphics.print(TextRun, 4, 32, 0, 2, 2)
end


-- love.quit
-- add a hook to allow test modules to fake quit
love.quit = function()
  if love.test.module ~= nil and love.test.module.fakequit then
    return true
  else
    return false
  end
end


-- added so bad threads dont fail
function love.threaderror(thread, errorstr) end


-- string split helper
function UtilStringSplit(str, splitter)
  local splits = {}
  for word in string.gmatch(str, splitter) do
    table.insert(splits, word)
  end
  return splits
end


-- string time formatter
function UtilTimeFormat(seconds)
  return string.format("%.3f", tostring(seconds))
end
