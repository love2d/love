-- love.audio


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- RecordingDevice (love.audio.getRecordingDevices)
love.test.audio.RecordingDevice = function(test)

  -- skip recording device on runners, they cant emulate it
  if GITHUB_RUNNER then
    return test:skipTest('cant emulate recording devices in CI')
  end

  -- check devices first
  local devices = love.audio.getRecordingDevices()
  if #devices == 0 then
    return test:skipTest('cant test this works: no recording devices found')
  end

  -- check object created and basics
  local device = devices[1]
  test:assertObject(device)
  test:assertMatch({1, 2}, device:getChannelCount(), 'check channel count is 1 or 2')
  test:assertNotEquals(nil, device:getName(), 'check has name')

  -- check initial data is empty as we haven't recorded anything yet 
  test:assertNotNil(device:getBitDepth())
  test:assertEquals(nil, device:getData(), 'check initial data empty')
  test:assertEquals(0, device:getSampleCount(), 'check initial sample empty')
  test:assertNotNil(device:getSampleRate())
  test:assertFalse(device:isRecording(), 'check not recording')

  -- start recording for a short time
  local startrecording = device:start(32000, 4000, 16, 1)
  test:waitFrames(10)
  test:assertTrue(startrecording, 'check recording started')
  test:assertTrue(device:isRecording(), 'check now recording')
  test:assertEquals(4000, device:getSampleRate(), 'check sample rate set')
  test:assertEquals(16, device:getBitDepth(), 'check bit depth set')
  test:assertEquals(1, device:getChannelCount(), 'check channel count set')
  local recording = device:stop()
  test:waitFrames(10)

  -- after recording 
  test:assertFalse(device:isRecording(), 'check not recording')
  test:assertEquals(nil, device:getData(), 'using stop should clear buffer')
  test:assertObject(recording)

end


-- Source (love.audio.newSource)
love.test.audio.Source = function(test)

  -- create stereo source
  local stereo = love.audio.newSource('resources/click.ogg', 'static')
  test:assertObject(stereo)

  -- check stereo props
  test:assertEquals(2, stereo:getChannelCount(), 'check stereo src')
  test:assertRange(stereo:getDuration("seconds"), 0, 0.1, 'check stereo seconds')
  test:assertNotNil(stereo:getFreeBufferCount())
  test:assertEquals('static', stereo:getType(), 'check stereo type')

  -- check cloning a stereo
  local clone = stereo:clone()
  test:assertEquals(2, clone:getChannelCount(), 'check clone stereo src')
  test:assertRange(clone:getDuration("seconds"), 0, 0.1, 'check clone stereo seconds')
  test:assertNotNil(clone:getFreeBufferCount())
  test:assertEquals('static', clone:getType(), 'check cloned stereo type')

  -- mess with stereo playing
  test:assertFalse(stereo:isPlaying(), 'check not playing')
  stereo:setLooping(true)
  stereo:play()
  test:assertTrue(stereo:isPlaying(), 'check now playing')
  test:assertTrue(stereo:isLooping(), 'check now playing')
  stereo:pause()
  stereo:seek(0.01, 'seconds')
  test:assertEquals(0.01, stereo:tell('seconds'), 'check seek/tell')
  stereo:stop()
  test:assertFalse(stereo:isPlaying(), 'check stopped playing')

  -- check volume limits
  stereo:setVolumeLimits(0.1, 0.5)
  local min, max = stereo:getVolumeLimits()
  test:assertRange(min, 0.1, 0.2, 'check min limit')
  test:assertRange(max, 0.5, 0.6, 'check max limit')

  -- check setting volume
  stereo:setVolume(1)
  test:assertEquals(1, stereo:getVolume(), 'check set volume')
  stereo:setVolume(0)
  test:assertEquals(0, stereo:getVolume(), 'check set volume')

  -- change some get/set props that can apply to stereo
  stereo:setPitch(2)
  test:assertEquals(2, stereo:getPitch(), 'check pitch change')

  -- create mono source
  local mono = love.audio.newSource('resources/clickmono.ogg', 'stream')
  test:assertObject(mono)
  test:assertEquals(1, mono:getChannelCount(), 'check mono src')
  test:assertEquals(2927, mono:getDuration("samples"), 'check mono seconds')
  test:assertEquals('stream', mono:getType(), 'check mono type')

  -- air absorption
  test:assertEquals(0, mono:getAirAbsorption(), 'get air absorption')
  mono:setAirAbsorption(1)
  test:assertEquals(1, mono:getAirAbsorption(), 'set air absorption')

  -- cone
  mono:setCone(0, 90*(math.pi/180), 1)
  local ia, oa, ov = mono:getCone()
  test:assertEquals(0, ia, 'check cone ia')
  test:assertEquals(math.floor(9000*(math.pi/180)), math.floor(oa*100), 'check cone oa')
  test:assertEquals(1, ov, 'check cone ov')

  -- direction
  mono:setDirection(3, 1, -1)
  local x, y, z = mono:getDirection()
  test:assertEquals(3, x, 'check direction x')
  test:assertEquals(1, y, 'check direction y')
  test:assertEquals(-1, z, 'check direction z')

  -- relative
  mono:setRelative(true)
  test:assertTrue(mono:isRelative(), 'check set relative')

  -- position
  mono:setPosition(1, 2, 3)
  x, y, z = mono:getPosition()
  test:assertEquals(x, 1, 'check pos x')
  test:assertEquals(y, 2, 'check pos y')
  test:assertEquals(z, 3, 'check pos z')

  -- velocity
  mono:setVelocity(1, 3, 4)
  x, y, z = mono:getVelocity()
  test:assertEquals(x, 1, 'check velocity x')
  test:assertEquals(y, 3, 'check velocity x')
  test:assertEquals(z, 4, 'check velocity x')

  -- rolloff
  mono:setRolloff(1)
  test:assertEquals(1, mono:getRolloff(), 'check rolloff set')

  -- create queue source
  local queue = love.audio.newQueueableSource(44100, 16, 1, 3)
  local sdata = love.sound.newSoundData(1024, 44100, 16, 1)
  test:assertObject(queue)
  local run = queue:queue(sdata)
  test:assertTrue(run, 'check queued sound')
  queue:stop()

  -- check making a filer
  local setfilter = stereo:setFilter({
    type = 'lowpass',
    volume = 0.5,
    highgain = 0.3
  })
  test:assertTrue(setfilter, 'check filter applied')
  local filter = stereo:getFilter()
  test:assertEquals('lowpass', filter.type, 'check filter type')
  test:assertEquals(0.5, filter.volume, 'check filter volume')
  test:assertRange(filter.highgain, 0.3, 0.4, 'check filter highgain')
  test:assertEquals(nil, filter.lowgain, 'check filter lowgain')

  -- add an effect
  local effsource = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.setEffect('testeffect', {
    type = 'flanger',
    volume = 0.75
  })
  local seteffect, err = effsource:setEffect('testeffect', {
    type = 'highpass',
    volume = 0.3,
    lowgain = 0.1
  })

  -- both these fail on 12 using stereo or mono, no err
  test:assertTrue(seteffect, 'check effect was applied')
  local filtersettings = effsource:getEffect('effectthatdoesntexist', {})
  test:assertNotNil(filtersettings)

  love.audio.stop(stereo)
  love.audio.stop(mono)
  love.audio.stop(effsource)

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.audio.getActiveEffects
love.test.audio.getActiveEffects = function(test)
  -- check we get a value
  test:assertNotNil(love.audio.getActiveEffects())
  -- check setting an effect active
  love.audio.setEffect('testeffect', {
    type = 'chorus',
    volume = 0.75
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
  love.audio.stop(testsource)
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
    volume = 0.75
  })
  test:assertNotNil(love.audio.getEffect('testeffect'))
  -- check effect values match creation values
  test:assertEquals('chorus', love.audio.getEffect('testeffect').type, 'check effect type')
  test:assertEquals(0.75, love.audio.getEffect('testeffect').volume, 'check effect volume')
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


