-- love.physics


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
----------------------------------OBJECTS---------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


-- Body (love.physics.newBody)
love.test.physics.Body = function(test)

  -- create body
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 0, 0, 'static')
  local body2 = love.physics.newBody(world, 30, 30, 'dynamic')
  love.physics.newRectangleShape(body1, 5, 5, 10, 10)
  love.physics.newRectangleShape(body2, 5, 5, 10, 10)
  test:assertObject(body1)

  -- check shapes
  test:assertEquals(1, #body1:getShapes(), 'check shapes total 1')
  test:assertEquals(1, #body2:getShapes(), 'check shapes total 2')
  test:assertNotEquals(nil, body1:getShape(), 'check shape 1')
  test:assertNotEquals(nil, body2:getShape(), 'check shape 2')

  -- check body active
  test:assertTrue(body1:isActive(), 'check active by def')

  -- check body bullet
  test:assertFalse(body1:isBullet(), 'check not bullet by def')
  body1:setBullet(true)
  test:assertTrue(body1:isBullet(), 'check set bullet')

  -- check fixed rotation
  test:assertFalse(body1:isFixedRotation(), 'check fix rot def')
  body1:setFixedRotation(true)
  test:assertTrue(body1:isFixedRotation(), 'check set fix rot')

  -- check sleeping/waking
  test:assertTrue(body1:isSleepingAllowed(), 'check sleep def')
  body1:setSleepingAllowed(false)
  test:assertFalse(body1:isSleepingAllowed(), 'check set sleep')
  body1:setSleepingAllowed(true)
  world:update(1)
  test:assertFalse(body1:isAwake(), 'check fell asleep')
  body1:setSleepingAllowed(false)
  body1:setType('dynamic')
  test:assertTrue(body1:isAwake(), 'check waking up')

  -- check touching
  test:assertFalse(body1:isTouching(body2))
  body2:setPosition(5, 5)
  world:update(1)
  test:assertTrue(body1:isTouching(body2))

  -- check children lists
  test:assertEquals(1, #body1:getContacts(), 'check contact list')
  test:assertEquals(0, #body1:getJoints(), 'check joints list')
  love.physics.newDistanceJoint(body1, body2, 5, 5, 10, 10, true)
  test:assertEquals(1, #body1:getJoints(), 'check joints list')

  -- check local points
  local x, y = body1:getLocalCenter()
  test:assertRange(x, 5, 6, 'check local center x')
  test:assertRange(y, 5, 6, 'check local center y')
  local lx, ly = body1:getLocalPoint(10, 10)
  test:assertRange(lx, 10, 11, 'check local point x')
  test:assertRange(ly, 9, 10, 'check local point y')
  local lx1, ly1, lx2, ly2 = body1:getLocalPoints(0, 5, 5, 10)
  test:assertRange(lx1, 0, 1, 'check local points x 1')
  test:assertRange(ly1, 3, 4, 'check local points y 1')
  test:assertRange(lx2, 5, 6, 'check local points x 2')
  test:assertRange(ly2, 9, 10, 'check local points y 2')

  -- check world points
  local wx, wy = body1:getWorldPoint(10.4, 9)
  test:assertRange(wx, 10, 11, 'check world point x')
  test:assertRange(wy, 10, 11, 'check world point y')
  local wx1, wy1, wx2, wy2 = body1:getWorldPoints(0.4, 4, 5.4, 9)
  test:assertRange(wx1, 0, 1, 'check world points x 1')
  test:assertRange(wy1, 5, 6, 'check world points y 1')
  test:assertRange(wx2, 5, 6, 'check world points x 2')
  test:assertRange(wy2, 10, 11, 'check world points y 2')

  -- check angular damping + velocity
  test:assertEquals(0, body1:getAngularDamping(), 'check angular damping')
  test:assertEquals(0, body1:getAngularVelocity(), 'check angular velocity')

  -- check world props
  test:assertObject(body1:getWorld())
  test:assertEquals(2, body1:getWorld():getBodyCount(), 'check world count')
  local cx, cy = body1:getWorldCenter()
  test:assertRange(cx, 4, 5, 'check world center x')
  test:assertRange(cy, 6, 7, 'check world center y')
  local vx, vy = body1:getWorldVector(5, 10)
  test:assertEquals(5, vx, 'check vector x')
  test:assertEquals(10, vy, 'check vector y')

  -- check inertia
  test:assertRange(body1:getInertia(), 5, 6, 'check inertia')

  -- check angle
  test:assertEquals(0, body1:getAngle(), 'check def angle')
  body1:setAngle(90 * (math.pi/180))
  test:assertEquals(math.floor(math.pi/2*100), math.floor(body1:getAngle()*100), 'check set angle')

  -- check gravity scale
  test:assertEquals(1, body1:getGravityScale(), 'check def grav')
  body1:setGravityScale(2)
  test:assertEquals(2, body1:getGravityScale(), 'check change grav')

  -- check damping
  test:assertEquals(0, body1:getLinearDamping(), 'check def lin damping')
  body1:setLinearDamping(0.1)
  test:assertRange(body1:getLinearDamping(), 0, 0.2, 'check change lin damping')

  -- check velocity
  local x2, y2 = body1:getLinearVelocity()
  test:assertEquals(1, x2, 'check def lin velocity x')
  test:assertEquals(1, y2, 'check def lin velocity y')
  body1:setLinearVelocity(4, 5)
  local x3, y3 = body1:getLinearVelocity()
  test:assertEquals(4, x3, 'check change lin velocity x')
  test:assertEquals(5, y3, 'check change lin velocity y')

  -- check mass 
  test:assertRange(body1:getMass(), 0.1, 0.2, 'check def mass')
  body1:setMass(10)
  test:assertEquals(10, body1:getMass(), 'check change mass')
  body1:setMassData(3, 5, 10, 1)
  local x4, y4, mass4, inertia4 = body1:getMassData()
  test:assertEquals(3, x4, 'check mass data change x')
  test:assertEquals(5, y4, 'check mass data change y')
  test:assertEquals(10, mass4, 'check mass data change mass')
  test:assertRange(inertia4, 340, 341, 'check mass data change inertia')
  body1:resetMassData()
  local x5, y5, mass5, inertia5 = body1:getMassData()
  test:assertRange(x5, 5, 6, 'check mass data reset x')
  test:assertRange(y5, 5, 6, 'check mass data reset y')
  test:assertRange(mass5, 0.1, 0.2, 'check mass data reset mass')
  test:assertRange(inertia5, 5, 6, 'check mass data reset inertia')
  test:assertFalse(body1:hasCustomMassData())

  -- check position
  local x6, y6 = body1:getPosition()
  test:assertRange(x6, -1, 0, 'check position x')
  test:assertRange(y6, 0, 1, 'check position y')
  body1:setPosition(10, 4)
  local x7, y7 = body1:getPosition()
  test:assertEquals(x7, 10, 'check set position x')
  test:assertEquals(y7, 4, 'check set position y')

  -- check type
  test:assertEquals('dynamic', body1:getType(), 'check type match')
  body1:setType('kinematic')
  body1:setType('static')
  test:assertEquals('static', body1:getType(), 'check type change')

  -- check userdata
  test:assertEquals(nil, body1:getUserData(), 'check user data')
  body1:setUserData({ love = 'cool' })
  test:assertEquals('cool', body1:getUserData().love, 'check set user data')

  -- check x/y direct
  test:assertEquals(10, math.floor(body1:getX()), 'check get x')
  test:assertEquals(4, math.floor(body1:getY()), 'check get y')
  body1:setX(0)
  body1:setY(0)
  test:assertEquals(0, body1:getX(), 'check get x')
  test:assertEquals(0, body1:getY(), 'check get y')

  -- apply angular impulse
  local vel = body2:getAngularVelocity()
  test:assertRange(vel, 0, 0, 'check velocity before')
  body2:applyAngularImpulse(10)
  local vel1 = body2:getAngularVelocity()
  test:assertRange(vel1, 5, 6, 'check velocity after 1')

  -- apply standard force
  local ang1 = body2:getAngle()
  test:assertRange(ang1, 0.1, 0.2, 'check initial angle 1')
  body2:applyForce(2, 3)
  world:update(2)
  local vel2 = body2:getAngularVelocity()
  local ang2 = body2:getAngle()
  test:assertRange(ang2, -0.1, 0, 'check angle after 2')
  test:assertRange(vel2, 1, 2, 'check velocity after 2')

  -- apply linear impulse
  body2:applyLinearImpulse(-4, -59)
  world:update(1)
  local ang3 = body2:getAngle()
  local vel3 = body2:getAngularVelocity()
  test:assertRange(ang3, -2, -1, 'check angle after 3')
  test:assertRange(vel3, 0, 1, 'check velocity after 3')

  -- apply torque
  body2:applyTorque(4)
  world:update(2)
  local ang4 = body2:getAngle()
  local vel4 = body2:getAngularVelocity()
  test:assertRange(ang4, -1, 0, 'check angle after 4')
  test:assertRange(vel4, 0, 1, 'check velocity after 4')

  -- check destroy
  test:assertFalse(body1:isDestroyed(), 'check not destroyed')
  body1:destroy()
  test:assertTrue(body1:isDestroyed(), 'check destroyed')

end


-- Contact (love.physics.World:getContacts)
love.test.physics.Contact = function(test)

  -- create a setup so we can access some contact objects
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 0, 0, 'dynamic')
  local body2 = love.physics.newBody(world, 10, 10, 'dynamic')
  local rectangle1 = love.physics.newRectangleShape(body1, 0, 0, 10, 10)
  local rectangle2 = love.physics.newRectangleShape(body2, 0, 0, 10, 10)
  rectangle1:setUserData('rec1')
  rectangle2:setUserData('rec2')

  -- used to check for collisions + no. of collisions
  local collided = false
  local pass = 1

  -- set callback for contact start 
  world:setCallbacks(
    function(shape_a, shape_b, contact)
      collided = true

      -- check contact object
      test:assertObject(contact)

      -- check child indices
      local indexA, indexB = contact:getChildren()
      test:assertEquals(1, indexA, 'check child indice a')
      test:assertEquals(1, indexB, 'check child indice b')

      -- check shapes match using userdata
      local shapeA, shapeB = contact:getShapes()
      test:assertEquals(shape_a:getUserData(), shapeA:getUserData(), 'check shape a matches')
      test:assertEquals(shape_b:getUserData(), shapeB:getUserData(), 'check shape b matches')

      -- check normal pos
      local nx, ny = contact:getNormal()
      test:assertEquals(1, nx, 'check normal x')
      test:assertEquals(0, ny, 'check normal y')

      -- check actual pos
      local px1, py1, px2, py2 = contact:getPositions()
      test:assertRange(px1, 5, 6, 'check collide x 1')
      test:assertRange(py1, 5, 6, 'check collide y 1')
      test:assertRange(px2, 5, 6, 'check collide x 2')
      test:assertRange(py2, 5, 6, 'check collide y 2')

      -- check touching
      test:assertTrue(contact:isTouching(), 'check touching')

      -- check enabled (we pass through twice to test on/off)
      test:assertEquals(pass == 1, contact:isEnabled(), 'check enabled for pass ' .. tostring(pass))

      -- check friction
      test:assertRange(contact:getFriction(), 0.2, 0.3, 'check def friction')
      contact:setFriction(0.1)
      test:assertRange(contact:getFriction(), 0.1, 0.2, 'check set friction')
      contact:resetFriction()
      test:assertRange(contact:getFriction(), 0.2, 0.3, 'check reset friction')

      -- check restitution
      test:assertEquals(0, contact:getRestitution(), 'check def restitution')
      contact:setRestitution(1)
      test:assertEquals(1, contact:getRestitution(), 'check set restitution')
      contact:resetRestitution()
      test:assertEquals(0, contact:getRestitution(), 'check reset restitution')
      pass = pass + 1

    end, function() end, function(shape_a, shape_b, contact) 
      if pass > 2 then
        contact:setEnabled(false)
      end
    end, function() end
  )

  -- check bodies collided
  world:update(1)
  test:assertTrue(collided, 'check bodies collided')

  -- update again for enabled check
  world:update(1)
  test:assertEquals(2, pass, 'check ran twice')

end


-- Joint (love.physics.newDistanceJoint)
love.test.physics.Joint = function(test)

  -- make joint
  local world = love.physics.newWorld(1, 1, true)
  local body1 = love.physics.newBody(world, 10, 10, 'dynamic')
  local body2 = love.physics.newBody(world, 20, 20, 'dynamic')
  local joint = love.physics.newDistanceJoint(body1, body2, 10, 10, 20, 20, true)
  test:assertObject(joint)

  -- check type
  test:assertEquals('distance', joint:getType(), 'check joint type')

  -- check not destroyed
  test:assertFalse(joint:isDestroyed(), 'check not destroyed')


  -- check reaction props
  world:update(1)
  local rx1, ry1 = joint:getReactionForce(1)
  test:assertEquals(0, rx1, 'check reaction force x')
  test:assertEquals(0, ry1, 'check reaction force y')
  local rx2, ry2 = joint:getReactionTorque(1)
  test:assertEquals(0, rx2, 'check reaction torque x')
  test:assertEquals(nil, ry2, 'check reaction torque y')

  -- check body pointer
  local b1, b2 = joint:getBodies()
  test:assertEquals(body1:getX(), b1:getX(), 'check body 1')
  test:assertEquals(body2:getX(), b2:getX(), 'check body 2')

  -- check joint anchors
  local x1, y1, x2, y2 = joint:getAnchors()
  test:assertRange(x1, 10, 11, 'check anchor x1')
  test:assertRange(y1, 10, 11, 'check anchor y1')
  test:assertRange(x2, 20, 21, 'check anchor x2')
  test:assertRange(y2, 20, 21, 'check anchor y2')
  test:assertTrue(joint:getCollideConnected(), 'check not colliding')

  -- test userdata
  test:assertEquals(nil, joint:getUserData(), 'check no data by def')
  joint:setUserData('hello')
  test:assertEquals('hello', joint:getUserData(), 'check set userdata')

  -- destroy
  joint:destroy()
  test:assertTrue(joint:isDestroyed(), 'check destroyed')

end


-- Shape (love.physics.newCircleShape)
-- @NOTE in 12.0 fixtures have been merged into shapes
love.test.physics.Shape = function(test)

  -- create shape
  local world = love.physics.newWorld(0, 0, false)
  local body1 = love.physics.newBody(world, 0, 0, 'dynamic')
  local shape1 = love.physics.newRectangleShape(body1, 5, 5, 10, 10)
  test:assertObject(shape1)

  -- check child count
  test:assertEquals(1, shape1:getChildCount(), 'check child count')

  -- check radius
  test:assertRange(shape1:getRadius(), 0, 0.4, 'check radius')

  -- check type match
  test:assertEquals('polygon', shape1:getType(), 'check rectangle type')

  -- check body pointer
  test:assertEquals(0, shape1:getBody():getX(), 'check body link')

  -- check category 
  test:assertEquals(1, shape1:getCategory(), 'check def category')
  shape1:setCategory(3, 5, 6)
  local categories = {shape1:getCategory()}
  test:assertEquals(14, categories[1] + categories[2] + categories[3], 'check set category')

  -- check sensor prop
  test:assertFalse(shape1:isSensor(), 'check sensor def')
  shape1:setSensor(true)
  test:assertTrue(shape1:isSensor(), 'check set sensor')
  shape1:setSensor(false)

  -- check not destroyed
  test:assertFalse(shape1:isDestroyed(), 'check not destroyed')

  -- check user data
  test:assertEquals(nil, shape1:getUserData(), 'check no user data')
  shape1:setUserData({ test = 14 })
  test:assertEquals(14, shape1:getUserData().test, 'check user data set')

  -- check bounding box
  -- polygons have an additional skin radius to help with collisions
  -- so this wont be 0, 0, 10, 10 as you'd think but has an additional 0.3 padding
  local topLeftX, topLeftY, bottomRightX, bottomRightY = shape1:computeAABB(0, 0, 0, 1)
  local tlx, tly, brx, bry = shape1:getBoundingBox(1)
  test:assertEquals(topLeftX, tlx, 'check bbox methods match tlx')
  test:assertEquals(topLeftY, tly, 'check bbox methods match tly')
  test:assertEquals(bottomRightX, brx, 'check bbox methods match brx')
  test:assertEquals(bottomRightY, bry, 'check bbox methods match bry')
  test:assertEquals(topLeftX, topLeftY, 'check bbox tl 1')
  test:assertRange(topLeftY, -0.3, -0.2, 'check bbox tl 2')
  test:assertEquals(bottomRightX, bottomRightY, 'check bbox br 1')
  test:assertRange(bottomRightX, 10.3, 10.4, 'check bbox br 2')

  -- check density
  test:assertEquals(1, shape1:getDensity(), 'check def density')
  shape1:setDensity(5)
  test:assertEquals(5, shape1:getDensity(), 'check set density')

  -- check mass
  local x1, y1, mass1, inertia1 = shape1:getMassData()
  test:assertRange(x1, 5, 5.1, 'check shape mass pos x')
  test:assertRange(y1, 5, 5.1, 'check shape mass pos y')
  test:assertRange(mass1, 0.5, 0.6, 'check mass at 1 density')
  test:assertRange(inertia1, 0, 0.1, 'check intertia at 1 density')
  local x2, y2, mass2, inertia2 = shape1:computeMass(1000)
  test:assertRange(mass2, 111, 112, 'check mass at 1000 density')
  test:assertRange(inertia2, 7407, 7408, 'check intertia at 1000 density')

  -- check friction
  test:assertRange(shape1:getFriction(), 0.2, 0.3, 'check def friction')
  shape1:setFriction(1)
  test:assertEquals(1, shape1:getFriction(), 'check set friction')

  -- check restitution
  test:assertEquals(0, shape1:getRestitution(), 'check def restitution')
  shape1:setRestitution(0.5)
  test:assertRange(shape1:getRestitution(), 0.5, 0.6, 'check set restitution')

  -- check points
  local bodyp = love.physics.newBody(world, 0, 0, 'dynamic')
  local shape2 = love.physics.newRectangleShape(bodyp, 5, 5, 10, 10)
  test:assertTrue(shape2:testPoint(5, 5), 'check point 5,5')
  test:assertTrue(shape2:testPoint(10, 10, 0, 15, 15), 'check point 15,15 after translate 10,10')
  test:assertFalse(shape2:testPoint(5, 5, 90, 10, 10), 'check point 10,10 after translate 5,5,90')
  test:assertFalse(shape2:testPoint(10, 10, 90, 5, 5), 'check point 5,5 after translate 10,10,90')
  test:assertFalse(shape2:testPoint(15, 15), 'check point 15,15')

  -- check ray cast
  local xn1, yn1, fraction1 = shape2:rayCast(-20, -20, 20, 20, 100, 0, 0, 0, 1)
  test:assertNotEquals(nil, xn1, 'check ray 1 x')
  test:assertNotEquals(nil, xn1, 'check ray 1 y')
  local xn2, yn2, fraction2 = shape2:rayCast(10, 10, -150, -150, 100, 0, 0, 0, 1)
  test:assertEquals(nil, xn2, 'check ray 2 x')
  test:assertEquals(nil, yn2, 'check ray 2 y')

  -- check filtering
  test:assertEquals(nil, shape2:getMask(), 'check no mask')
  shape2:setMask(1, 2, 3)
  test:assertEquals(3, #{shape2:getMask()}, 'check set mask')
  test:assertEquals(0, shape2:getGroupIndex(), 'check no index')
  shape2:setGroupIndex(-1)
  test:assertEquals(-1, shape2:getGroupIndex(), 'check set index')
  local cat, mask, group = shape2:getFilterData()
  test:assertEquals(1, cat, 'check filter cat')
  test:assertEquals(65528, mask, 'check filter mask')
  test:assertEquals(-1, group, 'check filter group')

  -- check destroyed
  shape1:destroy()
  test:assertTrue(shape1:isDestroyed(), 'check destroyed')
  shape2:destroy()

  -- run some collision checks using filters, setup new shapes 
  local body2 = love.physics.newBody(world, 5, 5, 'dynamic')
  local shape3 = love.physics.newRectangleShape(body1, 0, 0, 10, 10)
  local shape4 = love.physics.newRectangleShape(body2, 0, 0, 10, 10)
  local collisions = 0
  world:setCallbacks(
    function() collisions = collisions + 1 end,
    function() end,
    function() end,
    function() end
  )

  -- same positive group do collide
  shape3:setGroupIndex(1)
  shape4:setGroupIndex(1)
  world:update(1)
  test:assertEquals(1, collisions, 'check positive group collide')

  -- check negative group dont collide
  shape3:setGroupIndex(-1)
  shape4:setGroupIndex(-1)
  body2:setPosition(20, 20); world:update(1); body2:setPosition(0, 0); world:update(1)
  test:assertEquals(1, collisions, 'check negative group collide')

  -- check masks do collide
  shape3:setGroupIndex(0)
  shape4:setGroupIndex(0)
  shape3:setCategory(2)
  shape4:setMask(3)
  body2:setPosition(20, 20); world:update(1); body2:setPosition(0, 0); world:update(1)
  test:assertEquals(2, collisions, 'check mask collide')

  -- check masks not colliding
  shape3:setCategory(2)
  shape4:setMask(2, 4, 6)
  body2:setPosition(20, 20); world:update(1); body2:setPosition(0, 0); world:update(1)
  test:assertEquals(2, collisions, 'check mask not collide')

end


-- World (love.physics.newWorld)
love.test.physics.World = function(test)

  -- create new world
  local world = love.physics.newWorld(0, 0, false)
  local body1 = love.physics.newBody(world, 0, 0, 'dynamic')
  local rectangle1 = love.physics.newRectangleShape(body1, 0, 0, 10, 10)
  test:assertObject(world)

  -- check bodies in world
  test:assertEquals(1, #world:getBodies(), 'check 1 body')
  test:assertEquals(0, world:getBodies()[1]:getX(), 'check body prop x')
  test:assertEquals(0, world:getBodies()[1]:getY(), 'check body prop y')
  world:translateOrigin(-10, -10) -- check affects bodies
  test:assertRange(world:getBodies()[1]:getX(), 9, 11, 'check body prop change x')
  test:assertRange(world:getBodies()[1]:getY(), 9, 11, 'check body prop change y')
  test:assertEquals(1, world:getBodyCount(), 'check 1 body count')

  -- check shapes in world
  test:assertEquals(1, #world:getShapesInArea(0, 0, 10, 10), 'check shapes in area #1')
  test:assertEquals(0, #world:getShapesInArea(20, 20, 30, 30), 'check shapes in area #1')

  -- check world status
  test:assertFalse(world:isLocked(), 'check not updating')
  test:assertFalse(world:isSleepingAllowed(), 'check no sleep (till brooklyn)')
  world:setSleepingAllowed(true)
  test:assertTrue(world:isSleepingAllowed(), 'check can sleep')

  -- check world objects
  test:assertEquals(0, #world:getJoints(), 'check no joints')
  test:assertEquals(0, world:getJointCount(), 'check no joints count')
  test:assertEquals(0, world:getGravity(), 'check def gravity')
  test:assertEquals(0, #world:getContacts(), 'check no contacts')
  test:assertEquals(0, world:getContactCount(), 'check no contact count')

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

  -- setup so we can collide stuff to call the callbacks
  local body2 = love.physics.newBody(world, 10, 10, 'dynamic')
  local rectangle2 = love.physics.newRectangleShape(body2, 0, 0, 10, 10)
  test:assertFalse(beginContactCheck, 'check world didnt update after adding body')
  world:update(1)
  test:assertTrue(beginContactCheck, 'check contact start')
  test:assertTrue(preSolveCheck, 'check pre solve')
  test:assertTrue(postSolveCheck, 'check post solve')
  body2:setPosition(100, 100)
  world:update(1)
  test:assertTrue(endContactCheck, 'check contact end')

  -- check point checking
  local shapes = 0
  world:queryShapesInArea(0, 0, 10, 10, function(x)
    shapes = shapes + 1
  end)
  test:assertEquals(1, shapes, 'check shapes in area')

  -- check raycast
  world:rayCast(0, 0, 200, 200, function(x)
    shapes = shapes + 1
    return 1
  end)
  test:assertEquals(3, shapes, 'check shapes in raycast')
  test:assertEquals(world:rayCastClosest(0, 0, 200, 200), rectangle1, 'check closest raycast')
  test:assertNotEquals(nil, world:rayCastAny(0, 0, 200, 200), 'check any raycast')

  -- change collision logic
  test:assertEquals(nil, world:getContactFilter(), 'check def filter')
  world:update(1)
  world:setContactFilter(function(s1, s2)
    return false -- nothing collides
  end)
  body2:setPosition(10, 10)
  world:update(1)
  test:assertEquals(1, collisions, 'check collision logic change')

  -- check gravity
  world:setGravity(1, 1)
  test:assertEquals(1, world:getGravity(), 'check grav change')

  -- check destruction
  test:assertFalse(world:isDestroyed(), 'check not destroyed')
  world:destroy()
  test:assertTrue(world:isDestroyed(), 'check world destroyed')

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
  test:assertRange(love.physics.getDistance(shape1, shape2), 6, 7, 'check distance matches')
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
