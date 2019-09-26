--Basic example for model loading, keyboad interaction, a script-based animation
print("Starting Lua script for ModelViewer example")

--Todo:
-- Lua modules (for better organization, and maybe reloading?)
-- Allow pointlights to attach to nodes
-- Allow fill lights (no n-dot-l term) to hack global illum

--These 9 special variables are querried by the engine each
-- frame and used to set the camera pose.
--Here we set the intial camera pose:
CameraPosX = -4.0; CameraPosY = 2; CameraPosZ = -1.8
CameraDirX = 1.0; CameraDirY = -0.2; CameraDirZ = 0.4
CameraUpX = 0.0; CameraUpY = 1.0; CameraUpZ = 0.0

theta = 0; radius = 4 --Helper variabsle for camrea control

animatedModels = {}
velModel = {}
rotYVelModel = {}

--Special function that is called every frame
--The variable dt containts how much times has pased since it was last called
function frameUpdate(dt)

  --Update the camera based on radius and theta
  CameraPosX = radius*math.cos(theta)
  CameraPosZ = radius*math.sin(theta)
  CameraDirX = -CameraPosX
  CameraDirZ = -CameraPosZ

  --A simple animation system
  for modelID,v in pairs(animatedModels) do
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

--Special function that is called each frame. The variable
--keys containts information about which keys are currently .
function keyHandler(keys)

  --Move camera radius and theta based on up/down/left/right keys
  if keys.left then
    theta = theta + .03
  end
  if keys.right then
    theta = theta - .03
  end
  if keys.up then
    radius = radius - .05
  end
  if keys.down then
    radius = radius + .05
  end

  --Tab key cycles through models unhideing them from rendering one by one
  if keys.tab and not tabDownBefore then
    hideModel(model[drawModel])
    if keys.shift then --Shift-tab cycles backwards
      drawModel = (drawModel -1 % #model)
      if drawModel == 0 then drawModel = #model end
    else
      drawModel = (drawModel % #model) + 1
    end
    unhideModel(model[drawModel])
  end

  tabDownBefore = keys.tab
end

--Add base floor model
floor = addModel("Floor",0,.95,0)
setModelMaterial(floor,"Clay")

--Add several predefined models to be rendered
i = 1 --Lau is typically 1-indexed
model = {}
model[i] = addModel("Windmill",0,1,0); i = i+1
model[i] = addModel("Bookcase",0,1,0); i = i+1
model[i] = addModel("Ring",0,1,0); i = i+1
model[i] = addModel("Soccer Ball",0,1,0); i = i+1
model[i] = addModel("Thonet S43 Chair",0,1,0); i = i+1
model[i] = addModel("Silver Knot",0,1,0); i = i+1
model[i] = addModel("Gold Knot",0,1,0); i = i+1
model[i] = addModel("Frog",0,1,0); i = i+1
model[i] = addModel("Copper Pan",0,1,0); i = i+1
model[i] = addModel("Pool Table",0,1,0); i = i+1

--Choose 1 model to be drawn at a time, the rest will be hidden
drawModel = 1
for i = 1,#model do
  if drawModel ~= i then
    hideModel(model[i])
  end
end

--Set the 3rd model to rotate around it's the y-axis
rotYVelModel[model[3]] = 0.2  --radians per second
animatedModels[model[3]] = true