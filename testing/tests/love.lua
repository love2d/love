-- love
-- tests for the main love hooks + methods, mainly just that they exist

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.getVersion
love.test.love.getVersion = function(test)
  local major, minor, revision, codename = love.getVersion()
  test:assertGreaterEqual(0, major, 'check major is number')
  test:assertGreaterEqual(0, minor, 'check minor is number')
  test:assertGreaterEqual(0, revision, 'check revision is number')
  test:assertTrue(codename ~= nil, 'check has codename')
end


-- love.hasDeprecationOutput
love.test.love.hasDeprecationOutput = function(test)
  local enabled = love.hasDeprecationOutput()
  test:assertEquals(true, enabled, 'check enabled by default')
end


-- love.isVersionCompatible
love.test.love.isVersionCompatible = function(test)
  local major, minor, revision, _ = love.getVersion()
  test:assertTrue(love.isVersionCompatible(major, minor, revision), 'check own version')
end


-- love.setDeprecationOutput
love.test.love.setDeprecationOutput = function(test)
  local enabled = love.hasDeprecationOutput()
  test:assertEquals(true, enabled, 'check enabled by default')
  love.setDeprecationOutput(false)
  test:assertEquals(false, love.hasDeprecationOutput(), 'check disable')
  love.setDeprecationOutput(true)
end


-- love.errhand
love.test.love.errhand = function(test)
  test:assertTrue(type(love.errhand) == 'function', 'check defined')
end


-- love.run
love.test.love.run = function(test)
  test:assertTrue(type(love.run) == 'function', 'check defined')
end
