-- no revolute joint limits now, so it might be funny ;)

function hangManMind(head)
   if (engine.getTime()-head.lastTick)>head.delay then
      txt={"oh help me!", "release me!", "here, button is in the center", "oh god!", "heeeelp!"}
      print(txt[math.random(#txt)])
      head.lastTick=engine.getTime()
      head.delay=math.random(500,2000)
   end
end

function doTheHangman()
   hb=physics.createBody(physics.body_type_dynamic, 3, 16)
   hb:addFixture(physics.createFixtureDef(physics.createCircleShape(0.7), physics.layer_fg))
   head=entity.createEntity(hb, "head")
   head.lastTick=engine.getTime()
   head.delay=math.random(500,2000)
   head.onTick=hangManMind
   ropePos={}
   ropePos.x=3
   ropePos.y=20
   hangManJoint=physics.createDistanceJoint(head, groundEntity, head:getPosition(), ropePos, 10, 0.9, 1)

   bb=physics.createBody(physics.body_type_dynamic, 3, 14.25)
   bb:addFixture(physics.createFixtureDef(physics.createPolygonShape({-0.5,-1,0.5,-1,0.5,1,-0.5,1}), physics.layer_fg))
   body=entity.createEntity(bb, "body")
   
   hanchor=body:getPosition()
   hanchor.x=(hanchor.x+head:getPosition().x)/2
   hanchor.y=(hanchor.y+head:getPosition().y)/2

   physics.createDistanceJoint(head, body, head:getPosition(), hanchor, 10, 0.9, 1)

   legFix=physics.createFixtureDef(physics.createPolygonShape({-0.25,-1.1,0.25,-1.1,0.25,1.1,-0.25,1.1}), physics.layer_fg)
   l1b=physics.createBody(physics.body_type_dynamic, 2.75, 12)
   l1b:addFixture(legFix)
   leg1=entity.createEntity(l1b, "leg1")
   legAnchor=leg1:getPosition()
   legAnchor.y=legAnchor.y+1.1
   physics.createRevoluteJoint(leg1, body, legAnchor)
   l1b=physics.createBody(physics.body_type_dynamic, 3.25, 12)
   l1b:addFixture(legFix)
   leg2=entity.createEntity(l1b, "leg2")
   legAnchor=leg2:getPosition()
   legAnchor.y=legAnchor.y+1.1
   physics.createRevoluteJoint(leg2, body, legAnchor)

   armFix=physics.createFixtureDef(physics.createPolygonShape({-0.25,-0.75,0.25,-0.75,0.25,0.75,-0.25,0.75}), physics.layer_fg)
   a1b=physics.createBody(physics.body_type_dynamic, 2.25, 14.25)
   a1b:addFixture(armFix)
   arm1=entity.createEntity(a1b, "arm1")
   armAnchor=arm1:getPosition()
   armAnchor.y=armAnchor.y+0.75
   physics.createRevoluteJoint(arm1, body, armAnchor, 3, 20, 1)

   a2b=physics.createBody(physics.body_type_dynamic, 3.75, 14.25)
   a2b:addFixture(armFix)
   arm2=entity.createEntity(a2b, "arm2")
   armAnchor=arm2:getPosition()
   armAnchor.y=armAnchor.y+0.75
   physics.createRevoluteJoint(arm2, body, armAnchor, -3, 20, 1)
end



function onWorldCreate()
   groundShape=physics.createChainShape({0,0, 40, 0, 40, 20, 0, 20, 0,0})
   ground=physics.createBody(physics.body_type_static, 0, 0)
   ground:addFixture(physics.createFixtureDef(groundShape, physics.layer_fg))
   groundEntity=entity.createEntity(ground,"ground")
   groundEntity.onTick=groundOnTick
   doTheHangman()

   rf=physics.createFixtureDef(physics.createPolygonShape({-0.5, -0.5, 0.5,-0.5, 0.5, 0.5, -0.5, 0.5}), physics.layer_fg)
   rb=physics.createBody(physics.body_type_kinetic, 20, 2)
   rb:addFixture(rf)
   re=entity.createEntity(rb,"release")
   appSnd=sfx.createSound("sounds/app.ogg")
   re.onTouch=function() hangManJoint:destroy(); head.onTick=nil; print("thanks!!!!"); re:destroy();appSnd:play();crySnd:stop() end
   
   player.setSpawnPoint(6, 10)
   hitsounds={}
   hitsounds[1]=sfx.createSound("sounds/hit1.ogg")
   hitsounds[2]=sfx.createSound("sounds/hit2.ogg")
   hitsounds[3]=sfx.createSound("sounds/hit3.ogg")
   hitsounds[4]=sfx.createSound("sounds/hit4.ogg")
   hitsounds[5]=sfx.createSound("sounds/hit5.ogg")
   crySnd=sfx.createSound("sounds/cry.ogg"):play(-1)
end

function onWorldLoaded()
   engine.centerOnPlayer()
end
