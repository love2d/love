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
  test:assertFalse(video:isPlaying(), 'check not playing by def')

  -- check playing and pausing states
  video:play()
  test:assertTrue(video:isPlaying(), 'check now playing')
  video:seek(0.3)
  test:assertRange(video:tell(), 0.3, 0.4, 'check seek/tell')
  video:rewind()
  test:assertRange(video:tell(), 0, 0.1, 'check rewind')
  video:pause()
  test:assertFalse(video:isPlaying(), 'check paused')

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
