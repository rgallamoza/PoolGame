#ifndef BULLET_UTILS_HH

#define BULLET_UTILS_HH

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <sys/times.h>
#include <sys/time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

using namespace std;
using namespace glm;

//----------------------------------------------------------------------------

void initialize_bullet_simulator();
void delete_bullet_simulator();

void copy_ball_states_to_graphics_objects();
void copy_graphics_objects_to_ball_states();
void update_physics_simulation(float);

void hit_cueball(double);
bool is_balls_stopped();

void save_rigidbody_states();
void reset_rigidbody_states();

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif
