function groundOnTick()
   if player.getPosition()["y"]<-20 then
      print("ups?")
      engine.loadScript("liftExample.lua")
   end
end

function runLift(lift)
   lift.dir=1
   lift.wait=15
end

function stopLift(lift)
   lift.dir=-1
end

function liftOnTick(lift)
   pos=lift:getPosition()
   y=pos.y
   if lift.sound~=nil then
      lift.sound:setPosition(pos.x, pos.y)
   end
   if (lift.dir == 1 and y>16.5) or (lift.dir == -1 and y<4.5) then
      lift:setLinearVelocity(0,0)
      lift.dir=0
      lift.wait=100
      if lift.sound~=nil then
	 lift.sound:stop()
	 lift.sound=nil
      end
   end
   if lift.dir~=0 then
      if lift.dir<0 then
	 if lift.wait<0 then 
	    if lift.sound==nil then
	       lift.sound=liftSound:play(-1)
	    end
	    lift:setLinearVelocity(0, -4) 
	 end
      else
	 if lift.sound==nil then
	    lift.sound=liftSound:play(-1)
	 end
	 lift:setLinearVelocity(0, 3)
      end
      lift.wait=lift.wait-1
   end
end

function onWorldCreate()
   groundShape=physics.createChainShape({5.5,5.5,20.5,5.5,20.5,0.5,0.5,0.5,0.5,25.5,45.5,25.5,45.5,0.5,25.5,0.5,25.5,5.5,44.5,5.5,44.5,15.5,25.5,15.5,25.5,17.5,44.5,17.5,44.5,24.5,1.5,24.5,1.5,5.5,5.5,5.5})
   ground=physics.createBody(physics.body_type_static, 0, 0)
   ground:addFixture(physics.createFixtureDef(groundShape, physics.layer_fg))
   groundEntity=entity.createEntity(ground,"ground")
   groundEntity.onTick=groundOnTick

   liftShape=physics.createPolygonShape({-2,-1, 2, -1, 2, 1, -2, 1})
   liftBody=physics.createBody(physics.body_type_kinetic, 23, 4.5)
   liftBody:addFixture(physics.createFixtureDef(liftShape, physics.layer_fg))
   liftEntity=entity.createEntity(liftBody, "lift")
   liftEntity.onTouch=runLift
   liftEntity.onLeave=stopLift
   liftEntity.onTick=liftOnTick
   liftEntity.dir=0
   liftEntity.wait=0
   liftEntity.sound=nil
   player.setSpawnPoint(6, 10)
   hitsounds={}
   hitsounds[1]=sfx.createSound("sounds/hit1.ogg")
   hitsounds[2]=sfx.createSound("sounds/hit2.ogg")
   hitsounds[3]=sfx.createSound("sounds/hit3.ogg")
   hitsounds[4]=sfx.createSound("sounds/hit4.ogg")
   hitsounds[5]=sfx.createSound("sounds/hit5.ogg")
   liftSound=sfx.createSound("sounds/lift.ogg")
   ropeHitSound=sfx.createSound("sounds/coin.ogg")
end

function onWorldLoaded()
   engine.centerOnPlayer()
end

function player.onHit(e, v)
   soundid=math.random(1,5)
   hitsounds[soundid]:play(0,v/20)
end

function player.onRopeHit()
   ropeHitSound:play(0, 0.2)
end