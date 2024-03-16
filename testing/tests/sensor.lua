-- love.sensor
-- @NOTE we can't test this module fully as it's hardware dependent
-- however we can test methods do what is expected and can handle certain params

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------HELPERS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function testIsEnabled(test, sensorType)
  love.sensor.setEnabled(sensorType, true)
  test:assertTrue(love.sensor.isEnabled(sensorType), 'check ' .. sensorType .. ' enabled')
  love.sensor.setEnabled(sensorType, false)
  test:assertFalse(love.sensor.isEnabled(sensorType), 'check ' .. sensorType .. ' disabled')
end


local function testGetName(test, sensorType)
  love.sensor.setEnabled(sensorType, true)
  local ok, name = pcall(love.sensor.getName, sensorType)
  test:assertTrue(ok, 'check sensor.getName("' .. sensorType .. '") success')
  test:assertEquals(type(name), 'string', 'check sensor.getName("' .. sensorType .. '") return value type')

  love.sensor.setEnabled(sensorType, false)
  ok, name = pcall(love.sensor.getName, sensorType)
  test:assertFalse(ok, 'check sensor.getName("' .. sensorType .. '") errors when disabled')
end


local function testGetData(test, sensorType)
  love.sensor.setEnabled(sensorType, true)
  local ok, x, y, z = pcall(love.sensor.getData, sensorType)
  test:assertTrue(ok, 'check sensor.getData("' .. sensorType .. '") success')
  if ok then
    test:assertNotNil(x)
    test:assertNotNil(y)
    test:assertNotNil(z)
  end

  love.sensor.setEnabled(sensorType, false)
  ok, x, y, z = pcall(love.sensor.getData, sensorType)
  test:assertFalse(ok, 'check sensor.getData("' .. sensorType .. '") errors when disabled')
end

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


-- love.sensor.isEnabled and love.sensor.setEnabled
love.test.sensor.isEnabled = function(test)
  local accelerometer = love.sensor.hasSensor('accelerometer')
  local gyroscope = love.sensor.hasSensor('gyroscope')

  if accelerometer or gyroscope then
    if accelerometer then testIsEnabled(test, 'accelerometer') end
    if gyroscope then testIsEnabled(test, 'gyroscope') end
  else
    test:skipTest('neither accelerometer nor gyroscope are supported in this system')
  end
end


-- love.sensor.getName
love.test.sensor.getName = function(test)
  local accelerometer = love.sensor.hasSensor('accelerometer')
  local gyroscope = love.sensor.hasSensor('gyroscope')

  if accelerometer or gyroscope then
    if accelerometer then testGetName(test, 'accelerometer') end
    if gyroscope then testGetName(test, 'gyroscope') end
  else
    test:skipTest('neither accelerometer nor gyroscope are supported in this system')
  end
end


-- love.sensor.getData
love.test.sensor.getData = function(test)
  local accelerometer = love.sensor.hasSensor('accelerometer')
  local gyroscope = love.sensor.hasSensor('gyroscope')

  if accelerometer or gyroscope then
    if accelerometer then testGetData(test, 'accelerometer') end
    if gyroscope then testGetData(test, 'gyroscope') end
  else
    test:skipTest('neither accelerometer nor gyroscope are supported in this system')
  end
end
