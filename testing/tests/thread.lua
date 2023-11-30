-- love.thread


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------OBJECTS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Channel (love.thread.newChannel)
love.test.thread.Channel = function(test)

  -- create channel
  local channel = love.thread.getChannel('test')
  test:assertObject(channel)

  -- setup thread to use
  local threadcode1 = [[
    require("love.timer")
    love.timer.sleep(0.1)
    love.thread.getChannel('test'):push('hello world')
    love.timer.sleep(0.1)
    love.thread.getChannel('test'):push('me again')
  ]]
  local thread1 = love.thread.newThread(threadcode1)
  thread1:start()

  -- check message sent from thread to channel
  local msg1 = channel:demand()
  test:assertEquals('hello world', msg1, 'check 1st message was sent')
  thread1:wait()
  test:assertEquals(1, channel:getCount(), 'check still another message')
  test:assertEquals('me again', channel:peek(), 'check 2nd message pending')
  local msg2 = channel:pop()
  test:assertEquals('me again', msg2, 'check 2nd message was sent')
  channel:clear()

  -- setup another thread for some ping pong
  local threadcode2 = [[
    local function setChannel(channel, value)
      channel:clear()
      return channel:push(value)
    end
    local channel = love.thread.getChannel('test')
    local waiting = true
    local sent = nil
    while waiting == true do
      if sent == nil then
        sent = channel:performAtomic(setChannel, 'ping')
      end
      if channel:hasRead(sent) then
        local msg = channel:demand()
        if msg == 'pong' then 
          channel:push(msg)
          waiting = false
        end
      end
    end
  ]]

  -- first we run a thread that will send 1 ping
  local thread2 = love.thread.newThread(threadcode2)
  thread2:start()

  -- we wait for that ping to be sent and then send a pong back
  local msg3 = channel:demand()
  test:assertEquals('ping', msg3, 'check message recieved 1')

  -- thread should be waiting for us, and checking is the ping was read
  channel:supply('pong', 1)

  -- if it was then it should send back our pong and thread should die
  thread2:wait()
  local msg4 = channel:pop()
  test:assertEquals('pong', msg4, 'check message recieved 2')
  test:assertEquals(0, channel:getCount())

end


-- Thread (love.thread.newThread)
love.test.thread.Thread = function(test)

  -- create thread
  local threadcode = [[
    local b = 0
    for a=1,100000 do 
      b = b + a 
    end
  ]]
  local thread = love.thread.newThread(threadcode)
  test:assertObject(thread)

  -- check thread runs
  thread:start()
  test:assertTrue(thread:isRunning(), 'check started')
  thread:wait()
  test:assertFalse(thread:isRunning(), 'check finished')
  test:assertEquals(nil, thread:getError(), 'check no errors')

  -- check an invalid thread
  local badthreadcode = 'local b = 0\nreturn b + "string" .. 10'
  local badthread = love.thread.newThread(badthreadcode)
  badthread:start()
  badthread:wait()
  test:assertNotNil(badthread:getError())

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------METHODS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.thread.getChannel
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.thread.getChannel = function(test)
  test:assertObject(love.thread.getChannel('test'))
end


-- love.thread.newChannel
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.thread.newChannel = function(test)
  test:assertObject(love.thread.newChannel())
end


-- love.thread.newThread
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.thread.newThread = function(test)
  test:assertObject(love.thread.newThread('classes/TestSuite.lua'))
end
