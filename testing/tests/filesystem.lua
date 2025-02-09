-- love.filesystem


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- File (love.filesystem.newFile)
love.test.filesystem.File = function(test)

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
  local ok1, err1 = file1:write('hello')
  test:assertNotEquals(nil, err1, 'check cant write in read mode')
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
  local ok2, err2 = file1:write('helloworld')
  test:assertTrue(ok2, 'check file write')
  test:assertEquals(nil, err2, 'check no err writing')

  -- test open/closing
  file1:open('r')
  test:assertTrue(file1:isOpen(), 'check file is open')
  file1:close()
  test:assertFalse(file1:isOpen(), 'check file gets closed')
  file1:close()

  -- test buffering and flushing
  file1:open('w')
  local ok3, err3 = file1:setBuffer('full', 10000)
  test:assertTrue(ok3)
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
love.test.filesystem.FileData = function(test)

  -- create new obj
  local fdata = love.filesystem.newFileData('helloworld', 'test.txt')
  test:assertObject(fdata)
  test:assertEquals('test.txt', fdata:getFilename())
  test:assertEquals('txt', fdata:getExtension())

  -- check properties match expected
  test:assertEquals('helloworld', fdata:getString(), 'check data string')
  test:assertEquals(10, fdata:getSize(), 'check data size')

  -- check cloning the bytedata
  local clonedfdata = fdata:clone()
  test:assertObject(clonedfdata)
  test:assertEquals('helloworld', clonedfdata:getString(), 'check cloned data')
  test:assertEquals(10, clonedfdata:getSize(), 'check cloned size')

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.filesystem.append
love.test.filesystem.append = function(test)
	-- create a new file to test with
	love.filesystem.write('filesystem.append.txt', 'foo')
	-- try appending text and check new file contents/size matches
	local success, message = love.filesystem.append('filesystem.append.txt', 'bar')
  test:assertNotEquals(false, success, 'check success')
  test:assertEquals(nil, message, 'check no error msg')
	local contents, size = love.filesystem.read('filesystem.append.txt')
	test:assertEquals(contents, 'foobar', 'check file contents')
	test:assertEquals(size, 6, 'check file size')
  -- check appending a specific no. of bytes
  love.filesystem.append('filesystem.append.txt', 'foobarfoobarfoo', 6)
  contents, size = love.filesystem.read('filesystem.append.txt')
  test:assertEquals(contents, 'foobarfoobar', 'check appended contents')
  test:assertEquals(size, 12, 'check appended size')
  -- cleanup
  love.filesystem.remove('filesystem.append.txt')
end


-- love.filesystem.areSymlinksEnabled
-- @NOTE best can do here is just check not nil
love.test.filesystem.areSymlinksEnabled = function(test)
  test:assertNotNil(love.filesystem.areSymlinksEnabled())
end


-- love.filesystem.createDirectory
love.test.filesystem.createDirectory = function(test)
  -- try creating a dir + subdir and check both exist
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
  test:assertNotNil(love.filesystem.getAppdataDirectory())
end


-- love.filesystem.getCRequirePath
love.test.filesystem.getCRequirePath = function(test)
  -- check default value from documentation
  test:assertEquals('??', love.filesystem.getCRequirePath(), 'check default value')
end


-- love.filesystem.getDirectoryItems
love.test.filesystem.getDirectoryItems = function(test)
  -- create a dir + subdir with 2 files
  love.filesystem.createDirectory('foo/bar')
	love.filesystem.write('foo/file1.txt', 'file1')
  love.filesystem.write('foo/bar/file2.txt', 'file2')
  -- check both the file + subdir exist in the item list
  local files = love.filesystem.getDirectoryItems('foo')
  local hasfile = false
  local hasdir = false
  for _,v in ipairs(files) do
    local info = love.filesystem.getInfo('foo/'..v)
    if v == 'bar' and info.type == 'directory' then hasdir = true end
    if v == 'file1.txt' and info.type == 'file' then hasfile = true end
  end
  test:assertTrue(hasfile, 'check file exists')
  test:assertTrue(hasdir, 'check directory exists')
  -- cleanup
  love.filesystem.remove('foo/file1.txt')
  love.filesystem.remove('foo/bar/file2.txt')
  love.filesystem.remove('foo/bar')
  love.filesystem.remove('foo')
