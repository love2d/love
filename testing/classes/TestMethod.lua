-- @class - TestMethod
-- @desc - used to run a specific method from a module's /test/ suite
--         each assertion is tracked and then printed to output
TestMethod = {


  -- @method - TestMethod:new()
  -- @desc - create a new TestMethod object
  -- @param {string} method - string of method name to run
  -- @param {TestMethod} testmethod - parent testmethod this test belongs to
  -- @return {table} - returns the new Test object
  new = function(self, method, testmodule)
    local test = {
      testmodule = testmodule,
      method = method,
      asserts = {},
      start = love.timer.getTime(),
      finish = 0,
      count = 0,
      passed = false,
      skipped = false,
      skipreason = '',
      rgba_tolerance = 0,
      pixel_tolerance = 0,
      fatal = '',
      message = nil,
      result = {},
      colors = {
        red = {1, 0, 0, 1},
        redpale = {1, 0.5, 0.5, 1},
        red07 = {0.7, 0, 0, 1},
        green = {0, 1, 0, 1},
        greenhalf = {0, 0.5, 0, 1},
        greenfade = {0, 1, 0, 0.5},
        blue = {0, 0, 1, 1},
        bluefade = {0, 0, 1, 0.5},
        yellow = {1, 1, 0, 1},
        pink = {1, 0, 1, 1},
        black = {0, 0, 0, 1},
        white = {1, 1, 1, 1},
        lovepink = {214/255, 86/255, 151/255, 1},
        loveblue = {83/255, 168/255, 220/255, 1}
      },
      imgs = 1,
      delay = 0,
      delayed = false,
      store = {},
      co = nil
    }
    setmetatable(test, self)
    self.__index = self
    return test
  end,


  -- @method - TestMethod:assertEquals()
  -- @desc - used to assert two values are equals
  -- @param {any} expected - expected value of the test
  -- @param {any} actual - actual value of the test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertEquals = function(self, expected, actual, label)
    self.count = self.count + 1
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = expected == actual,
      message = 'expected \'' .. tostring(expected) .. '\' got \'' ..
        tostring(actual) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertTrue()
  -- @desc - used to assert a value is true
  -- @param {any} value - value to test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertTrue = function(self, value, label)
    self.count = self.count + 1
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = value == true,
      message = 'expected \'true\' got \'' ..
        tostring(value) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertFalse()
  -- @desc - used to assert a value is false
  -- @param {any} value - value to test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertFalse = function(self, value, label)
    self.count = self.count + 1
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = value == false,
      message = 'expected \'false\' got \'' ..
        tostring(value) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertNotEquals()
  -- @desc - used to assert two values are not equal
  -- @param {any} expected - expected value of the test
  -- @param {any} actual - actual value of the test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertNotEquals = function(self, expected, actual, label)
    self.count = self.count + 1
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = expected ~= actual,
      message = 'avoiding \'' .. tostring(expected) .. '\' got \'' ..
        tostring(actual) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertRange()
  -- @desc - used to check a value is within an expected range
  -- @param {number} actual - actual value of the test
  -- @param {number} min - minimum value the actual should be >= to
  -- @param {number} max - maximum value the actual should be <= to
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertRange = function(self, actual, min, max, label)
    self.count = self.count + 1
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = actual >= min and actual <= max,
      message = 'value \'' .. tostring(actual) .. '\' out of range \'' ..
        tostring(min) .. '-' .. tostring(max) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertMatch()
  -- @desc - used to check a value is within a list of values
  -- @param {number} list - list of valid values for the test
  -- @param {number} actual - actual value of the test to check is in the list
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertMatch = function(self, list, actual, label)
    self.count = self.count + 1
    local found = false
    for l=1,#list do
      if list[l] == actual then found = true end;
    end
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = found == true,
      message = 'value \'' .. tostring(actual) .. '\' not found in \'' ..
        table.concat(list, ',') .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertGreaterEqual()
  -- @desc - used to check a value is >= than a certain target value
  -- @param {any} target - value to check the test agaisnt
  -- @param {any} actual - actual value of the test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertGreaterEqual = function(self, target, actual, label)
    self.count = self.count + 1
    local passing = false
    if target ~= nil and actual ~= nil then
      passing = actual >= target
    end
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = passing,
      message = 'value \'' .. tostring(actual) .. '\' not >= \'' ..
        tostring(target) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertLessEqual()
  -- @desc - used to check a value is <= than a certain target value
  -- @param {any} target - value to check the test agaisnt
  -- @param {any} actual - actual value of the test
  -- @param {string} label - label for this test to use in exports
  -- @return {nil}
  assertLessEqual = function(self, target, actual, label)
    self.count = self.count + 1
    local passing = false
    if target ~= nil and actual ~= nil then
      passing = actual <= target
    end
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = passing,
      message = 'value \'' .. tostring(actual) .. '\' not <= \'' ..
        tostring(target) .. '\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertObject()
  -- @desc - used to check a table is a love object, this runs 3 seperate
  --         tests to check table has the basic properties of an object
  -- @note - actual object functionality tests have their own methods
  -- @param {table} obj - table to check is a valid love object
  -- @return {nil}
  assertObject = function(self, obj)
    self:assertNotNil(obj)
    self:assertEquals('userdata', type(obj), 'check is userdata')
    if obj ~= nil then
      self:assertNotEquals(nil, obj:type(), 'check has :type()')
    end
  end,


  -- @method - TestMethod:assertCoords()
  -- @desc - used to check a pair of values (usually coordinates)
  -- @param {table} obj - table to check is a valid love object
  -- @return {nil}
  assertCoords = function(self, expected, actual, label)
    self.count = self.count + 1
    local passing = false
    if expected ~= nil and actual ~= nil then
      if expected[1] == actual[1] and expected[2] == actual[2] then
        passing = true
      end
    end
    table.insert(self.asserts, {
      key = 'assert ' .. tostring(self.count),
      passed = passing,
      message = 'expected \'' .. tostring(expected[1]) .. 'x,' ..
        tostring(expected[2]) .. 'y\' got \'' ..
        tostring(actual[1]) .. 'x,' .. tostring(actual[2]) .. 'y\'',
      test = label or 'no label given'
    })
  end,


  -- @method - TestMethod:assertNotNil()
  -- @desc - quick assert for value not nil
  -- @param {any} value - value to check not nil
  -- @return {nil}
  assertNotNil = function (self, value, err)
    self:assertNotEquals(nil, value, 'check not nil')
    if err ~= nil then
      table.insert(self.asserts, {
        key = 'assert ' .. tostring(self.count),
        passed = false,
        message = err,
        test = 'assert not nil catch'
      })
    end
  end,


  -- @method - TestMethod:compareImg()
  -- @desc - compares a given image to the 'expected' version, with a tolerance of
  --         1px in any direction, and then saves it as the 'actual' version for
  --         report viewing
  -- @param {table} imgdata - imgdata to save as a png
  -- @return {nil}
  compareImg = function(self, imgdata)
    local expected_path = 'tempoutput/expected/love.test.graphics.' ..
      self.method .. '-' .. tostring(self.imgs) .. '.png'
    local ok, chunk, _ = pcall(love.image.newImageData, expected_path)
    if ok == false then return self:assertEquals(true, false, chunk) end
    local expected = chunk
    local iw = imgdata:getWidth()-1
    local ih = imgdata:getHeight()-1
    local differences = {}
    local rgba_tolerance = self.rgba_tolerance * (1/255)

    -- for each pixel, compare the expected vs the actual pixel data
    -- by default rgba_tolerance is 0
    for ix=0,iw do
      for iy=0,ih do
        local ir, ig, ib, ia = imgdata:getPixel(ix, iy)
        local points = {
          {expected:getPixel(ix, iy)}
        }
        if self.pixel_tolerance > 0 then
          if ix > 0 and iy < ih-1 then table.insert(points, {expected:getPixel(ix-1, iy+1)}) end
          if ix > 0 then table.insert(points, {expected:getPixel(ix-1, iy)}) end
          if ix > 0 and iy > 0 then table.insert(points, {expected:getPixel(ix-1, iy-1)}) end
          if iy < ih-1 then table.insert(points, {expected:getPixel(ix, iy+1)}) end
          if iy > 0 then table.insert(points, {expected:getPixel(ix, iy-1)}) end
          if ix < iw-1 and iy < ih-1 then table.insert(points, {expected:getPixel(ix+1, iy+1)}) end
          if ix < iw-1 then table.insert(points, {expected:getPixel(ix+1, iy)}) end
          if ix < iw-1 and iy > 0 then table.insert(points, {expected:getPixel(ix+1, iy-1)}) end
        end
        local has_match_r = false
        local has_match_g = false
        local has_match_b = false
        local has_match_a = false
        for t=1,#points do
          local epoint = points[t]
          if ir >= epoint[1] - rgba_tolerance and ir <= epoint[1] + rgba_tolerance then has_match_r = true; end
          if ig >= epoint[2] - rgba_tolerance and ig <= epoint[2] + rgba_tolerance then has_match_g = true; end
          if ib >= epoint[3] - rgba_tolerance and ib <= epoint[3] + rgba_tolerance then has_match_b = true; end
          if ia >= epoint[4] - rgba_tolerance and ia <= epoint[4] + rgba_tolerance then has_match_a = true; end
        end
        local matching = has_match_r and has_match_g and has_match_b and has_match_a
        local ymatch = ''
        local nmatch = ''
        if has_match_r then ymatch = ymatch .. 'r' else nmatch = nmatch .. 'r' end
        if has_match_g then ymatch = ymatch .. 'g' else nmatch = nmatch .. 'g' end
        if has_match_b then ymatch = ymatch .. 'b' else nmatch = nmatch .. 'b' end
        if has_match_a then ymatch = ymatch .. 'a' else nmatch = nmatch .. 'a' end
        local pixel = tostring(ir)..','..tostring(ig)..','..tostring(ib)..','..tostring(ia)
        self:assertEquals(true, matching, 'compare image pixel (' .. pixel .. ') at ' ..
          tostring(ix) .. ',' .. tostring(iy) .. ', matching = ' .. ymatch ..
          ', not matching = ' .. nmatch .. ' (' .. self.method .. '-' .. tostring(self.imgs) .. ')'
        )
        -- add difference co-ord for rendering later
        if matching ~= true then
          table.insert(differences, ix+1)
          table.insert(differences, iy+1)
        end
      end
    end
    local path = 'tempoutput/actual/love.test.graphics.' ..
      self.method .. '-' .. tostring(self.imgs) .. '.png'
    imgdata:encode('png', path)

    -- if we have differences draw them to a new canvas to display in HTML report
    local dpath = 'tempoutput/difference/love.test.graphics.' ..
      self.method .. '-' .. tostring(self.imgs) .. '.png'
    if #differences > 0 then
      local difference = love.graphics.newCanvas(iw+1, ih+1)
      love.graphics.setCanvas(difference)
        love.graphics.clear(0, 0, 0, 1)
        love.graphics.setColor(1, 0, 1, 1)
        love.graphics.points(differences)
        love.graphics.setColor(1, 1, 1, 1)
      love.graphics.setCanvas()
      love.graphics.readbackTexture(difference):encode('png', dpath)

    -- otherwise clear the old difference file (if any) to stop it coming up 
    -- in future reports when there's no longer a difference
    elseif love.filesystem.openFile(dpath, 'r') then
      love.filesystem.remove(dpath)
    end

    self.imgs = self.imgs + 1
  end,


  -- @method - TestMethod:exportImg()
  -- @desc - exports the given imgdata to the 'output/expected/' folder, to use when
  --         writing new graphics tests to set the expected image output
  -- @NOTE - you should not leave this method in when you are finished this is
  --         for test writing only
  -- @param {table} imgdata - imgdata to save as a png
  -- @param {integer} imgdata - index of the png, graphic tests are run sequentially
  --                            and each test image is numbered in order that its
  --                            compared to, so set the number here to match
  -- @return {nil}
  exportImg = function(self, imgdata, index)
    local path = 'tempoutput/expected/love.test.graphics.' ..
      self.method .. '-' .. tostring(index) .. '.png'
    imgdata:encode('png', path)
  end,


  -- @method - TestMethod:skipTest()
  -- @desc - used to mark this test as skipped for a specific reason
  -- @param {string} reason - reason why method is being skipped
  -- @return {nil}
  skipTest = function(self, reason)
    self.skipped = true
    self.skipreason = reason
  end,


  -- @method - TestMethod:waitFrames()
  -- @desc - yields the method for x amount of frames
  -- @param {number} frames - no. frames to wait
  -- @return {nil}
  waitFrames = function(self, frames)
    for _=1,frames do coroutine.yield() end
  end,


  -- @method - TestMethod:waitSeconds()
  -- @desc - yields the method for x amount of seconds
  -- @param {number} seconds - no. seconds to wait
  -- @return {nil}
  waitSeconds = function(self, seconds)
    local start = love.timer.getTime()
    while love.timer.getTime() < start + seconds do
      coroutine.yield()
    end
  end,


  -- @method - TestMethod:isOS()
  -- @desc - checks for a specific OS (or list of OSs)
  -- @param {string/s} - each arg passed will be checked as a valid OS, as long
  --                     as one passed the function will return true
  -- @return {boolean} - returns true if one of the OSs given matches actual OS
  isOS = function(self, ...)
    for os=1,select("#", ...) do
      if select(os, ...) == love.test.current_os then return true end
    end
    return false
  end,

  -- @method - TestMethod:isLuaVersion()
  -- @desc - checks for a specific Lua version (or list of versions)
  -- @param {number} - the minimum Lua version to check against
  -- @return {boolean} - returns true if the current Lua version is at least the given version
  isAtLeastLuaVersion = function(self, version)
    return love.test.lua_version >= version
  end,

  -- @method - TestMethod:isLuaJITEnabled()
  -- @desc - checks if LuaJIT is enabled
  -- @return {boolean} - returns true if LuaJIT is enabled
  isLuaJITEnabled = function(self)
    return love.test.has_lua_jit
  end,

  -- @method - TestMethod:evaluateTest()
  -- @desc - evaluates the results of all assertions for a final restult
  -- @return {nil}
  evaluateTest = function(self)
    local failure = ''
    local failures = 0

    -- check all asserts for failures, additional failures are also printed
    local assert_failures = {}
    for a=1,#self.asserts do
      if not self.asserts[a].passed and not self.skipped then
        if failure == '' then failure = self.asserts[a] end
        table.insert(assert_failures, self.asserts[a])
        failures = failures + 1
      end
    end
    if self.fatal ~= '' then failure = self.fatal end
    local passed = tostring(#self.asserts - failures)
    local total = '(' .. passed .. '/' .. tostring(#self.asserts) .. ')'

    -- skipped tests have a special log
    if self.skipped then
      self.testmodule.skipped = self.testmodule.skipped + 1
      love.test.totals[3] = love.test.totals[3] + 1
      self.result = {
        total = '',
        result = "SKIP",
        passed = false,
        message = '(0/0) - method skipped [' .. self.skipreason .. ']',
        failures = {}
      }
    else

      -- if no failure but has asserts, then passed
      if failure == '' and #self.asserts > 0 then
        self.passed = true
        self.testmodule.passed = self.testmodule.passed + 1
        love.test.totals[1] = love.test.totals[1] + 1
        self.result = {
          total = total,
          result = 'PASS',
          passed = true,
          message = nil,
          failures = {}
        }

      -- otherwise it failed
      else
        self.passed = false
        self.testmodule.failed = self.testmodule.failed + 1
        love.test.totals[2] = love.test.totals[2] + 1

        -- no asserts means invalid test
        if #self.asserts == 0 then
          local msg = 'no asserts defined'
          if self.fatal ~= '' then msg = self.fatal end
          self.result = {
            total = total,
            result = 'FAIL',
            passed = false,
            key = 'test',
            message = msg,
            failures = {}
          }

        -- otherwise we had failures, log the first and supply the list of
        -- additional failures if any for printResult()
        else
          local key = failure['key']
          if failure['test'] ~= nil then
            key = key .. ' [' .. failure['test'] .. ']'
          end
          local msg = failure['message']
          if self.fatal ~= '' then
            key = 'code'
            msg = self.fatal
          end
          self.result = {
            total = total,
            result = 'FAIL',
            passed = false,
            key = key,
            message = msg,
            failures = assert_failures
          }
        end
      end
    end
    self:printResult()
  end,


  -- @method - TestMethod:printResult()
  -- @desc - prints the result of the test to the console as well as appends
  --         the XML + HTML for the test to the testsuite output
  -- @return {nil}
  printResult = function(self)

    -- get total timestamp
    self.finish = love.timer.getTime() - self.start
    love.test.time = love.test.time + self.finish
    self.testmodule.time = self.testmodule.time + self.finish
    local endtime = UtilTimeFormat(love.timer.getTime() - self.start)

    -- get failure/skip message for output (if any)
    local failure = ''
    local output = ''
    if not self.passed and not self.skipped then
      failure = '\t\t\t<failure message="' .. self.result.key .. ' ' ..
        self.result.message .. '">' .. self.result.key .. ' ' .. self.result.message .. '</failure>\n'
        output = self.result.key .. ' ' ..  self.result.message
      -- append failures if any to report md
      love.test.mdfailures = love.test.mdfailures .. '> 🔴 ' .. self.method .. '  \n' ..
        '> ' .. output .. '  \n\n'
    end
    if output == '' and self.skipped then
      failure = '\t\t\t<skipped message="' .. self.skipreason .. '" />\n'
      output = self.skipreason
    end


    -- append XML for the test class result
    self.testmodule.xml = self.testmodule.xml .. '\t\t<testcase classname="' ..
      self.method .. '" name="' .. self.method .. '" assertions="' .. tostring(#self.asserts) ..
      '" time="' .. endtime .. '">\n' ..
      failure .. '\t\t</testcase>\n'

    -- unused currently, adds a preview image for certain graphics methods to the output
    local preview = ''
    if self.testmodule.module == 'graphics' then
      local filename = 'love.test.graphics.' .. self.method
      for f=1,5 do
        local fstr = tostring(f)
        if love.filesystem.openFile('tempoutput/actual/' .. filename .. '-' .. fstr .. '.png', 'r') then
          preview = preview .. '<div class="preview-wrap">'
          preview = preview .. '<div class="preview">' .. '<img src="expected/' .. filename .. '-' .. fstr .. '.png"/><p>Expected</p></div>' ..
            '<div class="preview">' .. '<img src="actual/' .. filename .. '-' .. fstr .. '.png"/><p>Actual</p></div>'
          if love.filesystem.openFile('tempoutput/difference/' .. filename .. '-' .. fstr .. '.png', 'r') then
            preview = preview .. '<div class="preview">' .. '<img src="difference/' .. filename .. '-' .. fstr .. '.png"/><p>Difference</p></div>'
          end
          preview = preview .. '</div>'
        end
      end
    end

    -- append HTML for the test class result
    local status = ''
    local cls = 'red'
    if self.passed then status = '<div class="icon pass"></div>'; cls = 'green' end
    if self.skipped then status = ''; cls = 'yellow' end
    self.testmodule.html = self.testmodule.html ..
      '<tr class=" ' .. cls .. '">' ..
        '<td>' .. status .. '</td>' ..
        '<td>' .. self.method .. '</td>' ..
        '<td>' .. endtime .. 's</td>' ..
        '<td>' .. output .. preview .. '</td>' ..
      '</tr>'

    -- add message if assert failed
    local msg = ''
    if self.result.message ~= nil and not self.skipped then
      msg = ' - ' .. self.result.key ..
        ' failed - (' .. self.result.message .. ')'
    end
    if self.skipped then
      msg = self.result.message
    end

    -- log final test result to console
    -- i know its hacky but its neat soz
    local tested = 'love.' .. self.testmodule.module .. '.' .. self.method .. '()'
    local matching = string.sub(self.testmodule.spacer, string.len(tested), 40)
    self.testmodule:log(
      self.testmodule.colors[self.result.result],
      '  ' .. tested .. matching,
      ' ==> ' .. self.result.result .. ' - ' .. endtime .. 's ' ..
      self.result.total .. msg
    )

    -- if we failed on multiple asserts, list them here - makes it easier for
    -- debugging new methods added that are failing multiple asserts
    if #self.result.failures > 1 then
      for f=2,#self.result.failures do
        local addf = self.result.failures[f]
        self.testmodule:log(
          self.testmodule.colors[self.result.result],
          '  ' .. tested .. matching,
          ' ==> ' ..
          addf['key'] .. ' [' .. addf['test'] .. '] failed - ' .. addf['message']
        )
      end
    end

  end


}
