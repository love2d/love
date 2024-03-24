-- love.data


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- ByteData (love.data.newByteData)
love.test.data.ByteData = function(test)

  -- create new obj
  local data = love.data.newByteData('helloworld')
  test:assertObject(data)

  -- check properties match expected
  test:assertEquals('helloworld', data:getString(), 'check data string')
  test:assertEquals(10, data:getSize(), 'check data size')

  -- check cloning the bytedata
  local cloneddata = data:clone()
  test:assertObject(cloneddata)
  test:assertEquals('helloworld', cloneddata:getString(), 'check cloned data')
  test:assertEquals(10, cloneddata:getSize(), 'check cloned size')

  -- check pointer access if allowed
  if data:getFFIPointer() ~= nil and ffi ~= nil then
    local pointer = data:getFFIPointer()
    local ptr = ffi.cast('uint8_t*', pointer)
    local byte5 = ptr[4]
    test:assertEquals('o', byte5)
  end

  -- check overwriting the byte data string
  data:setString('love!', 5)
  test:assertEquals('hellolove!', data:getString(), 'check change string')

end


-- CompressedData (love.data.compress)
love.test.data.CompressedData = function(test)

  -- create new compressed data
  local cdata = love.data.compress('data', 'zlib', 'helloworld', -1)
  test:assertObject(cdata)
  test:assertEquals('zlib', cdata:getFormat(), 'check format used')

  -- check properties match expected
  test:assertEquals(18, cdata:getSize())
  test:assertEquals('helloworld', love.data.decompress('data', cdata):getString())

  -- check cloning the data
  local clonedcdata = cdata:clone()
  test:assertObject(clonedcdata)
  test:assertEquals('zlib', clonedcdata:getFormat())
  test:assertEquals(18, clonedcdata:getSize())
  test:assertEquals('helloworld', love.data.decompress('data', clonedcdata):getString())

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.data.compress
love.test.data.compress = function(test)
  -- here just testing each combo 'works' - in decompress's test method
  -- we actually check the compress + decompress give the right value back
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
    { love.data.compress('string', 'deflate', 'aaaaaa', 1), 'string'},
    { love.data.compress('string', 'deflate', 'heloworld', -1), 'string'},
    { love.data.compress('string', 'deflate', 'heloworld', 0), 'string'},
    { love.data.compress('string', 'deflate', 'heloworld', 9), 'string'},
    { love.data.compress('data', 'lz4', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'lz4', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'lz4', 'helloworld', 9), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'zlib', 'helloworld', 9), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', -1), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', 0), 'userdata'},
    { love.data.compress('data', 'gzip', 'helloworld', 9), 'userdata'},
    { love.data.compress('data', 'deflate', 'heloworld', -1), 'userdata'},
    { love.data.compress('data', 'deflate', 'heloworld', 0), 'userdata'},
    { love.data.compress('data', 'deflate', 'heloworld', 9), 'userdata'},
  }
  for c=1,#compressions do
    test:assertNotNil(compressions[c][1])
    -- sense check return type and make sure bytedata returns are an object
    test:assertEquals(compressions[c][2], type(compressions[c][1]), 'check is userdata')
    if compressions[c][2] == 'userdata' then
      test:assertNotEquals(nil, compressions[c][1]:type(), 'check has :type()')
    end
  end
end


-- love.data.decode
love.test.data.decode = function(test)
  -- setup encoded strings
  local str1 = love.data.encode('string', 'base64', 'helloworld', 0)
  local str2 = love.data.encode('string', 'hex', 'helloworld')
  local str3 = love.data.encode('data', 'base64', 'helloworld', 0)
  local str4 = love.data.encode('data', 'hex', 'helloworld')
  -- check value matches expected when decoded back
  test:assertEquals('helloworld', love.data.decode('string', 'base64', str1), 'check string base64 decode')
  test:assertEquals('helloworld', love.data.decode('string', 'hex', str2), 'check string hex decode')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decode('data', 'base64', str3):getString(), 'check data base64 decode')
  test:assertEquals(love.data.newByteData('helloworld'):getString(), love.data.decode('data', 'hex', str4):getString(), 'check data hex decode')
end


-- love.data.decompress
love.test.data.decompress = function(test)
  -- setup compressed data for each combination
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
  -- check decompressed value matches whats expected
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
  -- here just testing each combo 'works' - in decode's test method
  -- we actually check the encode + decode give the right value back
  local encodes = {
    { love.data.encode('string', 'base64', 'helloworld', 0), 'string'},
    { love.data.encode('string', 'base64', 'helloworld', 2), 'string'},
    { love.data.encode('string', 'hex', 'helloworld'), 'string'},
    { love.data.encode('data', 'base64', 'helloworld', 0), 'userdata'},
    { love.data.encode('data', 'base64', 'helloworld', 2), 'userdata'},
    { love.data.encode('data', 'hex', 'helloworld'), 'userdata'}
  }
  for e=1,#encodes do
    test:assertNotNil(encodes[e][1])
    -- sense check return type and make sure bytedata returns are an object
    test:assertEquals(encodes[e][2], type(encodes[e][1]), 'check is usedata')
    if encodes[e][2] == 'userdata' then
      test:assertNotEquals(nil, encodes[e][1]:type(), 'check has :type()')
    end
  end

end


