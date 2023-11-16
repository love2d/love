-- love.graphics


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Canvas (love.graphics.newCanvas)
love.test.graphics.Canvas = function(test)
  -- create canvas with defaults
  local canvas = love.graphics.newCanvas(100, 100, {
    type = '2d',
    format = 'normal',
    readable = true,
    msaa = 0,
    dpiscale = love.graphics.getDPIScale(),
    mipmaps = 'auto'
  })
  test:assertObject(canvas)
  -- check texture settings
  test:assertEquals(love.graphics.getDPIScale(), canvas:getDPIScale(), 'check dpi scale')
  test:assertEquals(1, canvas:getDepth(), 'check depth is 2d')
  test:assertEquals(nil, canvas:getDepthSampleMode(), 'check depth sample nil')
  local min, mag, ani = canvas:getFilter()
  test:assertEquals('nearest', min, 'check filter def min')
  test:assertEquals('nearest', mag, 'check filter def mag')
  test:assertEquals(1, ani, 'check filter def ani')
  canvas:setFilter('linear', 'linear', 2)
  min, mag, ani = canvas:getFilter()
  test:assertEquals('linear', min, 'check filter changed min')
  test:assertEquals('linear', mag, 'check filter changed mag')
  test:assertEquals(2, ani, 'check filter changed ani')
  test:assertEquals(1, canvas:getLayerCount(), 'check 1 layer for 2d')
  test:assertEquals('2d', canvas:getTextureType(), 'check 2d')
  local horiz, vert = canvas:getWrap()
  test:assertEquals('clamp', horiz, 'check def wrap h')
  test:assertEquals('clamp', vert, 'check def wrap v')
  canvas:setWrap('repeat', 'repeat')
  horiz, vert = canvas:getWrap()
  test:assertEquals('repeat', horiz, 'check changed wrap h')
  test:assertEquals('repeat', vert, 'check changed wrap v')
  test:assertEquals(true, canvas:isReadable(), 'check canvas readable')
  test:assertEquals(1, canvas:getMSAA(), 'check samples match')
  -- check dimensions
  local cw, ch = canvas:getDimensions()
  test:assertEquals(100, cw, 'check canvas dim w')
  test:assertEquals(100, ch, 'check canvas dim h')
  test:assertEquals(cw, canvas:getWidth(), 'check canvas w matches dim')
  test:assertEquals(ch, canvas:getHeight(), 'check canvas h matches dim')
  local pw, ph = canvas:getPixelDimensions()
  test:assertEquals(100*love.graphics.getDPIScale(), pw, 'check pixel dim w')
  test:assertEquals(100*love.graphics.getDPIScale(), ph, 'check pixel dim h')
  test:assertEquals(pw, canvas:getPixelWidth(), 'check pixel w matches dim')
  test:assertEquals(ph, canvas:getPixelHeight(), 'check pixel h matches dim')
  -- check mipmaps
  local mode, sharpness = canvas:getMipmapFilter()
  test:assertEquals('linear', mode, 'check def minmap filter  mode')
  test:assertEquals(0, sharpness, 'check def minmap filter sharpness')
  local name, version, vendor, device = love.graphics.getRendererInfo()
  canvas:setMipmapFilter('nearest', 1)
  mode, sharpness = canvas:getMipmapFilter()
  test:assertEquals('nearest', mode, 'check changed minmap filter  mode')
  -- mipmap sharpness wont work on opengl/metal
  if string.match(name, 'OpenGL ES') == nil and string.match(name, 'Metal') == nil then
    test:assertEquals(1, sharpness, 'check changed minmap filter sharpness')
  end
  test:assertGreaterEqual(2, canvas:getMipmapCount()) -- docs say no mipmaps should return 1
  test:assertEquals('auto', canvas:getMipmapMode())
  -- check rendering
  canvas:renderTo(function()
    love.graphics.setColor(1, 0, 0)
    love.graphics.rectangle('fill', 0, 0, 200, 200)
    love.graphics.setColor(1, 1, 1, 1)
  end)
  local data = love.graphics.readbackTexture(canvas, {100, 0, 0, 0, 100, 100})
  local r, g, b, a = data:getPixel(2, 2)
  test:assertEquals(1, r, 'check canvas draw r')
  test:assertEquals(0, g, 'check canvas draw g')
  test:assertEquals(0, b, 'check canvas draw b')
  test:assertEquals(1, a, 'check canvas draw a')
  -- check using canvas in love.graphics.draw()
  local xcanvas = love.graphics.newCanvas()
  love.graphics.setCanvas(xcanvas)
    love.graphics.draw(canvas, 0, 0)
  love.graphics.setCanvas()
  data = love.graphics.readbackTexture(canvas, {100, 0, 0, 0, 100, 100})
  r, g, b, a = data:getPixel(2, 2)
  test:assertEquals(1, r, 'check canvas draw r')
  test:assertEquals(0, g, 'check canvas draw g')
  test:assertEquals(0, b, 'check canvas draw b')
  test:assertEquals(1, a, 'check canvas draw a')
  -- check depth samples
  local dcanvas = love.graphics.newCanvas(100, 100, {
    type = '2d',
    format = 'depth16',
    readable = true
  })
  test:assertEquals(nil, dcanvas:getDepthSampleMode(), 'check depth sample mode nil by def')
  dcanvas:setDepthSampleMode('equal')
  test:assertEquals('equal', dcanvas:getDepthSampleMode(), 'check depth sample mode set')
end


-- Font (love.graphics.newFont)
love.test.graphics.Font = function(test)
  -- create obj
  local font = love.graphics.newFont('resources/font.ttf', 8)
  test:assertObject(font)
  -- check properties match expected
  test:assertEquals(6, font:getAscent(), 'check ascent')
  test:assertEquals(6, font:getBaseline(), 'check baseline')
  test:assertEquals(1, font:getDPIScale(), 'check dpi')
  test:assertEquals(-2, font:getDescent(), 'check descent')
  test:assertEquals('nearest', font:getFilter(), 'check filter def')
  font:setFilter('linear', 'linear')
  test:assertEquals('linear', font:getFilter(), 'check filter change')
  font:setFilter('nearest', 'nearest')
  test:assertEquals(8, font:getHeight(), 'check height')
  test:assertEquals(0, font:getKerning('a', 'b'), 'check kerning')
  test:assertEquals(1, font:getLineHeight(), 'check line height')
  font:setLineHeight(2)
  test:assertEquals(2, font:getLineHeight(), 'check changed line height')
  font:setLineHeight(1) -- reset for drawing + wrap later
  test:assertEquals(24, font:getWidth('test'), 'check data size')
  test:assertEquals(true, font:hasGlyphs('test'), 'check data size')
  -- check font wrapping
  local width, wrappedtext = font:getWrap('LÖVE is an *awesome* framework you can use to make 2D games in Lua.', 50)
  test:assertEquals(48, width, 'check actual wrap width')
  test:assertEquals(8, #wrappedtext, 'check wrapped lines')
  test:assertEquals('LÖVE is an ', wrappedtext[1], 'check wrapped line')
  -- check drawing font 
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.setFont(font)
    love.graphics.print('Aa', 0, 5)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    white = {{0,3},{4,3},{7,4},{9,4},{10,5},{0,8},{4,8},{10,8}},
  }, 'font draw check')
  test:exportImg(imgdata)
  -- check font substitution
  local fontab = love.graphics.newImageFont('resources/font-letters-ab.png', 'AB')
  local fontcd = love.graphics.newImageFont('resources/font-letters-cd.png', 'CD')
  fontab:setFallbacks(fontcd)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 0)
    love.graphics.setFont(fontab)
    love.graphics.print('AB', 0, 0)
    love.graphics.print('CD', 0, 9)
  love.graphics.setCanvas()
  local imgdata2 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata2, {
    green = {{1,8},{6,8},{2,10},{5,10},{9,10}},
    black = {{9,9},{14,8},{14,10},{14,1},{1,10}}
  }, 'font draw check')
  test:exportImg(imgdata2)
end


-- Image (love.graphics.newImage)
love.test.graphics.Image = function(test)
  -- create object
  local image = love.graphics.newImage('resources/love.png', {
    dpiscale = 1,
    mipmaps = true
  })
  test:assertObject(image)
  -- check texture props
  test:assertEquals(love.graphics.getDPIScale(), image:getDPIScale(), 'check dpi scale')
  test:assertEquals(1, image:getDepth(), 'check depth is 2d')
  test:assertEquals(nil, image:getDepthSampleMode(), 'check depth sample nil')
  local min, mag, ani = image:getFilter()
  test:assertEquals('nearest', min, 'check filter def min')
  test:assertEquals('nearest', mag, 'check filter def mag')
  test:assertEquals(1, ani, 'check filter def ani')
  image:setFilter('linear', 'linear', 2)
  min, mag, ani = image:getFilter()
  test:assertEquals('linear', min, 'check filter changed min')
  test:assertEquals('linear', mag, 'check filter changed mag')
  test:assertEquals(2, ani, 'check filter changed ani')
  test:assertEquals(1, image:getLayerCount(), 'check 1 layer for 2d')
  test:assertEquals('2d', image:getTextureType(), 'check 2d')
  local horiz, vert = image:getWrap()
  test:assertEquals('clamp', horiz, 'check def wrap h')
  test:assertEquals('clamp', vert, 'check def wrap v')
  image:setWrap('repeat', 'repeat')
  horiz, vert = image:getWrap()
  test:assertEquals('repeat', horiz, 'check changed wrap h')
  test:assertEquals('repeat', vert, 'check changed wrap v')
  test:assertEquals(true, image:isReadable(), 'check canvas readable')
  test:assertEquals(1, image:getMSAA(), 'check samples match')
  -- check dimensions
  local cw, ch = image:getDimensions()
  test:assertEquals(64, cw, 'check canvas dim w')
  test:assertEquals(64, ch, 'check canvas dim h')
  test:assertEquals(cw, image:getWidth(), 'check canvas w matches dim')
  test:assertEquals(ch, image:getHeight(), 'check canvas h matches dim')
  local pw, ph = image:getPixelDimensions()
  test:assertEquals(64*love.graphics.getDPIScale(), pw, 'check pixel dim w')
  test:assertEquals(64*love.graphics.getDPIScale(), ph, 'check pixel dim h')
  test:assertEquals(pw, image:getPixelWidth(), 'check pixel w matches dim')
  test:assertEquals(ph, image:getPixelHeight(), 'check pixel h matches dim')
  -- check mipmaps
  local mode, sharpness = image:getMipmapFilter()
  test:assertEquals('linear', mode, 'check def minmap filter  mode')
  test:assertEquals(0, sharpness, 'check def minmap filter sharpness')
  
  local name, version, vendor, device = love.graphics.getRendererInfo()
  -- mipmap sharpness wont work on opengl/metal
  image:setMipmapFilter('nearest', 1)
  mode, sharpness = image:getMipmapFilter()
  test:assertEquals('nearest', mode, 'check changed minmap filter  mode')
  if string.match(name, 'OpenGL ES') == nil and string.match(name, 'Metal') == nil then
    test:assertEquals(1, sharpness, 'check changed minmap filter sharpness')
  end
  test:assertGreaterEqual(2, image:getMipmapCount()) -- docs say no mipmaps should return 1
  -- check image properties
  test:assertEquals(false, image:isCompressed(), 'check not compressed')
  test:assertEquals(false, image:isFormatLinear(), 'check not linear')
  local cimage = love.graphics.newImage('resources/love.dxt1')
  test:assertObject(cimage)
  test:assertEquals(true, cimage:isCompressed(), 'check is compressed')
  -- check pixel replacement
  local rimage = love.image.newImageData('resources/loveinv.png')
  image:replacePixels(rimage)
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.draw(image, 0, 0)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  local r1, g1, b1 = imgdata:getPixel(25, 25)
  test:assertEquals(3, r1+g1+b1, 'check back to white')
  test:exportImg(imgdata)
