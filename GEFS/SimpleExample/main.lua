--Simple Example
print("Starting Lua for Simple Example")

CameraPosX = -3.0
CameraPosY = 1.0
CameraPosZ = 0.0

CameraDirX = 1.0
CameraDirY = -0.0
CameraDirZ = -0.0

CameraUpX = 0.0
CameraUpY = 1.0
CameraUpZ = 0.0

animatedModels = {}
velModel = {}
rotYVelModel = {}

function frameUpdate(dt)
  for modelID,v in pairs(animatedModels) do
    --print("ID",modelID)
    local vel = velModel[modelID]
    if vel then 
      translateModel(modelID,dt*vel[1],dt*vel[2],dt*vel[3])
    end

    local rotYvel = rotYVelModel[modelID]
    if rotYvel then 
      rotateModel(modelID,rotYvel*dt, 0, 1, 0)
    end

  end
end

function keyHandler(keys)
  if keys.left then
    translateModel(dinoID,0,0,-0.1)
  end
  if keys.right then
    translateModel(dinoID,0,0,0.1)
  end
  if keys.up then
    translateModel(dinoID,0.1,0,0)
  end
  if keys.down then
    translateModel(dinoID,-0.1,0,0)
  end
end

teapotID = addModel("Teapot",0,0,0)
setModelMaterial(teapotID,"Shiny Red Plastic")
--setModelMaterial(teapotID,"Steel")
animatedModels[teapotID] = true
rotYVelModel[teapotID] = 1

floorID = addModel("FloorPart",0,0,0)
placeModel(floorID,0,-.02,0)
scaleModel(floorID,3,1,3)
setModelMaterial(floorID,"Gold")

dinoID = addModel("Dino",0,0,-.15)
