-- objects put in their own test methods to test all attributes and class methods


-- File (love.filesystem.newFile)
love.test.objects.File = function(test)

  -- setup a file to play with
  local file1 = love.filesystem.openFile('data.txt', 'w')
  file1:write('helloworld')
  test:assertObject(file1)
  file1:close()

  -- test read mode
  file1:open('r')
  test:assertEquals('r', file1:getMode(), 'check read mode')
  local contents, size = file1:read()
  test:assertEquals('helloworld', contents)
  test:assertEquals(10, size, 'check file read')
  test:assertEquals(10, file1:getSize())
  local ok, err = file1:write('hello')
  test:assertNotEquals(nil, err, 'check cant write in read mode')
  local iterator = file1:lines()
  test:assertNotEquals(nil, iterator, 'check can read lines')
  test:assertEquals('data.txt', file1:getFilename(), 'check filename matches')
  file1:close()

  -- test write mode
  file1:open('w')
  test:assertEquals('w', file1:getMode(), 'check write mode')
  contents, size = file1:read()
  test:assertEquals(nil, contents, 'check cant read file in write mode')
  test:assertEquals('string', type(size), 'check err message shown')
  ok, err = file1:write('helloworld')
  test:assertEquals(true, ok, 'check file write')
  test:assertEquals(nil, err, 'check no err writing')

  -- test open/closing
  file1:open('r')
  test:assertEquals(true, file1:isOpen(), 'check file is open')
  file1:close()
  test:assertEquals(false, file1:isOpen(), 'check file gets closed')
  file1:close()

  -- test buffering 
  -- @NOTE think I'm just not understanding how this is supposed to work?
  -- I thought if buffering is enabled then nothing should get written until 
  -- buffer overflows?
  file1:open('a')
  ok, err = file1:setBuffer('full', 10000)
  test:assertEquals(true, ok)
  test:assertEquals('full', file1:getBuffer())
  file1:write('morecontent')
  file1:close()
  file1:open('r')
  contents, size = file1:read()
  test:assertEquals('helloworld', contents, 'check buffered content wasnt written')
  file1:close()

  -- test buffering and flushing
  file1:open('w')
  ok, err = file1:setBuffer('full', 10000)
  test:assertEquals(true, ok)
  test:assertEquals('full', file1:getBuffer())
  file1:write('replacedcontent')
  file1:flush()
  file1:close()
  file1:open('r')
  contents, size = file1:read()
  test:assertEquals('replacedcontent', contents, 'check buffered content was written')
  file1:close()

  -- loop through file data with seek/tell until EOF
  file1:open('r')
  local counter = 0
  for i=1,100 do
    file1:seek(i)
    test:assertEquals(i, file1:tell())
    if file1:isEOF() == true then
      counter = i
      break
    end
  end
  test:assertEquals(counter, 15)
  file1:close()

  file1:release()

end


-- Source (love.audio.newSource)
-- love.test.objects.Source = function(test)
  -- local source1 = love.audio.newSource('resources/click.ogg', 'static')
  --source1:clone()
  --source1:getChannelCount()
  --source1:getDuration()
  --source1:isRelative()
  --source1:queue()
  --source1:getFreeBufferCount()
  --source1:getType()
  --source1:isPlaying()
  --source1:play()
  --source1:pause()
  --source1:stop()
  --source1:seek()
  --source1:tell()
  --source1:isLooping()
  --source1:setLooping()
  --source1:setAirAbsorption()
  --source1:getAirAbsorption()
  --source1:setAttenuationDistances()
  --source1:getAttenuationDistances()
  --source1:setCone()
  --source1:getCone()
  --source1:setDirection()
  --source1:getDirection()
  --source1:setEffect()
  --source1:getEffect()
  --source1:getActiveEffects()
  --source1:setFilter()
  --source1:getFilter()
  --source1:setPitch()
  --source1:getPitch()
  --source1:setPosition()
  --source1:getPosition()
  --source1:setRelative()
  --source1:setRolloff()
  --source1:getRolloff()
  --source1:setVelocity()
  --source1:getVelocity()
  --source1:setVolume()
  --source1:getVolume()
  --source1:setVolumeLimits()
  --source1:getVolumeLimits()
-- end

-- FileData (love.filesystem.newFileData)

-- ByteData (love.data.newByteData)
-- DataView (love.data.newDataView)

-- FontData (love.font.newFontData)
-- GlyphData (love.font.newGlyphData)
-- Rasterizer (love.font.newRasterizer)

-- CompressedImageData (love.image.newCompressedImageData)
-- ImageData (love.image.newImageData)

-- BezierCurve (love.math.newBezierCurve)
-- RandomGenerator (love.math.RandomGenerator)
-- Transform (love.math.Transform)

-- Decoder (love.sound.newDecoder)
-- SoundData (love.sound.newSoundData)

-- Channel (love.thread.newChannel)
-- Thread (love.thread.newThread)

-- VideoStream (love.thread.newVideoStream)

-- all the stuff from love.physics! barf

-- (love.graphics objs)
-- Canvas
-- Font
-- Image
-- Framebugger
-- Mesh
-- ParticleSystem
-- PixelEffect
-- Quad
-- Shader
-- SpriteBatch
-- Text
-- Video