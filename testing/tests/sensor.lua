-- love.sensor
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.sensor.hasSensor
love.test.sensor.hasSensor = function(test)
  -- but we can make sure that the SensorTypes can be passed
  local accelerometer = love.sensor.hasSensor('accelerometer')
  local gyroscope = love.sensor.hasSensor('gyroscope')
  test:assertNotNil(accelerometer)
  test:assertNotNil(gyroscope)
end