end


-- Mesh (love.graphics.newMesh)
love.test.graphics.Mesh = function(test)
  test:skipTest('test class needs writing')
end


-- ParticleSystem (love.graphics.newParticleSystem)
love.test.graphics.ParticleSystem = function(test)
  test:skipTest('test class needs writing')
end


-- Quad (love.graphics.newQuad)
love.test.graphics.Quad = function(test)
  local texture = love.graphics.newImage('resources/love.png')
  local quad = love.graphics.newQuad(0, 0, 32, 32, texture)
  test:assertObject(quad)
  -- check properties
  test:assertEquals(1, quad:getLayer(), 'check default layer')
  quad:setLayer(2)
  test:assertEquals(2, quad:getLayer(), 'check changed layer')
  local sw, sh = quad:getTextureDimensions()
  test:assertEquals(64, sw, 'check texture w')
  test:assertEquals(64, sh, 'check texture h')
  -- check drawing and viewport changes
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.draw(texture, quad, 0, 0)
    quad:setViewport(32, 32, 32, 32, 64, 64)
    love.graphics.draw(texture, quad, 32, 32)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata, {
    white = {{17,31},{31,31},{31,24},{32,32},{46,32},{32,46}},
    lovepink = {{2,31},{31,2}},
    loveblue = {{32,61},{61,32}}
  }, 'check quad drawing')
  test:exportImg(imgdata)
end


-- Shader (love.graphics.newShader)
love.test.graphics.Shader = function(test)
  -- check valid shader
  local pixelcode1 = [[
    extern Image tex2;
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texturecolor = Texel(tex2, texture_coords); 
      return texturecolor * color;
    }
  ]]
  local vertexcode1 = [[
    vec4 position(mat4 transform_projection, vec4 vertex_position) { 
      return transform_projection * vertex_position; 
    }
  ]]
  local shader1 = love.graphics.newShader(pixelcode1, vertexcode1)
  test:assertObject(shader1)
  test:assertEquals('vertex shader:\npixel shader:\n', shader1:getWarnings(), 'check shader valid')
  test:assertEquals(false, shader1:hasUniform('tex1'), 'check invalid uniform')
  test:assertEquals(true, shader1:hasUniform('tex2'), 'check valid uniform')
  -- check invalid shader
  local pixelcode2 = [[
    extern float ww;
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texturecolor = Texel(tex, texture_coords);
      float unused = ww * 3 * color;
      return texturecolor * color;
    }
  ]]
  local res, err = pcall(love.graphics.newShader, pixelcode2, vertexcode1)
  test:assertNotEquals(nil, err, 'check shader compile fails')
  -- check using a shader to draw + sending uniforms
  -- shader will return a given color if overwrite set to 1, otherwise def. draw
  local pixelcode3 = [[
    extern vec4 col;
    extern float overwrite;
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texcol = Texel(tex, texture_coords); 
      if (overwrite == 1.0) {
        return col;
      } else {
        return texcol * color;
      }
    }
  ]]
  local shader3 = love.graphics.newShader(pixelcode3, vertexcode1)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    -- set color to yellow
    love.graphics.setColor(1, 1, 0, 1)
    -- turn shader 'on' and use red to draw
    shader3:send('overwrite', 1)
    shader3:sendColor('col', {1, 0, 0, 1})
    love.graphics.setShader(shader3)
      love.graphics.rectangle('fill', 0, 0, 8, 8)
    love.graphics.setShader()
    -- turn shader 'off' and draw again
    shader3:send('overwrite', 0)
    love.graphics.setShader(shader3)
      love.graphics.rectangle('fill', 8, 8, 8, 8)
    love.graphics.setShader()
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {{1,1},{1,7},{7,7},{7,1}},
    yellow = {{8,8},{8,15},{15,15},{15,8}}
  }, 'shader draw check')
  test:exportImg(imgdata)
end


-- SpriteBatch (love.graphics.newSpriteBatch)
love.test.graphics.SpriteBatch = function(test)
  -- create batch
  local texture1 = love.graphics.newImage('resources/cubemap.png')
  local texture2 = love.graphics.newImage('resources/love.png')
  local quad1 = love.graphics.newQuad(32, 12, 1, 1, texture2) -- lovepink
  local quad2 = love.graphics.newQuad(32, 32, 1, 1, texture2) -- white
  local sbatch = love.graphics.newSpriteBatch(texture1, 5000)
  test:assertObject(sbatch)
  -- check basic props
  test:assertEquals(0, sbatch:getCount(), 'check batch size')
  test:assertEquals(5000, sbatch:getBufferSize(), 'check batch size')
  test:assertEquals(texture1:getWidth(), sbatch:getTexture():getWidth(), 'check texture match w')
  test:assertEquals(texture1:getHeight(), sbatch:getTexture():getHeight(), 'check texture match h')
  sbatch:setTexture(texture2)
  test:assertEquals(texture2:getWidth(), sbatch:getTexture():getWidth(), 'check texture change w')
  test:assertEquals(texture2:getHeight(), sbatch:getTexture():getHeight(), 'check texture change h')
  local r, g, b, a = sbatch:getColor()
  test:assertEquals(1, r, 'check initial color r')
  test:assertEquals(1, g, 'check initial color g')
  test:assertEquals(1, b, 'check initial color b')
  test:assertEquals(1, a, 'check initial color a')
  sbatch:setColor(1, 0, 0, 1)
  r, g, b, a = sbatch:getColor()
  test:assertEquals(1, r, 'check set color r')
  test:assertEquals(0, g, 'check set color g')
  test:assertEquals(0, b, 'check set color b')
  test:assertEquals(1, a, 'check set color a')
  -- check adding sprites
  local offset_x = 0
  local offset_y = 0
  local color = 'white'
  sbatch:setColor(1, 1, 1, 1)
  local sprites = {}
  for s=1,4096 do
    local spr = sbatch:add(quad1, offset_x, offset_y, 0, 1, 1)
    table.insert(sprites, {spr, offset_x, offset_y})
    offset_x = offset_x + 1
    if s % 64 == 0 then
      -- alternate row colors
      if color == 'white' then
        color = 'red'
        sbatch:setColor(1, 0, 0, 1)
      else
        color = 'white'
        sbatch:setColor(1, 1, 1, 1)
      end
      offset_y = offset_y + 1
      offset_x = 0
    end
  end
  test:assertEquals(4096, sbatch:getCount())
  -- test drawing and setting
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(sbatch, 0, 0)
  love.graphics.setCanvas()
  local imgdata1 = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata1, {
    lovepink = {{0,0},{63,2},{0,32},{63,32},{63,0},{63,2}}
  }, 'sbatch draw normal')
  test:exportImg(imgdata1)
  -- use set to change some sprites
  for s=1,2048 do
    sbatch:set(sprites[s][1], quad2, sprites[s][2], sprites[s][3]+1, 0, 1, 1)
  end
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(sbatch, 0, 0)
  love.graphics.setCanvas()
  local imgdata2 = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata2, {
    lovepink = {{0,32},{63,32}},
    black = {{0,0},{63,0}},
    white = {{0,1},{63,1},{0,31},{63,31}}
  }, 'sbatch draw set')
  test:exportImg(imgdata2)
  -- set drawRange and redraw
  sbatch:setDrawRange(1025, 2048)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(sbatch, 0, 0)
  love.graphics.setCanvas()
  local imgdata3 = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata3, {
    lovepink = {{0,32},{63,32}},
    black = {{0,0},{63,0},{0,48},{63,48}},
    white = {{0,17},{63,17},{0,31},{63,31}}
  }, 'sbatch draw drawrange')
  test:exportImg(imgdata3)
  -- clear and redraw
  sbatch:clear()
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(sbatch, 0, 0)
  love.graphics.setCanvas()
  local imgdata4 = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata4, {
    black = {{0,0},{63,0},{0,32},{63,32},{0,63},{63,63}},
  }, 'sbatch draw clear')
  test:exportImg(imgdata4)
  -- array texture sbatch
  local texture3 = love.graphics.newArrayImage({
    'resources/love.png',
    'resources/loveinv.png'
  })
  local asbatch = love.graphics.newSpriteBatch(texture3, 4096)
  local quad3 = love.graphics.newQuad(32, 52, 1, 1, texture3) -- loveblue
  sprites = {}
  for s=1,4096 do
    local spr = asbatch:addLayer(1, quad3, 0, s, math.floor(s/64), 1, 1)
    table.insert(sprites, {spr, s, math.floor(s/64)})
  end
  test:assertEquals(4096, asbatch:getCount(), 'check max batch size applies')
  for s=1,2048 do
    asbatch:setLayer(sprites[s][1], 2, sprites[s][2], sprites[s][3], 0, 1, 1)
  end
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(asbatch, 0, 0)
  love.graphics.setCanvas()
  local imgdata5 = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata5, {
    loveblue = {{31,2},{63,2},{3,30},{3,33},{16,47},{63,47}},
    lovepink = {{17,48},{63,48},{31,61},{63,61}},
    black = {{0,0},{63,0},{63,63},{63,0},{30,2},{30,61}},
  }, 'sbatch draw layers')
  test:exportImg(imgdata5)
