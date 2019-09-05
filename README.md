# PoolGame
Simple Pool/8-Ball-ish Game meant to show a working knowledge of OpenGL for my graduate-level Computer Graphics course. Uses GLFW and GLM libraries. CMake rules are provided below.
Pool Table, Balls, and Cue were modeled in Blender. Integrates Bullet physics library to create a collision mesh for Table and Balls, and simulate game physics.

```
*** CMAKE RULES ***
# pool
add_executable(pool
	pool/main.cpp
  pool/GameObj.cpp
	pool/Ball.cpp
	pool/Table.cpp
	pool/Cue.cpp
	pool/Bullet_Utils.cpp
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

	pool/shaders/BallShading.vertexshader
	pool/shaders/BallShading.fragmentshader
  pool/shaders/TableShading.vertexshader
	pool/shaders/TableShading.fragmentshader
  pool/shaders/CueShading.vertexshader
	pool/shaders/CueShading.fragmentshader
)
target_link_libraries(pool
	${ALL_LIBS}
	ANTTWEAKBAR_116_OGLCORE_GLFW
        BulletDynamics
        BulletCollision
        LinearMath
)
# Xcode and Visual working directories
set_target_properties(pool PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pool/")
create_target_launcher(pool WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pool/")

add_custom_command(
   TARGET pool POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/pool${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/pool/"
)
```
