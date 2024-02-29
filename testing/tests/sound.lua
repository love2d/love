-- love.sound


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Decoder (love.sound.newDecoder)
love.test.sound.Decoder = function(test)

  -- create obj
  local decoder = love.sound.newDecoder('resources/click.ogg')
  test:assertObject(decoder)

  -- check bit depth
  test:assertMatch({8, 16}, decoder:getBitDepth(), 'check bit depth')

  -- check channel count
  test:assertMatch({1, 2}, decoder:getChannelCount(), 'check channel count')

  -- check duration
  test:assertRange(decoder:getDuration(), 0.06, 0.07, 'check duration')

  -- check sample rate
  test:assertEquals(44100, decoder:getSampleRate(), 'check sample rate')

  -- check makes sound data (test in method below)
  test:assertObject(decoder:decode())

  -- check cloning sound
  local clone = decoder:clone()
  test:assertMatch({8, 16}, clone:getBitDepth(), 'check cloned bit depth')
  test:assertMatch({1, 2}, clone:getChannelCount(), 'check cloned channel count')
  test:assertRange(clone:getDuration(), 0.06, 0.07, 'check cloned duration')
  test:assertEquals(44100, clone:getSampleRate(), 'check cloned sample rate')

end


-- SoundData (love.sound.newSoundData)
love.test.sound.SoundData = function(test)

  -- create obj
  local sdata = love.sound.newSoundData('resources/click.ogg')
  test:assertObject(sdata)

  -- check data size + string
  test:assertEquals(11708, sdata:getSize(), 'check size')
  test:assertNotNil(sdata:getString())

  -- check bit depth
  test:assertMatch({8, 16}, sdata:getBitDepth(), 'check bit depth')

  -- check channel count
  test:assertMatch({1, 2}, sdata:getChannelCount(), 'check channel count')

  -- check duration
  test:assertRange(sdata:getDuration(), 0.06, 0.07, 'check duration')

  -- check samples
  test:assertEquals(44100, sdata:getSampleRate(), 'check sample rate')
  test:assertEquals(2927, sdata:getSampleCount(), 'check sample count')

  -- check cloning
  local clone = sdata:clone()
  test:assertEquals(11708, clone:getSize(), 'check clone size')
  test:assertNotNil(clone:getString())
  test:assertMatch({8, 16}, clone:getBitDepth(), 'check clone bit depth')
  test:assertMatch({1, 2}, clone:getChannelCount(), 'check clone channel count')
  test:assertRange(clone:getDuration(), 0.06, 0.07, 'check clone duration')
  test:assertEquals(44100, clone:getSampleRate(), 'check clone sample rate')
  test:assertEquals(2927, clone:getSampleCount(), 'check clone sample count')

  -- check sample setting
  test:assertRange(sdata:getSample(0.001), -0.1, 0, 'check sample 1')
  test:assertRange(sdata:getSample(0.005), -0.1, 0, 'check sample 1')
  sdata:setSample(0.002, 1)
  test:assertEquals(1, sdata:getSample(0.002), 'check setting sample manually')

  -- check copying from another sound
  local copy1 = love.sound.newSoundData('resources/tone.ogg')
  local copy2 = love.sound.newSoundData('resources/pop.ogg')
  local before = copy2:getSample(0.02)
  copy2:copyFrom(copy1, 0.01, 1, 0.02)
  test:assertNotEquals(before, copy2:getSample(0.02), 'check changed')

  -- check slicing
  local count = math.floor(copy1:getSampleCount()/2)
  local slice = copy1:slice(0, count)
  test:assertEquals(count, slice:getSampleCount(), 'check slice length')

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------



-- love.sound.newDecoder
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.sound.newDecoder = function(test)
  test:assertObject(love.sound.newDecoder('resources/click.ogg'))
end


-- love.sound.newSoundData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.sound.newSoundData = function(test)
  test:assertObject(love.sound.newSoundData('resources/click.ogg'))
  test:assertObject(love.sound.newSoundData(math.floor((1/32)*44100), 44100, 16, 1))
end