end


-- love.filesystem.getFullCommonPath
love.test.filesystem.getFullCommonPath = function(test)
  -- check standard paths
  local appsavedir = love.filesystem.getFullCommonPath('appsavedir')
  local appdocuments = love.filesystem.getFullCommonPath('appdocuments')
  local userhome = love.filesystem.getFullCommonPath('userhome')
  local userappdata = love.filesystem.getFullCommonPath('userappdata')
  local userdesktop = love.filesystem.getFullCommonPath('userdesktop')
  local userdocuments = love.filesystem.getFullCommonPath('userdocuments')
  test:assertNotNil(appsavedir)
  test:assertNotNil(appdocuments)
  test:assertNotNil(userhome)
  test:assertNotNil(userappdata)
  test:assertNotNil(userdesktop)
  test:assertNotNil(userdocuments)
  -- check invalid path
  local ok = pcall(love.filesystem.getFullCommonPath, 'fakepath')
  test:assertFalse(ok, 'check invalid common path')
end


-- love.filesystem.getIdentity
love.test.filesystem.getIdentity = function(test)
  -- check setting identity matches
  local original = love.filesystem.getIdentity()
  love.filesystem.setIdentity('lover')
  test:assertEquals('lover', love.filesystem.getIdentity(), 'check identity matches')
  -- put back to original value
  love.filesystem.setIdentity(original)
end


-- love.filesystem.getRealDirectory
love.test.filesystem.getRealDirectory = function(test)
  -- make a test dir + file first
  love.filesystem.createDirectory('foo')
  love.filesystem.write('foo/test.txt', 'test')
  -- check save dir matches the real dir we just wrote to
  test:assertEquals(love.filesystem.getSaveDirectory(),
    love.filesystem.getRealDirectory('foo/test.txt'), 'check directory matches')
  -- cleanup
  love.filesystem.remove('foo/test.txt')
  love.filesystem.remove('foo')
end


-- love.filesystem.getRequirePath
love.test.filesystem.getRequirePath = function(test)
  test:assertEquals('?.lua;?/init.lua',
    love.filesystem.getRequirePath(), 'check default value')
end


-- love.filesystem.getSource
-- @NOTE i dont think we can test this cos love calls it first
love.test.filesystem.getSource = function(test)
  test:skipTest('used internally')
end


-- love.filesystem.getSourceBaseDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getSourceBaseDirectory = function(test)
  test:assertNotNil(love.filesystem.getSourceBaseDirectory())
end


-- love.filesystem.getUserDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getUserDirectory = function(test)
  test:assertNotNil(love.filesystem.getUserDirectory())
end


-- love.filesystem.getWorkingDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getWorkingDirectory = function(test)
  test:assertNotNil(love.filesystem.getWorkingDirectory())
end


-- love.filesystem.getSaveDirectory
-- @NOTE i think this is too platform dependent to be tested nicely
love.test.filesystem.getSaveDirectory = function(test)
  test:assertNotNil(love.filesystem.getSaveDirectory())
end


-- love.filesystem.getInfo
love.test.filesystem.getInfo = function(test)
  -- create a dir and subdir with a file
  love.filesystem.createDirectory('foo/bar')
  love.filesystem.write('foo/bar/file2.txt', 'file2')
  -- check getinfo returns the correct values
  test:assertEquals(nil, love.filesystem.getInfo('foo/bar/file2.txt', 'directory'), 'check not directory')
  test:assertNotEquals(nil, love.filesystem.getInfo('foo/bar/file2.txt'), 'check info not nil')
  test:assertEquals(love.filesystem.getInfo('foo/bar/file2.txt').size, 5, 'check info size match')
  test:assertFalse(love.filesystem.getInfo('foo/bar/file2.txt').readonly, 'check readonly')
  -- @TODO test modified timestamp from info.modtime?
  -- cleanup
  love.filesystem.remove('foo/bar/file2.txt')
  love.filesystem.remove('foo/bar')
  love.filesystem.remove('foo')
end