-- love.audio.getPlaybackDevice
love.test.audio.getPlaybackDevice = function(test)
  test:assertNotNil(love.audio.getPlaybackDevice)
  test:assertNotNil(love.audio.getPlaybackDevice())
end


-- love.audio.getPlaybackDevices
love.test.audio.getPlaybackDevices = function(test)
  test:assertNotNil(love.audio.getPlaybackDevices)
  test:assertGreaterEqual(0, #love.audio.getPlaybackDevices(), 'check table')
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
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.audio.newQueueableSource = function(test)
  test:assertObject(love.audio.newQueueableSource(32, 8, 1, 8))
end


-- love.audio.newSource
-- @NOTE this is just basic nil checking, objs have their own test method
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
  love.audio.stop(source)
end


-- love.audio.play
love.test.audio.play = function(test)
  -- check playing source is detected
  local source = love.audio.newSource('resources/click.ogg', 'static')
  love.audio.play(source)
  test:assertTrue(source:isPlaying(), 'check something playing')
  love.audio.stop()
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
    volume = 0.75
  })
  test:assertTrue(effect, 'check effect created')
  -- check values set match
  local settings = love.audio.getEffect('testeffect')
  test:assertEquals('chorus', settings.type, 'check effect type')
  test:assertEquals(0.75, settings.volume, 'check effect volume')
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


-- love.audio.setPlaybackDevice
love.test.audio.setPlaybackDevice = function(test)
  -- check method
  test:assertNotNil(love.audio.setPlaybackDevice)

  -- check blank string name
  test:assertTrue(love.audio.setPlaybackDevice(''), 'check blank device is fine')

  -- check invalid name
  test:assertFalse(love.audio.setPlaybackDevice('loveFM'), 'check invalid device fails')

  -- check setting already set
  test:assertTrue(love.audio.setPlaybackDevice(love.audio.getPlaybackDevice()), 'check existing device is fine')
  
  -- if other devices to play with lets set a different one
  local devices = love.audio.getPlaybackDevices()
  if #devices > 1 then
    local another = ''
    local current = love.audio.getPlaybackDevice()
    for a=1,#devices do
      if devices[a] ~= current then
        another = devices[a]
        break
      end
    end
    if another ~= '' then
      -- check setting new device
      local success4, msg4 = love.audio.setPlaybackDevice(another)
      test:assertTrue(success4, 'check setting different device')
      -- check resetting to default
      local success5, msg5 = love.audio.setPlaybackDevice()
      test:assertTrue(success5, 'check resetting')
      test:assertEquals(current, love.audio.getPlaybackDevice())
    end
  end
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
  test:assertTrue(source:isPlaying(), 'check is playing')
  -- check source is then stopped
  love.audio.stop()
  test:assertFalse(source:isPlaying(), 'check stopped playing')
end
