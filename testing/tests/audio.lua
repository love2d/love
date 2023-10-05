-- love.audio


-- love.audio.getActiveEffects
love.test.audio.getActiveEffects = function(test)
  -- check we get a value
  test:assertNotNil(love.audio.getActiveEffects())
  -- check setting an effect active
  love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  test:assertEquals(1, #love.audio.getActiveEffects(), 'check 1 effect running')
  test:assertEquals('testeffect', love.audio.getActiveEffects()[1], 'check effect details')
end


-- love.audio.getActiveSourceCount
love.test.audio.getActiveSourceCount = function(test)
  -- check we get a value
  test:assertNotNil(love.audio.getActiveSourceCount())
  -- check source isn't active by default
  local testsource = love.audio.newSource('resources/click.ogg', 'static')
  test:assertEquals(0, love.audio.getActiveSourceCount(), 'check not active')
  -- check playing a source marks it as active
  love.audio.play(testsource)
  test:assertEquals(1, love.audio.getActiveSourceCount(), 'check now active')
  love.audio.pause()
end


-- love.audio.getDistanceModel
love.test.audio.getDistanceModel = function(test)
  -- check we get a value
  test:assertNotNil(love.audio.getDistanceModel())
  -- check default value from documentation
  test:assertEquals('inverseclamped', love.audio.getDistanceModel(), 'check default value')
  -- check get correct value after setting
  love.audio.setDistanceModel('inverse')
  test:assertEquals('inverse', love.audio.getDistanceModel(), 'check setting model')
end


-- love.audio.getDopplerScale
love.test.audio.getDopplerScale = function(test)
  -- check default value
  test:assertEquals(1, love.audio.getDopplerScale(), 'check default 1')
  -- check correct value after setting to 0
  love.audio.setDopplerScale(0)
  test:assertEquals(0, love.audio.getDopplerScale(), 'check setting to 0')
  love.audio.setDopplerScale(1)
end


-- love.audio.getEffect
love.test.audio.getEffect = function(test)
  -- check getting a non-existent effect
  test:assertEquals(nil, love.audio.getEffect('madeupname'), 'check wrong name')
  -- check getting a valid effect
  love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  test:assertNotNil(love.audio.getEffect('testeffect'))
  -- check effect values match creation values
  test:assertEquals('chorus', love.audio.getEffect('testeffect').type, 'check effect type')
  test:assertEquals(10, love.audio.getEffect('testeffect').volume, 'check effect volume')
end


-- love.audio.getMaxSceneEffects
-- @NOTE feel like this is platform specific number so best we can do is a nil?
love.test.audio.getMaxSceneEffects = function(test)
  test:assertNotNil(love.audio.getMaxSceneEffects())
end


-- love.audio.getMaxSourceEffects
-- @NOTE feel like this is platform specific number so best we can do is a nil?
love.test.audio.getMaxSourceEffects = function(test)
  test:assertNotNil(love.audio.getMaxSourceEffects())
end


-- love.audio.getOrientation
-- @NOTE is there an expected default listener pos?
love.test.audio.getOrientation = function(test)
  -- checking getting values matches what was set
  love.audio.setOrientation(1, 2, 3, 4, 5, 6)
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
  -- check getting values matches what was set
  love.audio.setPosition(1, 2, 3)
  local x, y, z = love.audio.getPosition()
  test:assertEquals(1, x, 'check x position')
  test:assertEquals(2, y, 'check y position')
  test:assertEquals(3, z, 'check z position')
end


-- love.audio.getRecordingDevices
-- @NOTE hardware dependent so best can do is not nil check
love.test.audio.getRecordingDevices = function(test)
  test:assertNotNil(love.audio.getRecordingDevices())
end


-- love.audio.getVelocity
love.test.audio.getVelocity = function(test)
  -- check getting values matches what was set
  love.audio.setVelocity(1, 2, 3)
  local x, y, z = love.audio.getVelocity()
  test:assertEquals(1, x, 'check x velocity')
  test:assertEquals(2, y, 'check y velocity')
  test:assertEquals(3, z, 'check z velocity')
end


-- love.audio.getVolume
love.test.audio.getVolume = function(test)
  -- check getting values matches what was set
  love.audio.setVolume(0.5)
  test:assertEquals(0.5, love.audio.getVolume(), 'check matches set')
end


-- love.audio.isEffectsSupported
love.test.audio.isEffectsSupported = function(test)
  test:assertNotNil(love.audio.isEffectsSupported())
end


-- love.audio.newQueueableSource
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.audio.newQueueableSource = function(test)
  test:assertObject(love.audio.newQueueableSource(32, 8, 1, 8))
end


-- love.audio.newSource
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.audio.newSource = function(test)
  test:assertObject(love.audio.newSource('resources/click.ogg', 'static'))
  test:assertObject(love.audio.newSource('resources/click.ogg', 'stream'))
end


-- love.audio.pause
love.test.audio.pause = function(test)
  -- check nothing paused (as should be nothing playing)
  local nopauses = love.audio.pause()
  test:assertEquals(0, #nopauses, 'check nothing paused')
  -- check 1 source paused after playing/pausing 1
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  local onepause = love.audio.pause()
  test:assertEquals(1, #onepause, 'check 1 paused')
end


-- love.audio.play
love.test.audio.play = function(test)
  -- check playing source is detected
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  test:assertEquals(true, source:isPlaying(), 'check something playing')
  love.audio.pause()
end


-- love.audio.setDistanceModel
love.test.audio.setDistanceModel = function(test)
  -- check setting each of the distance models is accepted and val returned
  local distancemodel = {
    'none', 'inverse', 'inverseclamped', 'linear', 'linearclamped',
    'exponent', 'exponentclamped'
  }
  for d=1,#distancemodel do
    love.audio.setDistanceModel(distancemodel[d])
    test:assertEquals(distancemodel[d], love.audio.getDistanceModel(),
      'check model set to ' .. distancemodel[d])
  end
end


-- love.audio.setDopplerScale
love.test.audio.setDopplerScale = function(test)
  -- check setting value is returned properly
  love.audio.setDopplerScale(0)
  test:assertEquals(0, love.audio.getDopplerScale(), 'check set to 0')
  love.audio.setDopplerScale(1)
  test:assertEquals(1, love.audio.getDopplerScale(), 'check set to 1')
end


-- love.audio.setEffect
love.test.audio.setEffect = function(test)
  -- check effect is set correctly
  local effect = love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 10
  })
  test:assertEquals(true, effect, 'check effect created')
  -- check values set match
  local settings = love.audio.getEffect('testeffect')
  test:assertEquals('chorus', settings.type, 'check effect type')
  test:assertEquals(10, settings.volume, 'check effect volume')
end


-- love.audio.setMixWithSystem
love.test.audio.setMixWithSystem = function(test)
  test:assertNotNil(love.audio.setMixWithSystem(true))
end


-- love.audio.setOrientation
love.test.audio.setOrientation = function(test)
  -- check setting orientation vals are returned
  love.audio.setOrientation(1, 2, 3, 4, 5, 6)
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
  -- check setting position vals are returned
  love.audio.setPosition(1, 2, 3)
  local x, y, z = love.audio.getPosition()
  test:assertEquals(1, x, 'check x position')
  test:assertEquals(2, y, 'check y position')
  test:assertEquals(3, z, 'check z position')
end


-- love.audio.setVelocity
love.test.audio.setVelocity = function(test)
  -- check setting velocity vals are returned
  love.audio.setVelocity(1, 2, 3)
  local x, y, z = love.audio.getVelocity()
  test:assertEquals(1, x, 'check x velocity')
  test:assertEquals(2, y, 'check y velocity')
  test:assertEquals(3, z, 'check z velocity')
end


-- love.audio.setVolume
love.test.audio.setVolume = function(test)
  -- check setting volume works
  love.audio.setVolume(0.5)
  test:assertEquals(0.5, love.audio.getVolume(), 'check set to 0.5')
end


-- love.audio.stop
love.test.audio.stop = function(test)
  -- check source is playing first
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  test:assertEquals(true, source:isPlaying(), 'check is playing')
  -- check source is then stopped
  love.audio.stop()
  test:assertEquals(false, source:isPlaying(), 'check stopped playing')
end
