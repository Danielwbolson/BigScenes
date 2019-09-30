--Daniel
print("Starting Lua for Daniel")

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


chaletID = addModel("Chalet", 0, 0, 0)
scaleModel(chaletID, 0.3, 0.3, 0.3)
