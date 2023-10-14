-- love.video


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------OBJECTS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- VideoStream (love.thread.newVideoStream)
love.test.video.VideoStream = function(test)
  -- create obj
  local video = love.video.newVideoStream('resources/sample.ogv')
  test:assertObject(video)
  -- check def properties
  test:assertEquals('resources/sample.ogv', video:getFilename(), 'check filename')
  test:assertEquals(false, video:isPlaying(), 'check not playing by def')
  -- check playing and pausing
  video:play()
  test:assertEquals(true, video:isPlaying(), 'check now playing')
  video:seek(0.3)
  test:assertEquals(0.3, video:tell(), 'check seek/tell')
  video:rewind()
  test:assertEquals(0, video:tell(), 'check rewind')
  video:pause()
  test:assertEquals(false, video:isPlaying(), 'check paused')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------METHODS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.video.newVideoStream
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.video.newVideoStream = function(test)
  test:assertObject(love.video.newVideoStream('resources/sample.ogv'))
end
