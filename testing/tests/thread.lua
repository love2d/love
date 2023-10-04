-- love.thread


-- love.thread.getChannel
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.getChannel = function(test)
  local channel = love.thread.getChannel('test')
  test:assertObject(channel)
  channel:release()
end


-- love.thread.newChannel
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.newChannel = function(test)
  local channel = love.thread.newChannel()
  test:assertObject(channel)
  channel:release()
end


-- love.thread.newThread
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.thread.newThread = function(test)
  local thread = love.thread.newThread('classes/TestSuite.lua')
  test:assertObject(thread)
  thread:release()
end