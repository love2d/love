--[[
    贪吃蛇游戏 — 用于验证 LÖVE 对 Lua 5.4 的支持

    使用的 Lua 5.4 特性:
    1. <const> 变量属性
    2. // 整数除法运算符
    3. 位运算符 (&, |, <<, >>)
    4. math.type() 区分 integer / float
    5. 内置 utf8 标准库
    6. warn() 警告系统
    7. math.maxinteger / math.mininteger 常量
    8. 整数与浮点数的严格区分
]]

-- ============================================================
--  特性 1: <const> 变量属性 (Lua 5.4)
-- ============================================================
local CELL   <const> = 20        -- 每格像素
local COLS   <const> = 30        -- 列数
local ROWS   <const> = 22        -- 行数
local WIDTH  <const> = COLS * CELL  -- 600
local HEIGHT <const> = ROWS * CELL  -- 440
local HUD_H  <const> = 40        -- 顶部信息栏高度
local PAD    <const> = 20        -- 边距
local BOT_H  <const> = 30        -- 底部信息栏高度
-- 设计分辨率（逻辑坐标系）
local DESIGN_W <const> = PAD + WIDTH + PAD    -- 640
local DESIGN_H <const> = HUD_H + HEIGHT + BOT_H  -- 510

-- 方向编码：用位运算做唯一标识
-- ============================================================
--  特性 2: 位运算符 (Lua 5.4)
-- ============================================================
local DIR_UP    <const> = 1 << 0  -- 0b0001
local DIR_DOWN  <const> = 1 << 1  -- 0b0010
local DIR_LEFT  <const> = 1 << 2  -- 0b0100
local DIR_RIGHT <const> = 1 << 3  -- 0b1000

-- 用位运算判断两个方向是否相反
local function isOpposite(a, b)
    -- UP|DOWN = 0b0011, LEFT|RIGHT = 0b1100
    local pair = a | b
    return pair == 0x3 or pair == 0xC
end

-- 方向 -> 速度向量 (用位运算解码)
local function dirToVec(d)
    -- ============================================================
    --  特性 3: // 整数除法 (Lua 5.4)
    -- ============================================================
    local horizontal = (d & 0xC) ~= 0  -- bit2 或 bit3
    if horizontal then
        return d == DIR_RIGHT and 1 or -1, 0
    else
        return 0, d == DIR_DOWN and 1 or -1
    end
end

-- ============================================================
--  特性 4: math.type() 区分 integer / float (Lua 5.4)
-- ============================================================
local function verifyIntegerType(value, name)
    local t = math.type(value)
    if t ~= "integer" then
        -- 特性 5: warn() 警告系统
        warn(string.format("%s 应该是 integer，但实际是 %s", name, tostring(t)))
        return false
    end
    return true
end

