-- love.image


-- love.image.isCompressed
-- @NOTE really we need to test each of the files listed here:
-- https://love2d.org/wiki/CompressedImageFormat
love.test.image.isCompressed = function(test)
  local compressed = love.image.isCompressed('resources/love.dxt1')
  test:assertEquals(true, compressed, 'check dxt1 valid compressed image')
end


-- love.image.newCompressedData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.image.newCompressedData = function(test)
  local imgdata = love.image.newCompressedData('resources/love.dxt1')
  test:assertObject(imgdata)
  imgdata:release()
end


-- love.image.newImageData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.image.newImageData = function(test)
  local imgdata = love.image.newImageData('resources/love.png')
  local rawdata = love.image.newImageData(16, 16, 'rgba8', nil)
  test:assertObject(imgdata)
  test:assertObject(rawdata)
  imgdata:release()
  rawdata:release()
end