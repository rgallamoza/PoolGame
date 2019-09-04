#ifndef GAMEOBJ_HH

#define GAMEOBJ_HH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <deque>
#include <string>

#include <sys/time.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

using namespace std;

class GameObj
{
public:
  // current position
  glm::vec3 position;

  // local axes
  glm::vec3 frame_x;
  glm::vec3 frame_y;
  glm::vec3 frame_z;

  GLuint vertexbuffer;
  GLuint colorbuffer;

  GameObj(double, double, double, // initial position
          glm::vec3,              // initial frame_x
          glm::vec3,              // initial frame_y
          glm::vec3               // initial frame_z
         );

  void draw(glm::mat4);
  void update();
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif
