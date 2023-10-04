-- love.window


-- love.window.close
-- @NOTE closing window should cause graphics to no longer be active
love.test.window.close = function(test)
  love.window.close()
  local active = false
  if love.graphics ~= nil then active = love.graphics.isActive() end
  test:assertEquals(false, active, 'check window active')
  love.window.setMode(256, 256) -- reset 
end


-- love.window.fromPixels
-- @NOTE dependent on the DPI value returned I think
love.test.window.fromPixels = function(test)
  local dpi = love.window.getDPIScale()
  local pixels = love.window.fromPixels(100)
  test:assertEquals(100/dpi, pixels, 'check dpi ratio')
end


-- love.window.getDPIScale
-- @NOTE i dont think there's a clever way to check this 
love.test.window.getDPIScale = function(test)
  test:assertNotEquals(nil, test, 'check not nil')
end


-- love.window.getDesktopDimensions
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDesktopDimensions = function(test)
  local w, h = love.window.getDesktopDimensions()
  test:assertNotEquals(nil, w, 'check not nil')
  test:assertNotEquals(nil, h, 'check not nil')
end


-- love.window.getDisplayCount
-- @NOTE cant wait for the test suite to be run headless and fail here
love.test.window.getDisplayCount = function(test)
  local count = love.window.getDisplayCount()
  test:assertGreaterEqual(1, count, 'check 1 display')
end


-- love.window.getDisplayName
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDisplayName = function(test)
  local name = love.window.getDisplayName(1)
  test:assertNotEquals(nil, name, 'check not nil')
end


-- love.window.getDisplayOrientation
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getDisplayOrientation = function(test)
  local orientation = love.window.getDisplayOrientation(1)
  test:assertNotEquals(nil, orientation, 'check not nil')
end


-- love.window.getFullscreen
love.test.window.getFullscreen = function(test)
  test:assertEquals(false, love.window.getFullscreen(), 'check not fullscreen')
  love.window.setFullscreen(true)
  test:assertEquals(true, love.window.getFullscreen(), 'check now fullscreen')
  love.window.setFullscreen(false)
end


-- love.window.getFullscreenModes
-- @NOTE dependent on hardware so best can do is not nil
love.test.window.getFullscreenModes = function(test)
  local modes = love.window.getFullscreenModes(1)
  test:assertNotEquals(nil, modes, 'check not nil')
end


-- love.window.getIcon
love.test.window.getIcon = function(test)
  test:assertEquals(nil, love.window.getIcon(), 'check nil by default') -- nil if not set
  local icon = love.image.newImageData('resources/love.png')
  love.window.setIcon(icon)
  test:assertNotEquals(nil, love.window.getIcon(), 'check not nil')
  icon:release()
end


-- love.window.getMode
-- @NOTE could prob add more checks on the flags here based on conf.lua
love.test.window.getMode = function(test)
  local w, h, flags = love.window.getMode()
  test:assertEquals(256, w, 'check w')
  test:assertEquals(256, h, 'check h')
  test:assertEquals(false, flags["fullscreen"], 'check fullscreen')
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
  test:assertNotEquals(nil, x, 'check not nil')
  test:assertNotEquals(nil, y, 'check not nil')
  test:assertNotEquals(nil, w, 'check not nil')
  test:assertNotEquals(nil, h, 'check not nil')
end


-- love.window.getTitle
love.test.window.getTitle = function(test)
  love.window.setTitle('love.testing')
  test:assertEquals('love.testing', love.window.getTitle(), 'check title match')
  love.window.setTitle('love.test')
end


-- love.window.getVSync
love.test.window.getVSync = function(test)
  test:assertNotEquals(nil, love.window.getVSync(), 'check not nil')
  love.window.setVSync(false)
  test:assertEquals(0, love.window.getVSync(), 'check vsync off')
  love.window.setVSync(true)
  test:assertEquals(1, love.window.getVSync(), 'check vsync on')
end


-- love.window.hasFocus
-- @NOTE cant really test as cant force focus?
love.test.window.hasFocus = function(test)
  test:assertNotEquals(nil, love.window.hasFocus(), 'check not nil')
end


-- love.window.hasMouseFocus
-- @NOTE cant really test as cant force focus?
love.test.window.hasMouseFocus = function(test)
  test:assertNotEquals(nil, love.window.hasMouseFocus(), 'check not nil')
end


-- love.window.isDisplaySleepEnabled
love.test.window.isDisplaySleepEnabled = function(test)
  test:assertNotEquals(nil, love.window.isDisplaySleepEnabled(), 'check not nil')
  love.window.setDisplaySleepEnabled(false)
  test:assertEquals(false, love.window.isDisplaySleepEnabled(), 'check sleep disabled')
  love.window.setDisplaySleepEnabled(true)
  test:assertEquals(true, love.window.isDisplaySleepEnabled(), 'check sleep enabled')
end


-- love.window.isMaximized
love.test.window.isMaximized = function(test)
  love.window.minimize()
  test:assertEquals(false, love.window.isMaximized(), 'check window maximized')
  love.window.maximize()
  test:assertEquals(true, love.window.isMaximized(), 'check window not maximized')
  love.window.restore()
