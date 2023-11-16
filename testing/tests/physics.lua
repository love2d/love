-- love.physics


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------OBJECTS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Body (love.physics.newBody)
love.test.physics.Body = function(test)
  test:skipTest('test class needs writing')
end


-- Contact (love.physics.World:getContacts)
love.test.physics.Contact = function(test)
  test:skipTest('test class needs writing')
end


-- Joint (love.physics.newDistanceJoint)
love.test.physics.Joint = function(test)
  -- make joint
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'dynamic')
  local body2 = love.physics.newBody(world, 20, 20, 'dynamic')
  local joint = love.physics.newDistanceJoint(body1, body2, 10, 10, 20, 20, true)
  test:assertObject(joint)
  -- check props
  test:assertEquals('distance', joint:getType(), 'check joint type')
  test:assertEquals(false, joint:isDestroyed(), 'check not destroyed')
  test:assertEquals(0, joint:getReactionForce(1), 'check reaction force')
  test:assertEquals(0, joint:getReactionTorque(1), 'check reaction torque')
  local b1, b2 = joint:getBodies()
  test:assertEquals(body1:getX(), b1:getX(), 'check body 1')
  test:assertEquals(body2:getX(), b2:getX(), 'check body 2')
  local x1, y1, x2, y2 = joint:getAnchors()
  test:assertEquals(10, math.floor(x1), 'check anchor x1')
  test:assertEquals(10, math.floor(y1), 'check anchor y1')
  test:assertEquals(20, math.floor(x2), 'check anchor x2')
  test:assertEquals(20, math.floor(y2), 'check anchor y2')
  test:assertEquals(true, joint:getCollideConnected(), 'check not colliding')
  -- test userdata
  test:assertEquals(nil, joint:getUserData(), 'check no data by def')
  joint:setUserData('hello')
  test:assertEquals('hello', joint:getUserData(), 'check set userdata')
  -- destroy
  joint:destroy()
  test:assertEquals(true, joint:isDestroyed(), 'check destroyed')
end


-- Shape (love.physics.newCircleShape)
-- @NOTE includes Fixture methods too now so enjoy
love.test.physics.Shape = function(test)
  test:skipTest('test class needs writing')
end


