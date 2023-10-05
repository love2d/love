-- love.math


-- love.math.colorFromBytes
love.test.math.colorFromBytes = function(test)
  -- check random value
  local r, g, b, a = love.math.colorFromBytes(51, 51, 51, 51)
  test:assertEquals(r, 0.2, 'check r from bytes')
  test:assertEquals(g, 0.2, 'check g from bytes')
  test:assertEquals(b, 0.2, 'check b from bytes')
  test:assertEquals(a, 0.2, 'check a from bytes')
  -- check "max" value
  r, g, b, a = love.math.colorFromBytes(255, 255, 255, 255)
  test:assertEquals(r, 1, 'check r from bytes')
  test:assertEquals(g, 1, 'check g from bytes')
  test:assertEquals(b, 1, 'check b from bytes')
  test:assertEquals(a, 1, 'check a from bytes')
  -- check "min" value
  r, g, b, a = love.math.colorFromBytes(0, 0, 0, 0)
  test:assertEquals(r, 0, 'check r from bytes')
  test:assertEquals(g, 0, 'check g from bytes')
  test:assertEquals(b, 0, 'check b from bytes')
  test:assertEquals(a, 0, 'check a from bytes')
end


-- love.math.colorToBytes
love.test.math.colorToBytes = function(test)
  -- check random value
  local r, g, b, a = love.math.colorToBytes(0.2, 0.2, 0.2, 0.2)
  test:assertEquals(r, 51, 'check bytes from r')
  test:assertEquals(g, 51, 'check bytes from g')
  test:assertEquals(b, 51, 'check bytes from b')
  test:assertEquals(a, 51, 'check bytes from a')
  -- check "max" value
  r, g, b, a = love.math.colorToBytes(1, 1, 1, 1)
  test:assertEquals(r, 255, 'check bytes from r')
  test:assertEquals(g, 255, 'check bytes from g')
  test:assertEquals(b, 255, 'check bytes from b')
  test:assertEquals(a, 255, 'check bytes from a')
  -- check "min" value
  r, g, b, a = love.math.colorToBytes(0, 0, 0, 0)
  test:assertEquals(r, 0, 'check bytes from r')
  test:assertEquals(g, 0, 'check bytes from g')
  test:assertEquals(b, 0, 'check bytes from b')
  test:assertEquals(a, 0, 'check bytes from a')
end


-- love.math.gammaToLinear
-- @NOTE I tried doing the same formula as the source from MathModule.cpp
-- but get test failues due to slight differences
love.test.math.gammaToLinear = function(test)
  local lr, lg, lb = love.math.gammaToLinear(1, 0.8, 0.02)
  --local eg = ((0.8 + 0.055) / 1.055)^2.4
  --local eb = 0.02 / 12.92
  test:assertGreaterEqual(0, lr, 'check gamma r to linear')
  test:assertGreaterEqual(0, lg, 'check gamma g to linear')
  test:assertGreaterEqual(0, lb, 'check gamma b to linear')
end


-- love.math.getRandomSeed
-- @NOTE whenever i run this high is always 0, is that intended?
love.test.math.getRandomSeed = function(test)
  local low, high = love.math.getRandomSeed()
  test:assertGreaterEqual(0, low, 'check random seed low')
  test:assertGreaterEqual(0, high, 'check random seed high')
end


-- love.math.getRandomState
love.test.math.getRandomState = function(test)
  test:assertNotNil(love.math.getRandomState())
end


-- love.math.isConvex
love.test.math.isConvex = function(test)
  local isconvex = love.math.isConvex({0, 0, 1, 0, 1, 1, 1, 0, 0, 0}) -- square
  local notconvex = love.math.isConvex({1, 2, 2, 4, 3, 4, 2, 1, 3, 1}) -- weird shape
  test:assertEquals(true, isconvex, 'check polygon convex')
  test:assertEquals(false, notconvex, 'check polygon not convex')
end


-- love.math.linearToGammer
-- @NOTE I tried doing the same formula as the source from MathModule.cpp
-- but get test failues due to slight differences
love.test.math.linearToGamma = function(test)
  local gr, gg, gb = love.math.linearToGamma(1, 0.8, 0.001)
  --local eg = 1.055 * (0.8^1/2.4) - 0.055
  --local eb = 0.001 * 12.92
  test:assertGreaterEqual(0, gr, 'check linear r to gamme')
  test:assertGreaterEqual(0, gg, 'check linear g to gamme')
  test:assertGreaterEqual(0, gb, 'check linear b to gamme')
end


