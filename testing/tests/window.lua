-- love.window


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------METHODS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.window.focus
love.test.window.focus = function(test)
  -- cant test as doesnt return anything
  test:assertEquals('function', type(love.window.focus), 'check method exists')
end


-- love.window.fromPixels
love.test.window.fromPixels = function(test)
  -- check dpi/pixel ratio as expected
  local dpi = love.window.getDPIScale()
  local pixels = love.window.fromPixels(100)
  test:assertEquals(100/dpi, pixels, 'check dpi ratio')
end


-- love.window.getDPIScale
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDPIScale = function(test)
  test:assertNotNil(test)
end


-- love.window.getDesktopDimensions
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDesktopDimensions = function(test)
  local w, h = love.window.getDesktopDimensions()
  test:assertNotNil(w)
  test:assertNotNil(h)
end


-- love.window.getDisplayCount
-- @NOTE cant wait for the test suite to be run headless and fail here
love.test.window.getDisplayCount = function(test)
  test:assertGreaterEqual(1, love.window.getDisplayCount(), 'check 1 display')
end


-- love.window.getDisplayName
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDisplayName = function(test)
  test:assertNotNil(love.window.getDisplayName(1))
end


-- love.window.getDisplayOrientation
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDisplayOrientation = function(test)
  test:assertNotNil(love.window.getDisplayOrientation(1))
end


-- love.window.getFullscreen
love.test.window.getFullscreen = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support fullscreen")
  end

  -- check not fullscreen to start
  test:assertFalse(love.window.getFullscreen(), 'check not fullscreen')
  love.window.setFullscreen(true)
  -- check now fullscreen
  test:assertTrue(love.window.getFullscreen(), 'check now fullscreen')
  love.window.setFullscreen(false) -- reset
end


-- love.window.getFullscreenModes
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getFullscreenModes = function(test)
  test:assertNotNil(love.window.getFullscreenModes(1))
end


-- love.window.getIcon
love.test.window.getIcon = function(test)
  -- check icon nil by default if not set
  test:assertEquals(nil, love.window.getIcon(), 'check nil by default')
  local icon = love.image.newImageData('resources/love.png')
  -- check getting icon not nil after setting
  love.window.setIcon(icon)
  test:assertNotNil(love.window.getIcon())
end


-- love.window.getMode
-- @NOTE could prob add more checks on the flags here based on conf.lua
love.test.window.getMode = function(test)
  local w, h, flags = love.window.getMode()
  test:assertEquals(360, w, 'check w')
  test:assertEquals(240, h, 'check h')
  test:assertFalse(flags["fullscreen"], 'check fullscreen')
end


-- love.window.getPosition
-- @NOTE anything we could check display index agaisn't in getPosition return?
love.test.window.getPosition = function(test)
  love.window.setPosition(100, 100, 1)
  local x, y, _ = love.window.getPosition()
  test:assertEquals(100, x, 'check position x')
  test:assertEquals(100, y, 'check position y')
end


-- love.window.getSafeArea
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getSafeArea = function(test)
  local x, y, w, h = love.window.getSafeArea()
  test:assertNotNil(x)
  test:assertNotNil(y)
  test:assertNotNil(w)
  test:assertNotNil(h)
end


-- love.window.getTitle
love.test.window.getTitle = function(test)
  -- check title returned is what was set
  love.window.setTitle('love.testing')
  test:assertEquals('love.testing', love.window.getTitle(), 'check title match')
  love.window.setTitle('love.test')
end


-- love.window.getVSync
love.test.window.getVSync = function(test)
  test:assertNotNil(love.window.getVSync())
end


-- love.window.hasFocus
-- @NOTE cant really test as cant force focus
love.test.window.hasFocus = function(test)
  test:assertNotNil(love.window.hasFocus())
end


-- love.window.hasMouseFocus
-- @NOTE cant really test as cant force focus
love.test.window.hasMouseFocus = function(test)
  test:assertNotNil(love.window.hasMouseFocus())
end


-- love.window.isDisplaySleepEnabled
love.test.window.isDisplaySleepEnabled = function(test)
  test:assertNotNil(love.window.isDisplaySleepEnabled())
  -- check disabled
  love.window.setDisplaySleepEnabled(false)
  test:assertFalse(love.window.isDisplaySleepEnabled(), 'check sleep disabled')
  -- check enabled
  love.window.setDisplaySleepEnabled(true)
  test:assertTrue(love.window.isDisplaySleepEnabled(), 'check sleep enabled')
end


-- love.window.isMaximized
love.test.window.isMaximized = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support window maximization")
  end

  test:assertFalse(love.window.isMaximized(), 'check window not maximized')
  love.window.maximize()
  test:waitFrames(10)
  -- on MACOS maximize wont get recognised immedietely so wait a few frames
  test:assertTrue(love.window.isMaximized(), 'check window now maximized')
  love.window.restore()
end


-- love.window.isMinimized
love.test.window.isMinimized = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support window minimization")
  end

  -- check not minimized to start
  test:assertFalse(love.window.isMinimized(), 'check window not minimized')
  -- try to minimize
  love.window.minimize()
  test:waitFrames(10)
  -- on linux minimize won't get recognized immediately, so wait a few frames
  test:assertTrue(love.window.isMinimized(), 'check window minimized')
  love.window.restore()
end


-- love.window.isOccluded
love.test.window.isOccluded = function(test)
  love.window.focus()
  test:assertFalse(love.window.isOccluded(), 'check window not occluded')
end


-- love.window.isOpen
love.test.window.isOpen = function(test)
  -- check open initially
  test:assertTrue(love.window.isOpen(), 'check window open')
  -- we check closing in test.window.close
end


