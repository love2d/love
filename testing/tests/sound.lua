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
  -- check decoder props
  test:assertMatch({8, 16}, decoder:getBitDepth(), 'check bit depth')
  test:assertMatch({1, 2}, decoder:getChannelCount(), 'check channel count')
  test:assertEquals(66, math.floor(decoder:getDuration()*1000), 'check duration')
  test:assertEquals(44100, decoder:getSampleRate(), 'check sample rate')
  -- check makes sound data (test in method below)
  test:assertObject(decoder:decode())
  -- check cloning sound
  local clone = decoder:clone()
  test:assertMatch({8, 16}, clone:getBitDepth(), 'check cloned bit depth')
  test:assertMatch({1, 2}, clone:getChannelCount(), 'check cloned channel count')
  test:assertEquals(66, math.floor(clone:getDuration()*1000), 'check cloned duration')
  test:assertEquals(44100, clone:getSampleRate(), 'check cloned sample rate')
end


-- SoundData (love.sound.newSoundData)
love.test.sound.SoundData = function(test)
  -- create obj
  local sdata = love.sound.newSoundData('resources/click.ogg')
  test:assertObject(sdata)
  -- check data props
  test:assertEquals(11708, sdata:getSize(), 'check size')
  test:assertNotNil(sdata:getString())
  test:assertMatch({8, 16}, sdata:getBitDepth(), 'check bit depth')
  test:assertMatch({1, 2}, sdata:getChannelCount(), 'check channel count')
  test:assertEquals(66, math.floor(sdata:getDuration()*1000), 'check duration')
  test:assertEquals(44100, sdata:getSampleRate(), 'check sample rate')
  test:assertEquals(2927, sdata:getSampleCount(), 'check sample count')
  -- check cloning
  local clone = sdata:clone()
  test:assertEquals(11708, clone:getSize(), 'check clone size')
  test:assertNotNil(clone:getString())
  test:assertMatch({8, 16}, clone:getBitDepth(), 'check clone bit depth')
  test:assertMatch({1, 2}, clone:getChannelCount(), 'check clone channel count')
  test:assertEquals(66, math.floor(clone:getDuration()*1000), 'check clone duration')
  test:assertEquals(44100, clone:getSampleRate(), 'check clone sample rate')
  test:assertEquals(2927, clone:getSampleCount(), 'check clone sample count')
  -- check sample setting
  test:assertEquals(-22, math.floor(sdata:getSample(0.001)*100000), 'check sample 1')
  test:assertEquals(-22, math.floor(sdata:getSample(0.005)*100000), 'check sample 1')
  sdata:setSample(0.002, 1)
  test:assertEquals(1, sdata:getSample(0.002), 'check setting sample manually')
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