-- love.data.getPackedSize
love.test.data.getPackedSize = function(test)
  local pack1 = love.data.getPackedSize('>xI3b')
  local pack2 = love.data.getPackedSize('>I2B')
  local pack3 = love.data.getPackedSize('>I4I4I4I4x')
  test:assertEquals(5, pack1, 'check pack size 1')
  test:assertEquals(3, pack2, 'check pack size 2')
  test:assertEquals(17, pack3, 'check pack size 3')
end


-- love.data.hash
love.test.data.hash = function(test)
  -- setup all the different hashing types
  local str1 = love.data.hash('string', 'md5', 'helloworld')
  local str2 = love.data.hash('string', 'sha1', 'helloworld')
  local str3 = love.data.hash('string', 'sha224', 'helloworld')
  local str4 = love.data.hash('string', 'sha256', 'helloworld')
  local str5 = love.data.hash('string', 'sha384', 'helloworld')
  local str6 = love.data.hash('string', 'sha512', 'helloworld')
  local data1 = love.data.hash('data', 'md5', 'helloworld')
  local data2 = love.data.hash('data', 'sha1', 'helloworld')
  local data3 = love.data.hash('data', 'sha224', 'helloworld')
  local data4 = love.data.hash('data', 'sha256', 'helloworld')
  local data5 = love.data.hash('data', 'sha384', 'helloworld')
  local data6 = love.data.hash('data', 'sha512', 'helloworld')
  -- check encoded hash value matches what's expected for that algo
    -- test container string
  test:assertEquals('fc5e038d38a57032085441e7fe7010b0', love.data.encode("string", "hex", str1), 'check string md5 encode')
  test:assertEquals('6adfb183a4a2c94a2f92dab5ade762a47889a5a1', love.data.encode("string", "hex", str2), 'check string sha1 encode')
  test:assertEquals('b033d770602994efa135c5248af300d81567ad5b59cec4bccbf15bcc', love.data.encode("string", "hex", str3), 'check string sha224 encode')
  test:assertEquals('936a185caaa266bb9cbe981e9e05cb78cd732b0b3280eb944412bb6f8f8f07af', love.data.encode("string", "hex", str4), 'check string sha256 encode')
  test:assertEquals('97982a5b1414b9078103a1c008c4e3526c27b41cdbcf80790560a40f2a9bf2ed4427ab1428789915ed4b3dc07c454bd9', love.data.encode("string", "hex", str5), 'check string sha384 encode')
  test:assertEquals('1594244d52f2d8c12b142bb61f47bc2eaf503d6d9ca8480cae9fcf112f66e4967dc5e8fa98285e36db8af1b8ffa8b84cb15e0fbcf836c3deb803c13f37659a60', love.data.encode("string", "hex", str6), 'check string sha512 encode')
    -- test container data
  test:assertEquals('fc5e038d38a57032085441e7fe7010b0', love.data.encode("string", "hex", data1), 'check data md5 encode')
  test:assertEquals('6adfb183a4a2c94a2f92dab5ade762a47889a5a1', love.data.encode("string", "hex", data2), 'check data sha1 encode')
  test:assertEquals('b033d770602994efa135c5248af300d81567ad5b59cec4bccbf15bcc', love.data.encode("string", "hex", data3), 'check data sha224 encode')
  test:assertEquals('936a185caaa266bb9cbe981e9e05cb78cd732b0b3280eb944412bb6f8f8f07af', love.data.encode("string", "hex", data4), 'check data sha256 encode')
  test:assertEquals('97982a5b1414b9078103a1c008c4e3526c27b41cdbcf80790560a40f2a9bf2ed4427ab1428789915ed4b3dc07c454bd9', love.data.encode("string", "hex", data5), 'check data sha384 encode')
  test:assertEquals('1594244d52f2d8c12b142bb61f47bc2eaf503d6d9ca8480cae9fcf112f66e4967dc5e8fa98285e36db8af1b8ffa8b84cb15e0fbcf836c3deb803c13f37659a60', love.data.encode("string", "hex", data6), 'check data sha512 encode')
end


-- love.data.newByteData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.data.newByteData = function(test)
  test:assertObject(love.data.newByteData('helloworld'))
end


-- love.data.newDataView
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.data.newDataView = function(test)
  test:assertObject(love.data.newDataView(love.data.newByteData('helloworld'), 0, 10))
end


-- love.data.pack
love.test.data.pack = function(test)
  local packed1 = love.data.pack('string', '>I4I4I4I4', 9999, 1000, 1010, 2030)
  local packed2 = love.data.pack('data', '>I4I4I4I4', 9999, 1000, 1010, 2030)
  local a, b, c, d = love.data.unpack('>I4I4I4I4', packed1)
  local e, f, g, h = love.data.unpack('>I4I4I4I4', packed2)
  test:assertEquals(9999+9999, a+e, 'check packed 1')
  test:assertEquals(1000+1000, b+f, 'check packed 2')
  test:assertEquals(1010+1010, c+g, 'check packed 3')
  test:assertEquals(2030+2030, d+h, 'check packed 4')
end


-- love.data.unpack
love.test.data.unpack = function(test)
  local packed1 = love.data.pack('string', '>s5s4I3', 'hello', 'love', 100)
  local packed2 = love.data.pack('data', '>s5I2', 'world', 20)
  local a, b, c = love.data.unpack('>s5s4I3', packed1)
  local d, e = love.data.unpack('>s5I2', packed2)
  test:assertEquals(a .. ' ' .. d, 'hello world', 'check unpack 1')
  test:assertEquals(b, 'love', 'check unpack 2')
  test:assertEquals(c - e, 80, 'check unpack 3')
end