end


-- Text (love.graphics.newTextBatch)
love.test.graphics.Text = function(test)
  local font = love.graphics.newFont('resources/font.ttf', 8)
  local plaintext = love.graphics.newTextBatch(font, 'test')
  -- check text properties
  test:assertObject(plaintext)
  test:assertEquals(font:getHeight(), plaintext:getFont():getHeight(), 'check font matches')
  local tw, th = plaintext:getDimensions()
  test:assertEquals(24, tw, 'check initial dim w')
  test:assertEquals(8, th, 'check initial dim h')
  test:assertEquals(tw, plaintext:getWidth(), 'check initial dim w')
  test:assertEquals(th, plaintext:getHeight(), 'check initial dim h')
  -- check changing text
  plaintext:add('more text', 100, 0, 0)
  test:assertEquals(49, plaintext:getDimensions(), 'check adding text')
  plaintext:set('test')
  test:assertEquals(24, plaintext:getDimensions(), 'check resetting text')
  plaintext:clear()
  test:assertEquals(0, plaintext:getDimensions(), 'check clearing text')
  -- check drawing + setting more complex text
  local colortext = love.graphics.newTextBatch(font, {{1, 0, 0, 1}, 'test'})
  test:assertObject(colortext)
  colortext:setf('LÖVE is an *awesome* framework you can use to make 2D games in Lua', 60, 'right')
  colortext:addf({{1, 1, 0}, 'overlap'}, 1000, 'left')
  local font2 = love.graphics.newFont('resources/font.ttf', 8)
  colortext:setFont(font2)
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.draw(colortext, 0, 10)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{1,9},{8,13},{16,11},{22,10},{25,7},{29,9},{32,13},{34,15}},
    white = {{17,13},{30,12},{38,9},{44,13},{58,13},{8,29},{58,29},{57,37},{5,39},{57,45},{1,55}}
  }, 'text draw check')
  test:exportImg(imgdata)
end


-- Video (love.graphics.newVideo)
love.test.graphics.Video = function(test)
  -- create video
  local video = love.graphics.newVideo('resources/sample.ogv')
  test:assertObject(video)
  -- check basic props
  local w, h = video:getDimensions()
  test:assertEquals(496, w, 'check vid dim w')
  test:assertEquals(502, h, 'check vid dim h')
  test:assertEquals(w, video:getWidth(), 'check vid width match')
  test:assertEquals(h, video:getHeight(), 'check vid height match')
  local min, mag, ani = video:getFilter()
  test:assertEquals('nearest', min, 'check def filter min')
  test:assertEquals('nearest', mag, 'check def filter mag')
  test:assertEquals(1, ani, 'check def filter ani')
  video:setFilter('linear', 'linear', 2)
  min, mag, ani = video:getFilter()
  test:assertEquals('linear', min, 'check changed filter min')
  test:assertEquals('linear', mag, 'check changed filter mag')
  test:assertEquals(2, ani, 'check changed filter ani')
  test:assertEquals(false, video:isPlaying(), 'check paused by default')
  test:assertEquals(0, video:tell(), 'check 0:00 by default')
  -- covered by their own obj tests in video but check returns obj
  local source = video:getSource()
  test:assertObject(source)
  local stream = video:getStream()
  test:assertObject(stream)
  -- check playing / pausing / seeking
  video:play()
  test:waitFrames(60)
  video:pause()
  test:assertEquals(1, math.ceil(video:tell()), 'check video playing for 1s')
  video:seek(0.2)
  test:assertEquals(0.2, video:tell(), 'check video seeking')
  video:rewind()
  test:assertEquals(0, video:tell(), 'check video rewind')
  video:setFilter('nearest', 'nearest', 1)
  video:play()
  -- check actuall drawing with the vid 
  local canvas = love.graphics.newCanvas(500, 500)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(1, 0, 0, 1)
    love.graphics.draw(video, 0, 0)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {500, 0, 0, 0, 500, 500})
  test:assertPixels(imgdata, {
    black = {{0,0},{495,0},{495,499},{0,499}},
    red = {{499,0},{499,499}}
  }, 'video draw')
  test:exportImg(imgdata)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------DRAWING-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.arc
love.test.graphics.arc = function(test)
  -- draw some arcs using pi format
  local canvas = love.graphics.newCanvas(32, 32)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.arc('line', "pie", 16, 16, 16, 0 * (math.pi/180), 360 * (math.pi/180), 10)
    love.graphics.arc('fill', "pie", 16, 16, 16, 270 * (math.pi/180), 45 * (math.pi/180), 10)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.arc('line', "pie", 16, 16, 16, 0 * (math.pi/180), 90 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.arc('line', "pie", 16, 16, 16, 180 * (math.pi/180), 135 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata1 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata1, {
    white = {{11,0},{20,0},{0,13},{0,14},{31,13},{31,14},{15,14}},
    black = {{16,14},{16,16},{30,14},{30,16}},
    yellow = {{15,15},{15,16},{16,15},{0,15},{4,27},{5,26},{14,17}},
    red = {{15,17},{15,31},{17,15},{31,15},{28,26},{27,27}}
  }, 'arc pi')
  -- draw some arcs with open format
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.arc('line', "open", 16, 16, 16, 0 * (math.pi/180), 315 * (math.pi/180), 10)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.arc('fill', "open", 16, 16, 16, 0 * (math.pi/180), 180 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.arc('fill', "open", 16, 16, 16, 180 * (math.pi/180), 90 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata2 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata2, {
    white = {{11,0},{20,0},{26,4},{5,4},{0,15},{19,31},{31,19}},
    black = {{27,5},{27,4},{26,5},{1,15},{31,15}},
    yellow = {{0,17},{0,19},{12,31},{14,31},{6,23},{7,24}},
    red = {{0,16},{31,16},{31,18},{30,21},{18,31},{15,16},{16,16}}
  }, 'arc open')
  -- draw some arcs with closed format
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.arc('line', "closed", 16, 16, 16, 0 * (math.pi/180), 315 * (math.pi/180), 10)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.arc('fill', "closed", 16, 16, 16, 0 * (math.pi/180), 180 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.arc('line', "closed", 16, 16, 16, 180 * (math.pi/180), 90 * (math.pi/180), 10)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata3 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata3, {
    white = {{11,0},{20,0},{26,4},{5,4},{0,15},{19,31},{31,19}},
    yellow = {{0,17},{0,19},{12,31},{14,31}},
    red = {{31,16},{31,18},{30,21},{18,31},{15,16},{16,16}}
  }, 'arc open')
  test:exportImg(imgdata1)
  test:exportImg(imgdata2)
  test:exportImg(imgdata3)
end


-- love.graphics.circle
love.test.graphics.circle = function(test)
  -- draw some circles
  local canvas = love.graphics.newCanvas(32, 32)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.circle('fill', 16, 16, 16)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.circle('line', 16, 16, 16)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.circle('fill', 16, 16, 8)
    love.graphics.setColor(0, 1, 0, 1)
    love.graphics.circle('fill', 16, 16, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    white = {{13,8},{18,8},{23,13},{23,18}},
    green = {
      {15,12},{16,12},{13,13},{18,13},{12,15},{12,16},{13,18},{18,18},
      {15,19},{16,19},{19,15},{19,16}
    },
    black = {{10,0},{21,0},{0,10},{0,21},{31,10},{31,21},{10,31},{21,31}},
    yellow = {
      {11,10},{10,11},{8,14},{8,17},{10,20},{11,21},{14,23},{17,23},{20,21},
      {21,20},{23,17},{23,14},{20,10},{21,11},{17,8},{14,8}
    },
    red = {{11,0},{20,0},{11,31},{20,31},{0,11},{0,20},{31,20},{31,11}}
  }, 'circle')
  test:exportImg(imgdata)
end



-- love.graphics.clear
love.test.graphics.clear = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.clear(1, 1, 0, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{0,0},{15,0},{0,15},{15,15},{8,8}}
  }, 'clear')
  test:exportImg(imgdata)
end


-- love.graphics.discard
love.test.graphics.discard = function(test)
  -- from the docs: "on some desktops this may do nothing"
  test:skipTest('cant test this worked')
end


-- love.graphics.draw
love.test.graphics.draw = function(test)
  local canvas1 = love.graphics.newCanvas(32, 32)
  local canvas2 = love.graphics.newCanvas(32, 32)
  local transform = love.math.newTransform( )
  transform:translate(16, 0)
  transform:scale(0.5, 0.5)
  love.graphics.setCanvas(canvas1)
    love.graphics.clear(0, 0, 0, 1)
    -- img, offset
    love.graphics.draw(Logo.texture, Logo.img, 0, 0, 0, 1, 1, 16, 16)
  love.graphics.setCanvas()
  love.graphics.setCanvas(canvas2)
    love.graphics.clear(1, 0, 0, 1)
    -- canvas, scale, shear, transform obj
    love.graphics.draw(canvas1, 0, 0, 0, 0.5, 0.5, 0, 0, 2, 2)
    love.graphics.draw(canvas1, 0, 16, 0, 0.5, 0.5)
    love.graphics.draw(canvas1, 16, 16, 0, 0.5, 0.5)
    love.graphics.draw(canvas1, transform)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas2, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    lovepink = {{23,3},{23,19},{7,19},{0,0},{16,0},{0,16},{16,16}},
    loveblue = {{0,31},{15,17},{15,31},{16,31},{31,17},{31,31},{16,15},{31,15}},
    white = {{15, 15},{6,19},{8,19},{22,19},{24,19},{22,3},{24,3}},
    red = {{0,1},{1,0},{15,0},{15,7},{0,15},{7,15}}
  }, 'drawing')
  test:exportImg(imgdata)
