#include "GameObj.hh"

// construct a generic GameObj

GameObj::GameObj(double init_x, double init_y, double init_z,
		             glm::vec3 init_frame_x,
                 glm::vec3 init_frame_y,
                 glm::vec3 init_frame_z
                )
{
  position = glm::vec3(init_x, init_y, init_z);

  frame_x = glm::normalize(init_frame_x);
  frame_y = glm::normalize(init_frame_y);
  frame_z = glm::normalize(init_frame_z);

  glGenBuffers(1, &vertexbuffer);
  glGenBuffers(1, &colorbuffer);
}
