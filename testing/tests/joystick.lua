-- love.joystick
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.joystick.getGamepadMappingString
love.test.joystick.getGamepadMappingString = function(test)
  local mapping = love.joystick.getGamepadMappingString('faker')
  test:assertEquals(nil, mapping, 'check no mapping for fake gui')
end


-- love.joystick.getJoystickCount
love.test.joystick.getJoystickCount = function(test)
  local count = love.joystick.getJoystickCount()
  test:assertGreaterEqual(0, count, 'check number')
end


-- love.joystick.getJoysticks
love.test.joystick.getJoysticks = function(test)
  local joysticks = love.joystick.getJoysticks()
  test:assertGreaterEqual(0, #joysticks, 'check is count')
end


-- love.joystick.loadGamepadMappings
love.test.joystick.loadGamepadMappings = function(test)
  local ok, err = pcall(love.joystick.loadGamepadMappings, 'fakefile.txt')
  test:assertEquals(false, ok, 'check invalid file')
  love.joystick.loadGamepadMappings('resources/mappings.txt')
end


-- love.joystick.saveGamepadMappings
love.test.joystick.saveGamepadMappings = function(test)
  love.joystick.loadGamepadMappings('resources/mappings.txt')
  local mapping = love.joystick.saveGamepadMappings()
  test:assertGreaterEqual(0, #mapping, 'check something mapped')
end


-- love.joystick.setGamepadMapping
love.test.joystick.setGamepadMapping = function(test)
  local guid = '030000005e040000130b000011050000'
  local mappings = {
    love.joystick.setGamepadMapping(guid, 'a', 'button', 1, nil),
    love.joystick.setGamepadMapping(guid, 'b', 'button', 2, nil),
    love.joystick.setGamepadMapping(guid, 'x', 'button', 3, nil),
    love.joystick.setGamepadMapping(guid, 'y', 'button', 4, nil),
    love.joystick.setGamepadMapping(guid, 'back', 'button', 5, nil),
    love.joystick.setGamepadMapping(guid, 'start', 'button', 6, nil),
    love.joystick.setGamepadMapping(guid, 'guide', 'button', 7, nil),
    love.joystick.setGamepadMapping(guid, 'leftstick', 'button', 8, nil),
    love.joystick.setGamepadMapping(guid, 'leftshoulder', 'button', 9, nil),
    love.joystick.setGamepadMapping(guid, 'rightstick', 'button', 10, nil),
    love.joystick.setGamepadMapping(guid, 'rightshoulder', 'button', 11, nil),
    love.joystick.setGamepadMapping(guid, 'dpup', 'button', 12, nil),
    love.joystick.setGamepadMapping(guid, 'dpdown', 'button', 13, nil),
    love.joystick.setGamepadMapping(guid, 'dpleft', 'button', 14, nil),
    love.joystick.setGamepadMapping(guid, 'dpright', 'button', 15, nil),
    love.joystick.setGamepadMapping(guid, 'dpup', 'button', 12, 'u'),
    love.joystick.setGamepadMapping(guid, 'dpdown', 'button', 13, 'd'),
    love.joystick.setGamepadMapping(guid, 'dpleft', 'button', 14, 'l'),
    love.joystick.setGamepadMapping(guid, 'dpright', 'button', 15, 'r'),
    love.joystick.setGamepadMapping(guid, 'dpup', 'hat', 12, 'lu'),
    love.joystick.setGamepadMapping(guid, 'dpdown', 'hat', 13, 'ld'),
    love.joystick.setGamepadMapping(guid, 'dpleft', 'hat', 14, 'ru'),
    love.joystick.setGamepadMapping(guid, 'dpright', 'hat', 15, 'rd'),
    love.joystick.setGamepadMapping(guid, 'leftstick', 'axis', 8, 'c')
  }
  for m=1,#mappings do
    test:assertEquals(true, mappings[m], 'check mapping #' .. tostring(m))
  end
end
