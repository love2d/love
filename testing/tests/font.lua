-- love.font


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- GlyphData (love.font.newGlyphData)
love.test.font.GlyphData = function(test)

  -- create obj
  local rasterizer = love.font.newRasterizer('resources/font.ttf')
  local gdata = love.font.newGlyphData(rasterizer, 97) -- 'a'
  test:assertObject(gdata)

  -- check properties match expected
  test:assertNotNil(gdata:getString())
  test:assertEquals(128, gdata:getSize(), 'check data size')
  test:assertEquals(9, gdata:getAdvance(), 'check advance')
  test:assertEquals('la8', gdata:getFormat(), 'check format')

  -- @TODO 
  --[[
    currently these will return 0 and '' respectively as not implemented
    https://github.com/love2d/love/blob/12.0-development/src/modules/font/freetype/TrueTypeRasterizer.cpp#L140-L141
    "basically I haven't decided what to do here yet, because of the more 
    advanced text shaping that happens in love 12 having a unicode codepoint 
    associated with a glyph probably doesn't make sense in the first place"
  ]]--
  --test:assertEquals(97, gdata:getGlyph(), 'check glyph number') - returns 0
  --test:assertEquals('a', gdata:getGlyphString(), 'check glyph string') - returns ''

  -- check height + width
  test:assertEquals(8, gdata:getHeight(), 'check height')
  test:assertEquals(8, gdata:getWidth(), 'check width')

  -- check boundary / dimensions
  local x, y, w, h = gdata:getBoundingBox()
  local dw, dh = gdata:getDimensions()
  test:assertEquals(0, x, 'check bbox x')
  test:assertEquals(-3, y, 'check bbox y')
  test:assertEquals(8, w, 'check bbox w')
  test:assertEquals(14, h, 'check bbox h')
  test:assertEquals(8, dw, 'check dim width')
  test:assertEquals(8, dh, 'check dim height')

  -- check bearing
  local bw, bh = gdata:getBearing()
  test:assertEquals(0, bw, 'check bearing w')
  test:assertEquals(11, bh, 'check bearing h')

end


-- Rasterizer (love.font.newRasterizer)
love.test.font.Rasterizer = function(test)

  -- create obj
  local rasterizer = love.font.newRasterizer('resources/font.ttf')
  test:assertObject(rasterizer)

  -- check advance
  test:assertEquals(9, rasterizer:getAdvance(), 'check advance')

  -- check ascent/descent
  test:assertEquals(9, rasterizer:getAscent(), 'check ascent')
  test:assertEquals(-3, rasterizer:getDescent(), 'check descent')

  -- check glyphcount
  test:assertEquals(77, rasterizer:getGlyphCount(), 'check glyph count')

  -- check specific glyphs
  test:assertObject(rasterizer:getGlyphData('L'))
  test:assertTrue(rasterizer:hasGlyphs('L', 'O', 'V', 'E'), 'check LOVE')

  -- check height + lineheight
  test:assertEquals(12, rasterizer:getHeight(), 'check height')
  test:assertEquals(15, rasterizer:getLineHeight(), 'check line height')

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.font.newBMFontRasterizer
love.test.font.newBMFontRasterizer = function(test)
  local rasterizer = love.font.newBMFontRasterizer('resources/love.png');
  test:assertObject(rasterizer)
end


-- love.font.newGlyphData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.font.newGlyphData = function(test)
  local img = love.image.newImageData('resources/love.png')
  local rasterizer = love.font.newImageRasterizer(img, 'ABC', 0, 1);
  local glyphdata = love.font.newGlyphData(rasterizer, 65)
  test:assertObject(glyphdata)
end


-- love.font.newImageRasterizer
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.font.newImageRasterizer = function(test)
  local img = love.image.newImageData('resources/love.png')
  local rasterizer = love.font.newImageRasterizer(img, 'ABC', 0, 1);
  test:assertObject(rasterizer)
end


-- love.font.newRasterizer
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.font.newRasterizer = function(test)
  test:assertObject(love.font.newRasterizer('resources/font.ttf'))
end


-- love.font.newTrueTypeRasterizer
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.font.newTrueTypeRasterizer = function(test)
  test:assertObject(love.font.newTrueTypeRasterizer(12, "normal", 1))
  test:assertObject(love.font.newTrueTypeRasterizer('resources/font.ttf', 8, "normal", 1))
end
