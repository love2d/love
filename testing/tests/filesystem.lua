-- love.filesystem


-- love.filesystem.append
love.test.filesystem.append = function(test)
	-- setup
	love.filesystem.write('filesystem.append.txt', 'foo')
	-- test
	local success, message = love.filesystem.append('filesystem.append.txt', 'bar')
  test:assertNotEquals(false, success, 'check success')
  test:assertEquals(nil, message, 'check no error msg')
	local contents, size = love.filesystem.read('filesystem.append.txt')
	test:assertEquals(contents, 'foobar', 'check file contents')
	test:assertEquals(size, 6, 'check file size')
  love.filesystem.append('filesystem.append.txt', 'foobarfoobarfoo', 6)
  contents, size = love.filesystem.read('filesystem.append.txt')
  test:assertEquals(contents, 'foobarfoobar', 'check appended contents')
  test:assertEquals(size, 12, 'check appended size')
  -- cleanup
  love.filesystem.remove('filesystem.append.txt')
end


-- love.filesystem.areSymlinksEnabled
-- @NOTE  this one, need to see if default val is consistent on platforms?
love.test.filesystem.areSymlinksEnabled = function(test)
  test:assertNotEquals(nil, love.filesystem.areSymlinksEnabled())
end


-- love.filesystem.createDirectory
love.test.filesystem.createDirectory = function(test)
  local success = love.filesystem.createDirectory('foo/bar')
  test:assertNotEquals(false, success, 'check success')
  test:assertNotEquals(nil, love.filesystem.getInfo('foo', 'directory'), 'check directory created')
  test:assertNotEquals(nil, love.filesystem.getInfo('foo/bar', 'directory'), 'check subdirectory created')
  -- cleanup
  love.filesystem.remove('foo/bar')
  love.filesystem.remove('foo')
end


-- love.filesystem.getAppdataDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getAppdataDirectory = function(test)
  test:assertNotEquals(nil, love.filesystem.getAppdataDirectory(), 'check not nill')
end


-- love.filesystem.getCRequirePath
love.test.filesystem.getCRequirePath = function(test)
  test:assertEquals('??', love.filesystem.getCRequirePath(), 'check default value')
end


-- love.filesystem.getDirectoryItems
love.test.filesystem.getDirectoryItems = function(test)
  -- setup
  love.filesystem.createDirectory('foo/bar')
	love.filesystem.write('foo/file1.txt', 'file1')
  love.filesystem.write('foo/bar/file2.txt', 'file2')
  -- tests
  local files = love.filesystem.getDirectoryItems('foo')
  local hasfile = false
  local hasdir = false
  for _,v in ipairs(files) do
    local info = love.filesystem.getInfo('foo/'..v)
    if v == 'bar' and info.type == 'directory' then hasdir = true end
    if v == 'file1.txt' and info.type == 'file' then hasfile = true end
  end
  test:assertEquals(true, hasfile, 'check file exists')
  test:assertEquals(true, hasdir, 'check directory exists')
  -- cleanup
  love.filesystem.remove('foo/file1.txt')
  love.filesystem.remove('foo/bar/file2.txt')
  love.filesystem.remove('foo/bar')
  love.filesystem.remove('foo')
end


-- love.filesystem.getIdentity
love.test.filesystem.getIdentity = function(test)
  -- setup
  local original = love.filesystem.getIdentity()
  love.filesystem.setIdentity('lover')
  -- test
  test:assertEquals('lover', love.filesystem.getIdentity(), 'check identity matches')
  -- cleanup
  love.filesystem.setIdentity(original)
end


-- love.filesystem.getRealDirectory
love.test.filesystem.getRealDirectory = function(test)
  -- setup
  love.filesystem.createDirectory('foo')
  love.filesystem.write('foo/test.txt', 'test')
  -- test
  test:assertEquals(love.filesystem.getSaveDirectory(), love.filesystem.getRealDirectory('foo/test.txt'), 'check directory matches')
  -- cleanup
  love.filesystem.remove('foo/test.txt')
  love.filesystem.remove('foo')
end


-- love.filesystem.getRequirePath
love.test.filesystem.getRequirePath = function(test)
  test:assertEquals('?.lua;?/init.lua', love.filesystem.getRequirePath(), 'check default value')
end


-- love.filesystem.getSource
-- @NOTE i dont think we can test this cos love calls it first
love.test.filesystem.getSource = function(test)
  test:skipTest('not sure we can test when its internal?')
end


