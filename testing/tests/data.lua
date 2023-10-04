-- love.data


-- love.data.compress
love.test.data.compress = function(test)
  -- here just testing each combo 'works', in decompress's test method
  -- we actually check the compress + decompress give the right value back
  -- setup
  local compressions = {
    { love.data.compress('string', 'lz4', 'helloworld', -1), 'string'},
    { love.data.compress('string', 'lz4', 'helloworld', 0), 'string'},
    { love.data.compress('string', 'lz4', 'helloworld', 9), 'string'},
    { love.data.compress('string', 'zlib', 'helloworld', -1), 'string'},
    { love.data.compress('string', 'zlib', 'helloworld', 0), 'string'},
    { love.data.compress('string', 'zlib', 'helloworld', 9), 'string'},
    { love.data.compress('string', 'gzip', 'helloworld', -1), 'string'},
    { love.data.compress('string', 'gzip', 'helloworld', 0), 'string'},
    { love.data.compress('string', 'gzip', 'helloworld', 9), 'string'},
    { love.data.compress('data', 'lz4', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'lz4', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'lz4', 'helloworld', 9), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', 9), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', 9), 'userdata'}
  }
  -- tests
  for c=1,#compressions do
    test:assertNotEquals(nil, compressions[c][1], 'check not nil')
    -- sense check return type and make sure bytedata returns are an object
    test:assertEquals(compressions[c][2], type(compressions[c][1]), 'check is userdata')
    if compressions[c][2] == 'userdata' then
      test:assertNotEquals(nil, compressions[c][1]:type(), 'check has :type()')
    end
  end
end


-- love.data.decode
love.test.data.decode = function(test)
  -- setup
  local str1 = love.data.encode('string', 'base64', 'helloworld', 0)
  local str2 = love.data.encode('string', 'hex', 'helloworld')
  local str3 = love.data.encode('data', 'base64', 'helloworld', 0)
  local str4 = love.data.encode('data', 'hex', 'helloworld')
  -- tests
  test:assertEquals('helloworld', love.data.decode('string', 'base64', str1), 'check string base64 decode')
  test:assertEquals('helloworld', love.data.decode('string', 'hex', str2), 'check string hex decode')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decode('data', 'base64', str3):getString(), 'check data base64 decode')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decode('data', 'hex', str4):getString(), 'check data hex decode')
end


-- love.data.decompress
love.test.data.decompress = function(test)
  -- setup
  local str1 = love.data.compress('string', 'lz4', 'helloworld', -1)
  local str2 = love.data.compress('string', 'lz4', 'helloworld', 0)
  local str3 = love.data.compress('string', 'lz4', 'helloworld', 9)
  local str4 = love.data.compress('string', 'zlib', 'helloworld', -1)
  local str5 = love.data.compress('string', 'zlib', 'helloworld', 0)
  local str6 = love.data.compress('string', 'zlib', 'helloworld', 9)
  local str7 = love.data.compress('string', 'gzip', 'helloworld', -1)
  local str8 = love.data.compress('string', 'gzip', 'helloworld', 0)
  local str9 = love.data.compress('string', 'gzip', 'helloworld', 9)
  local str10 = love.data.compress('data', 'lz4', 'helloworld', -1)
  local str11 = love.data.compress('data', 'lz4', 'helloworld', 0)
  local str12 = love.data.compress('data', 'lz4', 'helloworld', 9)
  local str13 = love.data.compress('data', 'zlib', 'helloworld', -1)
  local str14 = love.data.compress('data', 'zlib', 'helloworld', 0)
  local str15 = love.data.compress('data', 'zlib', 'helloworld', 9)
  local str16 = love.data.compress('data', 'gzip', 'helloworld', -1)
  local str17 = love.data.compress('data', 'gzip', 'helloworld', 0)
  local str18 = love.data.compress('data', 'gzip', 'helloworld', 9)
  -- tests
  test:assertEquals('helloworld', love.data.decompress('string', 'lz4', str1), 'check string lz4 decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'lz4', str2), 'check string lz4 decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'lz4', str3), 'check string lz4 decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'zlib', str4), 'check string zlib decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'zlib', str5), 'check string zlib decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'zlib', str6), 'check string zlib decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'gzip', str7), 'check string glib decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'gzip', str8), 'check string glib decompress')
  test:assertEquals('helloworld', love.data.decompress('string', 'gzip', str9), 'check string glib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'lz4', str10):getString(), 'check data lz4 decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'lz4', str11):getString(), 'check data lz4 decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'lz4', str12):getString(), 'check data lz4 decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'zlib', str13):getString(), 'check data zlib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'zlib', str14):getString(), 'check data zlib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'zlib', str15):getString(), 'check data zlib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'gzip', str16):getString(), 'check data glib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'gzip', str17):getString(), 'check data glib decompress')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decompress('data', 'gzip', str18):getString(), 'check data glib decompress')
