-- objects put in their own test methods to test all attributes and class methods


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------AUDIO---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- RecordingDevice (love.audio.getRecordingDevices)
love.test.objects.RecordingDevice = function(test)
  test:skipTest('test class needs writing')
end


-- Source (love.audio.newSource)
love.test.objects.Source = function(test)
  test:skipTest('test class needs writing')
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
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------DATA----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- ByteData (love.data.newByteData)
love.test.objects.ByteData = function(test)
  test:skipTest('test class needs writing')
end


-- CompressedData (love.data.compress)
love.test.objects.CompressedData = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
---------------------------------FILESYSTEM-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


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
  -- file1:open('a')
  -- ok, err = file1:setBuffer('full', 10000)
  -- test:assertEquals(true, ok)
  -- test:assertEquals('full', file1:getBuffer())
  -- file1:write('morecontent')
  -- file1:close()
  -- file1:open('r')
  -- contents, size = file1:read()
  -- test:assertEquals('helloworld', contents, 'check buffered content wasnt written')
  -- file1:close()

  -- @NOTE :close() commits buffer content so need to check before not after

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

end


-- FileData (love.filesystem.newFileData)
love.test.objects.FileData = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------FONT----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- GlyphData (love.font.newGlyphData)
love.test.objects.GlyphData = function(test)
  test:skipTest('test class needs writing')
end


-- Rasterizer (love.font.newRasterizer)
love.test.objects.Rasterizer = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
---------------------------------GRAPHICS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Canvas (love.graphics.newCanvas)
love.test.objects.Canvas = function(test)
  test:skipTest('test class needs writing')
end


-- Font (love.graphics.newFont)
love.test.objects.Font = function(test)
  test:skipTest('test class needs writing')
end


-- Image (love.graphics.newImage)
love.test.objects.Image = function(test)
  test:skipTest('test class needs writing')
end


-- Mesh (love.graphics.newMesh)
love.test.objects.Mesh = function(test)
  test:skipTest('test class needs writing')
end


-- ParticleSystem (love.graphics.newParticleSystem)
love.test.objects.ParticleSystem = function(test)
  test:skipTest('test class needs writing')
end


-- Quad (love.graphics.newQuad)
love.test.objects.Quad = function(test)
  test:skipTest('test class needs writing')
end


-- Shader (love.graphics.newShader)
love.test.objects.Shader = function(test)
  test:skipTest('test class needs writing')
end


-- SpriteBatch (love.graphics.newSpriteBatch)
love.test.objects.SpriteBatch = function(test)
  test:skipTest('test class needs writing')
end


-- Text (love.graphics.newTextBatch)
love.test.objects.Text = function(test)
  test:skipTest('test class needs writing')
end


-- Texture (love.graphics.newTexture)
love.test.objects.Texture = function(test)
  test:skipTest('test class needs writing')
end


-- Video (love.graphics.newVideo)
love.test.objects.Video = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-----------------------------------IMAGE----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- CompressedImageData (love.image.newCompressedImageData)
love.test.objects.CompressedImageData = function(test)
  test:skipTest('test class needs writing')
end


-- ImageData (love.image.newImageData)
love.test.objects.ImageData = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------MATH----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- BezierCurve (love.math.newBezierCurve)
love.test.objects.BezierCurve = function(test)
  test:skipTest('test class needs writing')
end


-- RandomGenerator (love.math.RandomGenerator)
love.test.objects.RandomGenerator = function(test)
  test:skipTest('test class needs writing')
end


-- Transform (love.math.Transform)
love.test.objects.Transform = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------PHYSICS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Body (love.physics.newBody)
love.test.objects.Body = function(test)
  test:skipTest('test class needs writing')
end


-- Contact (love.physics.World:getContacts)
love.test.objects.Contact = function(test)
  test:skipTest('test class needs writing')
end


-- Fixture (love.physics.newFixture)
love.test.objects.Fixture = function(test)
  test:skipTest('test class needs writing')
end


-- Joint (love.physics.newDistanceJoint)
love.test.objects.Joint = function(test)
  test:skipTest('test class needs writing')
end


-- Shape (love.physics.newCircleShape)
love.test.objects.Shape = function(test)
  test:skipTest('test class needs writing')
end


-- World (love.physics.newWorld)
love.test.objects.World = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-----------------------------------SOUND----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Decoder (love.sound.newDecoder)
love.test.objects.Decoder = function(test)
  test:skipTest('test class needs writing')
end


-- SoundData (love.sound.newSoundData)
love.test.objects.SoundData = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------THREAD----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Channel (love.thread.newChannel)
love.test.objects.Channel = function(test)
  test:skipTest('test class needs writing')
end


-- Thread (love.thread.newThread)
love.test.objects.Thread = function(test)
  test:skipTest('test class needs writing')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-----------------------------------VIDEO----------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- VideoStream (love.thread.newVideoStream)
love.test.objects.VideoStream = function(test)
  test:skipTest('test class needs writing')
end