-- ============================================================
--  特性 5: 内置 utf8 标准库 (Lua 5.4)
--  用于正确处理 UTF-8 中文字符串长度和遍历
-- ============================================================
local function utf8Title(text)
    local charCount = utf8.len(text)
    local codepoints = {}
    for _, code in utf8.codes(text) do
        codepoints[#codepoints + 1] = code
    end
    return charCount, codepoints
end

-- 蛇身颜色：用位移生成渐变
local function snakeColor(index, total)
    -- ============================================================
    --  特性 6: 位运算 + 整数除法 生成颜色
    -- ============================================================
    local base = 80 + (175 * index) // total
    local r = (base & 0xFF) / 255
    local g = ((base + 80) & 0xFF) / 255
    local b = 0.2
    return r, g, b
end

-- 游戏状态
local snake, food, dir, nextDir, score, timer, speed, state
local lua54Features = {}  -- 记录已验证的特性
local font, smallFont      -- 中文字体

local function spawnFood()
    local occupied = {}
    for _, seg in ipairs(snake) do
        occupied[seg.y * COLS + seg.x] = true
    end
    local free = {}
    for y = 0, ROWS - 1 do
        for x = 0, COLS - 1 do
            if not occupied[y * COLS + x] then
                free[#free + 1] = { x = x, y = y }
            end
        end
    end
    if #free > 0 then
        food = free[love.math.random(#free)]
    end
end

local function resetGame()
    -- ============================================================
    --  特性 7: math.maxinteger (Lua 5.4)
    -- ============================================================
    local seed = os.time() & 0xFFFFFFFF  -- 用位运算截断
    love.math.setRandomSeed(seed)

    local cx = COLS // 2  -- 整数除法
    local cy = ROWS // 2

    snake = {
        { x = cx, y = cy },
        { x = cx - 1, y = cy },
        { x = cx - 2, y = cy },
    }
    dir = DIR_RIGHT
    nextDir = DIR_RIGHT
    score = 0
    timer = 0
    speed = 0.12
    state = "playing"

    spawnFood()
end

-- ============================================================
--  验证所有 Lua 5.4 特性
-- ============================================================
local function verifyLua54()
    lua54Features = {}

    -- 1) 版本检查
    local major = _VERSION:match("Lua (%d+%.%d+)")
    lua54Features[#lua54Features + 1] = {
        name = "<const> 变量属性",
        pass = true,  -- 如果这文件能加载就说明 <const> 有效
        detail = "文件成功加载，<const> 语法被接受"
    }

    -- 2) // 整数除法
    local divResult = 7 // 2
    local isInt = math.type(divResult) == "integer" and divResult == 3
    lua54Features[#lua54Features + 1] = {
        name = "// 整数除法",
        pass = isInt,
        detail = string.format("7 // 2 = %s (type: %s)", divResult, math.type(divResult))
    }

    -- 3) 位运算符
    local bitResult = (0xFF & 0x0F) | (1 << 4)
    lua54Features[#lua54Features + 1] = {
        name = "位运算符 &, |, <<",
        pass = bitResult == 0x1F,
        detail = string.format("(0xFF & 0x0F) | (1 << 4) = 0x%X", bitResult)
    }

    -- 4) math.type()
    local intType = math.type(42)
    local fltType = math.type(3.14)
    lua54Features[#lua54Features + 1] = {
        name = "math.type() 类型区分",
        pass = intType == "integer" and fltType == "float",
        detail = string.format("42→%s, 3.14→%s", intType, fltType)
    }

    -- 5) 内置 utf8 库
    local testStr = "贪吃蛇"
    local ok, charLen = pcall(function() return utf8.len(testStr) end)
    lua54Features[#lua54Features + 1] = {
        name = "内置 utf8 标准库",
        pass = ok and charLen == 3,
        detail = ok and string.format("utf8.len(\"%s\") = %d", testStr, charLen) or "不可用"
    }

    -- 6) warn() 函数
    local hasWarn = type(warn) == "function"
    lua54Features[#lua54Features + 1] = {
        name = "warn() 警告系统",
        pass = hasWarn,
        detail = hasWarn and "warn 函数存在" or "warn 函数不存在"
    }

    -- 7) math.maxinteger / math.mininteger
    local hasMaxInt = math.maxinteger ~= nil and math.mininteger ~= nil
    lua54Features[#lua54Features + 1] = {
        name = "math.maxinteger 常量",
        pass = hasMaxInt,
        detail = hasMaxInt and string.format("max=%d", math.maxinteger) or "不存在"
    }

    -- 8) 整数语义
    local a = 10
    local b = 3
    local intDiv = a // b           -- 整数除法得整数
    local floatDiv = a / b          -- 普通除法得浮点
    lua54Features[#lua54Features + 1] = {
        name = "整数/浮点严格区分",
        pass = math.type(intDiv) == "integer" and math.type(floatDiv) == "float",
        detail = string.format("10//3 type=%s, 10/3 type=%s", math.type(intDiv), math.type(floatDiv))
    }

    -- 打印验证结果
    print("═══════════════════════════════════════")
    print("  Lua 5.4 特性验证报告  (" .. _VERSION .. ")")
    print("═══════════════════════════════════════")
    local allPass = true
    for i, f in ipairs(lua54Features) do
        local icon = f.pass and "✓" or "✗"
        print(string.format("  %s %d. %s", icon, i, f.name))
        print(string.format("      %s", f.detail))
        if not f.pass then allPass = false end
    end
    print("═══════════════════════════════════════")
    print(allPass and "  全部通过！Lua 5.4 完全支持！" or "  存在不通过项，请检查")
    print("═══════════════════════════════════════")
end

-- 计算缩放，让设计分辨率适配窗口，居中显示
local function getScale()
    local ww, wh = love.graphics.getDimensions()
    local sx = ww / DESIGN_W
    local sy = wh / DESIGN_H
    local s = math.min(sx, sy)
    local ox = (ww - DESIGN_W * s) / 2
    local oy = (wh - DESIGN_H * s) / 2
    return s, ox, oy
end

function love.load()
    love.graphics.setBackgroundColor(0.08, 0.08, 0.12)

    -- 加载中文字体 (挂载 Windows 系统字体目录)
    love.filesystem.mountFullPath("C:/Windows/Fonts", "sysfont", "read")
    font = love.graphics.newFont("sysfont/msyh.ttc", 16)
    smallFont = love.graphics.newFont("sysfont/msyh.ttc", 13)
    love.graphics.setFont(font)

    -- 验证 Lua 5.4 特性
    verifyLua54()

    -- 验证整数类型
    verifyIntegerType(CELL, "CELL")
    verifyIntegerType(COLS, "COLS")

    -- utf8 标题测试
    local titleLen, _ = utf8Title("贪吃蛇游戏")
    print(string.format("标题 UTF-8 字符数: %d", titleLen))

    resetGame()
end