end


-- love.window.isMinimized
love.test.window.isMinimized = function(test)
  test:assertEquals(false, love.window.isMinimized(), 'check window not minimized')
  love.window.minimize()
  test:assertEquals(true, love.window.isMinimized(), 'check window minimized')
  love.window.restore()
end


-- love.window.isOpen
love.test.window.isOpen = function(test)
  test:assertEquals(true, love.window.isOpen(), 'check window open')
  love.window.close()
  test:assertEquals(false, love.window.isOpen(), 'check window closed')
  love.window.setMode(256, 256) -- reset 
end


-- love.window.isVisible
love.test.window.isVisible = function(test)
  test:assertEquals(true, love.window.isVisible(), 'check window visible')
  love.window.close()
  test:assertEquals(false, love.window.isVisible(), 'check window not visible')
  love.window.setMode(256, 256) -- reset 
end


-- love.window.maximize
love.test.window.maximize = function(test)
  love.window.maximize()
  test:assertEquals(true, love.window.isMaximized(), 'check window maximized')
  love.window.restore()
end


-- love.window.minimize
love.test.window.minimize = function(test)
  love.window.minimize()
  test:assertEquals(true, love.window.isMinimized(), 'check window minimized')
  love.window.restore()
end


-- love.window.requestAttention
love.window.requestAttention = function(test)
  test:skipTest('cant really test this')
end


-- love.window.restore
love.test.window.restore = function(test)
  love.window.minimize()
  test:assertEquals(true, love.window.isMinimized(), 'check window minimized')
  love.window.restore()
  test:assertEquals(false, love.window.isMinimized(), 'check window restored')
end


-- love.window.setDisplaySleepEnabled
love.test.window.setDisplaySleepEnabled = function(test)
  love.window.setDisplaySleepEnabled(false)
  test:assertEquals(false, love.window.isDisplaySleepEnabled(), 'check sleep disabled')
  love.window.setDisplaySleepEnabled(true)
  test:assertEquals(true, love.window.isDisplaySleepEnabled(), 'check sleep enabled')
end


-- love.window.setFullscreen
love.test.window.setFullscreen = function(test)
  love.window.setFullscreen(true)
  test:assertEquals(true, love.window.getFullscreen(), 'check fullscreen')
  love.window.setFullscreen(false)
  test:assertEquals(false, love.window.getFullscreen(), 'check not fullscreen')
end


-- love.window.setIcon
-- @NOTE could check the image data itself?
love.test.window.setIcon = function(test)
  local icon = love.image.newImageData('resources/love.png')
  love.window.setIcon(icon)
  test:assertNotEquals(nil, love.window.getIcon(), 'check icon not nil')
  icon:release()
end


-- love.window.setMode
-- @NOTE same as getMode could be checking more flag properties
love.test.window.setMode = function(test)
  love.window.setMode(512, 512, {
    fullscreen = false,
    resizable = false
  })
  local width, height, flags = love.window.getMode()
  test:assertEquals(512, width, 'check window w match')
  test:assertEquals(512, height, 'check window h match')
  test:assertEquals(false, flags["fullscreen"], 'check window not fullscreen')
  test:assertEquals(false, flags["resizable"], 'check window not resizeable')
  love.window.setMode(256, 256, {
    fullscreen = false,
    resizable = true
  })
end

-- love.window.setPosition
love.test.window.setPosition = function(test)
  love.window.setPosition(100, 100, 1)
  local x, y, _ = love.window.getPosition()
  test:assertEquals(100, x, 'check position x')
  test:assertEquals(100, y, 'check position y')
end


-- love.window.setTitle
love.test.window.setTitle = function(test)
  love.window.setTitle('love.testing')
  test:assertEquals('love.testing', love.window.getTitle(), 'check title matches')
  love.window.setTitle('love.test')
end


-- love.window.setVSync
love.test.window.setVSync = function(test)
  love.window.setVSync(false)
  test:assertEquals(0, love.window.getVSync(), 'check vsync off')
  love.window.setVSync(true)
  test:assertEquals(1, love.window.getVSync(), 'check vsync on')
end


-- love.window.showMessageBox
-- @NOTE if running headless would need to skip anyway cos can't press it
-- skipping here cos it's annoying
love.test.window.showMessageBox = function(test)
  test:skipTest('skipping cos annoying to test with')
end


-- love.window.toPixels
love.test.window.toPixels = function(test)
  local dpi = love.window.getDPIScale()
  local pixels = love.window.toPixels(50)
  test:assertEquals(50*dpi, pixels, 'check dpi ratio')
end


-- love.window.updateMode
love.test.window.updateMode = function(test)
  love.window.setMode(512, 512, {
    fullscreen = false,
    resizable = false
  })
  love.window.updateMode(256, 256, nil)
  local width, height, flags = love.window.getMode()
  test:assertEquals(256, width, 'check window w match')
  test:assertEquals(256, height, 'check window h match')
  test:assertEquals(false, flags["fullscreen"], 'check window not fullscreen')
  test:assertEquals(false, flags["resizable"], 'check window not resizeable')
  love.window.setMode(256, 256, {
    fullscreen = false,
    resizable = true
  })
end