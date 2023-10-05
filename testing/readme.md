# löve.test
Basic testing suite for the löve APIs, based off of [this issue](https://github.com/love2d/love/issues/1745)

Currently written for löve 12

---

## Primary Goals
- [x] Simple pass/fail tests in Lua with minimal setup 
- [x] Ability to run all tests with a simple command.
- [x] Ability to see how many tests are passing/failing
- [x] No platform-specific dependencies / scripts
- [x] Ability to run a subset of tests
- [x] Ability to easily run an individual test.

---

## Running Tests
The initial pass is to keep things as simple as possible, and just run all the tests inside Löve to match how they'd be used by developers in-engine.
To run the tests, download the repo and then run the main.lua as you would a löve game, i.e:

WINDOWS: `& 'c:\Program Files\LOVE\love.exe' PATH_TO_TESTING_FOLDER --console`  
MACOS: `/Applications/love.app/Contents/MacOS/love PATH_TO_TESTING_FOLDER`

By default all tests will be run for all modules.  

If you want to specify a module you can add:  
`--runSpecificModules filesystem`  
For multiple modules, provide a comma seperate list:  
`--runSpecificModules filesystem,audio,data"`

If you want to specify only 1 specific method only you can use:  
`--runSpecificMethod filesystem write`

All results will be printed in the console per method as PASS, FAIL, or SKIP with total assertions met on a module level and overall level.  

An `XML` file in the style of [JUnit XML](https://www.ibm.com/docs/en/developer-for-zos/14.1?topic=formats-junit-xml-format) will be generated in your save directory, along with a `HTML` file with a summary of all tests (including visuals for love.graphics tests). 
> Note that this can only be viewed properly locally as the generated images are written to the save directory.   
> An example of both types of output can be found in the `/output` folder

---

## Architecture
Each method has it's own test method written in `/tests` under the matching module name.

When you run the tests, a single TestSuite object is created which handles the progress + totals for all the tests.  
Each module has a TestModule object created, and each test method has a TestMethod object created which keeps track of assertions for that method. You can currently do the following assertions:
- **assertEquals**(expected, actual)
- **assertNotEquals**(expected, actual)
- **assertRange**(actual, min, max)
- **assertMatch**({option1, option2, option3 ...}, actual) 
- **assertGreaterEqual**(expected, actual)
- **assertLessEqual**(expected, actual)
- **assertObject**(table)

Example test method:
```lua
-- love.filesystem.read test method
-- all methods should be put under love.test.MODULE.METHOD, matching the API
love.test.filesystem.read = function(test)
  -- setup any data needed then run any asserts using the passed test object
  local content, size = love.filesystem.read('resources/test.txt')
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('helloworld', content, 'check content match')
  test:assertEquals(10, size, 'check size match')
  content, size = love.filesystem.read('resources/test.txt', 5)
  test:assertNotEquals(nil, content, 'check not nil')
  test:assertEquals('hello', content, 'check content match')
  test:assertEquals(5, size, 'check size match')
  -- no need to return anything just cleanup any objs if needed
end
```

After each test method is ran, the assertions are totalled up, printed, and we move onto the next method! Once all methods in the suite are run a total pass/fail/skip is given for that module and we move onto the next module (if any)

For sanity-checking, if it's currently not covered or we're not sure how to test yet we can set the test to be skipped with `test:skipTest(reason)` - this way we still see the method listed in the tests without it affected the pass/fail totals

---

## Coverage
This is the status of all module tests currently.  
"objects" is a special module to cover any object specific tests, i.e. testing a File object functions as expected
```lua
-- [x] audio        26 PASSED |  0 FAILED |  0 SKIPPED
-- [x] data          7 PASSED |  0 FAILED |  3 SKIPPED      [SEE BELOW]
-- [x] event         4 PASSED |  0 FAILED |  2 SKIPPED      [SEE BELOW]
-- [x] filesystem   27 PASSED |  0 FAILED |  2 SKIPPED
-- [x] font          4 PASSED |  0 FAILED |  1 SKIPPED      [SEE BELOW]
-- [ ] graphics     65 PASSED |  0 FAILED | 31 SKIPPED      [SEE BELOW]
-- [x] image         3 PASSED |  0 FAILED |  0 SKIPPED
-- [x] math         17 PASSED |  0 FAILED |  0 SKIPPED
-- [x] physics      22 PASSED |  0 FAILED |  0 SKIPPED
-- [x] sound         2 PASSED |  0 FAILED |  0 SKIPPED
-- [x] system        6 PASSED |  0 FAILED |  2 SKIPPED
-- [x] thread        3 PASSED |  0 FAILED |  0 SKIPPED
-- [x] timer         6 PASSED |  0 FAILED |  0 SKIPPED
-- [x] video         1 PASSED |  0 FAILED |  0 SKIPPED
-- [x] window       32 PASSED |  2 FAILED |  2 SKIPPED      [SEE BELOW]

-- [ ] objects      STILL TO BE DONE
--------------------------------------------------------------------------------
-- [x] totals      226 PASSED |  4 FAILED | 43 SKIPPED
```

The following modules are not covered as we can't really emulate input nicely:  
`joystick`, `keyboard`, `mouse`, and `touch`

---

## Todo / Skipped
Modules with some small bits needed or needing sense checking:
- **love.data** - packing methods need writing cos i dont really get what they are
- **love.event** - love.event.wait or love.event.pump need writing if possible I dunno how to check
- **love.font** - newBMFontRasterizer() wiki entry is wrong so not sure whats expected
- **love.graphics** - still need to do tests for the drawing and state methods
- **love.image** - ideally isCompressed should have an example of all compressed files love can take
- **love.math** - linearToGamma + gammaToLinear using direct formulas don't get same value back
- **love.objects** - not started properly yet

---

## Failures
- **love.window.isMaximized()** - returns false after calling love.window.maximize?
- **love.window.maximize()** - same as above

---

## Stretch Goals
- [ ] Tests can compare visual results to a reference image
- [ ] Ability to see all visual results at a glance
- [ ] Automatic testing that happens after every commit
- [ ] Ability to test loading different combinations of modules
- [ ] Performance tests

There is some unused code in the Test.lua class to add preview vs actual images to the HTML output