-- love.window.isVisible
love.test.window.isVisible = function(test)
  -- check visible initially
  test:assertTrue(love.window.isVisible(), 'check window visible')
end


-- love.window.maximize
love.test.window.maximize = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support window maximization")
  end

  test:assertFalse(love.window.isMaximized(), 'check window not maximized')
  -- check maximizing is set
  love.window.maximize()
  test:waitFrames(10)
  -- on macos we need to wait a few frames
  test:assertTrue(love.window.isMaximized(), 'check window maximized')
  love.window.restore()
end


-- love.window.minimize
love.test.window.minimize = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support window minimization")
  end

  test:assertFalse(love.window.isMinimized(), 'check window not minimized')
  -- check minimizing is set
  love.window.minimize()
  test:waitFrames(10)
  -- on linux we need to wait a few frames
  test:assertTrue(love.window.isMinimized(), 'check window maximized')
  love.window.restore()
end


-- love.window.requestAttention
love.test.window.requestAttention = function(test)
  test:skipTest('cant test this worked')
end


-- love.window.restore
love.test.window.restore = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support window minimization")
  end

  -- check minimized to start
  love.window.minimize()
  test:waitFrames(10)
  love.window.restore()
  test:waitFrames(10)
  -- check restoring the state of the window
  test:assertFalse(love.window.isMinimized(), 'check window restored')
end


-- love.window.setDisplaySleepEnabled
love.test.window.setDisplaySleepEnabled = function(test)
  -- check disabling sleep
  love.window.setDisplaySleepEnabled(false)
  test:assertFalse(love.window.isDisplaySleepEnabled(), 'check sleep disabled')
  -- check setting it back to enabled
  love.window.setDisplaySleepEnabled(true)
  test:assertTrue(love.window.isDisplaySleepEnabled(), 'check sleep enabled')
end


-- love.window.setFullscreen
love.test.window.setFullscreen = function(test)
  if GITHUB_RUNNER and test:isOS('Linux') then
    return test:skipTest("xvfb on Linux doesn't support fullscreen")
  end

  -- check fullscreen is set
  love.window.setFullscreen(true)
  test:assertTrue(love.window.getFullscreen(), 'check fullscreen')
  -- check setting back to normal
  love.window.setFullscreen(false)
  test:assertFalse(love.window.getFullscreen(), 'check not fullscreen')
end


-- love.window.setIcon
-- @NOTE could check the image data itself?
love.test.window.setIcon = function(test)
  -- check setting an icon returns the val
  local icon = love.image.newImageData('resources/love.png')
  love.window.setIcon(icon)
  test:assertNotEquals(nil, love.window.getIcon(), 'check icon not nil')
end


-- love.window.setMode
-- @NOTE same as getMode could be checking more flag properties
love.test.window.setMode = function(test)
  -- set window mode
  love.window.setMode(512, 512, {
    fullscreen = false,
    resizable = false
  })
  -- check what we set is returned
  local width, height, flags = love.window.getMode()
  test:assertEquals(512, width, 'check window w match')
  test:assertEquals(512, height, 'check window h match')
  test:assertFalse(flags["fullscreen"], 'check window not fullscreen')
  test:assertFalse(flags["resizable"], 'check window not resizeable')
  love.window.setMode(360, 240, {
    fullscreen = false,
    resizable = true
  })
end

-- love.window.setPosition
love.test.window.setPosition = function(test)
  -- check position is returned
  love.window.setPosition(100, 100, 1)
  test:waitFrames(10)
  local x, y, _ = love.window.getPosition()
  test:assertEquals(100, x, 'check position x')
  test:assertEquals(100, y, 'check position y')
end


-- love.window.setTitle
love.test.window.setTitle = function(test)
  -- check setting title val is returned
  love.window.setTitle('love.testing')
  test:assertEquals('love.testing', love.window.getTitle(), 'check title matches')
  love.window.setTitle('love.test')
end


-- love.window.setVSync
love.test.window.setVSync = function(test)
  love.window.setVSync(0)
  test:assertNotNil(love.window.getVSync())
end


-- love.window.showMessageBox
-- @NOTE if running headless would need to skip anyway cos can't press it
love.test.window.showMessageBox = function(test)
  test:skipTest('cant test this worked')
end


-- love.window.toPixels
love.test.window.toPixels = function(test)
  -- check dpi/pixel ratio is as expected
  local dpi = love.window.getDPIScale()
  local pixels = love.window.toPixels(50)
  test:assertEquals(50*dpi, pixels, 'check dpi ratio')
end


-- love.window.updateMode
love.test.window.updateMode = function(test)
  -- set initial mode
  love.window.setMode(512, 512, {
    fullscreen = false,
    resizable = false
  })
  -- update mode with some props but not others
  love.window.updateMode(360, 240, nil)
  -- check only changed values changed
  local width, height, flags = love.window.getMode()
  test:assertEquals(360, width, 'check window w match')
  test:assertEquals(240, height, 'check window h match')
  test:assertFalse(flags["fullscreen"], 'check window not fullscreen')
  test:assertFalse(flags["resizable"], 'check window not resizeable')
  love.window.setMode(360, 240, { -- reset
    fullscreen = false,
    resizable = true
  })

  -- test different combinations of the backbuffer depth/stencil buffer.
  test:waitFrames(1)
  love.window.updateMode(360, 240, {depth = false, stencil = false})
  test:waitFrames(1)
  love.window.updateMode(360, 240, {depth = true, stencil = true})
  test:waitFrames(1)
  love.window.updateMode(360, 240, {depth = true, stencil = false})
  test:waitFrames(1)
  love.window.updateMode(360, 240, {depth = false, stencil = true})
end
