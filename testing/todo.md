# TODO
These are all the outstanding methods that require test coverage, along with a few bits that still need doing / discussion.  

## General 
- ability to test loading different combinations of modules if needed?
- performance tests? need to discuss what + how, might be better as a seperate thing
- check expected behaviour of mount + unmount with common path
  try uncommenting love.filesystem.unmountCommonPath and you'll see the issues
- revisit love.audio.setPlaybackDevice when we update openal soft for MacOS

## Physics
- love.physics.World:rayCastAny
- love.physics.World:rayCastClosest
- love.physics.World:getShapesInArea
- love.physics.Body:getShapes
- love.physics.Body:getShape
- love.physics.Body:hasCustomMassData

## Graphics
- love.graphics.copyBuffer
- love.graphics.copyBufferToTexture
- love.graphics.copyTextureToBuffer
- love.graphics.readbackTexture
- love.graphics.readbackTextureAsync
- love.graphics.readbackBuffer
- love.graphics.readbackBufferAsync
- love.graphics.newComputeShader
- love.graphics.dispatchThreadgroupos
- love.graphics.dispatchIndirect
- love.graphics.newTexture
- love.graphics.drawFromShader
- love.graphics.drawFromShaderIndirect
- love.graphics.drawIndirect
- love.graphics.getQuadIndexBuffer
- love.graphics.setBlendState
- love.graphics.setOrthoProjection
- love.graphics.setPerspectiveProjection
- love.graphics.resetProjection
- Mesh:getAttachedAttributes
- Shader:hasStage
