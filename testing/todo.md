# TODO
These are all the outstanding methods that require test coverage, along with a few bits that still need doing / discussion.  

## General 
- ability to test loading different combinations of modules if needed?
- check expected behaviour of mount + unmount with common path
  try uncommenting love.filesystem.unmountCommonPath and you'll see the issues
- revisit love.audio.setPlaybackDevice when we update openal soft for MacOS

## Graphics
- love.graphics.copyBuffer()
- love.graphics.copyBufferToTexture()
- love.graphics.copyTextureToBuffer()
- love.graphics.readbackTexture()
- love.graphics.readbackTextureAsync()
- love.graphics.readbackBuffer()
- love.graphics.readbackBufferAsync()
- love.graphics.newComputeShader()
- love.graphics.dispatchThreadgroups()
- love.graphics.dispatchIndirect()
- love.graphics.drawFromShader()
- love.graphics.drawFromShaderIndirect()
- love.graphics.drawIndirect()
- love.graphics.getQuadIndexBuffer()
- love.graphics.setBlendState()
- love.graphics.resetProjection()
- love.graphics.Mesh:getAttachedAttributes()
- love.graphics.Shader:hasStage()