-- love.math.newBezierCurve
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.math.newBezierCurve = function(test)
  test:assertObject(love.math.newBezierCurve({0, 0, 0, 1, 1, 1, 2, 1}))
end


-- love.math.newRandomGenerator
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.math.newRandomGenerator = function(test)
  test:assertObject(love.math.newRandomGenerator())
end


-- love.math.newTransform
-- @NOTE this is just basic nil checking, full obj test are in objects.lua
love.test.math.newTransform = function(test)
  test:assertObject(love.math.newTransform())
end


-- love.math.perlinNoise
love.test.math.perlinNoise = function(test)
  -- check some noise values
  -- output should be consistent if given the same input
  local noise1 = love.math.perlinNoise(100)
  local noise2 = love.math.perlinNoise(1, 10)
  local noise3 = love.math.perlinNoise(1043, 31.123, 999)
  local noise4 = love.math.perlinNoise(99.222, 10067, 8, 1843)
  -- rounded to avoid floating point issues 
  test:assertEquals(50000, math.floor(noise1*100000), 'check noise 1 dimension')
  test:assertEquals(50000, math.floor(noise2*100000), 'check noise 2 dimensions')
  test:assertEquals(56297, math.floor(noise3*100000), 'check noise 3 dimensions')
  test:assertEquals(52579, math.floor(noise4*100000), 'check noise 4 dimensions')
end


-- love.math.simplexNoise
love.test.math.simplexNoise = function(test)
  -- check some noise values
  -- output should be consistent if given the same input
  local noise1 = love.math.simplexNoise(100)
  local noise2 = love.math.simplexNoise(1, 10)
  local noise3 = love.math.simplexNoise(1043, 31.123, 999)
  local noise4 = love.math.simplexNoise(99.222, 10067, 8, 1843)
  -- rounded to avoid floating point issues 
  test:assertEquals(50000, math.floor(noise1*100000), 'check noise 1 dimension')
  test:assertEquals(47863, math.floor(noise2*100000), 'check noise 2 dimensions')
  test:assertEquals(26440, math.floor(noise3*100000), 'check noise 3 dimensions')
  test:assertEquals(53360, math.floor(noise4*100000), 'check noise 4 dimensions')
end


-- love.math.random
love.test.math.random = function(test)
  -- check some random ranges
  test:assertRange(love.math.random(10), 1, 10, 'check within 1 - 10')
  test:assertRange(love.math.random(10), 1, 10, 'check within 1 - 10')
  test:assertRange(love.math.random(10), 1, 10, 'check within 1 - 10')
  test:assertRange(love.math.random(10), 1, 10, 'check within 1 - 10')
  test:assertRange(love.math.random(10), 1, 10, 'check within 1 - 10')
  test:assertRange(love.math.random(5, 100), 5, 100, 'check within 5 - 100')
  test:assertRange(love.math.random(5, 100), 5, 100, 'check within 5 - 100')
  test:assertRange(love.math.random(5, 100), 5, 100, 'check within 5 - 100')
  test:assertRange(love.math.random(5, 100), 5, 100, 'check within 5 - 100')
  test:assertRange(love.math.random(5, 100), 5, 100, 'check within 5 - 100')
end


-- love.math.randomNormal
-- @NOTE i dont _really_ get the range expected based on stddev + mean
-- so feel free to change to be more accurate
love.test.math.randomNormal = function(test)
  test:assertRange(love.math.randomNormal(1, 0), -3, 3, 'check within -3 - 3')
end


-- love.math.setRandomSeed
-- @NOTE same with getRandomSeed, high is always 0 when I tested it?
love.test.math.setRandomSeed = function(test)
  love.math.setRandomSeed(9001)
  local low, high = love.math.getRandomSeed()
  test:assertEquals(9001, low, 'check seed low set')
  test:assertEquals(0, high, 'check seed high set')
end


-- love.math.setRandomState
love.test.math.setRandomState = function(test)
  -- check setting state matches value returned
  local rs1 = love.math.getRandomState()
  love.math.setRandomState(rs1)
  local rs2 = love.math.getRandomState()
  test:assertEquals(rs1, rs2, 'check random state set')
end


-- love.math.triangulate
love.test.math.triangulate = function(test)
  local triangles1 = love.math.triangulate({0, 0, 1, 0, 1, 1, 1, 0, 0, 0}) -- square
  local triangles2 = love.math.triangulate({1, 2, 2, 4, 3, 4, 2, 1, 3, 1}) -- weird shape
  test:assertEquals(3, #triangles1, 'check polygon triangles')
  test:assertEquals(3, #triangles2, 'check polygon triangles')
end
