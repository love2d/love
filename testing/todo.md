`/Applications/love_12.app/Contents/MacOS/love ./testing`

# v0.2

## Todo
- finish graphics state methods
- start graphics drawing methods
- start object methods
- look into XML reader github actions to display results in readme?
  dorny/test-reporter@v1.6.0 seems to do it
  would need to run the tests first then could use it like:

  - name: Run Tests
  - run: PATH_TO_BUILT_APP ./testing --runAllTests
  - name: Test Report
    uses: dorny/test-reporter@v1
    with:
      name: Test Output
      path: output/*.xml
      reporter: jest-junit
      
  and d. check format: https://github.com/testmoapp/junitxml


- need a platform: format table somewhere for compressed formats (i.e. DXT not supported)
