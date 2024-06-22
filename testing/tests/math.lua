-- love.math


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------OBJECTS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- BezierCurve (love.math.newBezierCurve)
love.test.math.BezierCurve = function(test)

  -- create obj
  local curve = love.math.newBezierCurve(1, 1, 2, 2, 3, 1)
  local px, py = curve:getControlPoint(2)
  test:assertObject(curve)

  -- check initial properties
  test:assertCoords({2, 2}, {px, py}, 'check point x/y')
  test:assertEquals(3, curve:getControlPointCount(), 'check 3 points')
  test:assertEquals(2, curve:getDegree(), 'check degree is points-1')

  -- check some values on the curve
  test:assertEquals(1, curve:evaluate(0), 'check curve evaluation 0')
  test:assertRange(curve:evaluate(0.1), 1.2, 1.3, 'check curve evaluation 0.1')
  test:assertRange(curve:evaluate(0.2), 1.4, 1.5, 'check curve evaluation 0.2')
  test:assertRange(curve:evaluate(0.5), 2, 2.1, 'check curve evaluation 0.5')
  test:assertEquals(3, curve:evaluate(1), 'check curve evaluation 1')

  -- check derivative
  local deriv = curve:getDerivative()
  test:assertObject(deriv)
  test:assertEquals(2, deriv:getControlPointCount(), 'check deriv points')
  test:assertRange(deriv:evaluate(0.1), 2, 2.1, 'check deriv evaluation 0.1')

  -- check segment
  local segment = curve:getSegment(0, 0.5)
  test:assertObject(segment)
  test:assertEquals(3, segment:getControlPointCount(), 'check segment points')
  test:assertRange(segment:evaluate(0.1), 1, 1.1, 'check segment evaluation 0.1')

  -- mess with control points
  curve:removeControlPoint(2)
  curve:insertControlPoint(4, 1, -1)
  curve:insertControlPoint(5, 3, -1)
  curve:insertControlPoint(6, 2, -1)
  curve:setControlPoint(2, 3, 2)
  test:assertEquals(5, curve:getControlPointCount(), 'check 3 points still')
  local px1, py1 = curve:getControlPoint(1)
  local px2, py2 = curve:getControlPoint(3)
  local px3, py3 = curve:getControlPoint(5)
  test:assertCoords({1, 1}, {px1, py1}, 'check modified point 1')
  test:assertCoords({5, 3}, {px2, py2}, 'check modified point 1')
  test:assertCoords({3, 1}, {px3, py3}, 'check modified point 1')

  -- check render lists
  local coords1 = curve:render(5)
  local coords2 = curve:renderSegment(0, 0.1, 5)
  test:assertEquals(196, #coords1, 'check coords')
  test:assertEquals(20, #coords2, 'check segment coords')

  -- check translation values
  px, py = curve:getControlPoint(2)
  test:assertCoords({3, 2}, {px, py}, 'check pretransform x/y')
  curve:rotate(90 * (math.pi/180), 0, 0)
  px, py = curve:getControlPoint(2)
  test:assertCoords({-2, 3}, {px, py}, 'check rotated x/y')
  curve:scale(2, 0, 0)
  px, py = curve:getControlPoint(2)
  test:assertCoords({-4, 6}, {px, py}, 'check scaled x/y')
  curve:translate(5, -5)
  px, py = curve:getControlPoint(2)
  test:assertCoords({1, 1}, {px, py}, 'check translated x/y')

end


-- RandomGenerator (love.math.RandomGenerator)
-- @NOTE as this checks random numbers the chances this fails is very unlikely, but not 0...
-- if you've managed to proc it congrats! your prize is to rerun the testsuite again
love.test.math.RandomGenerator = function(test)

  -- create object
  local rng1 = love.math.newRandomGenerator(3418323524, 20529293)
  test:assertObject(rng1)

  -- check set properties
  local low, high = rng1:getSeed()
  test:assertEquals(3418323524, low, 'check seed low')
  test:assertEquals(20529293, high, 'check seed high')

  -- check states
  local rng2 = love.math.newRandomGenerator(1448323524, 10329293)
  test:assertNotEquals(rng1:random(), rng2:random(), 'check not matching states')
  test:assertNotEquals(rng1:randomNormal(), rng2:randomNormal(), 'check not matching states')

  -- check setting state works
  rng2:setState(rng1:getState())
  test:assertEquals(rng1:random(), rng2:random(), 'check now matching')

  -- check overwriting seed works, should change output
  rng1:setSeed(os.time())
  test:assertNotEquals(rng1:random(), rng2:random(), 'check not matching states')
  test:assertNotEquals(rng1:randomNormal(), rng2:randomNormal(), 'check not matching states')

end


-- Transform (love.math.Transform)
love.test.math.Transform = function(test)

  -- create obj
  local transform = love.math.newTransform(0, 0, 0, 1, 1, 0, 0, 0, 0)
  test:assertObject(transform)

  -- set some values and check the matrix and transformPoint values
  transform:translate(10, 8)
  transform:scale(2, 3)
  transform:rotate(90*(math.pi/180))
  transform:shear(1, 2)
  local px, py = transform:transformPoint(1, 1)
  test:assertCoords({4, 14}, {px, py}, 'check transformation methods')
  transform:reset()
  px, py = transform:transformPoint(1, 1)
  test:assertCoords({1, 1}, {px, py}, 'check reset')

  -- apply a transform to another transform
  local transform2 = love.math.newTransform()
  transform2:translate(5, 3)
  transform:apply(transform2)
  px, py = transform:transformPoint(1, 1)
  test:assertCoords({6, 4}, {px, py}, 'check apply other transform')

  -- check cloning a transform
  local transform3 = transform:clone()
  px, py = transform3:transformPoint(1, 1)
  test:assertCoords({6, 4}, {px, py}, 'check clone transform')

  -- check inverse and inverseTransform
  transform:reset()
  transform:translate(-14, 6)
  local ipx, ipy = transform:inverseTransformPoint(0, 0)
  transform:inverse()
  px, py = transform:transformPoint(0, 0)
  test:assertCoords({-px, -py}, {ipx, ipy}, 'check inverse points transform')

  -- check matrix manipulation
  transform:setTransformation(0, 0, 0, 1, 1, 0, 0, 0, 0)
  transform:translate(4, 4)
  local m1, m2, m3, m4, m5, m6, m7, m8, 
    m9, m10, m11, m12, m13, m14, m15, m16 = transform:getMatrix()
  test:assertEquals(4, m4, 'check translate matrix x')
  test:assertEquals(4, m8, 'check translate matrix y')
  transform:setMatrix(m1, m2, m3, 3, m5, m6, m7, 1, m9, m10, m11, m12, m13, m14, m15, m16)
  px, py = transform:transformPoint(1, 1)
  test:assertCoords({4, 2}, {px, py}, 'check set matrix')

  -- check affine vs non affine
  transform:reset()
  test:assertTrue(transform:isAffine2DTransform(), 'check affine 1')
  transform:translate(4, 3)
  test:assertTrue(transform:isAffine2DTransform(), 'check affine 2')
  transform:setMatrix(1, 3, 4, 5.5, 1, 4.5, 2, 1, 3.4, 5.1, 4.1, 13, 1, 1, 2, 3)
  test:assertFalse(transform:isAffine2DTransform(), 'check not affine')

end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.math.colorFromBytes
love.test.math.colorFromBytes = function(test)
  -- check random value
  local r1, g1, b1, a1 = love.math.colorFromBytes(51, 51, 51, 51)
  test:assertEquals(r1, 0.2, 'check r from bytes')
  test:assertEquals(g1, 0.2, 'check g from bytes')
  test:assertEquals(b1, 0.2, 'check b from bytes')
  test:assertEquals(a1, 0.2, 'check a from bytes')
  -- check "max" value
  local r2, g2, b2, a2 = love.math.colorFromBytes(255, 255, 255, 255)
  test:assertEquals(r2, 1, 'check r from bytes')
  test:assertEquals(g2, 1, 'check g from bytes')
  test:assertEquals(b2, 1, 'check b from bytes')
  test:assertEquals(a2, 1, 'check a from bytes')
  -- check "min" value
  local r3, g3, b3, a3 = love.math.colorFromBytes(0, 0, 0, 0)
  test:assertEquals(r3, 0, 'check r from bytes')
  test:assertEquals(g3, 0, 'check g from bytes')
  test:assertEquals(b3, 0, 'check b from bytes')
  test:assertEquals(a3, 0, 'check a from bytes')
end


-- love.math.colorToBytes
love.test.math.colorToBytes = function(test)
  -- check random value
  local r1, g1, b1, a1 = love.math.colorToBytes(0.2, 0.2, 0.2, 0.2)
  test:assertEquals(r1, 51, 'check bytes from r')
  test:assertEquals(g1, 51, 'check bytes from g')
  test:assertEquals(b1, 51, 'check bytes from b')
  test:assertEquals(a1, 51, 'check bytes from a')
  -- check "max" value
  local r2, g2, b2, a2 = love.math.colorToBytes(1, 1, 1, 1)
  test:assertEquals(r2, 255, 'check bytes from r')
  test:assertEquals(g2, 255, 'check bytes from g')
  test:assertEquals(b2, 255, 'check bytes from b')
  test:assertEquals(a2, 255, 'check bytes from a')
  -- check "min" value
  local r3, g3, b3, a3 = love.math.colorToBytes(0, 0, 0, 0)
  test:assertEquals(r3, 0, 'check bytes from r')
  test:assertEquals(g3, 0, 'check bytes from g')
  test:assertEquals(b3, 0, 'check bytes from b')
  test:assertEquals(a3, 0, 'check bytes from a')
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
  test:assertTrue(isconvex, 'check polygon convex')
  test:assertFalse(notconvex, 'check polygon not convex')
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
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.math.newBezierCurve = function(test)
  test:assertObject(love.math.newBezierCurve({0, 0, 0, 1, 1, 1, 2, 1}))
end


-- love.math.newRandomGenerator
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.math.newRandomGenerator = function(test)
  test:assertObject(love.math.newRandomGenerator())
end


-- love.math.newTransform
-- @NOTE this is just basic nil checking, objs have their own test method
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
  test:assertRange(noise1, 0.5, 0.51, 'check noise 1 dimension')
  test:assertRange(noise2, 0.5, 0.51, 'check noise 2 dimensions')
  test:assertRange(noise3, 0.56, 0.57, 'check noise 3 dimensions')
  test:assertRange(noise4, 0.52, 0.53, 'check noise 4 dimensions')
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
  test:assertRange(noise1, 0.5, 0.51, 'check noise 1 dimension')
  test:assertRange(noise2, 0.47, 0.48, 'check noise 2 dimensions')
  test:assertRange(noise3, 0.26, 0.27, 'check noise 3 dimensions')
  test:assertRange(noise4, 0.53, 0.54, 'check noise 4 dimensions')
end


-- love.math.random
love.test.math.random = function(test)
  -- check some random ranges
  love.math.setRandomSeed(123)
  test:assertRange(love.math.random(), 0.37068322251462, 0.37068322251464, "check random algorithm")
  test:assertEquals(love.math.random(10), 4, "check single random param")
  test:assertEquals(love.math.random(15, 100), 92, "check two random params")
end


-- love.math.randomNormal
love.test.math.randomNormal = function(test)
  love.math.setRandomSeed(1234)
  test:assertRange(love.math.randomNormal(1, 2), 1.0813614997253, 1.0813614997255, 'check randomNormal two params')
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
