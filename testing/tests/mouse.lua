-- love.mouse
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.mouse.getCursor
love.test.mouse.getCursor = function(test)
  local cursor = love.mouse.getCursor()
  test:assertEquals(nil, cursor, 'check nil initially')
  -- try setting a cursor to check return if supported
  if love.mouse.isCursorSupported() then
    love.mouse.setCursor(love.mouse.getSystemCursor("hand"))
    local newcursor = love.mouse.getCursor()
    test:assertObject(newcursor)
    love.mouse.setCursor()
  end
end


-- love.mouse.getPosition
love.test.mouse.getPosition = function(test)
  love.mouse.setPosition(0, 0) -- cant predict
  local x, y = love.mouse.getPosition()
  test:assertEquals(0, x, 'check x pos')
  test:assertEquals(0, y, 'check y pos')
end


-- love.mouse.getRelativeMode
love.test.mouse.getRelativeMode = function(test)
  local enabled = love.mouse.getRelativeMode()
  test:assertEquals(false, enabled, 'check relative mode')
  love.mouse.setRelativeMode(true)
  test:assertEquals(true, love.mouse.getRelativeMode(), 'check enabling')
end


-- love.mouse.getSystemCursor
love.test.mouse.getSystemCursor = function(test)
  local hand = love.mouse.getSystemCursor('hand')
  test:assertObject(hand)
  local ok, err = pcall(love.mouse.getSystemCursor, 'love')
  test:assertEquals(false, ok, 'check invalid cursor')
end


-- love.mouse.getX
love.test.mouse.getX = function(test)
  love.mouse.setPosition(0, 0) -- cant predict
  local x = love.mouse.getX()
  test:assertEquals(0, x, 'check x pos')
  love.mouse.setX(10)
  test:assertEquals(10, love.mouse.getX(), 'check set x')
end


-- love.mouse.getY
love.test.mouse.getY = function(test)
  love.mouse.setPosition(0, 0) -- cant predict
  local y = love.mouse.getY()
  test:assertEquals(0, y, 'check x pos')
  love.mouse.setY(10)
  test:assertEquals(10, love.mouse.getY(), 'check set y')
end


-- love.mouse.isCursorSupported
love.test.mouse.isCursorSupported = function(test)
  test:assertNotNil(love.mouse.isCursorSupported())
end


-- love.mouse.isDown
love.test.mouse.isDown = function(test)
  test:assertNotNil(love.mouse.isDown())
end


-- love.mouse.isGrabbed
love.test.mouse.isGrabbed = function(test)
  test:assertNotNil(love.mouse.isGrabbed())
end


-- love.mouse.isVisible
love.test.mouse.isVisible = function(test)
  local visible = love.mouse.isVisible()
  test:assertEquals(true, visible, 'check visible default')
  love.mouse.setVisible(false)
  test:assertEquals(false, love.mouse.isVisible(), 'check invisible')
  love.mouse.setVisible(true)
end


-- love.mouse.newCursor
love.test.mouse.newCursor = function(test)
  -- new cursor might fail if not supported
  if love.mouse.isCursorSupported() then
    local cursor = love.mouse.newCursor('resources/love.png', 0, 0)
    test:assertObject(cursor)
  else
    test:skipTest('cursor not supported on this system')
  end
end


-- love.mouse.setCursor
love.test.mouse.setCursor = function(test)
  -- cant set cursor if not supported
  if love.mouse.isCursorSupported() then
    love.mouse.setCursor()
    test:assertEquals(nil, love.mouse.getCursor(), 'check reset')
    love.mouse.setCursor(love.mouse.getSystemCursor('hand'))
    test:assertObject(love.mouse.getCursor())
  else
    test:skipTest('cursor not supported on this system')
  end
end


-- love.mouse.setGrabbed
-- @NOTE can fail if you move the mouse a bunch while the test runs
love.test.mouse.setGrabbed = function(test)
  test:assertEquals(false, love.mouse.isGrabbed(), 'check not grabbed')
  love.mouse.setGrabbed(true)
  test:assertEquals(true, love.mouse.isGrabbed(), 'check now grabbed')
  love.mouse.setGrabbed(false)
end


-- love.mouse.setPosition
love.test.mouse.setPosition = function(test)
  love.mouse.setPosition(10, 10)
  local x, y = love.mouse.getPosition()
  test:assertEquals(10, x, 'check x position')
  test:assertEquals(10, y, 'check y position')
  love.mouse.setPosition(15, 20)
  local x2, y2 = love.mouse.getPosition()
  test:assertEquals(15, x2, 'check new x position')
  test:assertEquals(20, y2, 'check new y position')
end


-- love.mouse.setRelativeMode
love.test.mouse.setRelativeMode = function(test)
  love.mouse.setRelativeMode(true)
  local enabled = love.mouse.getRelativeMode()
  test:assertEquals(true, enabled, 'check relative mode')
  love.mouse.setRelativeMode(false)
  test:assertEquals(false, love.mouse.getRelativeMode(), 'check disabling')
end


-- love.mouse.setVisible
love.test.mouse.setVisible = function(test)
  local visible = love.mouse.isVisible()
  test:assertEquals(true, visible, 'check visible default')
  love.mouse.setVisible(false)
  test:assertEquals(false, love.mouse.isVisible(), 'check invisible')
  love.mouse.setVisible(true)
end


-- love.mouse.setX
love.test.mouse.setX = function(test)
  love.mouse.setX(30)
  local x = love.mouse.getX()
  test:assertEquals(30, x, 'check x pos')
  love.mouse.setX(10)
  test:assertEquals(10, love.mouse.getX(), 'check set x')
end


-- love.mouse.setY
love.test.mouse.setY = function(test)
  love.mouse.setY(12)
  local y = love.mouse.getY()
  test:assertEquals(12, y, 'check x pos')
  love.mouse.setY(10)
  test:assertEquals(10, love.mouse.getY(), 'check set y')
end
