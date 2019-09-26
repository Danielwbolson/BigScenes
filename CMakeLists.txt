cmake_minimum_required(VERSION 3.1.3)

# To specify a Win64 build on Windows:
# cmake -G "Visual Studio 15 2017 Win64" ..
# To get an XCode Build:
# cmake -G "Xcode" ..
# To ask for Debug vs. Release
# cmake -DCMAKE_BUILD_TYPE=Debug (or Release)

set(PROJECT_NAME "Game_Engine")
 
project(${PROJECT_NAME})
 
#Set C++11
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
 
find_package(OpenGL REQUIRED)
 
if (UNIX AND NOT APPLE)
 find_package( SDL2 REQUIRED component main)
 set( SDL2_LIBRARIES "SDL2")
 set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )
elseif (APPLE)
  find_package( SDL2 REQUIRED component main)
elseif(MSVC)
  link_directories( E:/SDL2-2.0.10/lib/x64 )
  set( SDL2_INCLUDE_DIRS E:/SDL2-2.0.10/include )
  set( SDL2_LIBRARIES SDL2 SDL2main )
endif()

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
endif()
 
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})

 
file(GLOB srcFiles engineCode/*.cpp external/*.cpp)
#message(STATUS ${srcFiles} )
 
file(GLOB glad_src glad/*.c)
file(GLOB imgui_src imgui/*.cpp)
file(GLOB_RECURSE lua_src Lua/*.c)
list(REMOVE_ITEM lua_src "${CMAKE_SOURCE_DIR}/Lua/src/lua.c")
list(REMOVE_ITEM lua_src "${CMAKE_SOURCE_DIR}/Lua/src/luac.c")
 
add_library(glad ${glad_src})
add_library(imgui ${imgui_src})
add_library(Lua ${lua_src})
 
#message( "DL-libs: " ${CMAKE_DL_LIBS} )
 
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
 
 
add_executable(engine ${srcFiles})
target_link_libraries(engine ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} glad imgui Lua ${CMAKE_DL_LIBS})
 
set_target_properties( engine
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../GEFS"
)
 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