end


-- love.graphics.drawInstanced
love.test.graphics.drawInstanced = function(test)
  local image = love.graphics.newImage('resources/love.png')
  local vertices = {
		{ 0, 0, 0, 0, 1, 0, 0 },
		{ image:getWidth(), 0, 1, 0, 0, 1, 0 },
		{ image:getWidth(), image:getHeight(), 1, 1, 0, 0, 1 },
		{ 0, image:getHeight(), 0, 1, 1, 1, 0 },
  }
  local mesh = love.graphics.newMesh(vertices, 'fan')
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.drawInstanced(mesh, 1000, 0, 0, 0, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {64, 0, 0, 0, 64, 64})
  test:assertPixels(imgdata, {
    red = {{0,0}},
    green = {{63,0}},
    blue = {{63,63}},
    yellow = {{0,63}}
  }, 'draw instances')
  test:exportImg(imgdata)
end


-- love.graphics.drawLayer
love.test.graphics.drawLayer = function(test)
  local image = love.graphics.newArrayImage({
    'resources/love.png', 'resources/loveinv.png',
    'resources/love.png', 'resources/loveinv.png'
  })
  local canvas = love.graphics.newCanvas(64, 64)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.drawLayer(image, 1, 0, 0, 0, 1, 1)
    love.graphics.drawLayer(image, 2, 32, 0, 0, 0.5, 0.5)
    love.graphics.drawLayer(image, 4, 0, 32, 0, 0.5, 0.5)
    love.graphics.drawLayer(image, 3, 32, 32, 0, 2, 2, 16, 16)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    lovepink = {{30,2},{33,2},{2,30},{2,33},{4,60},{4,63},{60,4},{63,4},{31,23},{32,23}},
    loveblue = {{14,33},{17,33},{46,1},{49,1},{1,46},{1,49},{33,14},{33,17}},
    black = {{0,0},{63,0},{0,63},{39,6},{40,6},{6,39},{6,40},{6,55},{55,6}},
    white = {{46,11},{48,11},{14,43},{16,43},{30,23},{33,23},{34,54},{53,40},{63,63}}
  }, 'draw layer')
  test:exportImg(imgdata)
end


-- love.graphics.ellipse
love.test.graphics.ellipse = function(test)
  local canvas = love.graphics.newCanvas(32, 32)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.ellipse('fill', 16, 16, 16, 8)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.ellipse('fill', 24, 24, 10, 24)
    love.graphics.setColor(1, 0, 1, 1)
    love.graphics.ellipse('fill', 16, 0, 8, 16)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {{0,14},{0,17},{7,9},{7,22},{14,15},{14,16}},
    pink = {{15,15},{16,15},{8,0},{8,4},{23,0},{23,4},{13,14},{18,14}},
    yellow = {{24,0},{25,0},{14,17},{14,30},{15,31},{31,8}}
  }, 'ellipses')
  test:exportImg(imgdata)
end


-- love.graphics.flushBatch
love.test.graphics.flushBatch = function(test)
  love.graphics.flushBatch()
  local initial = love.graphics.getStats()['drawcalls']
  local canvas = love.graphics.newCanvas(32, 32)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 32, 32)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  love.graphics.flushBatch()
  local after = love.graphics.getStats()['drawcalls']
  test:assertEquals(initial+1, after, 'check drawcalls increased')
end


-- love.graphics.line
love.test.graphics.line = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.line(1,1,16,1,16,16,1,16,1,1)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.line({0,0,8,8,16,0,8,8,16,16,8,8,0,16})
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{0,0},{15,0},{0,15},{15,15},{7,7},{8,7},{8,7},{8,8}},
    red = {{1,0},{14,0},{0,1},{0,14},{15,1},{15,14},{1,15},{14,15}}
  }, 'lines')
  test:exportImg(imgdata)
end


-- love.graphics.points
love.test.graphics.points = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.points(1,1,16,1,16,16,1,16,1,1)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.points({2,2,8,8,15,2,8,9,15,15,9,9,2,15,9,8})
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{1,1},{14,1},{1,14},{14,14},{7,7},{8,7},{8,7},{8,8}},
    red = {{0,0},{15,0},{15,15},{0,15}}
  }, 'points')
  test:exportImg(imgdata)
end


-- love.graphics.polygon
love.test.graphics.polygon = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.polygon("fill", 1, 1, 4, 5, 8, 10, 16, 2, 7, 3, 5, 16, 16, 16, 1, 8)
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.polygon("line", {2, 2, 4, 5, 3, 7, 8, 15, 12, 4, 5, 10})
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{1,0},{1,1},{5,9},{7,14},{8,14},{12,3}},
    red = {{2,1},{1,2},{1,7},{5,15},{14,15},{8,8},{14,2},{7,1}}
  }, 'polygon')
  test:exportImg(imgdata)
end


-- love.graphics.print
love.test.graphics.print = function(test)
  love.graphics.setFont(Font)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.print('love', 0, 3, 0, 1, 1, 0, 0)
    love.graphics.setColor(0, 1, 0, 1)
    love.graphics.print('ooo', 0, 3, 0, 2, 2, 0, 0)
    love.graphics.setColor(0, 0, 1, 1)
    love.graphics.print('hello', 0, 3, 90*(math.pi/180), 1, 1, 0, 8)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {{0,0},{1,0},{1,1},{2,6},{4,4},{7,6},{10,2},{11,5},{14,3},{14,4}},
    green = {
      {2,1},{2,2},{0,3},{1,3},{1,8},{2,9},{7,10},{8,8},{9,4},{13,3},{14,2},
      {13,8},{14,9}
    },
    blue = {
      {4,15},{10,15},{4,12},{6,12},{8,12},{5,9},{7,9},{4,3},{10,3},{8,6},{7,7},
      {4,7},{7,13},{8,12}
    }
  }, 'print')
  test:exportImg(imgdata)
end


-- love.graphics.printf
love.test.graphics.printf = function(test)
  love.graphics.setFont(Font)
  local canvas = love.graphics.newCanvas(32, 32)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.printf('love', 0, 0, 8, "left")
    love.graphics.setColor(0, 1, 0, 1)
    love.graphics.printf('love', 0, 5, 16, "right")
    love.graphics.setColor(0, 0, 1, 1)
    love.graphics.printf('love', 0, 7, 32, "center")
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {
      {1,0},{1,1},{0,3},{2,3},{2,7},{0,9},{3,11},{4,10},{0,15},{4,15},{2,19},
      {0,24},{1,23},{3,23},{4,24},{0,26},{1,27},{2,27},{3,27}
    },
    green = {
      {1,2},{0,8},{1,8},{2,8},{4,7},{5,8},{7,8},{8,7},{10,4},{14,4},{11,7},
      {12,8},{10,13},{11,12},{13,12},{14,13},{10,15},{11,16}
    },
    blue = {{6,4},{6,10},{9,7},{10,6},{16,9},{18,9},{21,8},{25,8}}
  }, 'printf')
  test:exportImg(imgdata)
end


-- love.graphics.rectangle
love.test.graphics.rectangle = function(test)
  -- setup, draw a 16x16 red rectangle with a blue central square
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setColor(0, 0, 1, 1)
    love.graphics.rectangle('fill', 6, 6, 4, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata1 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  -- test, check red bg and blue central square
  test:assertPixels(imgdata1, {
    red = {{0,0},{15,0},{15,15},{0,15}},
    blue = {{6,6},{9,6},{9,9},{6,9}}
  }, 'fill')
  -- clear canvas to do some line testing
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('line', 1, 1, 15, 15) -- red border
    love.graphics.setColor(0, 0, 1, 1)
    love.graphics.rectangle('line', 1, 1, 2, 15) -- 3x16 left aligned blue outline
    love.graphics.setColor(0, 1, 0, 1)
    love.graphics.rectangle('line', 11, 1, 5, 15) -- 6x16 right aligned green outline
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata2 = love.graphics.readbackTexture(canvas, {1, 1, 0, 0, 16, 16})
  -- -- check corners and inner corners
  test:assertPixels(imgdata2, {
    red = {{3,0},{9,0},{3,15,9,15}},
    blue = {{0,0},{2,0},{0,15},{2,15}},
    green = {{10,0},{15,0},{10,15},{15,15}},
    black = {
      {1,1},{1,14},{3,1},{9,1},{3,14},
      {9,14},{11,1},{14,1},{11,14},{14,14}
    }
  }, 'line')
  test:exportImg(imgdata1)
  test:exportImg(imgdata2)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------OBJECT CREATION---------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.captureScreenshot
love.test.graphics.captureScreenshot = function(test)
  love.graphics.captureScreenshot('example-screenshot.png')
  test:waitFrames(10)
  -- need to wait until end of the frame for the screenshot
  test:assertNotNil(love.filesystem.openFile('example-screenshot.png', 'r'))
  love.filesystem.remove('example-screenshot.png')
end


-- love.graphics.newArrayImage
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newArrayImage = function(test)
  test:assertObject(love.graphics.newArrayImage({
    'resources/love.png', 'resources/love2.png', 'resources/love3.png'
  }))
end

-- love.graphics.newCanvas
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newCanvas = function(test)
  test:assertObject(love.graphics.newCanvas(16, 16, {
    type = '2d',
    format = 'normal',
    readable = true,
    msaa = 0,
    dpiscale = 1,
    mipmaps = 'none'
  }))
  test:assertObject(love.graphics.newCanvas(1000, 1000))
end


-- love.graphics.newCubeImage
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newCubeImage = function(test)
  test:assertObject(love.graphics.newCubeImage('resources/cubemap.png', {
    mipmaps = false,
    linear = false
  }))
end


-- love.graphics.newFont
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newFont = function(test)
  test:assertObject(love.graphics.newFont('resources/font.ttf'))
  test:assertObject(love.graphics.newFont('resources/font.ttf', 8, "normal", 1))
end


-- love.graphics.newImage
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newImage = function(test)
  test:assertObject(love.graphics.newImage('resources/love.png', {
    mipmaps = false,
    linear = false,
    dpiscale = 1
  }))
end


-- love.graphics.newImageFont
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newImageFont = function(test)
  test:assertObject(love.graphics.newImageFont('resources/love.png', 'ABCD', 1))
end


-- love.graphics.newMesh
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newMesh = function(test)
  test:assertObject(love.graphics.newMesh({{1, 1, 0, 0, 1, 1, 1, 1}}, 'fan', 'dynamic'))
end


-- love.graphics.newParticleSystem
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newParticleSystem = function(test)
  local imgdata = love.graphics.newImage('resources/love.png')
  test:assertObject(love.graphics.newParticleSystem(imgdata, 1000))
end


-- love.graphics.newQuad
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newQuad = function(test)
  local imgdata = love.graphics.newImage('resources/love.png')
  test:assertObject(love.graphics.newQuad(0, 0, 16, 16, imgdata))
end


-- love.graphics.newShader
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newShader = function(test)
  local pixelcode = [[
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texturecolor = Texel(tex, texture_coords); 
      return texturecolor * color;
    }
  ]]
  local vertexcode = [[
    vec4 position(mat4 transform_projection, vec4 vertex_position) { 
      return transform_projection * vertex_position; 
    }
  ]]
  test:assertObject(love.graphics.newShader(pixelcode, vertexcode))
