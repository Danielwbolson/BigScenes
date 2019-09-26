--Simple Lua Example
print("Starting Lua for Simple Example")
CameraPosX = 3.0
CameraPosY = 1.7
CameraPosZ = -1.2
CameraDirX = -1.0
CameraDirY = -0.4
CameraDirZ = 0.4

CameraUpX = 0.0
CameraUpY = 1.0
CameraUpZ = 0.0
function keyHandler(keys)
  --Handle key events
end

function frameUpdate(dt)
 --Update the scene 
 rotateModel(table,0.1*dt,0,1,0)
end

table = addModel("Table",0,0,0)