-- love.filesystem.isFused
love.test.filesystem.isFused = function(test)
  -- kinda assuming you'd run the testsuite in a non-fused game
  test:assertEquals(love.filesystem.isFused(), false, 'check not fused')
end


-- love.filesystem.lines
love.test.filesystem.lines = function(test)
  -- check lines returns the 3 lines expected
  love.filesystem.write('file.txt', 'line1\nline2\nline3')
  local linenum = 1
  for line in love.filesystem.lines('file.txt') do
    test:assertEquals('line' .. tostring(linenum), line, 'check line matches')
    -- also check it removes newlines like the docs says it does
    test:assertEquals(nil, string.find(line, '\n'), 'check newline removed')
    linenum = linenum + 1
  end
  -- cleanup
  love.filesystem.remove('file.txt')
end


-- love.filesystem.load
love.test.filesystem.load = function(test)
  -- setup some fake lua files
  love.filesystem.write('test1.lua', 'function test()\nreturn 1\nend\nreturn test()')
  love.filesystem.write('test2.lua', 'function test()\nreturn 1')

  if test:isAtLeastLuaVersion(5.2) or test:isLuaJITEnabled() then
    -- check file that doesn't exist
    local chunk1, errormsg1 = love.filesystem.load('faker.lua', 'b')
    test:assertEquals(nil, chunk1, 'check file doesnt exist')
    -- check valid lua file (text load)
    local chunk2, errormsg2 = love.filesystem.load('test1.lua', 't')
    test:assertEquals(nil, errormsg2, 'check no error message')
    test:assertEquals(1, chunk2(), 'check lua file runs')
  else
    local _, errormsg3 = love.filesystem.load('test1.lua', 'b')
    test:assertNotEquals(nil, errormsg3, 'check for an error message')

    local _, errormsg4 = love.filesystem.load('test1.lua', 't')
    test:assertNotEquals(nil, errormsg4, 'check for an error message')
  end

  -- check valid lua file (any load)
  local chunk5, errormsg5 = love.filesystem.load('test1.lua', 'bt')
  test:assertEquals(nil, errormsg5, 'check no error message')
  test:assertEquals(1, chunk5(), 'check lua file runs')

  -- check invalid lua file
  local ok, chunk, err = pcall(love.filesystem.load, 'test2.lua')
  test:assertFalse(ok, 'check invalid lua file')
  -- cleanup
  love.filesystem.remove('test1.lua')
  love.filesystem.remove('test2.lua')
end


-- love.filesystem.mount
love.test.filesystem.mount = function(test)
  -- write an example zip to savedir to use
  local contents, size = love.filesystem.read('resources/test.zip') -- contains test.txt
  love.filesystem.write('test.zip', contents, size)
  -- check mounting file and check contents are mounted
  local success = love.filesystem.mount('test.zip', 'test')
  test:assertTrue(success, 'check success')
  test:assertNotEquals(nil, love.filesystem.getInfo('test'), 'check mount not nil')
  test:assertEquals('directory', love.filesystem.getInfo('test').type, 'check directory made')
  test:assertNotEquals(nil, love.filesystem.getInfo('test/test.txt'), 'check file not nil')
  test:assertEquals('file', love.filesystem.getInfo('test/test.txt').type, 'check file type')
  -- cleanup
  love.filesystem.remove('test/test.txt')
  love.filesystem.remove('test')
  love.filesystem.remove('test.zip')
end


-- love.filesystem.mountFullPath
love.test.filesystem.mountFullPath = function(test)
  -- mount something in the working directory
  local mount = love.filesystem.mountFullPath(love.filesystem.getSource() .. '/tests', 'tests', 'read')
  test:assertTrue(mount, 'check can mount')
  -- check reading file through mounted path label
  local contents, _ = love.filesystem.read('tests/audio.lua')
  test:assertNotEquals(nil, contents)
  local unmount = love.filesystem.unmountFullPath(love.filesystem.getSource() .. '/tests')
  test:assertTrue(unmount, 'reset mount')
end


