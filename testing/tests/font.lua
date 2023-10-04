-- love.font


-- love.font.newBMFontRasterizer
-- @NOTE the wiki specifies diff. params to source code and trying to do 
-- what source code wants gives some errors still
love.test.font.newBMFontRasterizer = function(test)
  test:skipTest('wiki and source dont match, not sure expected usage')
end


-- love.font.newGlyphData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.font.newGlyphData = function(test)
  local img = love.image.newImageData('resources/love.png')
  local rasterizer = love.font.newImageRasterizer(img, 'ABC', 0, 1);
  local glyphdata = love.font.newGlyphData(rasterizer, 65)
  test:assertObject(glyphdata)
  img:release()
  rasterizer:release()
  glyphdata:release()
end


-- love.font.newImageRasterizer
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.font.newImageRasterizer = function(test)
  local img = love.image.newImageData('resources/love.png')
  local rasterizer = love.font.newImageRasterizer(img, 'ABC', 0, 1);
  test:assertObject(rasterizer)
  img:release()
  rasterizer:release()
end


-- love.font.newRasterizer
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.font.newRasterizer = function(test)
  local rasterizer = love.font.newRasterizer('resources/font.ttf');
  test:assertObject(rasterizer)
  rasterizer:release()
end


-- love.font.newTrueTypeRasterizer
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.font.newTrueTypeRasterizer = function(test)
  local defaltraster = love.font.newTrueTypeRasterizer(12, "normal", 1)
  local customraster = love.font.newTrueTypeRasterizer('resources/font.ttf', 8, "normal", 1)
  test:assertObject(defaltraster)
  test:assertObject(customraster)
  defaltraster:release()
  customraster:release()
end