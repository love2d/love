-- @class - TestModule
-- @desc - used to run tests for a given module, each test method will spawn
--         a love.test.Test object
TestModule = {


  -- @method - TestModule:new()
  -- @desc - create a new Suite object
  -- @param {string} module - string of love module the suite is for
  -- @return {table} - returns the new Suite object
  new = function(self, module, method)
    local testmodule = {
      time = 0,
      spacer = '                                        ',
      colors = {
        PASS = 'green', FAIL = 'red', SKIP = 'grey'
      },
      colormap = {
        grey = '\27[37m',
        green = '\27[32m',
        red = '\27[31m',
        yellow = '\27[33m'
      },
      xml = '',
      html = '',
      tests = {},
      running = {},
      called = {},
      passed = 0,
      failed = 0,
      skipped = 0,
      module = module,
      method = method,
      index = 1,
      start = false,
    }
    setmetatable(testmodule, self)
    self.__index = self
    return testmodule
  end,


  -- @method - TestModule:log()
  -- @desc - log to console with specific colors, split out to make it easier
  --         to adjust all console output across the tests
  -- @param {string} color - color key to use for the log
  -- @param {string} line - main message to write (LHS)
  -- @param {string} result - result message to write (RHS)
  -- @return {nil}
  log = function(self, color, line, result)
    if result == nil then result = '' end
    print(self.colormap[color] .. line .. result)
  end,


  -- @method - TestModule:runTests()
  -- @desc - starts the running of tests and sets up the list of methods to test
  -- @param {string} module - module to set for the test suite
  -- @param {string} method - specific method to test, if nil all methods tested
  -- @return {nil}
  runTests = function(self)
    self.running = {}
    self.passed = 0
    self.failed = 0
    if self.method ~= nil then
      table.insert(self.running, self.method)
    else
      for i,_ in pairs(love.test[self.module]) do
        table.insert(self.running, i)
      end
      table.sort(self.running)
    end
    self.index = 1
    self.start = true
    self:log('yellow', '\nlove.' .. self.module .. '.testmodule.start')
  end,


  -- @method - TestModule:printResult()
  -- @desc - prints the result of the module to the console as well as appends
  --         the XML + HTML for the test to the testsuite output
  -- @return {nil}
  printResult = function(self)
    local finaltime = UtilTimeFormat(self.time)
    local status = '<div class="icon fail"></div>'
    if self.failed == 0 then status = '<div class="icon pass"></div>' end
    -- add md row to main output
    love.test.mdrows = love.test.mdrows .. '| ' .. status .. 
      ' ' .. self.module .. 
      ' | ' .. tostring(self.passed) .. 
      ' | ' .. tostring(self.failed) .. 
      ' | ' .. tostring(self.skipped) .. 
      ' | ' .. finaltime .. 's |' .. '\n'
    -- add xml to main output
    love.test.xml = love.test.xml .. '\t<testsuite name="love.' .. self.module .. 
      '" tests="' .. tostring(self.passed) .. 
      '" failures="' .. tostring(self.failed) .. 
      '" skipped="' .. tostring(self.skipped) ..
      '" time="' .. finaltime .. '">\n' .. self.xml .. '\t</testsuite>\n'
    -- add html to main output
    local module_cls = 'toggle close'
    local module_txt = '▶'
    local wrap_cls = ''
    if self.failed > 0 then
      module_cls = 'toggle open'
      module_txt = '▼'
      wrap_cls = 'fail'
    end
    love.test.html = love.test.html .. '<div class="module ' .. wrap_cls .. '">' ..
      '<div class="' .. module_cls .. '" onclick="toggle(this)">' .. module_txt .. '</div>' .. 
      '<h2>' .. status .. '&nbsp;love.' .. self.module .. '</h2><ul class="summary">' ..
      '<li class="l' .. tostring(self.passed) .. '">' .. tostring(self.passed) .. ' Passed</li>' ..
      '<li class="l' .. tostring(self.failed) .. '">' .. tostring(self.failed) .. ' Failed</li>' ..
      '<li class="l' .. tostring(self.skipped) .. '">' .. tostring(self.skipped) .. ' Skipped</li>' ..
      '<li>' .. finaltime .. 's</li>' .. '</ul><br/><br/>' ..
      '<table><thead><tr><td width="20px"></td><td width="100px">Method</td><td width="60px">Time</td><td>Details</td></tr></thead><tbody>' ..
      self.html .. '</tbody></table></div>'
    -- print module results to console
    self:log('yellow', 'love.' .. self.module .. '.testmodule.end')
    local failedcol = '\27[31m'
    if self.failed == 0 then failedcol = '\27[37m' end
    self:log('green', tostring(self.passed) .. ' PASSED' .. ' || ' .. 
      failedcol .. tostring(self.failed) .. ' FAILED || \27[37m' .. 
      tostring(self.skipped) .. ' SKIPPED || ' .. finaltime .. 's')
    self.start = false
    self.fakequit = false
  end


}
