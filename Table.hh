#include "GameObj.hh"

class Table : public GameObj
{
public:

  Table(double, double, double  // Initial position
       );

  void draw(glm::mat4);
};
