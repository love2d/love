-- love.image


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- CompressedImageData (love.image.newCompressedImageData)
love.test.image.CompressedImageData = function(test)

  -- create obj
  local idata = love.image.newCompressedData('resources/love.dxt1')
  test:assertObject(idata)

  -- check string + size
  test:assertNotEquals(nil, idata:getString(), 'check data string')
  test:assertEquals(2744, idata:getSize(), 'check data size')

  -- check img dimensions
  local iw, ih = idata:getDimensions()
  test:assertEquals(64, iw, 'check image dimension w')
  test:assertEquals(64, ih, 'check image dimension h')
  test:assertEquals(64, idata:getWidth(), 'check image direct w')
  test:assertEquals(64, idata:getHeight(), 'check image direct h')

  -- check format
  test:assertEquals('DXT1', idata:getFormat(), 'check image format')

  -- check mipmap count
  test:assertEquals(7, idata:getMipmapCount(), 'check mipmap count')

  -- check linear
  test:assertFalse(idata:isLinear(), 'check not linear')
  idata:setLinear(true)
  test:assertTrue(idata:isLinear(), 'check now linear')

end


-- ImageData (love.image.newImageData)
love.test.image.ImageData = function(test)

  -- create obj
  local idata = love.image.newImageData('resources/love.png')
  test:assertObject(idata)

  -- check string + size
  test:assertNotEquals(nil, idata:getString(), 'check data string')
  test:assertEquals(16384, idata:getSize(), 'check data size')

  -- check img dimensions
  local iw, ih = idata:getDimensions()
  test:assertEquals(64, iw, 'check image dimension w')
  test:assertEquals(64, ih, 'check image dimension h')
  test:assertEquals(64, idata:getWidth(), 'check image direct w')
  test:assertEquals(64, idata:getHeight(), 'check image direct h')

  -- check format
  test:assertEquals('rgba8', idata:getFormat(), 'check image format')
  
  -- manipulate image data so white heart is black
  local mapdata = function(x, y, r, g, b, a)
    if r == 1 and g == 1 and b == 1 then
      r = 0; g = 0; b = 0
    end
    return r, g, b, a
  end
  idata:mapPixel(mapdata, 0, 0, 64, 64)
  local r1, g1, b1 = idata:getPixel(25, 25)
  test:assertEquals(0, r1+g1+b1, 'check mapped black')

  -- map some other data into the idata
  local idata2 = love.image.newImageData('resources/loveinv.png')
  idata:paste(idata2, 0, 0, 0, 0)
  r1, g1, b1 = idata:getPixel(25, 25)
  test:assertEquals(3, r1+g1+b1, 'check back to white')

  -- set pixels directly
  idata:setPixel(25, 25, 1, 0, 0, 1)
  local r2, g2, b2 = idata:getPixel(25, 25)
  test:assertEquals(1, r2+g2+b2, 'check set to red')

  -- check encoding to an image (png)
  idata:encode('png', 'test-encode.png')
  local read1 = love.filesystem.openFile('test-encode.png', 'r')
  test:assertNotNil(read1)
  love.filesystem.remove('test-encode.png')

  -- check encoding to an image (exr)
  local edata = love.image.newImageData(100, 100, 'r16f')
  edata:encode('exr', 'test-encode.exr')
  local read2 = love.filesystem.openFile('test-encode.exr', 'r')
  test:assertNotNil(read2)
  love.filesystem.remove('test-encode.exr')

  -- check linear
  test:assertFalse(idata:isLinear(), 'check not linear')
  idata:setLinear(true)
  test:assertTrue(idata:isLinear(), 'check now linear')

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.image.isCompressed
-- @NOTE really we need to test each of the files listed here:
-- https://love2d.org/wiki/CompressedImageFormat
-- also need to be platform dependent (e.g. dxt not suppored on phones)
love.test.image.isCompressed = function(test)
  test:assertTrue(love.image.isCompressed('resources/love.dxt1'), 
    'check dxt1 valid compressed image')
end


-- love.image.newCompressedData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.image.newCompressedData = function(test)
  test:assertObject(love.image.newCompressedData('resources/love.dxt1'))
end


-- love.image.newImageData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.image.newImageData = function(test)
  test:assertObject(love.image.newImageData('resources/love.png'))
  test:assertObject(love.image.newImageData(16, 16, 'rgba8', nil))
end