end


-- love.graphics.newSpriteBatch
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newSpriteBatch = function(test)
  local imgdata = love.graphics.newImage('resources/love.png')
  test:assertObject(love.graphics.newSpriteBatch(imgdata, 1000))
end


-- love.graphics.newText
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newTextBatch = function(test)
  local font = love.graphics.newFont('resources/font.ttf')
  test:assertObject(love.graphics.newTextBatch(font, 'helloworld'))
end


-- love.graphics.newVideo
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newVideo = function(test)
  test:assertObject(love.graphics.newVideo('resources/sample.ogv', {
    audio = false,
    dpiscale = 1
  }))
end


-- love.graphics.newVolumeImage
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.graphics.newVolumeImage = function(test)
  test:assertObject(love.graphics.newVolumeImage({
    'resources/love.png', 'resources/love2.png', 'resources/love3.png'
  }, {
    mipmaps = false,
    linear = false
  }))
end


-- love.graphics.validateShader
love.test.graphics.validateShader = function(test)
  local pixelcode = [[
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texturecolor = Texel(tex, texture_coords); 
      return texturecolor * color;
    }
  ]]
  local vertexcode = [[
    vec4 position(mat4 transform_projection, vec4 vertex_position) { 
      return transform_projection * vertex_position; 
    }
  ]]
  -- check made up code first
  local status, _ = love.graphics.validateShader(true, 'nothing here', 'or here')
  test:assertEquals(false, status, 'check invalid shader code')
  -- check real code 
  status, _ = love.graphics.validateShader(true, pixelcode, vertexcode)
  test:assertEquals(true, status, 'check valid shader code')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
---------------------------------GRAPHICS STATE---------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.getBackgroundColor
love.test.graphics.getBackgroundColor = function(test)
  -- check default bg is black
  local r, g, b, a = love.graphics.getBackgroundColor()
  test:assertEquals(0, r, 'check default background r')
  test:assertEquals(0, g, 'check default background g')
  test:assertEquals(0, b, 'check default background b')
  test:assertEquals(1, a, 'check default background a')
  -- check set value returns correctly
  love.graphics.setBackgroundColor(1, 1, 1, 0)
  r, g, b, a = love.graphics.getBackgroundColor()
  test:assertEquals(1, r, 'check updated background r')
  test:assertEquals(1, g, 'check updated background g')
  test:assertEquals(1, b, 'check updated background b')
  test:assertEquals(0, a, 'check updated background a')
  love.graphics.setBackgroundColor(0, 0, 0, 1) -- reset
end


-- love.graphics.getBlendMode
love.test.graphics.getBlendMode = function(test)
  -- check default blend mode
  local mode, alphamode = love.graphics.getBlendMode()
  test:assertEquals('alpha', mode, 'check default blend mode')
  test:assertEquals('alphamultiply', alphamode, 'check default alpha blend')
  -- check set mode returns correctly
  love.graphics.setBlendMode('add', 'premultiplied')
  mode, alphamode = love.graphics.getBlendMode()
  test:assertEquals('add', mode, 'check changed blend mode')
  test:assertEquals('premultiplied', alphamode, 'check changed alpha blend')
  love.graphics.setBlendMode('alpha', 'alphamultiply') -- reset
end


-- love.graphics.getCanvas
love.test.graphics.getCanvas = function(test)
  -- by default should be nil if drawing to real screen
  test:assertEquals(nil, love.graphics.getCanvas(), 'check no canvas set')
  -- should return not nil when we target a canvas
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
  test:assertObject(love.graphics.getCanvas())
  love.graphics.setCanvas()
end


-- love.graphics.getColor
love.test.graphics.getColor = function(test)
  -- by default should be white
  local r, g, b, a = love.graphics.getColor()
  test:assertEquals(1, r, 'check default color r')
  test:assertEquals(1, g, 'check default color g')
  test:assertEquals(1, b, 'check default color b')
  test:assertEquals(1, a, 'check default color a')
  -- check set color is returned correctly
  love.graphics.setColor(0, 0, 0, 0)
  r, g, b, a = love.graphics.getColor()
  test:assertEquals(0, r, 'check changed color r')
  test:assertEquals(0, g, 'check changed color g')
  test:assertEquals(0, b, 'check changed color b')
  test:assertEquals(0, a, 'check changed color a')
  love.graphics.setColor(1, 1, 1, 1) -- reset
end


-- love.graphics.getColorMask
love.test.graphics.getColorMask = function(test)
  -- by default should all be active
  local r, g, b, a = love.graphics.getColorMask()
  test:assertEquals(true, r, 'check default color mask r')
  test:assertEquals(true, g, 'check default color mask g')
  test:assertEquals(true, b, 'check default color mask b')
  test:assertEquals(true, a, 'check default color mask a')
  -- check set color mask is returned correctly
  love.graphics.setColorMask(false, false, true, false)
  r, g, b, a = love.graphics.getColorMask()
  test:assertEquals(false, r, 'check changed color mask r')
  test:assertEquals(false, g, 'check changed color mask g')
  test:assertEquals(true,  b, 'check changed color mask b')
  test:assertEquals(false, a, 'check changed color mask a')
  love.graphics.setColorMask(true, true, true, true) -- reset
end


-- love.graphics.getDefaultFilter
love.test.graphics.getDefaultFilter = function(test)
  -- we set this already for testsuite so we know what it should be
  local min, mag, anisotropy = love.graphics.getDefaultFilter()
  test:assertEquals('nearest', min, 'check default filter min')
  test:assertEquals('nearest', mag, 'check default filter mag')
  test:assertEquals(1, anisotropy, 'check default filter mag')
end


-- love.graphics.getDepthMode
love.test.graphics.getDepthMode = function(test)
  -- by default should be always/write
  local comparemode, write = love.graphics.getDepthMode()
  test:assertEquals('always', comparemode, 'check default compare depth')
  test:assertEquals(false, write, 'check default depth buffer write')
end


-- love.graphics.getFont
love.test.graphics.getFont = function(test)
  test:assertObject(love.graphics.getFont())
end


-- love.graphics.getFrontFaceWinding
love.test.graphics.getFrontFaceWinding = function(test)
  -- check default winding
  test:assertEquals('ccw', love.graphics.getFrontFaceWinding())
  -- check setting value changes it correctly
  love.graphics.setFrontFaceWinding('cw')
  test:assertEquals('cw', love.graphics.getFrontFaceWinding())
  love.graphics.setFrontFaceWinding('ccw') -- reset
end


-- love.graphics.getLineJoin
love.test.graphics.getLineJoin = function(test)
  -- check default line join
  test:assertEquals('miter', love.graphics.getLineJoin())
  -- check set value returned correctly
  love.graphics.setLineJoin('none')
  test:assertEquals('none', love.graphics.getLineJoin())
  love.graphics.setLineJoin('miter') -- reset
end


-- love.graphics.getLineStyle
love.test.graphics.getLineStyle = function(test)
  -- we know this should be as testsuite sets it!
  test:assertEquals('rough', love.graphics.getLineStyle())
  -- check set value returned correctly
  love.graphics.setLineStyle('smooth')
  test:assertEquals('smooth', love.graphics.getLineStyle())
  love.graphics.setLineStyle('rough') -- reset
end


-- love.graphics.getLineWidth
love.test.graphics.getLineWidth = function(test)
  -- we know this should be as testsuite sets it!
  test:assertEquals(1, love.graphics.getLineWidth())
  -- check set value returned correctly
  love.graphics.setLineWidth(10)
  test:assertEquals(10, love.graphics.getLineWidth())
  love.graphics.setLineWidth(1) -- reset
end


-- love.graphics.getMeshCullMode
love.test.graphics.getMeshCullMode = function(test)
  -- get default mesh culling
  test:assertEquals('none', love.graphics.getMeshCullMode())
  -- check set value returned correctly
  love.graphics.setMeshCullMode('front')
  test:assertEquals('front', love.graphics.getMeshCullMode())
  love.graphics.setMeshCullMode('back') -- reset
end


-- love.graphics.getPointSize
love.test.graphics.getPointSize = function(test)
  -- get default point size
  test:assertEquals(1, love.graphics.getPointSize())
  -- check set value returned correctly
  love.graphics.setPointSize(10)
  test:assertEquals(10, love.graphics.getPointSize())
  love.graphics.setPointSize(1) -- reset
end


