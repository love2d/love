`/Applications/love_12.app/Contents/MacOS/love ./testing`

## TESTSUITE
- [ ] setStencilMode to replace setStencilTest
- [ ] start graphics drawing methods
- [ ] start object methods

## GRAPHICS
Methods that need a better actual graphics check if possible:
- [ ] setDepthMode
- [ ] setFrontFaceWinding
- [ ] setMeshCullMode

## FUTURE
- [ ] need a platform: format table somewhere for compressed formats (i.e. DXT not supported)
      could add platform as global to command and then use in tests?
- [ ] use coroutines for the delay action? i.e. wrap each test call in coroutine 
      and then every test can use coroutine.yield() if needed
- [ ] could nil check some joystick and keyboard methods?

## GITHUB ACTION CI
- [ ] linux needs to run xvfb-run with the appimage
- [ ] windows can try installing mesa for opengl replacement
- [ ] ios test run?

Can't run --renderers metal on github action images:
Run love-macos/love.app/Contents/MacOS/love testing --renderers metal
Cannot create Metal renderer: Metal is not supported on this system.
Cannot create graphics: no supported renderer on this system.
Error: Cannot create graphics: no supported renderer on this system.

Can't run test suite on windows as it stands:
Unable to create renderer
This program requires a graphics card and video drivers which support OpenGL 2.1 or OpenGL ES 2.