-- love.filesystem.getSourceBaseDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getSourceBaseDirectory = function(test)
  test:assertNotEquals(nil, love.filesystem.getSourceBaseDirectory(), 'check not nil')
end


-- love.filesystem.getUserDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getUserDirectory = function(test)
  test:assertNotEquals(nil, love.filesystem.getUserDirectory(), 'check not nil')
end


-- love.filesystem.getWorkingDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getWorkingDirectory = function(test)
  test:assertNotEquals(nil, love.filesystem.getWorkingDirectory(), 'check not nil')
end


-- love.filesystem.getSaveDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getSaveDirectory = function(test)
  test:assertNotEquals(nil, love.filesystem.getSaveDirectory(), 'check not nil')
end


-- love.filesystem.getInfo
love.test.filesystem.getInfo = function(test)
  -- setup
  love.filesystem.createDirectory('foo/bar')
  love.filesystem.write('foo/bar/file2.txt', 'file2')
  -- tests
  test:assertEquals(nil, love.filesystem.getInfo('foo/bar/file2.txt', 'directory'), 'check not directory')
  test:assertNotEquals(nil, love.filesystem.getInfo('foo/bar/file2.txt'), 'check info not nil')
  test:assertEquals(love.filesystem.getInfo('foo/bar/file2.txt').size, 5, 'check info size match')
  -- @TODO test modified timestamp from info.modtime?
  -- cleanup
  love.filesystem.remove('foo/bar/file2.txt')
  love.filesystem.remove('foo/bar')
  love.filesystem.remove('foo')
end


-- love.filesystem.isFused
love.test.filesystem.isFused = function(test)
  -- kinda assuming you'd run the testsuite in a non-fused game
  test:assertEquals(love.filesystem.isFused(), false, 'check default value')
end


-- love.filesystem.lines
love.test.filesystem.lines = function(test)
  -- setup
  love.filesystem.write('file.txt', 'line1\nline2\nline3')
  -- tests
  local linenum = 1
  for line in love.filesystem.lines('file.txt') do
    test:assertEquals('line' .. tostring(linenum), line, 'check line matches')
    test:assertEquals(nil, string.find(line, '\n'), 'check newline removed')
    linenum = linenum + 1
  end
  -- cleanup
  love.filesystem.remove('file.txt')
end


-- love.filesystem.load
love.test.filesystem.load = function(test)
  -- setup
  love.filesystem.write('test1.lua', 'function test()\nreturn 1\nend\nreturn test()')
  love.filesystem.write('test2.lua', 'function test()\nreturn 1')
  -- tests
  local chunk, errormsg = love.filesystem.load('faker.lua')
  test:assertEquals(nil, chunk, 'check file doesnt exist')
  chunk, errormsg = love.filesystem.load('test1.lua')
  test:assertEquals(nil, errormsg, 'check no error message')
  test:assertEquals(1, chunk(), 'check lua file runs')
  local ok, chunk, err = pcall(love.filesystem.load, 'test2.lua')
  test:assertEquals(false, ok, 'check invalid lua file')
  -- cleanup
  love.filesystem.remove('test1.lua')
  love.filesystem.remove('test2.lua')
end


-- love.filesystem.mount
love.test.filesystem.mount = function(test)
  -- setup 
  local contents, size = love.filesystem.read('resources/test.zip') -- contains test.txt
  love.filesystem.write('test.zip', contents, size)
  -- tests
  local success = love.filesystem.mount('test.zip', 'test')
  test:assertEquals(true, success, 'check success')
  test:assertNotEquals(nil, love.filesystem.getInfo('test'), 'check mount not nil')
  test:assertEquals('directory', love.filesystem.getInfo('test').type, 'check directory made')
  test:assertNotEquals(nil, love.filesystem.getInfo('test/test.txt'), 'check file not nil')
  test:assertEquals('file', love.filesystem.getInfo('test/test.txt').type, 'check file type')
  -- cleanup
  love.filesystem.remove('test/test.txt')
  love.filesystem.remove('test')
  love.filesystem.remove('test.zip')
end


-- love.filesystem.newFile
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.filesystem.newFile = function(test)
  -- setup
  local file = love.filesystem.newFile('file1')
  local file_r, err_r = love.filesystem.newFile('file2', 'r')
  local file_w, err_w = love.filesystem.newFile('file2', 'w')
  local file_a, err_a = love.filesystem.newFile('file2', 'a')
  local file_c, err_c = love.filesystem.newFile('file2', 'c')
  -- tests
  test:assertNotEquals(nil, file, 'check file made')
  test:assertNotEquals(nil, file_r, 'check file made')
  test:assertNotEquals(nil, file_w, 'check file made')
  test:assertNotEquals(nil, file_a, 'check file made')
  test:assertNotEquals(nil, file_c, 'check file made')
  -- cleanup
  if file ~= nil then file:release() end
  if file_r ~= nil then file_r:release() end
  if file_w ~= nil then file_w:release() end
  if file_a ~= nil then file_a:release() end
  if file_c ~= nil then file_c:release() end