-- love.graphics.getScissor
love.test.graphics.getScissor = function(test)
  -- should be no scissor atm
  local x, y, w, h = love.graphics.getScissor()
  test:assertEquals(nil, x, 'check no scissor')
  test:assertEquals(nil, y, 'check no scissor')
  test:assertEquals(nil, w, 'check no scissor')
  test:assertEquals(nil, h, 'check no scissor')
  -- check set value returned correctly
  love.graphics.setScissor(0, 0, 16, 16)
  x, y, w, h = love.graphics.getScissor()
  test:assertEquals(0, x, 'check scissor set')
  test:assertEquals(0, y, 'check scissor set')
  test:assertEquals(16, w, 'check scissor set')
  test:assertEquals(16, h, 'check scissor set')
  love.graphics.setScissor() -- reset
end


-- love.graphics.getShader
love.test.graphics.getShader = function(test)
  -- should be no shader active
  test:assertEquals(nil, love.graphics.getShader(), 'check no active shader')
end


-- love.graphics.getStackDepth
love.test.graphics.getStackDepth = function(test)
  -- by default should be none
  test:assertEquals(0, love.graphics.getStackDepth(), 'check no transforms in stack')
  -- now add 3
  love.graphics.push()
  love.graphics.push()
  love.graphics.push()
  test:assertEquals(3, love.graphics.getStackDepth(), 'check 3 transforms in stack')
  -- now remove 2
  love.graphics.pop()
  love.graphics.pop()
  test:assertEquals(1, love.graphics.getStackDepth(), 'check 1 transforms in stack')
  -- now back to 0
  love.graphics.pop()
  test:assertEquals(0, love.graphics.getStackDepth(), 'check no transforms in stack')
end


-- love.graphics.getStencilMode
love.test.graphics.getStencilMode = function(test)
  -- check default vals
  local action, comparemode, value = love.graphics.getStencilMode( )
  test:assertEquals('keep', action, 'check default stencil action')
  test:assertEquals('always', comparemode, 'check default stencil compare')
  test:assertEquals(0, value, 'check default stencil value')
  -- check set stencil values is returned
  love.graphics.setStencilMode('replace', 'less', 255)
  local action, comparemode, value = love.graphics.getStencilMode()
  test:assertEquals('replace', action, 'check changed stencil action')
  test:assertEquals('less', comparemode, 'check changed stencil compare')
  test:assertEquals(255, value, 'check changed stencil value')
  love.graphics.setStencilMode() -- reset
end


-- love.graphics.intersectScissor
love.test.graphics.intersectScissor = function(test)
  -- make a scissor for the left half, then interset to make the top half
  -- then we should be able to fill the canvas with red and only top 4x4 is filled
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.origin()
    love.graphics.setScissor(0, 0, 8, 16)
    love.graphics.intersectScissor(0, 0, 4, 4)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.setScissor()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { 
    red = {{0,0},{3,3}},
    black ={{4,0},{0,4},{4,4}}
  }, 'intersect scissor')
  test:exportImg(imgdata)
end


-- love.graphics.isActive
love.test.graphics.isActive = function(test)
  local name, version, vendor, device = love.graphics.getRendererInfo()
  if string.find(name, 'Vulkan') ~= nil then
    test:skipTest('love.graphics.isActive() crashes on Vulkan')
  else 
    test:assertEquals(true, love.graphics.isActive(), 'check graphics is active') -- i mean if you got this far
  end
end


-- love.graphics.isGammaCorrect
love.test.graphics.isGammaCorrect = function(test)
  -- we know the config so know this is false
  test:assertNotNil(love.graphics.isGammaCorrect())
end


-- love.graphics.isWireframe
love.test.graphics.isWireframe = function(test)
  local name, version, vendor, device = love.graphics.getRendererInfo()
  if string.match(name, 'OpenGL ES') then
    test:skipTest('Wireframe not supported on OpenGL ES')
  else
    -- check off by default
    test:assertEquals(false, love.graphics.isWireframe(), 'check no wireframe by default')
    -- check on when enabled
    love.graphics.setWireframe(true)
    test:assertEquals(true, love.graphics.isWireframe(), 'check wireframe is set')
    love.graphics.setWireframe(false) -- reset
  end
end


-- love.graphics.reset
love.test.graphics.reset = function(test)
  -- reset should reset current canvas and any colors/scissor
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setBackgroundColor(0, 0, 1, 1)
  love.graphics.setColor(0, 1, 0, 1)
  love.graphics.setCanvas(canvas)
  love.graphics.reset()
  local r, g, b, a = love.graphics.getBackgroundColor()
  test:assertEquals(1, r+g+b+a, 'check background reset')
  r, g, b, a = love.graphics.getColor()
  test:assertEquals(4, r+g+b+a, 'check color reset')
  test:assertEquals(nil, love.graphics.getCanvas(), 'check canvas reset')
  love.graphics.setDefaultFilter("nearest", "nearest")
  love.graphics.setLineStyle('rough')
  love.graphics.setPointSize(1)
  love.graphics.setLineWidth(1)
end


-- love.graphics.setBackgroundColor
love.test.graphics.setBackgroundColor = function(test)
  -- check background is set
  love.graphics.setBackgroundColor(1, 0, 0, 1)
  local r, g, b, a = love.graphics.getBackgroundColor()
  test:assertEquals(1, r, 'check set bg r')
  test:assertEquals(0, g, 'check set bg g')
  test:assertEquals(0, b, 'check set bg b')
  test:assertEquals(1, a, 'check set bg a')
  love.graphics.setBackgroundColor(0, 0, 0, 1)
end


-- love.graphics.setBlendMode
love.test.graphics.setBlendMode = function(test)
  -- create fully white canvas, then draw diff. pixels through blendmodes
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0.5, 0.5, 0.5, 1)
    love.graphics.setBlendMode('add', 'alphamultiply')
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.points({1,1})
    love.graphics.setBlendMode('subtract', 'alphamultiply')
    love.graphics.setColor(1, 1, 1, 0.5)
    love.graphics.points({16,1})
    love.graphics.setBlendMode('multiply', 'premultiplied')
    love.graphics.setColor(0, 1, 0, 1)
    love.graphics.points({16,16})
    love.graphics.setBlendMode('replace', 'premultiplied')
    love.graphics.setColor(0, 0, 1, 0.5)
    love.graphics.points({1,16})
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  -- check the 4 corners
  test:assertPixels(imgdata, {
    redpale = {{0,0}},
    black = {{15,0}},
    greenhalf = {{15,15}},
    bluefade = {{0,15}}
  }, 'blend mode')
  love.graphics.setBlendMode('alpha', 'alphamultiply') -- reset 
  test:exportImg(imgdata)
end


-- love.graphics.setCanvas
love.test.graphics.setCanvas = function(test)
  -- make 2 canvas, set to each, draw one to the other, check output
  local canvas1 = love.graphics.newCanvas(16, 16)
  local canvas2 = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas1)
    test:assertEquals(canvas1, love.graphics.getCanvas(), 'check canvas 1 set')
    love.graphics.clear(1, 0, 0, 1)
  love.graphics.setCanvas(canvas2)
    test:assertEquals(canvas2, love.graphics.getCanvas(), 'check canvas 2 set')
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.draw(canvas1, 0, 0)
  love.graphics.setCanvas()
  test:assertEquals(nil, love.graphics.getCanvas(), 'check no canvas set')
  local imgdata = love.graphics.readbackTexture(canvas2, {16, 0, 0, 0, 16, 16})
  -- check 2nd canvas is red
  test:assertPixels(imgdata, {
    red = {{0,0},{15,0},{15,15},{0,15}}
  }, 'set canvas')
  test:exportImg(imgdata)
end


-- love.graphics.setColor
love.test.graphics.setColor = function(test)
  -- set colors, draw rect, check color 
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    local r, g, b, a = love.graphics.getColor()
    test:assertEquals(1, r, 'check r set')
    test:assertEquals(0, g, 'check g set')
    test:assertEquals(0, b, 'check b set')
    test:assertEquals(1, a, 'check a set')
    love.graphics.points({{1,1},{6,1},{11,1},{16,1}})
    love.graphics.setColor(1, 1, 0, 1)
    love.graphics.points({{1,2},{6,2},{11,2},{16,2}})
    love.graphics.setColor(0, 1, 0, 0.5)
    love.graphics.points({{1,3},{6,3},{11,3},{16,3}})
    love.graphics.setColor(0, 0, 1, 1)
    love.graphics.points({{1,4},{6,4},{11,4},{16,4}})
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {{0,0},{5,0},{10,0},{15,0}},
    yellow = {{0,1},{5,1},{10,1},{15,1}},
    greenhalf = {{0,2},{5,2},{10,2},{15,2}},
    blue = {{0,3},{5,3},{10,3},{15,3}}
  }, 'set color')
  test:exportImg(imgdata)
end


-- love.graphics.setColorMask
love.test.graphics.setColorMask = function(test)
  -- set mask, draw stuff, check output pixels
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    -- mask off blue
    love.graphics.setColorMask(true, true, false, true)
    local r, g, b, a = love.graphics.getColorMask()
    test:assertEquals(r, true, 'check r mask')
    test:assertEquals(g, true, 'check g mask')
    test:assertEquals(b, false, 'check b mask')
    test:assertEquals(a, true, 'check a mask')
    -- draw "black" which should then turn to yellow
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setColorMask(true, true, true, true)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    yellow = {{0,0},{0,15},{15,15},{15,0}}
  }, 'set color mask')
  test:exportImg(imgdata)
end


-- love.graphics.setDefaultFilter
love.test.graphics.setDefaultFilter = function(test)
  -- check setting filter val works
  love.graphics.setDefaultFilter('linear', 'linear', 1)
  local min, mag, anisotropy = love.graphics.getDefaultFilter()
  test:assertEquals('linear', min, 'check default filter min')
  test:assertEquals('linear', mag, 'check default filter mag')
  test:assertEquals(1, anisotropy, 'check default filter mag')
  love.graphics.setDefaultFilter('nearest', 'nearest', 1) -- reset
end


