-- love.event


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.event.clear
love.test.event.clear = function(test)
  -- push some events first
  love.event.push('test', 1, 2, 3)
  love.event.push('test', 1, 2, 3)
  love.event.push('test', 1, 2, 3)
  -- check after calling clear there are no events left
  love.event.clear()
  local count = 0
  for n, a, b, c, d, e, f in love.event.poll() do
    count = count + 1
  end
  test:assertEquals(0, count, 'check no events')
end


-- love.event.poll
love.test.event.poll = function(test)
  -- push some events first
  love.event.push('test', 1, 2, 3)
  love.event.push('test', 1, 2, 3)
  love.event.push('test', 1, 2, 3)
  -- check poll recieves all events
  local count = 0
  for n, a, b, c, d, e, f in love.event.poll() do
    count = count + 1
  end
  test:assertEquals(3, count, 'check 3 events')
end


-- love.event.pump
-- @NOTE dont think can really test as internally used
love.test.event.pump = function(test)
  test:skipTest('used internally')
end


-- love.event.push
love.test.event.push = function(test)
  -- check pushing some different types
  love.event.push('add', 1, 2, 3)
  love.event.push('ignore', 1, 2, 3)
  love.event.push('add', 1, 2, 3)
  love.event.push('ignore', 1, 2, 3)
  local count = 0
  for n, a, b, c, d, e, f in love.event.poll() do
    if n == 'add' then
      count = count + a + b + c
    end
  end
  test:assertEquals(12, count, 'check total events')
end


-- love.event.quit
love.test.event.quit = function(test)
  -- setting this overrides the quit hook to prevent actually quitting
  love.test.module.fakequit = true
  love.event.quit(0)
  -- if it failed we'd have quit here
  test:assertTrue(true, 'check quit hook called')
end


-- love.event.wait
-- @NOTE not sure best way to test this one
love.test.event.wait = function(test)
  test:skipTest('used internally')
end
