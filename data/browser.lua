function onWorldCreate()

   str="hangmanExample.lua  liftExample.lua  testScript2.lua  testScript.lua"
   n=5
   for word in str:gmatch("%S+") do
      shape=physics.createChainShape({n, 8, n+5, 8, n+5, 13, n, 13, n, 8})
      box=physics.createBody(physics.body_type_static, 0, 0)
      box:addFixture(physics.createFixtureDef(shape, physics.layer_fg))
      entity.createEntity(box,word)
      n=n+10
   end

   groundShape=physics.createChainShape({0, 100, 0, 0, n, 0, n, 100, 0, 100})
   ground=physics.createBody(physics.body_type_static, 0, 0)
   ground:addFixture(physics.createFixtureDef(groundShape, physics.layer_fg))
   groundEntity=entity.createEntity(ground,"browser.lua")

   player.setSpawnPoint(2, 2)
end

function onWorldLoaded()
   engine.centerOnPlayer()
end

function player.onRopeHit(e)
   print("Loading", e:getName())
   engine.loadScript(e:getName())
end

function player.onHit(e)
   if e~=groundEntity then
     print("Demo name:", e:getName())
   end
end