end


-- love.data.encode
love.test.data.encode = function(test)
  -- here just testing each combo 'works', in decode's test method
  -- we actually check the encode + decode give the right value back
  -- setup
  local encodes = {
    { love.data.encode('string', 'base64', 'helloworld', 0), 'string'},
    { love.data.encode('string', 'base64', 'helloworld', 2), 'string'},
    { love.data.encode('string', 'hex', 'helloworld'), 'string'},
    { love.data.encode('data', 'base64', 'helloworld', 0), 'userdata'},
    { love.data.encode('data', 'base64', 'helloworld', 2), 'userdata'},
    { love.data.encode('data', 'hex', 'helloworld'), 'userdata'}
  }
  -- tests
  for e=1,#encodes do
    test:assertNotEquals(nil, encodes[e][1], 'check not nil')
    -- sense check return type and make sure bytedata returns are an object
    test:assertEquals(encodes[e][2], type(encodes[e][1]), 'check is usedata')
    if encodes[e][2] == 'userdata' then
      test:assertNotEquals(nil, encodes[e][1]:type(), 'check has :type()')
    end
  end

end


-- love.data.getPackedSize
-- @NOTE I dont really get the packing types of lua so best someone else writes this one
love.test.data.getPackedSize = function(test)
  test:skipTest('dont understand lua packing types')
end


-- love.data.hash
love.test.data.hash = function(test)
  -- setup
  local str1 = love.data.hash('md5', 'helloworld')
  local str2 = love.data.hash('sha1', 'helloworld')
  local str3 = love.data.hash('sha224', 'helloworld')
  local str4 = love.data.hash('sha256', 'helloworld')
  local str5 = love.data.hash('sha384', 'helloworld')
  local str6 = love.data.hash('sha512', 'helloworld')
  -- tests
  test:assertEquals('fc5e038d38a57032085441e7fe7010b0', love.data.encode("string", "hex", str1), 'check md5 encode')
  test:assertEquals('6adfb183a4a2c94a2f92dab5ade762a47889a5a1', love.data.encode("string", "hex", str2), 'check sha1 encode')
  test:assertEquals('b033d770602994efa135c5248af300d81567ad5b59cec4bccbf15bcc', love.data.encode("string", "hex", str3), 'check sha224 encode')
  test:assertEquals('936a185caaa266bb9cbe981e9e05cb78cd732b0b3280eb944412bb6f8f8f07af', love.data.encode("string", "hex", str4), 'check sha256 encode')
  test:assertEquals('97982a5b1414b9078103a1c008c4e3526c27b41cdbcf80790560a40f2a9bf2ed4427ab1428789915ed4b3dc07c454bd9', love.data.encode("string", "hex", str5), 'check sha384 encode')
  test:assertEquals('1594244d52f2d8c12b142bb61f47bc2eaf503d6d9ca8480cae9fcf112f66e4967dc5e8fa98285e36db8af1b8ffa8b84cb15e0fbcf836c3deb803c13f37659a60', love.data.encode("string", "hex", str6), 'check sha512 encode')
end


-- love.data.newByteData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.data.newByteData = function(test)
  -- setup
  local bytedata = love.data.newByteData('helloworld')
  -- tests
  test:assertObject(bytedata)
end


-- love.data.newDataView
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.data.newDataView = function(test)
  -- setup
  local dataview = love.data.newDataView(love.data.newByteData('helloworld'), 0, 10)
  -- tests
  test:assertObject(dataview)
end


-- love.data.pack
-- @NOTE i dont really get the packing types of lua so best someone else writes this one
love.test.data.pack = function(test)
  test:skipTest('dont understand lua packing types')
end


-- love.data.unpack
-- @NOTE i dont really get the packing types of lua so best someone else writes this one
love.test.data.unpack = function(test)
  test:skipTest('dont understand lua packing types')
end