--Daniel
print("Starting Lua for Daniel")

CameraPosX = 15.0
CameraPosY = 1.0
CameraPosZ = 5.0

CameraDirX = 0.0
CameraDirY = 0.0
CameraDirZ = -1.0

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
	CameraPosX = CameraPosX - 0.1;
  end
  if keys.right then
	CameraPosX = CameraPosX + 0.1;
  end
  if keys.up then
	CameraPosZ = CameraPosZ - 0.1;
  end
  if keys.down then
	CameraPosZ = CameraPosZ + 0.1;
  end
end

for i=0,100 do
	for j=0,100 do
		teapotID = addModel("Teapot", i, 0, -j)
		setModelMaterial(teapotID,"Shiny Red Plastic")
	end
end

