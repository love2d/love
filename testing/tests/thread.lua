-- love.thread


-- love.thread.getChannel
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.getChannel = function(test)
  test:assertObject(love.thread.getChannel('test'))
end


-- love.thread.newChannel
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.newChannel = function(test)
  test:assertObject(love.thread.newChannel())
end


-- love.thread.newThread
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.newThread = function(test)
  test:assertObject(love.thread.newThread('classes/TestSuite.lua'))
end