-- love.graphics.setDepthMode
love.test.graphics.setDepthMode = function(test)
  -- check documented modes are valid
  local comparemode, write = love.graphics.getDepthMode()
  local modes = {
    'equal', 'notequal', 'less', 'lequal', 'gequal',
    'greater', 'never', 'always'
  }
  for m=1,#modes do
    love.graphics.setDepthMode(modes[m], true)
    test:assertEquals(modes[m], love.graphics.getDepthMode(), 'check depth mode ' .. modes[m] .. ' set')
  end
  love.graphics.setDepthMode(comparemode, write)
  -- @TODO better graphics drawing specific test
end


-- love.graphics.setFont
love.test.graphics.setFont = function(test)
  -- set font doesnt return anything so draw with the test font
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setFont(Font)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.print('love', 0, 3)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {
      {0,0},{0,6},{2,6},{6,2},
      {4,4},{8,4},{6,6},{10,2},
      {14,2},{12,6}
    }
  }, 'set font for print')
  test:exportImg(imgdata)
end


-- love.graphics.setFrontFaceWinding
love.test.graphics.setFrontFaceWinding = function(test)
  -- check documented modes are valid
  local original = love.graphics.getFrontFaceWinding()
  love.graphics.setFrontFaceWinding('cw')
  test:assertEquals('cw', love.graphics.getFrontFaceWinding(), 'check ffw cw set')
  love.graphics.setFrontFaceWinding('ccw')
  test:assertEquals('ccw', love.graphics.getFrontFaceWinding(), 'check ffw ccw set')
  love.graphics.setFrontFaceWinding(original)
  -- @TODO better graphics drawing specific test
end


-- love.graphics.setLineJoin
love.test.graphics.setLineJoin = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setFont(Font)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    local line = {0,1,8,1,8,8}
    love.graphics.setLineStyle('rough')
    love.graphics.setLineWidth(2)
    love.graphics.setColor(1, 0, 0)
    love.graphics.setLineJoin('bevel')
    love.graphics.line(line)
    love.graphics.translate(0, 4)
    love.graphics.setColor(1, 1, 0)
    love.graphics.setLineJoin('none')
    love.graphics.line(line)
    love.graphics.translate(0, 4)
    love.graphics.setColor(0, 0, 1)
    love.graphics.setLineJoin('miter')
    love.graphics.line(line)
    love.graphics.setColor(1, 1, 1)
    love.graphics.setLineWidth(1)
    love.graphics.origin()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    black = {{8,0}},
    red = {{8,4}},
    yellow = {{8,7}},
    blue = {{8,8}}
  }, 'set line join')
  test:exportImg(imgdata)
end


-- love.graphics.setLineStyle
love.test.graphics.setLineStyle = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setFont(Font)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0)
    local line = {0,1,16,1}
    love.graphics.setLineStyle('rough')
    love.graphics.line(line)
    love.graphics.translate(0, 4)
    love.graphics.setLineStyle('smooth')
    love.graphics.line(line)
    love.graphics.setLineStyle('rough')
    love.graphics.setColor(1, 1, 1)
    love.graphics.origin()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    red = {{0,0},{7,0},{15,0}},
    red07 = {{0,4},{7,4},{15,4}}
  }, 'set line style')
  test:exportImg(imgdata)
end


-- love.graphics.setLineWidth
love.test.graphics.setLineWidth = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setFont(Font)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    local line = {0,1,8,1,8,8}
    love.graphics.setColor(1, 0, 0)
    love.graphics.setLineWidth(2)
    love.graphics.line(line)
    love.graphics.translate(0, 4)
    love.graphics.setColor(1, 1, 0)
    love.graphics.setLineWidth(3)
    love.graphics.line(line)
    love.graphics.translate(0, 4)
    love.graphics.setColor(0, 0, 1)
    love.graphics.setLineWidth(4)
    love.graphics.line(line)
    love.graphics.setColor(1, 1, 1)
    love.graphics.setLineWidth(1)
    love.graphics.origin()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, {
    black = {{0,2},{6,2},{0,6},{5,6},{0,11},{5,11}},
    red = {{0,0},{0,1},{7,2},{8,2}},
    yellow = {{0,3},{0,5},{6,6},{8,6}},
    blue = {{0,7},{0,10},{6,15},{9,15}}
  }, 'set line width')
  test:exportImg(imgdata)
end


-- love.graphics.setMeshCullMode
love.test.graphics.setMeshCullMode = function(test)
  -- check documented modes are valid
  local original = love.graphics.getMeshCullMode()
  local modes = {'back', 'front', 'none'}
  for m=1,#modes do
    love.graphics.setMeshCullMode(modes[m])
    test:assertEquals(modes[m], love.graphics.getMeshCullMode(), 'check mesh cull mode ' .. modes[m] .. ' was set')
  end
  love.graphics.setMeshCullMode(original)
  -- @TODO better graphics drawing specific test
end


-- love.graphics.setScissor
love.test.graphics.setScissor = function(test)
  -- make a scissor for the left half
  -- then we should be able to fill the canvas with red and only left is filled
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.origin()
    love.graphics.setScissor(0, 0, 8, 16)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.setScissor()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { 
    red = {{0,0},{7,0},{0,15},{7,15}},
    black ={{8,0},{8,15},{15,0},{15,15}}
  }, 'set scissor')
  test:exportImg(imgdata)
end


-- love.graphics.setShader
love.test.graphics.setShader = function(test)
  -- make a shader that will only ever draw yellow
  local pixelcode = [[
    vec4 effect(vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords) { 
      vec4 texturecolor = Texel(tex, texture_coords); 
      return vec4(1.0,1.0,0.0,1.0);
    }
  ]]
  local vertexcode = [[
    vec4 position(mat4 transform_projection, vec4 vertex_position) { 
      return transform_projection * vertex_position; 
    }
  ]]
  local shader = love.graphics.newShader(pixelcode, vertexcode)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setShader(shader)
      -- draw red rectangle
      love.graphics.setColor(1, 0, 0, 1)
      love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setShader()
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { 
    yellow = {{0,0},{15,0},{0,15},{15,15}},
  }, 'check shader set to yellow')
  test:exportImg(imgdata)
end


-- love.graphics.setStencilMode
love.test.graphics.setStencilMode = function(test)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas({canvas, stencil=true})
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setStencilMode('replace', 'always', 1)
    love.graphics.circle('fill', 8, 8, 6)
    love.graphics.setStencilMode('keep', 'greater', 0)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 16, 16)
    love.graphics.setColor(1, 1, 1, 1)
    love.graphics.setStencilMode()
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { 
    red = {{6,2},{9,2},{2,6},{2,9},{13,6},{9,6},{6,13},{9,13}}
  }, 'check stencil test')
  test:exportImg(imgdata)
end


-- love.graphics.setWireframe
love.test.graphics.setWireframe = function(test)
  local name, version, vendor, device = love.graphics.getRendererInfo()
  if string.match(name, 'OpenGL ES') then
    test:skipTest('Wireframe not supported on OpenGL ES')
  else
    -- check wireframe outlines
    love.graphics.setWireframe(true)
    local canvas = love.graphics.newCanvas(16, 16)
    love.graphics.setCanvas(canvas)
      love.graphics.clear(0, 0, 0, 1)
      love.graphics.setColor(1, 1, 0, 1)
      love.graphics.rectangle('fill', 2, 2, 13, 13)
      love.graphics.setColor(1, 1, 1, 1)
      love.graphics.setWireframe(false)
    love.graphics.setCanvas()
    local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
    test:assertPixels(imgdata, { 
      yellow = {{1,14},{14,1},{14,14},{2,2},{13,13}},
      black = {{2,13},{13,2}}
    }, 'set wireframe')
    test:exportImg(imgdata)
  end
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-------------------------------COORDINATE SYSTEM--------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.applyTransform
love.test.graphics.applyTransform = function(test)
  -- use transform object to translate the drawn rectangle
  local transform = love.math.newTransform()
  transform:translate(10, 0)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.applyTransform(transform)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{10, 0}} }, 'apply transform 10')
  test:exportImg(imgdata)
end


-- love.graphics.inverseTransformPoint
love.test.graphics.inverseTransformPoint = function(test)
  -- start with 0, 0
  local sx, sy = love.graphics.inverseTransformPoint(0, 0)
  test:assertEquals(0, sx, 'check starting x is 0')
  test:assertEquals(0, sy, 'check starting y is 0')
  -- check translation effects the point 
  love.graphics.translate(1, 5)
  sx, sy = love.graphics.inverseTransformPoint(1, 5)
  test:assertEquals(0, sx, 'check transformed x is 0')
  test:assertEquals(0, sy, 'check transformed y is 0')
  love.graphics.origin()
end


-- love.graphics.origin
love.test.graphics.origin = function(test)
  -- if we do some translations and scaling
  -- using .origin() should reset it all and draw the pixel at 0,0
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.origin()
    love.graphics.translate(10, 10)
    love.graphics.scale(1, 1)
    love.graphics.shear(20, 20)
    love.graphics.origin()
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{0, 0}} }, 'origin check')
  test:exportImg(imgdata)
end


-- love.graphics.pop
love.test.graphics.pop = function(test)
  -- if we push at the start, and then run a pop
  -- it should reset it all and draw the pixel at 0,0
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.push()
    love.graphics.translate(10, 10)
    love.graphics.scale(1, 1)
    love.graphics.shear(20, 20)
    love.graphics.pop()
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{0, 0}} }, 'pop 1')
  test:exportImg(imgdata)
end


-- love.graphics.push
love.test.graphics.push = function(test)
  -- if we push at the start, do some stuff, then another push
  -- 1 pop should only go back 1 push and draw the pixel at 1, 1
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.push()
    love.graphics.scale(1, 1)
    love.graphics.shear(20, 20)
    love.graphics.push()
    love.graphics.translate(1, 1)
    love.graphics.pop()
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{1, 1}} }, 'push 1')
  test:exportImg(imgdata)
end


-- love.graphics.replaceTransform
love.test.graphics.replaceTransform = function(test)
  -- if use transform object to translate
  -- set some normal transforms first which should get overwritten
  local transform = love.math.newTransform()
  transform:translate(10, 0)
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.scale(2, 2)
    love.graphics.translate(10, 10)
    love.graphics.replaceTransform(transform)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{10, 0}} }, 'replace transform 10')
  test:exportImg(imgdata)
