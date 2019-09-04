#include "GameObj.hh"

class Cue : public GameObj
{
public:

  Cue();

  void draw(glm::mat4);
  void draw_lines(glm::mat4);
  void update();
};
