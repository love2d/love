-- love.keyboard
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.keyboard.getKeyFromScancode
love.test.keyboard.getKeyFromScancode = function(test)
  test:assertEquals('function', type(love.keyboard.getKeyFromScancode))
end


-- love.keyboard.getScancodeFromKey
love.test.keyboard.getScancodeFromKey = function(test)
  test:assertEquals('function', type(love.keyboard.getScancodeFromKey))
end


-- love.keyboard.hasKeyRepeat
love.test.keyboard.hasKeyRepeat = function(test)
  local enabled = love.keyboard.hasKeyRepeat()
  test:assertNotNil(enabled)
end


-- love.keyboard.hasScreenKeyboard
love.test.keyboard.hasScreenKeyboard = function(test)
  local enabled = love.keyboard.hasScreenKeyboard()
  test:assertNotNil(enabled)
end


-- love.keyboard.hasTextInput
love.test.keyboard.hasTextInput = function(test)
  local enabled = love.keyboard.hasTextInput()
  test:assertNotNil(enabled)
end


-- love.keyboard.isDown
love.test.keyboard.isDown = function(test)
  local keydown = love.keyboard.isDown('a')
  test:assertNotNil(keydown)
end


-- love.keyboard.isScancodeDown
love.test.keyboard.isScancodeDown = function(test)
  local keydown = love.keyboard.isScancodeDown('a')
  test:assertNotNil(keydown)
end


-- love.keyboard.setKeyRepeat
love.test.keyboard.setKeyRepeat = function(test)
  love.keyboard.setKeyRepeat(true)
  local enabled = love.keyboard.hasKeyRepeat()
  test:assertEquals(true, enabled, 'check key repeat set')
end


-- love.keyboard.setTextInput
love.test.keyboard.setTextInput = function(test)
  love.keyboard.setTextInput(false)
  test:assertEquals(false, love.keyboard.hasTextInput(), 'check disable text input')
end
