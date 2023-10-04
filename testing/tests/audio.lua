-- love.audio


-- love.audio.getActiveEffects
love.test.audio.getActiveEffects = function(test)
  -- tests
  test:assertNotEquals(nil, love.audio.getActiveEffects(), 'check not nil')
  test:assertEquals(0, #love.audio.getActiveEffects(), 'check no effects running')
  love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  test:assertEquals(1, #love.audio.getActiveEffects(), 'check 1 effect running')
  test:assertEquals('testeffect', love.audio.getActiveEffects()[1], 'check effect details')
end


-- love.audio.getActiveSourceCount
love.test.audio.getActiveSourceCount = function(test)
  -- tests
  test:assertNotEquals(nil, love.audio.getActiveSourceCount(), 'check not nil')
  test:assertEquals(0, love.audio.getActiveSourceCount(), 'check 0 by default')
  local testsource = love.audio.newSource('resources/click.ogg', 'static')
  test:assertEquals(0, love.audio.getActiveSourceCount(), 'check not active')
  love.audio.play(testsource)
  test:assertEquals(1, love.audio.getActiveSourceCount(), 'check now active')
  love.audio.pause()
  testsource:release()
end


-- love.audio.getDistanceModel
love.test.audio.getDistanceModel = function(test)
  -- tests
  test:assertNotEquals(nil, love.audio.getDistanceModel(), 'check not nil')
  test:assertEquals('inverseclamped', love.audio.getDistanceModel(), 'check default value')
  love.audio.setDistanceModel('inverse')
  test:assertEquals('inverse', love.audio.getDistanceModel(), 'check setting model')
end


-- love.audio.getDopplerScale
love.test.audio.getDopplerScale = function(test)
  test:assertEquals(1, love.audio.getDopplerScale(), 'check default 1')
  love.audio.setDopplerScale(0)
  test:assertEquals(0, love.audio.getDopplerScale(), 'check setting to 0')
  love.audio.setDopplerScale(1)
end


-- love.audio.getEffect
love.test.audio.getEffect = function(test)
  -- setup
  love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  -- tests
  test:assertEquals(nil, love.audio.getEffect('madeupname'), 'check wrong name')
  test:assertNotEquals(nil, love.audio.getEffect('testeffect'), 'check not nil')
  test:assertEquals('chorus', love.audio.getEffect('testeffect').type, 'check effect type')
  test:assertEquals(10, love.audio.getEffect('testeffect').volume, 'check effect volume')
end


-- love.audio.getMaxSceneEffects
-- @NOTE feel like this is platform specific number so best we can do is a nil?
love.test.audio.getMaxSceneEffects = function(test)
  test:assertNotEquals(nil, love.audio.getMaxSceneEffects(), 'check not nil')
end


-- love.audio.getMaxSourceEffects
-- @NOTE feel like this is platform specific number so best we can do is a nil?
love.test.audio.getMaxSourceEffects = function(test)
  test:assertNotEquals(nil, love.audio.getMaxSourceEffects(), 'check not nil')
end


-- love.audio.getOrientation
-- @NOTE is there an expected default listener pos?
love.test.audio.getOrientation = function(test)
  -- setup
  love.audio.setOrientation(1, 2, 3, 4, 5, 6)
  -- tests
  local fx, fy, fz, ux, uy, uz = love.audio.getOrientation()
  test:assertEquals(1, fx, 'check fx orientation')
  test:assertEquals(2, fy, 'check fy orientation')
  test:assertEquals(3, fz, 'check fz orientation')
  test:assertEquals(4, ux, 'check ux orientation')
  test:assertEquals(5, uy, 'check uy orientation')
  test:assertEquals(6, uz, 'check uz orientation')
end


-- love.audio.getPosition
-- @NOTE is there an expected default listener pos?
love.test.audio.getPosition = function(test)
  -- setup
  love.audio.setPosition(1, 2, 3)
  -- tests
  local x, y, z = love.audio.getPosition()
  test:assertEquals(1, x, 'check x position')
  test:assertEquals(2, y, 'check y position')
  test:assertEquals(3, z, 'check z position')
end


-- love.audio.getRecordingDevices
love.test.audio.getRecordingDevices = function(test)
  test:assertNotEquals(nil, love.audio.getRecordingDevices(), 'check not nil')
end


-- love.audio.getVelocity
love.test.audio.getVelocity = function(test)
  -- setup
  love.audio.setVelocity(1, 2, 3)
  -- tests
  local x, y, z = love.audio.getVelocity()
  test:assertEquals(1, x, 'check x velocity')
  test:assertEquals(2, y, 'check y velocity')
  test:assertEquals(3, z, 'check z velocity')
end


-- love.audio.getVolume
love.test.audio.getVolume = function(test)
  -- setup
  love.audio.setVolume(0.5)
  -- tests
  test:assertNotEquals(nil, love.audio.getVolume(), 'check not nil')
  test:assertEquals(0.5, love.audio.getVolume(), 'check matches set')
end


-- love.audio.isEffectsSupported
love.test.audio.isEffectsSupported = function(test)
  test:assertNotEquals(nil, love.audio.isEffectsSupported(), 'check not nil')
end


-- love.audio.newQueueableSource
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.audio.newQueueableSource = function(test)
  local source = love.audio.newQueueableSource(32, 8, 1, 8)
  test:assertObject(source)
  source:release()
end


-- love.audio.newSource
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.audio.newSource = function(test)
  -- setup
  local source1 = love.audio.newSource('resources/click.ogg', 'static')
  local source2 = love.audio.newSource('resources/click.ogg', 'stream')
  -- tests
  test:assertObject(source1)
  test:assertObject(source2)
  -- cleanup
  source1:release()
  source2:release()
end


-- love.audio.pause
love.test.audio.pause = function(test)
  -- tests
  local nopauses = love.audio.pause()
  test:assertNotEquals(nil, nopauses, 'check not nil')
  test:assertEquals(0, #nopauses, 'check nothing paused')
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  local onepause = love.audio.pause()
  test:assertEquals(1, #onepause, 'check 1 paused')
  source:release()
end


-- love.audio.play
love.test.audio.play = function(test)
  -- setup
  local source = love.audio.newSource('resources/click.ogg', 'static')
  -- tests
  love.audio.play(source)
  test:assertEquals(true, source:isPlaying(), 'check something playing')
  love.audio.pause()
  source:release()
end


-- love.audio.setDistanceModel
love.test.audio.setDistanceModel = function(test)
  -- tests
  local distancemodel = {
    'none', 'inverse', 'inverseclamped', 'linear', 'linearclamped',
    'exponent', 'exponentclamped'
  }
  for d=1,#distancemodel do
    love.audio.setDistanceModel(distancemodel[d])
    test:assertEquals(distancemodel[d], love.audio.getDistanceModel(), 'check model set to ' .. distancemodel[d])
  end
end


-- love.audio.setDopplerScale
love.test.audio.setDopplerScale = function(test)
  -- tests
  love.audio.setDopplerScale(0)
  test:assertEquals(0, love.audio.getDopplerScale(), 'check set to 0')
  love.audio.setDopplerScale(1)
  test:assertEquals(1, love.audio.getDopplerScale(), 'check set to 1')
end


-- love.audio.setEffect
love.test.audio.setEffect = function(test)
  -- tests
  local effect = love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  test:assertEquals(true, effect, 'check effect created')
  local settings = love.audio.getEffect('testeffect')
  test:assertEquals('chorus', settings.type, 'check effect type')
  test:assertEquals(10, settings.volume, 'check effect volume')
end


-- love.audio.setMixWithSystem
love.test.audio.setMixWithSystem = function(test)
  test:assertNotEquals(nil, love.audio.setMixWithSystem(true), 'check not nil')
end


-- love.audio.setOrientation
love.test.audio.setOrientation = function(test)
  -- setup
  love.audio.setOrientation(1, 2, 3, 4, 5, 6)
  -- tests
  local fx, fy, fz, ux, uy, uz = love.audio.getOrientation()
  test:assertEquals(1, fx, 'check fx orientation')
  test:assertEquals(2, fy, 'check fy orientation')
  test:assertEquals(3, fz, 'check fz orientation')
  test:assertEquals(4, ux, 'check ux orientation')
  test:assertEquals(5, uy, 'check uy orientation')
  test:assertEquals(6, uz, 'check uz orientation')
end


-- love.audio.setPosition
love.test.audio.setPosition = function(test)
  -- setup
  love.audio.setPosition(1, 2, 3)
  -- tests
  local x, y, z = love.audio.getPosition()
  test:assertEquals(1, x, 'check x position')
  test:assertEquals(2, y, 'check y position')
  test:assertEquals(3, z, 'check z position')
end


-- love.audio.setVelocity
love.test.audio.setVelocity = function(test)
  -- setup
  love.audio.setVelocity(1, 2, 3)
  -- tests
  local x, y, z = love.audio.getVelocity()
  test:assertEquals(1, x, 'check x velocity')
  test:assertEquals(2, y, 'check y velocity')
  test:assertEquals(3, z, 'check z velocity')
end


-- love.audio.setVolume
love.test.audio.setVolume = function(test)
  -- setup
  love.audio.setVolume(0.5)
  -- tests
  test:assertNotEquals(nil, love.audio.getVolume(), 'check not nil')
  test:assertEquals(0.5, love.audio.getVolume(), 'check set to 0.5')
end


-- love.audio.stop
love.test.audio.stop = function(test)
  -- setup
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  -- tests
  test:assertEquals(true, source:isPlaying(), 'check is playing')
  love.audio.stop()
  test:assertEquals(false, source:isPlaying(), 'check stopped playing')
  source:release()
end