-- love.filesystem.unmountFullPath
love.test.filesystem.unmountFullPath = function(test)
  -- try unmounting something we never mounted
  local unmount1 = love.filesystem.unmountFullPath(love.filesystem.getSource() .. '/faker')
  test:assertFalse(unmount1, 'check not mounted to start with')
  -- mount something to unmount after
  love.filesystem.mountFullPath(love.filesystem.getSource() .. '/tests', 'tests', 'read')
  local unmount2 = love.filesystem.unmountFullPath(love.filesystem.getSource() .. '/tests')
  test:assertTrue(unmount2, 'check unmounted')
end


-- love.filesystem.mountCommonPath
love.test.filesystem.mountCommonPath = function(test)
  -- check if we can mount all the expected paths
  local mount1 = love.filesystem.mountCommonPath('appsavedir', 'appsavedir', 'readwrite')
  local mount2 = love.filesystem.mountCommonPath('appdocuments', 'appdocuments', 'readwrite')
  local mount3 = love.filesystem.mountCommonPath('userhome', 'userhome', 'readwrite')
  local mount4 = love.filesystem.mountCommonPath('userappdata', 'userappdata', 'readwrite')
  -- userdesktop isnt valid on linux
  if not test:isOS('Linux') then
    local mount5 = love.filesystem.mountCommonPath('userdesktop', 'userdesktop', 'readwrite')
    test:assertTrue(mount5, 'check mount userdesktop')
  end
  local mount6 = love.filesystem.mountCommonPath('userdocuments', 'userdocuments', 'readwrite')
  local ok = pcall(love.filesystem.mountCommonPath, 'fakepath', 'fake', 'readwrite')
  test:assertFalse(mount1, 'check mount appsavedir') -- This is already mounted, we can't do it again.
  test:assertTrue(mount2, 'check mount appdocuments')
  test:assertTrue(mount3, 'check mount userhome')
  test:assertTrue(mount4, 'check mount userappdata')
  test:assertTrue(mount6, 'check mount userdocuments')
  test:assertFalse(ok, 'check mount invalid common path fails')
end


-- love.filesystem.unmountCommonPath
--love.test.filesystem.unmountCommonPath = function(test)
--  -- check unmounting invalid
--  local ok = pcall(love.filesystem.unmountCommonPath, 'fakepath')
--  test:assertFalse(ok, 'check unmount invalid common path')
--  -- check mounting valid paths
--  love.filesystem.mountCommonPath('appsavedir', 'appsavedir', 'read')
--  love.filesystem.mountCommonPath('appdocuments', 'appdocuments', 'read')
--  love.filesystem.mountCommonPath('userhome', 'userhome', 'read')
--  love.filesystem.mountCommonPath('userappdata', 'userappdata', 'read')
--  love.filesystem.mountCommonPath('userdesktop', 'userdesktop', 'read')
--  love.filesystem.mountCommonPath('userdocuments', 'userdocuments', 'read')
--  local unmount1 = love.filesystem.unmountCommonPath('appsavedir')
--  local unmount2 = love.filesystem.unmountCommonPath('appdocuments')
--  local unmount3 = love.filesystem.unmountCommonPath('userhome')
--  local unmount4 = love.filesystem.unmountCommonPath('userappdata')
--  local unmount5 = love.filesystem.unmountCommonPath('userdesktop')
--  local unmount6 = love.filesystem.unmountCommonPath('userdocuments')
--  test:assertTrue(unmount1, 'check unmount appsavedir')
--  test:assertTrue(unmount2, 'check unmount appdocuments')
--  test:assertTrue(unmount3, 'check unmount userhome')
--  test:assertTrue(unmount4, 'check unmount userappdata')
--  test:assertTrue(unmount5, 'check unmount userdesktop')
--  test:assertTrue(unmount6, 'check unmount userdocuments')
--  -- remount or future tests fail
--  love.filesystem.mountCommonPath('appsavedir', 'appsavedir', 'readwrite')
--  love.filesystem.mountCommonPath('appdocuments', 'appdocuments', 'readwrite')
--  love.filesystem.mountCommonPath('userhome', 'userhome', 'readwrite')
--  love.filesystem.mountCommonPath('userappdata', 'userappdata', 'readwrite')
--  love.filesystem.mountCommonPath('userdesktop', 'userdesktop', 'readwrite')
--  love.filesystem.mountCommonPath('userdocuments', 'userdocuments', 'readwrite')
--end