end


-- love.filesystem.newFileData
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.filesystem.newFileData = function(test)
  local data = love.filesystem.newFileData('helloworld', 'file1')
  test:assertNotEquals(nil, data, 'check not nil')
  data:release()
end


-- love.filesystem.read
love.test.filesystem.read = function(test)
  local content, size = love.filesystem.read('resources/test.txt')
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('helloworld', content, 'check content match')
  test:assertEquals(10, size, 'check size match')
  content, size = love.filesystem.read('resources/test.txt', 5)
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('hello', content, 'check content match')
  test:assertEquals(5, size, 'check size match')
end


-- love.filesystem.remove
love.test.filesystem.remove = function(test)
  -- setup
  love.filesystem.createDirectory('foo/bar')
  love.filesystem.write('foo/bar/file2.txt', 'helloworld')
  -- tests
  test:assertEquals(false, love.filesystem.remove('foo'), 'check fail when file inside')
  test:assertEquals(false, love.filesystem.remove('foo/bar'), 'check fail when file inside')
  test:assertEquals(true, love.filesystem.remove('foo/bar/file2.txt'), 'check file removed')
  test:assertEquals(true, love.filesystem.remove('foo/bar'), 'check subdirectory removed')
  test:assertEquals(true, love.filesystem.remove('foo'), 'check directory removed')
  -- cleanup not needed here
end


-- love.filesystem.setCRequirePath
love.test.filesystem.setCRequirePath = function(test)
  love.filesystem.setCRequirePath('/??')
  test:assertEquals('/??', love.filesystem.getCRequirePath(), 'check crequirepath value')
  love.filesystem.setCRequirePath('??')
end


-- love.filesystem.setIdentity
love.test.filesystem.setIdentity = function(test)
  -- setup
  local original = love.filesystem.getIdentity()
  -- test
  love.filesystem.setIdentity('lover')
  test:assertEquals('lover', love.filesystem.getIdentity(), 'check indentity value')
  -- cleanup
  love.filesystem.setIdentity(original)
end


-- love.filesystem.setRequirePath
love.test.filesystem.setRequirePath = function(test)
  -- test
  love.filesystem.setRequirePath('?.lua;?/start.lua')
  test:assertEquals('?.lua;?/start.lua', love.filesystem.getRequirePath(), 'check require path')
  -- cleanup
  love.filesystem.setRequirePath('?.lua;?/init.lua')
end


-- love.filesystem.setSource
-- @NOTE dont think can test this cos used internally?
love.test.filesystem.setSource = function(test)
  test:skipTest('not sure we can test when its internal?')
end


-- love.filesystem.unmount
love.test.filesystem.unmount = function(test)
  --setup
  local contents, size = love.filesystem.read('resources/test.zip') -- contains test.txt
  love.filesystem.write('test.zip', contents, size)
  love.filesystem.mount('test.zip', 'test')
  -- tests
  test:assertNotEquals(nil, love.filesystem.getInfo('test/test.txt'), 'check mount exists')
  love.filesystem.unmount('test.zip')
  test:assertEquals(nil, love.filesystem.getInfo('test/test.txt'), 'check unmounted')
  -- cleanup
  love.filesystem.remove('test/test.txt')
  love.filesystem.remove('test')
  love.filesystem.remove('test.zip')
end


-- love.filesystem.write
love.test.filesystem.write = function(test)
  -- setup
  love.filesystem.write('test1.txt', 'helloworld')
  love.filesystem.write('test2.txt', 'helloworld', 10)
  love.filesystem.write('test3.txt', 'helloworld', 5)
  -- test
  test:assertEquals('helloworld', love.filesystem.read('test1.txt'), 'check read file')
  test:assertEquals('helloworld', love.filesystem.read('test2.txt'), 'check read all')
  test:assertEquals('hello', love.filesystem.read('test3.txt'), 'check read partial')
  -- cleanup
  love.filesystem.remove('test1.txt')
  love.filesystem.remove('test2.txt')
  love.filesystem.remove('test3.txt')
end