end


-- love.graphics.rotate
love.test.graphics.rotate = function(test)
  -- starting at 0,0, we rotate by 90deg and then draw
  -- we can then check the drawn rectangle is rotated
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.translate(4, 0)
    love.graphics.rotate(90 * (math.pi/180))
    love.graphics.rectangle('fill', 0, 0, 4, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{0,0},{3,0},{3,3},{0,3}} }, 'rotate 90')
  test:exportImg(imgdata)
end


-- love.graphics.scale
love.test.graphics.scale = function(test)
  -- starting at 0,0, we scale by 4x and then draw
  -- we can then check the drawn rectangle covers the whole canvas
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.scale(4, 4)
    love.graphics.rectangle('fill', 0, 0, 4, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{1,1},{1,15},{15,1},{15,15}} }, 'scale 4x')
end


-- love.graphics.shear
love.test.graphics.shear = function(test)
  -- starting at 0,0, we shear by 2x and then draw
  -- we can then check the drawn rectangle has moved over
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.origin()
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.shear(2, 0)
    love.graphics.rectangle('fill', 0, 0, 4, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata1 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata1, { red = {{1,0},{4,0},{7,3},{10,3}} }, 'shear x')
  -- same again at 0,0, we shear by 2y and then draw
  -- we can then check the drawn rectangle has moved down
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.origin()
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.shear(0, 2)
    love.graphics.rectangle('fill', 0, 0, 4, 4)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata2 = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata2, { red = { {0,1},{0,4},{3,7},{3,10}} }, 'shear y')
  test:exportImg(imgdata1)
  test:exportImg(imgdata2)
end


-- love.graphics.transformPoint
love.test.graphics.transformPoint = function(test)
  -- start with 0, 0
  local sx, sy = love.graphics.transformPoint(0, 0)
  test:assertEquals(0, sx, 'check starting x is 0')
  test:assertEquals(0, sy, 'check starting y is 0')
  -- check translation effects the point 
  love.graphics.translate(1, 5)
  sx, sy = love.graphics.transformPoint(0, 0)
  test:assertEquals(1, sx, 'check transformed x is 0')
  test:assertEquals(5, sy, 'check transformed y is 10')
end


-- love.graphics.translate
love.test.graphics.translate = function(test)
  -- starting at 0,0, we translate 4 times and draw a pixel at each point
  -- we can then check the 4 points are now red
  local canvas = love.graphics.newCanvas(16, 16)
  love.graphics.setCanvas(canvas)
    love.graphics.clear(0, 0, 0, 1)
    love.graphics.setColor(1, 0, 0, 1)
    love.graphics.translate(5, 0)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.translate(0, 5)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.translate(-5, 0)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.translate(0, -5)
    love.graphics.rectangle('fill', 0, 0, 1, 1)
    love.graphics.setColor(1, 1, 1, 1)
  love.graphics.setCanvas()
  local imgdata = love.graphics.readbackTexture(canvas, {16, 0, 0, 0, 16, 16})
  test:assertPixels(imgdata, { red = {{5,0},{0,5},{5,5},{0,0}} }, 'translate 4x')
  test:exportImg(imgdata)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-------------------------------------WINDOW-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.getDPIScale
-- @NOTE hardware dependent so can't check result
love.test.graphics.getDPIScale = function(test)
  test:assertNotNil(love.graphics.getDPIScale())
end


-- love.graphics.getDimensions
love.test.graphics.getDimensions = function(test)
  -- check graphics dimensions match window dimensions 
  local gwidth, gheight = love.graphics.getDimensions()
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wwidth, gwidth, 'check graphics dimension w matches window w')
  test:assertEquals(wheight, gheight, 'check graphics dimension h matches window h')
end


-- love.graphics.getHeight
love.test.graphics.getHeight = function(test)
  -- check graphics height match window height 
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wheight, love.graphics.getHeight(), 'check graphics h matches window h')
end


-- love.graphics.getPixelDimensions
love.test.graphics.getPixelDimensions = function(test)
  -- check graphics dimensions match window dimensions relative to dpi
  local dpi = love.graphics.getDPIScale()
  local gwidth, gheight = love.graphics.getPixelDimensions()
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wwidth, gwidth/dpi, 'check graphics pixel dpi w matches window w')
  test:assertEquals(wheight, gheight/dpi, 'check graphics pixel dpi h matches window h')
end


-- love.graphics.getPixelHeight
love.test.graphics.getPixelHeight = function(test)
  -- check graphics height match window height relative to dpi
  local dpi = love.graphics.getDPIScale()
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wheight,love.graphics.getPixelHeight()/dpi, 'check graphics pixel dpi h matches window h')
end


-- love.graphics.getPixelWidth
love.test.graphics.getPixelWidth = function(test)
  -- check graphics width match window width relative to dpi
  local dpi = love.graphics.getDPIScale()
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wwidth, love.graphics.getWidth()/dpi, 'check graphics pixel dpi w matches window w')
end


-- love.graphics.getWidth
love.test.graphics.getWidth = function(test)
  -- check graphics width match window width 
  local wwidth, wheight, _ = love.window.getMode()
  test:assertEquals(wwidth, love.graphics.getWidth(), 'check graphics w matches window w')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-------------------------------SYSTEM INFORMATION-------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.graphics.getTextureFormats
love.test.graphics.getTextureFormats = function(test)
  local formats = {
    'hdr', 'r8i', 'r8ui', 'r16i', 'r16ui', 'r32i', 'r32ui', 'rg8i', 'rg8ui',
    'rg16i', 'rg16ui', 'rg32i', 'rg32ui', 'bgra8', 'r8', 'rgba8i', 'rgba8ui',
    'rgba16i', 'rg8', 'rgba32i', 'rgba32ui', 'rgba8', 'DXT1', 'r16', 'DXT5',
    'rg16', 'BC4s', 'rgba16', 'BC5s', 'r16f', 'BC6hs', 'BC7', 'PVR1rgb2',
    'rg16f', 'PVR1rgba2', 'rgba16f', 'ETC1', 'r32f', 'ETC2rgba', 'rg32f',
    'EACr', 'rgba32f', 'EACrg', 'rgba4', 'ASTC4x4', 'ASTC5x4', 'rgb5a1',
    'ASTC6x5', 'rgb565', 'ASTC8x5', 'ASTC8x6', 'rgb10a2', 'ASTC10x5',
    'rg11b10f', 'ASTC10x8', 'ASTC10x10', 'ASTC12x10', 'ASTC12x12', 'normal',
    'srgba8', 'la8', 'ASTC10x6', 'ASTC8x8', 'ASTC6x6', 'ASTC5x5', 'EACrgs',
    'EACrs', 'ETC2rgba1', 'ETC2rgb', 'PVR1rgba4', 'PVR1rgb4', 'BC6h',
    'BC5', 'BC4', 'DXT3', 'stencil8', 'rgba16ui', 'bgra8srgb'
  }
  local supported = love.graphics.getTextureFormats({ canvas = true })
  test:assertNotNil(supported)
  for f=1,#formats do
    test:assertNotEquals(nil, supported[formats[f] ], 'expected a key for format: ' .. formats[f])
  end
end


-- love.graphics.getRendererInfo
-- @NOTE hardware dependent so best can do is nil checking
love.test.graphics.getRendererInfo = function(test)
  local name, version, vendor, device = love.graphics.getRendererInfo()
  test:assertNotNil(name)
  test:assertNotNil(version)
  test:assertNotNil(vendor)
  test:assertNotNil(device)
end


-- love.graphics.getStats
-- @NOTE cant really predict some of these so just nil check for most
love.test.graphics.getStats = function(test)
  local stattypes = {
    'drawcalls', 'canvasswitches', 'texturememory', 'shaderswitches',
    'drawcallsbatched', 'textures', 'fonts'
  }
  local stats = love.graphics.getStats()
  for s=1,#stattypes do
    test:assertNotEquals(nil, stats[stattypes[s] ], 'expected a key for stat: ' .. stattypes[s])
  end
end


-- love.graphics.getSupported
love.test.graphics.getSupported = function(test)
  -- cant check values as hardware dependent but we can check the keys in the 
  -- table match what the documentation lists
  local gfs = {
    'clampzero', 'lighten', 'glsl3', 'instancing', 'fullnpot', 
    'pixelshaderhighp', 'shaderderivatives', 'indirectdraw', 'mipmaprange',
    'copyrendertargettobuffer', 'copytexturetobuffer', 'copybuffer',
    'indexbuffer32bit', 'multirendertargetformats', 'clampone', 'blendminmax',
    'glsl4'
  }
  local features = love.graphics.getSupported()
  for g=1,#gfs do
    test:assertNotEquals(nil, features[gfs[g] ], 'expected a key for graphic feature: ' .. gfs[g])
  end
end


-- love.graphics.getSystemLimits
love.test.graphics.getSystemLimits = function(test)
  -- cant check values as hardware dependent but we can check the keys in the 
  -- table match what the documentation lists
  local glimits = {
    'texelbuffersize', 'shaderstoragebuffersize', 'threadgroupsx', 
    'threadgroupsy', 'pointsize', 'texturesize', 'texturelayers', 'volumetexturesize',
    'cubetexturesize', 'anisotropy', 'texturemsaa', 'rendertargets', 'threadgroupsz'
  }
  local limits = love.graphics.getSystemLimits()
  for g=1,#glimits do
    test:assertNotEquals(nil, limits[glimits[g] ], 'expected a key for system limit: ' .. glimits[g])
  end
end


-- love.graphics.getTextureTypes
love.test.graphics.getTextureTypes = function(test)
  -- cant check values as hardware dependent but we can check the keys in the 
  -- table match what the documentation lists
  local ttypes = {
    '2d', 'array', 'cube', 'volume'
  }
  local types = love.graphics.getTextureTypes()
  for t=1,#ttypes do
    test:assertNotEquals(nil, types[ttypes[t] ], 'expected a key for texture type: ' .. ttypes[t])
  end
end
