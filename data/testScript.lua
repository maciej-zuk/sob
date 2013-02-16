function onWorldCreate()
   groundShape=physics.createChainShape({0,0, 15,0, 20,5, 25,5, 40,0, 45,0, 50,5, 45,15, 55,15, 53,10, 55,0, 68,2, 80,0, 92,5, 100,3, 108,12, 108,-10, 0,-10, 0,0})
   topShape=physics.createChainShape({108,12, 107,29, 88,41, 70,38, 58,24, 55,22, 45,21, 42,27, 31,23, 27,14, 9,17, 0,15, 0,0, 0,50, 108,50, 108,12})

   ground=physics.createBody(physics.body_type_static, 0, 0)
   ground:addFixture(physics.createFixtureDef(groundShape, physics.layer_fg+physics.layer_bg))
   groundEntity=entity.createEntity(ground,"ground")

   top=physics.createBody(physics.body_type_static, 0, 0)
   top:addFixture(physics.createFixtureDef(topShape, physics.layer_fg))
   top:addFixture(physics.createFixtureDef(physics.createChainShape({75,15, 95,15}), physics.layer_fg))
   topEntity=entity.createEntity(top,"top")

   krecidelko=physics.createBody(physics.body_type_dynamic, 85, 35)
   krecidelko:addFixture(physics.createFixtureDef(physics.createCircleShape(3),physics.layer_fg))
   krecidelkoEntity=entity.createEntity(krecidelko,"krecidelko")
   krecidelkoJoint=physics.createRevoluteJoint(groundEntity, krecidelkoEntity, krecidelkoEntity:getPosition(), 10);
   krecidelkoEntity.timeout=-1
   function krecidelkoEntity:onTick()
      if self.timeout>0 then
	 self.timeout=self.timeout-1
	 if self.timeout==0 then
	    print("krecidelko umarlo po sekundach: ",(engine.getTime()-self.ts)/1000.0)
	    self:destroy()
	 end
      end
   end
   player.setSpawnPoint(10, 10)
end

function onWorldLoaded()
   engine.centerOnPlayer()
   -- tak na prawde nie ma znaczenia gdzie zdefiniuje te zmienne tylko
   -- centerOnPlayer moze byc wykonany tutaj (no i w callbackach...)
   added=0
   lastTime=engine.getTime()
   dieOnHitGround=true
end

function spam(e1, e2, v)
   newTime=engine.getTime()
   delta=newTime-lastTime
   if added==50 or delta<100 then return end
   lastTime=newTime
   e1.onHit=nil
   for x=1,1 do -- 1,2 - reakcja lawinowa ;)
      added=added+1
      bd=physics.createBody(physics.body_type_dynamic, 85+10*math.sin(2*math.pi*added/10.0), 30)
      bd:addFixture(physics.createFixtureDef(physics.createCircleShape(0.75),physics.layer_fg, 1, 1, 0.5))
      b=entity.createEntity(bd,"b")
      b.onHit=spam
   end
end

function krecidelkoDestination()
   print("krecidelko powinno zniknac po 4*60 onTickach, czyli jakichs 4 sekundach")
   krecidelkoEntity.timeout=4*60 -- must obey your destination! ;)
   krecidelkoEntity.ts=engine.getTime()
end

function player.onRopeHit(ent)
   if ent==krecidelkoEntity then
      dieOnHitGround=false
   end
end


function player.onHit(other, vel)
   if other==krecidelkoEntity then
      print("muczooo circles!")
      krecidelkoJoint:destroy()
      krecidelkoEntity.onHit=krecidelkoDestination
      player.onHit=nil
      bd=physics.createBody(physics.body_type_dynamic, 85, 30)
      bd:addFixture(physics.createFixtureDef(physics.createCircleShape(0.75),physics.layer_fg, 1, 1, 0.5))
      b=entity.createEntity(bd,"b")
      b.onHit=spam
      -- engine.centerOnEntity(b)
   end
end