-- love.filesystem.openFile
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.filesystem.openFile = function(test)
  test:assertNotNil(love.filesystem.openFile('file2.txt', 'w'))
  test:assertNotNil(love.filesystem.openFile('file2.txt', 'r'))
  test:assertNotNil(love.filesystem.openFile('file2.txt', 'a'))
  test:assertNotNil(love.filesystem.openFile('file2.txt', 'c'))
  love.filesystem.remove('file2.txt')
end


-- love.filesystem.newFileData
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.filesystem.newFileData = function(test)
  test:assertNotNil(love.filesystem.newFileData('helloworld', 'file1'))
end


-- love.filesystem.read
love.test.filesystem.read = function(test)
  -- check reading a full file
  local content, size = love.filesystem.read('resources/test.txt')
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('helloworld', content, 'check content match')
  test:assertEquals(10, size, 'check size match')
  -- check reading partial file
  content, size = love.filesystem.read('resources/test.txt', 5)
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('hello', content, 'check content match')
  test:assertEquals(5, size, 'check size match')
end


-- love.filesystem.remove
love.test.filesystem.remove = function(test)
  -- create a dir + subdir with a file
  love.filesystem.createDirectory('foo/bar')
  love.filesystem.write('foo/bar/file2.txt', 'helloworld')
  -- check removing files + dirs (should fail to remove dir if file inside)
  test:assertFalse(love.filesystem.remove('foo'), 'check fail when file inside')
  test:assertFalse(love.filesystem.remove('foo/bar'), 'check fail when file inside')
  test:assertTrue(love.filesystem.remove('foo/bar/file2.txt'), 'check file removed')
  test:assertTrue(love.filesystem.remove('foo/bar'), 'check subdirectory removed')
  test:assertTrue(love.filesystem.remove('foo'), 'check directory removed')
  -- cleanup not needed here hopefully...
end


-- love.filesystem.setCRequirePath
love.test.filesystem.setCRequirePath = function(test)
  -- check setting path val is returned
  love.filesystem.setCRequirePath('/??')
  test:assertEquals('/??', love.filesystem.getCRequirePath(), 'check crequirepath value')
  love.filesystem.setCRequirePath('??')
end


-- love.filesystem.setIdentity
love.test.filesystem.setIdentity = function(test)
  -- check setting identity val is returned
  local original = love.filesystem.getIdentity()
  love.filesystem.setIdentity('lover')
  test:assertEquals('lover', love.filesystem.getIdentity(), 'check indentity value')
  -- return value to original
  love.filesystem.setIdentity(original)
end


-- love.filesystem.setRequirePath
love.test.filesystem.setRequirePath = function(test)
  -- check setting path val is returned
  love.filesystem.setRequirePath('?.lua;?/start.lua')
  test:assertEquals('?.lua;?/start.lua', love.filesystem.getRequirePath(), 'check require path')
  -- reset to default
  love.filesystem.setRequirePath('?.lua;?/init.lua')
end


-- love.filesystem.setSource
love.test.filesystem.setSource = function(test)
  test:skipTest('used internally')
end


-- love.filesystem.unmount
love.test.filesystem.unmount = function(test)
  -- create a zip file mounted to use
  local contents, size = love.filesystem.read('resources/test.zip') -- contains test.txt
  love.filesystem.write('test.zip', contents, size)
  love.filesystem.mount('test.zip', 'test')
  -- check mounted, unmount, then check its unmounted
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
  -- check writing a bunch of files matches whats read back
  love.filesystem.write('test1.txt', 'helloworld')
  love.filesystem.write('test2.txt', 'helloworld', 10)
  love.filesystem.write('test3.txt', 'helloworld', 5)
  test:assertEquals('helloworld', love.filesystem.read('test1.txt'), 'check read file')
  test:assertEquals('helloworld', love.filesystem.read('test2.txt'), 'check read all')
  test:assertEquals('hello', love.filesystem.read('test3.txt'), 'check read partial')
  -- cleanup
  love.filesystem.remove('test1.txt')
  love.filesystem.remove('test2.txt')
  love.filesystem.remove('test3.txt')
end
