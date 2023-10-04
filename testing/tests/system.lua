-- love.system


-- love.system.getClipboardText
love.test.system.getClipboardText = function(test)
  -- ignore if not using window
  if love.test.windowmode == false then return test:skipTest('clipboard only available in window mode') end
  -- setup
  love.system.setClipboardText('helloworld')
  -- test
  test:assertEquals('helloworld', love.system.getClipboardText(), 'check clipboard match')
end


-- love.system.getOS
love.test.system.getOS = function(test)
  local os = love.system.getOS()
  test:assertMatch({'OS X', 'Windows', 'Linux', 'Android', 'iOS'}, os, 'check value matches')
end


-- love.system.getPowerInfo
love.test.system.getPowerInfo = function(test)
  local state, percent, seconds = love.system.getPowerInfo()
  test:assertMatch({'unknown', 'battery', 'nobattery', 'charging', 'charged'}, state, 'check value matches')
  if percent ~= nil then
    test:assertRange(percent, 0, 100, 'check value within range')
  end
  if seconds ~= nil then
    test:assertRange(seconds, 0, 100, 'check value within range')
  end
end


-- love.system.getProcessorCount
love.test.system.getProcessorCount = function(test)
  test:assertGreaterEqual(0, love.system.getProcessorCount(), 'check not nil') -- youd hope right
end


-- love.system.hasBackgroundMusic
love.test.system.hasBackgroundMusic = function(test)
  test:assertNotEquals(nil, love.system.hasBackgroundMusic(), 'check not nil')
end


-- love.system.openURL
love.test.system.openURL = function(test)
  test:skipTest('gets annoying to test everytime')
  --test:assertNotEquals(nil, love.system.openURL('https://love2d.org'), 'check open URL')
end


-- love.system.getClipboardText
love.test.system.setClipboardText = function(test)
  -- ignore if not using window
  if love.test.windowmode == false then return test:skipTest('clipboard only available in window mode') end
  -- test
  love.system.setClipboardText('helloworld')
  test:assertEquals('helloworld', love.system.getClipboardText(), 'check set text')
end


-- love.system.vibrate
-- @NOTE cant really test this
love.test.system.vibrate = function(test)
  test:skipTest('cant really test this')
end