function love.update(dt)
    if state ~= "playing" then return end

    timer = timer + dt
    if timer < speed then return end
    timer = timer - speed

    -- 应用方向
    if not isOpposite(nextDir, dir) then
        dir = nextDir
    end

    local dx, dy = dirToVec(dir)
    local head = snake[1]
    local nx = (head.x + dx) % COLS  -- 穿墙
    local ny = (head.y + dy) % ROWS

    -- 自碰检测
    for i = 1, #snake do
        if snake[i].x == nx and snake[i].y == ny then
            state = "gameover"
            return
        end
    end

    -- 移动
    table.insert(snake, 1, { x = nx, y = ny })

    -- 吃食物
    if food and nx == food.x and ny == food.y then
        score = score + 1
        -- 每 5 分加速 (用整数除法判断)
        if score % 5 == 0 then
            speed = speed * 0.9
        end
        spawnFood()
    else
        table.remove(snake)
    end
end

function love.draw()
    local scale, ox, oy = getScale()

    -- 应用缩放和居中偏移，之后所有坐标都用设计分辨率
    love.graphics.push()
    love.graphics.translate(ox, oy)
    love.graphics.scale(scale)

    -- 绘制游戏区域背景
    love.graphics.setColor(0.06, 0.06, 0.10)
    love.graphics.rectangle("fill", 0, 0, DESIGN_W, DESIGN_H)

    love.graphics.push()
    love.graphics.translate(PAD, HUD_H)

    -- 网格背景
    love.graphics.setColor(0.12, 0.12, 0.18)
    for y = 0, ROWS - 1 do
        for x = 0, COLS - 1 do
            if (x + y) & 1 == 0 then  -- 位运算做棋盘格
                love.graphics.rectangle("fill",
                    x * CELL, y * CELL, CELL, CELL)
            end
        end
    end

    -- 食物
    if food then
        local pulse = 0.7 + 0.3 * math.sin(love.timer.getTime() * 6)
        love.graphics.setColor(1, 0.3, 0.3, pulse)
        love.graphics.rectangle("fill",
            food.x * CELL + 2, food.y * CELL + 2, CELL - 4, CELL - 4,
            4, 4)
    end

    -- 蛇身
    local total = #snake
    for i, seg in ipairs(snake) do
        local r, g, b = snakeColor(total - i, total)
        love.graphics.setColor(r, g, b)
        local pad = i == 1 and 1 or 2
        love.graphics.rectangle("fill",
            seg.x * CELL + pad, seg.y * CELL + pad,
            CELL - pad * 2, CELL - pad * 2,
            i == 1 and 4 or 2, i == 1 and 4 or 2)
    end

    love.graphics.pop()

    -- HUD
    love.graphics.setColor(0.9, 0.9, 0.9)
    love.graphics.print(string.format(
        "得分: %d  |  %s  |  速度: %.0f%%  |  蛇长: %d",
        score, _VERSION,
        (0.12 / speed) * 100,
        #snake
    ), PAD, 12)

    -- Lua 5.4 特性标记
    love.graphics.setColor(0.4, 0.9, 0.4)
    love.graphics.setFont(smallFont)
    local featureTag = string.format(
        "整数除法: %d  位运算: 0x%X  math.type(1)=%s",
        7 // 2, (0xAB & 0x0F) | (1 << 7), math.type(1)
    )
    love.graphics.print(featureTag, PAD, HUD_H + HEIGHT + 6)
    love.graphics.setFont(font)

    -- Game Over
    if state == "gameover" then
        love.graphics.setColor(0, 0, 0, 0.7)
        love.graphics.rectangle("fill", 0, 0, DESIGN_W, DESIGN_H)

        love.graphics.setColor(1, 0.3, 0.3)
        local msg = "游戏结束!"
        love.graphics.printf(msg, 0, 180, DESIGN_W, "center")
        love.graphics.setColor(0.9, 0.9, 0.9)
        love.graphics.printf(
            string.format("得分: %d    按 R 重新开始", score),
            0, 220, DESIGN_W, "center")

        -- 显示 Lua 5.4 验证结果
        love.graphics.setFont(smallFont)
        love.graphics.setColor(0.5, 1, 0.5)
        local y = 260
        for i, f in ipairs(lua54Features) do
            local icon = f.pass and "✓" or "✗"
            if not f.pass then love.graphics.setColor(1, 0.4, 0.4) end
            love.graphics.printf(
                string.format("%s %s: %s", icon, f.name, f.detail),
                40, y, DESIGN_W - 80, "left")
            y = y + 18
            love.graphics.setColor(0.5, 1, 0.5)
        end
        love.graphics.setFont(font)
    end

    -- 闭合缩放变换
    love.graphics.pop()
end

function love.keypressed(key)
    if key == "up"    or key == "w" then nextDir = DIR_UP end
    if key == "down"  or key == "s" then nextDir = DIR_DOWN end
    if key == "left"  or key == "a" then nextDir = DIR_LEFT end
    if key == "right" or key == "d" then nextDir = DIR_RIGHT end

    if key == "r" and state == "gameover" then
        resetGame()
    end

    if key == "escape" then
        love.event.quit()
    end
end
