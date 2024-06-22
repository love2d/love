-- love.timer


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------METHODS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.timer.getAverageDelta
-- @NOTE not sure if you could reliably get a specific delta?
love.test.timer.getAverageDelta = function(test)
  test:assertNotNil(love.timer.getAverageDelta())
end

-- love.timer.getDelta
-- @NOTE not sure if you could reliably get a specific delta?
love.test.timer.getDelta = function(test)
  test:assertNotNil(love.timer.getDelta())
end


-- love.timer.getFPS
-- @NOTE not sure if you could reliably get a specific FPS?
love.test.timer.getFPS = function(test)
  test:assertNotNil(love.timer.getFPS())
end


-- love.timer.getTime
love.test.timer.getTime = function(test)
  local starttime = love.timer.getTime()
  love.timer.sleep(0.1)
  local endtime = love.timer.getTime() - starttime
  test:assertRange(endtime, 0.05, 1, 'check 0.1s passes')
end


-- love.timer.sleep
love.test.timer.sleep = function(test)
  local starttime = love.timer.getTime()
  love.timer.sleep(0.1)
  test:assertRange(love.timer.getTime() - starttime, 0.05, 1, 'check 0.1s passes')
end


-- love.timer.step
-- @NOTE not sure if you could reliably get a specific step val?
love.test.timer.step = function(test)
  test:assertNotNil(love.timer.step())
end
