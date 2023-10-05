-- love.image


-- love.image.isCompressed
-- @NOTE really we need to test each of the files listed here:
-- https://love2d.org/wiki/CompressedImageFormat
-- also need to be platform dependent (e.g. dxt not suppored on phones)
love.test.image.isCompressed = function(test)
  test:assertEquals(true, love.image.isCompressed('resources/love.dxt1'), 
    'check dxt1 valid compressed image')
end


-- love.image.newCompressedData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.image.newCompressedData = function(test)
  test:assertObject(love.image.newCompressedData('resources/love.dxt1'))
end


-- love.image.newImageData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.image.newImageData = function(test)
  test:assertObject(love.image.newImageData('resources/love.png'))
  test:assertObject(love.image.newImageData(16, 16, 'rgba8', nil))
end
