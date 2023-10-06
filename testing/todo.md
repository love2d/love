`/Applications/love_12.app/Contents/MacOS/love ./testing`

## CI
- [ ] ignore test suite for windows AMD
- [ ] add test run to linux + ios builds
- [ ] add metal/vulkan runs

## TESTSUITE
- [ ] finish graphics state methods
- [ ] start graphics drawing methods
- [ ] start object methods

## FUTURE
- [ ] pass in err string returns to the test output
  maybe even assertNotNil could use the second value automatically
  test:assertNotNil(love.filesystem.openFile('file2', 'r')) wouldn't have to change
- [ ] some joystick/input stuff could be at least nil checked maybe?
- [ ] need a platform: format table somewhere for compressed formats (i.e. DXT not supported)
  could add platform as global to command and then use in tests?