-- World (love.physics.newWorld)
love.test.physics.World = function(test)
  -- create new world
  local world = love.physics.newWorld(0, 0, false)
  local body1 = love.physics.newBody(world, 0, 0, 'dynamic')
  local rectangle1 = love.physics.newRectangleShape(body1, 0, 0, 10, 10)
  test:assertObject(world)
  -- check defaults
  test:assertEquals(1, #world:getBodies(), 'check 1 body')
  test:assertEquals(0, world:getBodies()[1]:getX(), 'check body prop x')
  test:assertEquals(0, world:getBodies()[1]:getY(), 'check body prop y')
  world:translateOrigin(-10, -10)
  test:assertEquals(10, math.floor(world:getBodies()[1]:getX()), 'check body prop change x')
  test:assertEquals(10, math.floor(world:getBodies()[1]:getY()), 'check body prop change y')
  test:assertEquals(1, world:getBodyCount(), 'check 1 body count')
  test:assertEquals(false, world:isDestroyed(), 'check not destroyed')
  test:assertEquals(false, world:isLocked(), 'check not updating')
  test:assertEquals(0, #world:getJoints(), 'check no joints')
  test:assertEquals(0, world:getJointCount(), 'check no joints count')
  test:assertEquals(0, world:getGravity(), 'check def gravity')
  test:assertEquals(0, #world:getContacts(), 'check no contacts')
  test:assertEquals(0, world:getContactCount(), 'check no contact count')
  test:assertEquals(false, world:isSleepingAllowed(), 'check no sleep (till brooklyn)')
  world:setSleepingAllowed(true)
  test:assertEquals(true, world:isSleepingAllowed(), 'check can sleep')
  -- check callbacks are called
  local beginContact, endContact, preSolve, postSolve = world:getCallbacks()
  test:assertEquals(nil, beginContact, 'check no begin contact callback')
  test:assertEquals(nil, endContact, 'check no end contact callback')
  test:assertEquals(nil, preSolve, 'check no pre solve callback')
  test:assertEquals(nil, postSolve, 'check no post solve callback')
  local beginContactCheck = false
  local endContactCheck = false
  local preSolveCheck = false
  local postSolveCheck = false
  local collisions = 0
  world:setCallbacks(
    function() beginContactCheck = true; collisions = collisions + 1 end,
    function() endContactCheck = true end,
    function() preSolveCheck = true end,
    function() postSolveCheck = true end
  )
  local body2 = love.physics.newBody(world, 10, 10, 'dynamic')
  local rectangle2 = love.physics.newRectangleShape(body2, 0, 0, 10, 10)
  test:assertEquals(false, beginContactCheck, 'check world didnt update after adding body')
  world:update(1)
  test:assertEquals(true, beginContactCheck, 'check contact start')
  test:assertEquals(true, preSolveCheck, 'check pre solve')
  test:assertEquals(true, postSolveCheck, 'check post solve')
  body2:setPosition(100, 100)
  world:update(1)
  test:assertEquals(true, endContactCheck, 'check contact end')
  -- check point checking
  local shapes = 0
  world:queryShapesInArea(0, 0, 10, 10, function(x)
    shapes = shapes + 1
  end)
  test:assertEquals(1, shapes, 'check shapes in area')
  world:rayCast(0, 0, 200, 200, function(x)
    shapes = shapes + 1
    return 1
  end)
  test:assertEquals(3, shapes, 'check shapes in raycast')
  -- change collision logic
  test:assertEquals(nil, world:getContactFilter(), 'check def filter')
  world:update(1)
  world:setContactFilter(function(s1, s2)
    return false -- nothing collides
  end)
  body2:setPosition(10, 10)
  world:update(1)
  test:assertEquals(1, collisions, 'check collision logic change')
  -- final bits
  world:setGravity(1, 1)
  test:assertEquals(1, world:getGravity(), 'check grav change')
  world:destroy()
  test:assertEquals(true, world:isDestroyed(), 'check world destroyed')
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------------------------------------METHODS-------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- love.physics.getDistance
love.test.physics.getDistance = function(test)
  -- setup two fixtues to check
  local world = love.physics.newWorld(0, 0, false)
  local body = love.physics.newBody(world, 10, 10, 'static')
  local shape1 = love.physics.newEdgeShape(body, 0, 0, 5, 5)
  local shape2 = love.physics.newEdgeShape(body, 10, 10, 15, 15)
  -- check distance between them
  test:assertEquals(647106, math.floor(love.physics.getDistance(shape1, shape2)*100000), 'check distance matches')
end


-- love.physics.getMeter
love.test.physics.getMeter = function(test)
  -- check value set is returned
  love.physics.setMeter(30)
  test:assertEquals(30, love.physics.getMeter(), 'check meter matches')
end


-- love.physics.newBody
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newBody = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  test:assertObject(body)
end


-- love.physics.newChainShape
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newChainShape = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  test:assertObject(love.physics.newChainShape(body, true, 0, 0, 1, 0, 1, 1, 0, 1))
end


-- love.physics.newCircleShape
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newCircleShape = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  test:assertObject(love.physics.newCircleShape(body, 10))
end


-- love.physics.newDistanceJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newDistanceJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newDistanceJoint(body1, body2, 10, 10, 20, 20, true)
  test:assertObject(obj)
end


-- love.physics.newEdgeShape
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newEdgeShape = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  local obj = love.physics.newEdgeShape(body, 0, 0, 10, 10)
  test:assertObject(obj)
end


-- love.physics.newFrictionJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newFrictionJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newFrictionJoint(body1, body2, 15, 15, true)
  test:assertObject(obj)
end


-- love.physics.newGearJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newGearJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'dynamic')
  local body2 = love.physics.newBody(world, 20, 20, 'dynamic')
  local body3 = love.physics.newBody(world, 30, 30, 'dynamic')
  local body4 = love.physics.newBody(world, 40, 40, 'dynamic')
  local joint1 = love.physics.newPrismaticJoint(body1, body2, 10, 10, 20, 20, true)
  local joint2 = love.physics.newPrismaticJoint(body3, body4, 30, 30, 40, 40, true)
  local obj = love.physics.newGearJoint(joint1, joint2, 1, true)
  test:assertObject(obj)
end


-- love.physics.newMotorJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newMotorJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newMotorJoint(body1, body2, 1)
  test:assertObject(obj)
end


-- love.physics.newMouseJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newMouseJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  local obj = love.physics.newMouseJoint(body, 10, 10)
  test:assertObject(obj)
end


-- love.physics.newPolygonShape
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newPolygonShape = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  local obj = love.physics.newPolygonShape(body, {0, 0, 2, 3, 2, 1, 3, 1, 5, 1})
  test:assertObject(obj)
end


-- love.physics.newPrismaticJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newPrismaticJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newPrismaticJoint(body1, body2, 10, 10, 20, 20, true)
  test:assertObject(obj)
end


-- love.physics.newPulleyJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newPulleyJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newPulleyJoint(body1, body2, 10, 10, 20, 20, 15, 15, 25, 25, 1, true)
  test:assertObject(obj)
end


-- love.physics.newRectangleShape
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newRectangleShape = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body = love.physics.newBody(world, 10, 10, 'static')
  local shape1 = love.physics.newRectangleShape(body, 10, 20)
  local shape2 = love.physics.newRectangleShape(body, 10, 10, 40, 30, 10)
  test:assertObject(shape1)
  test:assertObject(shape2)
end


-- love.physics.newRevoluteJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newRevoluteJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newRevoluteJoint(body1, body2, 10, 10, true)
  test:assertObject(obj)
end


-- love.physics.newRopeJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newRopeJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newRopeJoint(body1, body2, 10, 10, 20, 20, 50, true)
  test:assertObject(obj)
end


-- love.physics.newWeldJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newWeldJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newWeldJoint(body1, body2, 10, 10, true)
  test:assertObject(obj)
end


-- love.physics.newWheelJoint
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newWheelJoint = function(test)
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'static')
  local body2 = love.physics.newBody(world, 20, 20, 'static')
  local obj = love.physics.newWheelJoint(body1, body2, 10, 10, 5, 5, true)
  test:assertObject(obj)
end


-- love.physics.newWorld
-- @NOTE this is just basic nil checking, objs have their own test method
love.test.physics.newWorld = function(test)
  local world = love.physics.newWorld(1, 1, true)
  test:assertObject(world)
end


-- love.physics.setMeter
love.test.physics.setMeter = function(test)
  -- set initial meter
  local world = love.physics.newWorld(1, 1, true)
  love.physics.setMeter(30)
  local body = love.physics.newBody(world, 300, 300, "dynamic")
  -- check changing meter changes pos value relatively
  love.physics.setMeter(10)
  local x, y = body:getPosition()
  test:assertEquals(100, x, 'check pos x')
  test:assertEquals(100, y, 'check pos y')
end
