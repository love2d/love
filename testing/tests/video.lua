-- love.video


-- love.video.newVideoStream
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.video.newVideoStream = function(test)
  local videostream = love.video.newVideoStream('resources/sample.ogv')
  test:assertObject(videostream)
  videostream:release()
end