-- love.touch
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.touch.getPosition
-- @TODO is there a way to fake the touchid pointer?
love.test.touch.getPosition = function(test)
  test:assertNotNil(love.touch.getPosition)
  test:assertEquals('function', type(love.touch.getPosition))
end


-- love.touch.getPressure
-- @TODO is there a way to fake the touchid pointer?
love.test.touch.getPressure = function(test)
  test:assertNotNil(love.touch.getPressure)
  test:assertEquals('function', type(love.touch.getPressure))
end


-- love.touch.getTouches
love.test.touch.getTouches = function(test)
  test:assertEquals('function', type(love.touch.getTouches))
end
