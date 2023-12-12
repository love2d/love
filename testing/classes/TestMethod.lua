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


  -- @method - TestMethod:assertPixels()
  -- @desc - checks a list of coloured pixels agaisnt given imgdata
  -- @param {ImageData} imgdata - image data to check
  -- @param {table} pixelchecks - map of colors to list of pixel coords, i.e.
  --                              { blue = { {1, 1}, {2, 2}, {3, 4} } }
  -- @return {nil}
  assertPixels = function(self, imgdata, pixelchecks, label)
    for i, v in pairs(pixelchecks) do
      local col = self.colors[i]
      local pixels = v
      for p=1,#pixels do
        local coord = pixels[p]
        local tr, tg, tb, ta = imgdata:getPixel(coord[1], coord[2])
        local compare_id = tostring(coord[1]) .. ',' .. tostring(coord[2])
        -- prevent us getting stuff like 0.501960785 for 0.5 red 
        tr = math.floor((tr*10)+0.5)/10
        tg = math.floor((tg*10)+0.5)/10
        tb = math.floor((tb*10)+0.5)/10
        ta = math.floor((ta*10)+0.5)/10
        col[1] = math.floor((col[1]*10)+0.5)/10
        col[2] = math.floor((col[2]*10)+0.5)/10
        col[3] = math.floor((col[3]*10)+0.5)/10
        col[4] = math.floor((col[4]*10)+0.5)/10
        self:assertEquals(col[1], tr, 'check pixel r for ' .. i .. ' at ' .. compare_id .. '(' .. label .. ')')
        self:assertEquals(col[2], tg, 'check pixel g for ' .. i .. ' at ' .. compare_id .. '(' .. label .. ')')
        self:assertEquals(col[3], tb, 'check pixel b for ' .. i .. ' at ' .. compare_id .. '(' .. label .. ')')
        self:assertEquals(col[4], ta, 'check pixel a for ' .. i .. ' at ' .. compare_id .. '(' .. label .. ')')
      end
    end
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
    local expected = love.image.newImageData(
      'tempoutput/expected/love.test.graphics.' .. self.method .. '-' .. 
      tostring(self.imgs) .. '.png'
    )
    local iw = imgdata:getWidth()-2
    local ih = imgdata:getHeight()-2
    local rgba_tolerance = self.rgba_tolerance * (1/255)
    for ix=2,iw do
      for iy=2,ih do
        local ir, ig, ib, ia = imgdata:getPixel(ix, iy)
        local points = {
          {expected:getPixel(ix, iy)}
        }
        if self.pixel_tolerance > 0 then
          table.insert(points, {expected:getPixel(ix-1, iy+1)})
          table.insert(points, {expected:getPixel(ix-1, iy)})
          table.insert(points, {expected:getPixel(ix-1, iy-1)})
          table.insert(points, {expected:getPixel(ix, iy+1)})
          table.insert(points, {expected:getPixel(ix, iy-1)})
          table.insert(points, {expected:getPixel(ix+1, iy+1)})
          table.insert(points, {expected:getPixel(ix+1, iy)})
          table.insert(points, {expected:getPixel(ix+1, iy-1)})
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
      end
    end
    local path = 'tempoutput/actual/love.test.graphics.' .. 
      self.method .. '-' .. tostring(self.imgs) .. '.png'
    imgdata:encode('png', path)
    self.imgs = self.imgs + 1
  end,


  -- @method - TestMethod:skipTest()
  -- @desc - used to mark this test as skipped for a specific reason
  -- @param {string} reason - reason why method is being skipped
  -- @return {nil}
  skipTest = function(self, reason)
    self.skipped = true
    self.skipreason = reason
  end,


  waitFrames = function(self, frames)
    for i=1,frames do coroutine.yield() end
  end,


  waitSeconds = function(self, seconds)
    local start = love.timer.getTime()
    while love.timer.getTime() < start + seconds do
      coroutine.yield()
    end
  end,


  -- @method - TestMethod:evaluateTest()
  -- @desc - evaluates the results of all assertions for a final restult
  -- @return {nil}
  evaluateTest = function(self)
    local failure = ''
    local failures = 0
    for a=1,#self.asserts do
      -- @TODO show all failed assertion methods?
      -- currently just shows the first assert that failed
      if not self.asserts[a].passed and not self.skipped then
        if failure == '' then failure = self.asserts[a] end
        failures = failures + 1
      end
    end
    if self.fatal ~= '' then failure = self.fatal end
    local passed = tostring(#self.asserts - failures)
    local total = '(' .. passed .. '/' .. tostring(#self.asserts) .. ')'
    if self.skipped then
      self.testmodule.skipped = self.testmodule.skipped + 1
      love.test.totals[3] = love.test.totals[3] + 1
      self.result = { 
        total = '', 
        result = "SKIP", 
        passed = false, 
        message = '(0/0) - method skipped [' .. self.skipreason .. ']'
      }
    else
      if failure == '' and #self.asserts > 0 then
        self.passed = true
        self.testmodule.passed = self.testmodule.passed + 1
        love.test.totals[1] = love.test.totals[1] + 1
        self.result = { 
          total = total, 
          result = 'PASS', 
          passed = true, 
          message = nil
        }
      else
        self.passed = false
        self.testmodule.failed = self.testmodule.failed + 1
        love.test.totals[2] = love.test.totals[2] + 1
        if #self.asserts == 0 then
          local msg = 'no asserts defined'
          if self.fatal ~= '' then msg = self.fatal end
          self.result = { 
            total = total, 
            result = 'FAIL', 
            passed = false, 
            key = 'test', 
            message = msg 
          }
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
            message = msg
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
      if love.filesystem.openFile('tempoutput/actual/' .. filename .. '-1.png', 'r') then
        preview = '<div class="preview">' .. '<img src="expected/' .. filename .. '-1.png"/><p>Expected</p></div>' ..
          '<div class="preview">' .. '<img src="actual/' .. filename .. '-1.png"/><p>Actual</p></div>'
      end
      if love.filesystem.openFile('tempoutput/actual/' .. filename .. '-2.png', 'r') then
        preview = preview .. '<div class="preview">' .. '<img src="expected/' .. filename .. '-2.png"/><p>Expected</p></div>' ..
          '<div class="preview">' .. '<img src="actual/' .. filename .. '-2.png"/><p>Actual</p></div>'
      end
      if love.filesystem.openFile('tempoutput/actual/' .. filename .. '-3.png', 'r') then
        preview = preview .. '<div class="preview">' .. '<img src="expected/' .. filename .. '-3.png"/><p>Expected</p></div>' ..
          '<div class="preview">' .. '<img src="actual/' .. filename .. '-3.png"/><p>Actual</p></div>'
      end
    end

    -- append HTML for the test class result 
    local status = '🔴'
    local cls = 'red'
    if self.passed then status = '🟢'; cls = '' end
    if self.skipped then status = '🟡'; cls = '' end
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
  end


}
