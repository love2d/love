TestSuite = {


  -- @method - TestSuite:new()
  -- @desc - creates a new TestSuite object that handles all the tests
  -- @return {table} - returns the new TestSuite object
  new = function(self)
    local test = {

      -- testsuite internals
      modules = {},
      module = nil,
      test = nil,
      testcanvas = nil,
      current = 1,
      output = '',
      totals = {0, 0, 0},
      time = 0,
      xml = '',
      html = '',
      mdrows = '',
      mdfailures = '',
      delayed = nil,
      fakequit = false,
      windowmode = true,
      current_os = love._os,
      lua_version = tonumber(_VERSION:match("%d%.%d")),
      has_lua_jit = type(jit) == 'table',

      -- love modules to test
      audio = {},
      data = {},
      event = {},
      filesystem = {},
      font = {},
      graphics = {},
      image = {},
      joystick = {},
      love = {},
      keyboard = {},
      math = {},
      mouse = {},
      physics = {},
      sensor = {},
      sound = {},
      system = {},
      thread = {},
      timer = {},
      touch = {},
      video = {},
      window = {}

    }
    setmetatable(test, self)
    self.__index = self
    return test
  end,


  -- @method - TestSuite:runSuite()
  -- @desc - called in love.update, runs through every method or every module
  -- @param {number} delta - delta from love.update to track time elapsed
  -- @return {nil}
  runSuite = function(self, delta)

      -- stagger between tests
    if self.module ~= nil then

        if self.module.start then

          -- work through each test method 1 by 1
          if self.module.index <= #self.module.running then

            -- run method once
            if self.module.called[self.module.index] == nil then
              self.module.called[self.module.index] = true
              local method = self.module.running[self.module.index]
              self.test = TestMethod:new(method, self.module)
              TextRun = 'love.' .. self.module.module .. '.' .. method

              self.test.co = coroutine.create(function()
                local ok, chunk, err = pcall(
                  love.test[love.test.module.module][method],
                  love.test.test
                )
                if ok == false then
                  love.test.test['passed'] = false
                  love.test.test['fatal'] = tostring(chunk) .. tostring(err)
                end
              end)


            -- once called we have a corouting, so just call resume every frame
            -- until we have finished
            else

              -- move onto next yield if any
              -- pauses can be set with TestMethod:waitFrames(frames)
              coroutine.resume(self.test.co)

              -- when wait finished (or no yields)
              if coroutine.status(self.test.co) == 'dead' then
                -- now we're all done evaluate the test
                local ok, chunk, err = pcall(self.test.evaluateTest, self.test)
                if ok == false then
                  self.test.passed = false
                  self.test.fatal = tostring(chunk) .. tostring(err)
                end
                -- save having to :release() anything we made in the last test
                collectgarbage("collect")
                -- move onto the next test
                self.module.index = self.module.index + 1
              end

            end

          -- once all tests have run
          else

            -- print module results and add to output
            self.module:printResult()

            -- if we have more modules to go run the next one
            self.current = self.current + 1
            if #self.modules >= self.current then
              self.module = self.modules[self.current]
              self.module:runTests()

            -- otherwise print the final results and export output
            else
              self:printResult()
              love.event.quit(0)
            end

          end
        end
      end

  end,


  -- @method - TestSuite:printResult()
  -- @desc - prints the result of the whole test suite as well as writes
  --         the MD, XML + HTML of the testsuite output
  -- @return {nil}
  printResult = function(self)
    local finaltime = UtilTimeFormat(self.time)

    -- in case we dont have love.graphics loaded, for future module specific disabling
    local name = 'NONE'
    local version = 'NONE'
    local vendor = 'NONE'
    local device = 'NONE'
    if love.graphics then
      name, version, vendor, device = love.graphics.getRendererInfo()
    end

    local md = '<!-- PASSED ' .. tostring(self.totals[1]) ..
      ' || FAILED ' .. tostring(self.totals[2]) ..
      ' || SKIPPED ' .. tostring(self.totals[3]) ..
      ' || TIME ' .. finaltime .. ' -->\n\n### Info\n' ..
      '**' .. tostring(self.totals[1] + self.totals[2] + self.totals[3]) .. '** tests were completed in **' ..
      finaltime .. 's** with **' ..
      tostring(self.totals[1]) .. '** passed, **' ..
      tostring(self.totals[2]) .. '** failed, and **' ..
      tostring(self.totals[3]) .. '** skipped\n\n' ..
      'Renderer: ' .. name .. ' | ' .. version .. ' | ' .. vendor .. ' | ' .. device .. '\n\n' ..
      '### Report\n' ..
      '| Module                | Pass | Fail | Skip | Time   |\n' ..
      '| --------------------- | ------ | ------ | ------- | ------ |\n' ..
      self.mdrows .. '\n### Failures\n' .. self.mdfailures

    local xml = '<testsuites name="love.test" tests="' .. tostring(self.totals[1]) ..
      '" failures="' .. tostring(self.totals[2]) ..
      '" skipped="' .. tostring(self.totals[3]) ..
      '" time="' .. finaltime .. '">\n'

    local status = '<div class="icon fail"></div>'
    if self.totals[2] == 0 then status = '<div class="icon pass"></div>' end
    local html = [[
      <html>
        <head>
          <style>
          * { font-family: monospace; margin: 0; font-size: 11px; padding: 0; }
          body { margin: 40px 50px 50px 50px; overflow-y: scroll; background: #222; }
          h1 { font-weight: normal; color: #eee; font-size: 12px; width: 140px; border-radius: 2px; padding: 5px 0; float: left; background: #333;  }
          h2 { font-weight: normal; color: #eee; font-size: 12px; width: 140px; border-radius: 2px;  }
          .summary { z-index: 10; position: relative; list-style: none; margin: 0; padding: 0; float: right; } 
          .summary li { color: #111; float: left; border-radius: 2px; background: #eee; padding: 5px; margin-right: 10px; text-align: right; }
          table { color: #eee; background: #444; margin: 5px 0 0 10px; width: calc(100% - 20px); max-width: 800px; border-collapse: collapse }
          table thead { background: #333; }
          table th, table td { padding: 2px 4px; font-size: 11px; }
          tr.red { background: #d26666; color: #111; }
          tr.yellow { background: slategrey; }
          .wrap { max-width: 800px; padding-top: 30px; margin: auto; position: relative; }
          .preview-wrap { display: inline-block; height: 80px; padding: 5px 0 0 5px; margin: 5px; background: rgba(0, 0, 0, 0.1); }
          .preview { width: 64px; height: 80px; float: left; margin-right: 10px; }
          .preview:nth-last-child(1) { margin-right: 5px; }
          .preview img { width: 100%; image-rendering: pixelated; }
          .preview p { text-align: center; }
          .module { margin-top: 10px; position: relative; }
          .module h2 { float: left; margin: 0; padding: 5px 0 0 35px; }
          .module .toggle { background: #2d9966; color: #111; left: 10px; width: 14px; border-radius: 2px; padding: 6px; text-align: center; cursor: pointer; position: absolute; }
          .module.fail .toggle { background: #d26666; }  
          .module.fail h2 { color: #d26666; }
          .toggle.close ~ table { display: none; }
          .summary li:nth-child(1) { background: #2d9966; min-width: 70px; }
          .summary li:nth-child(2) { background: #d26666; min-width: 70px; }
          .summary li:nth-child(3) { background: slategrey; min-width: 70px; }
          .summary li:nth-child(4) { background: #bbb; min-width: 60px; }
          .summary li.l0 { opacity: 0.2; }
          .renderer { position: absolute; top: 8px; right: 10px; color: #eee; }
          h1 { width: 100%; top: 0; position: absolute; height: 50px; left: 0; }
          table .icon.pass { position: relative; width: 8px; height: 8px; border-radius: 8px; margin-left: 6px; }
          table .icon.pass:after { content: '✓'; top: -3px; position: absolute; color: #2d9966; font-size: 12px;  }
          </style>
          <script type="text/javascript">
            function toggle(el) { 
              el.className = el.className == 'toggle close' ? 'toggle open' : 'toggle close'; 
              el.innerText = el.className == 'toggle close' ? '▶' : '▼';
            }
          </script>
        </head>
        <body>]]
    local wrap_cls = ''
    if self.totals[2] > 0 then wrap_cls = 'fail' end
    html = html .. '<div class="wrap ' .. wrap_cls .. '"><h1>' .. status .. '&nbsp;love.test report</h1>' ..
    '<p class="renderer">Renderer: ' .. name .. ' | ' .. version .. ' | ' .. vendor .. ' | ' .. device .. '</p>' ..
    '<ul class="summary">'
    html = html ..
      '<li>' .. tostring(self.totals[1]) .. ' Passed</li>' ..
      '<li>' .. tostring(self.totals[2]) .. ' Failed</li>' ..
      '<li>' .. tostring(self.totals[3]) .. ' Skipped</li>' ..
      '<li>' .. finaltime .. 's</li></ul><br/><br/>'

    love.filesystem.write('tempoutput/' .. self.output .. '.xml', xml .. self.xml .. '</testsuites>')
    love.filesystem.write('tempoutput/' .. self.output .. '.html', html .. self.html .. '</div></body></html>')
    love.filesystem.write('tempoutput/' .. self.output .. '.md', md)

    self.module:log('grey', '\nFINISHED - ' .. finaltime .. 's\n')
    local failedcol = '\27[31m'
    if self.totals[2] == 0 then failedcol = '\27[37m' end
    self.module:log('green', tostring(self.totals[1]) .. ' PASSED' .. ' || ' .. failedcol .. tostring(self.totals[2]) .. ' FAILED || \27[37m' .. tostring(self.totals[3]) .. ' SKIPPED')

  end


}
