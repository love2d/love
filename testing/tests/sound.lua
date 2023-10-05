-- love.sound


-- love.sound.newDecoder
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.sound.newDecoder = function(test)
  test:assertObject(love.sound.newDecoder('resources/click.ogg'))
end


-- love.sound.newSoundData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.sound.newSoundData = function(test)
  test:assertObject(love.sound.newSoundData('resources/click.ogg'))
  test:assertObject(love.sound.newSoundData(math.floor((1/32)*44100), 44100, 16, 1))
end
