#include "GameObj.hh"

const double ballRadius = 0.1;
enum BallType {cueball,eightball,striped,solid};

class Ball : public GameObj
{
public:

  // ID: 0 is cue ball, otherwise ID corresponds to ball #
  int id;

  // Type of ball
  BallType type;

  // Whether ball is potted or not
  bool potted;

  // Previous position (before cue hit)
  glm::vec3 prev_position;

  // Previous local axes (before cue hit)
  glm::vec3 prev_frame_x;
  glm::vec3 prev_frame_y;
  glm::vec3 prev_frame_z;

  Ball(int,                     // id
       double, double, double,  // Initial position
       glm::vec3,               // Initial frame_x
       glm::vec3,               // Initial frame_y
       glm::vec3                // Initial frame_z
      );

  void draw(glm::mat4);
};

void save_ball_states();
void reset_ball_states();
bool examine_balls();
