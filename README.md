# PoolGame
Simple Pool/8-Ball-ish Game meant to show a working knowledge of OpenGL for my graduate-level Computer Graphics course. Uses GLFW and GLM libraries. CMake rules are provided below (assumes files are placed in a directory named 'PoolGame').
Pool Table, Balls, and Cue were modeled in Blender. Integrates Bullet physics library to create a collision mesh for Table and Balls, and simulate game physics.

```
*** CMAKE RULES ***
# PoolGame
add_executable(PoolGame
	PoolGame/main.cpp
  	PoolGame/GameObj.cpp
	PoolGame/Ball.cpp
	PoolGame/Table.cpp
	PoolGame/Cue.cpp
	PoolGame/Bullet_Utils.cpp
	common/shader.cpp
	common/shader.hpp
	common/controls.cpp
	common/controls.hpp
	common/texture.cpp
	common/texture.hpp
	common/objloader.cpp
	common/objloader.hpp
	common/vboindexer.cpp
	common/vboindexer.hpp

	PoolGame/shaders/BallShading.vertexshader
	PoolGame/shaders/BallShading.fragmentshader
  	PoolGame/shaders/TableShading.vertexshader
	PoolGame/shaders/TableShading.fragmentshader
  	PoolGame/shaders/CueShading.vertexshader
	PoolGame/shaders/CueShading.fragmentshader
)
target_link_libraries(PoolGame
	${ALL_LIBS}
	ANTTWEAKBAR_116_OGLCORE_GLFW
        BulletDynamics
        BulletCollision
        LinearMath
)
# Xcode and Visual working directories
set_target_properties(PoolGame PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PoolGame/")
create_target_launcher(PoolGame WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/PoolGame/")

add_custom_command(
   TARGET PoolGame POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/PoolGame${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/PoolGame/"
)
```
