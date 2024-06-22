-- love.system


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------METHODS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.system.getClipboardText
love.test.system.getClipboardText = function(test)
  -- ignore if not using window
  if love.test.windowmode == false then 
    return test:skipTest('clipboard only available in window mode') 
  end
  -- check clipboard value is set
  love.system.setClipboardText('helloworld')
  test:assertEquals('helloworld', love.system.getClipboardText(), 'check clipboard match')
end


-- love.system.getOS
love.test.system.getOS = function(test)
  -- check os is in documented values
  local os = love.system.getOS()
  local options = {'OS X', 'Windows', 'Linux', 'Android', 'iOS'}
  test:assertMatch(options, os, 'check value matches')
end


-- love.system.getPreferredLocales
love.test.system.getPreferredLocales = function(test)
  local locale = love.system.getPreferredLocales()
  test:assertNotNil(locale)
  test:assertEquals('table', type(locale), 'check returns table')
end


-- love.system.getPowerInfo
love.test.system.getPowerInfo = function(test)
  -- check battery state is one of the documented states
  local state, percent, seconds = love.system.getPowerInfo()
  local states = {'unknown', 'battery', 'nobattery', 'charging', 'charged'}
  test:assertMatch(states, state, 'check value matches')
  -- if percent/seconds check within expected range
  if percent ~= nil then
    test:assertRange(percent, 0, 100, 'check battery percent within range')
  end
  if seconds ~= nil then
    test:assertNotNil(seconds)
  end
end


-- love.system.getProcessorCount
love.test.system.getProcessorCount = function(test)
  test:assertNotNil(love.system.getProcessorCount()) -- youd hope right
end


-- love.system.hasBackgroundMusic
love.test.system.hasBackgroundMusic = function(test)
  test:assertNotNil(love.system.hasBackgroundMusic())
end


-- love.system.openURL
love.test.system.openURL = function(test)
  test:skipTest('cant test this worked')
  --test:assertNotEquals(nil, love.system.openURL('https://love2d.org'), 'check open URL')
end


-- love.system.getClipboardText
love.test.system.setClipboardText = function(test)
  -- ignore if not using window
  if love.test.windowmode == false then 
    return test:skipTest('clipboard only available in window mode') 
  end
  -- check value returned is what was set
  love.system.setClipboardText('helloworld')
  test:assertEquals('helloworld', love.system.getClipboardText(), 'check set text')
end


-- love.system.vibrate
-- @NOTE cant really test this
love.test.system.vibrate = function(test)
  test:skipTest('cant test this worked')
end
