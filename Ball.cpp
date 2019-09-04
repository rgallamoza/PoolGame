#include "Ball.hh"

////////////////////////////////////////////////////////////////////////////////

extern glm::mat4 ViewMat;
extern glm::mat4 ProjectionMat;

extern GLuint ballLightID;

extern GLuint ballMatrixID;
extern GLuint ballModelMatrixID;
extern GLuint ballViewMatrixID;
extern vector<GLuint> ballTextureIDs;
extern vector<GLuint> ballTextures;
extern vector<unsigned short> ballIndices;

extern GLuint ballVertexbuffer;
extern GLuint ballUvbuffer;
extern GLuint ballNormalbuffer;
extern GLuint ballElementbuffer;

extern int numBalls;
vector<Ball*> ballArray;

extern bool is_physics_active;

extern float pottedHeightThreshold;
extern BallType scoringType;

extern bool breaking;
extern bool won;

extern int numPotted;
extern int numPottedDuringHit;
extern int score;

//Precalculated initial positions
double ballInitPos[] = {
  0.0, 0.1, 0.0,
  0*-0.1, 0.1, -2.0-0*sqrt(0.03),
  1*-0.1, 0.1, -2.0-1*sqrt(0.03),
  2*-0.1, 0.1, -2.0-2*sqrt(0.03),
  3*-0.1, 0.1, -2.0-3*sqrt(0.03),
  4*-0.1, 0.1, -2.0-4*sqrt(0.03),
  2*-0.1, 0.1, -2.0-4*sqrt(0.03),
  1*-0.1, 0.1, -2.0-3*sqrt(0.03),
  0*-0.1, 0.1, -2.0-2*sqrt(0.03),
  -1*-0.1, 0.1, -2.0-1*sqrt(0.03),
  -2*-0.1, 0.1, -2.0-2*sqrt(0.03),
  -1*-0.1, 0.1, -2.0-3*sqrt(0.03),
  0*-0.1, 0.1, -2.0-4*sqrt(0.03),
  -2*-0.1, 0.1, -2.0-4*sqrt(0.03),
  -3*-0.1, 0.1, -2.0-3*sqrt(0.03),
  -4*-0.1, 0.1, -2.0-4*sqrt(0.03)
};

////////////////////////////////////////////////////////////////////////////////

Ball::Ball(int _id,
           double init_x, double init_y, double init_z,
           glm::vec3 init_frame_x,
           glm::vec3 init_frame_y,
           glm::vec3 init_frame_z
         ) : GameObj(init_x, init_y, init_z, init_frame_x, init_frame_y, init_frame_z)
{
  id = _id;
  potted = false;

  switch(_id)
  {
    case 0:
      type = cueball;
      break;

    case 1:case 2:case 3:case 4:case 5:case 6:case 7:
      type = solid;
      break;

    case 8:
      type = eightball;
      break;

    case 9:case 10:case 11:case 12:case 13:case 14:case 15:
      type = striped;
      break;
  }
}

void Ball::draw(glm::mat4 Model)
{
  // set light position
  glm::vec3 lightPos = glm::vec3(0,6,0);
  glUniform3f(ballLightID, lightPos.x, lightPos.y, lightPos.z);

  glm::mat4 RotationMatrix = glm::mat4(glm::vec4(frame_x, 0),
                                       glm::vec4(frame_y, 0),
                                       glm::vec4(frame_z, 0),
                                       glm::vec4(0, 0, 0, 1));

  glm::mat4 TranslationMatrix = translate(glm::mat4(), glm::vec3(position.x, position.y, position.z));

  glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

  glm::mat4 MVP = ProjectionMat * ViewMat * Model * ModelMatrix;

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(ballMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(ballModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
  glUniformMatrix4fv(ballViewMatrixID, 1, GL_FALSE, &ViewMat[0][0]);

  // Bind this object's texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ballTextures[id]);

  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(ballTextureIDs[id], 0);

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, ballVertexbuffer);
  glVertexAttribPointer(
      0,                  // attribute
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, ballUvbuffer);
  glVertexAttribPointer(
      1,                                // attribute
      2,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

  // 3rd attribute buffer : normals
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, ballNormalbuffer);
  glVertexAttribPointer(
      2,                                // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

  // Index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballElementbuffer);

  // Draw the triangles !
  glDrawElements(
     GL_TRIANGLES,           // mode
     ballIndices.size(),    // count
     GL_UNSIGNED_SHORT,      // type
     (void*)0                // element array buffer offset
     );

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void save_ball_states()
{
  for(int i=0;i<numBalls;i++)
  {
    ballArray[i]->prev_position = ballArray[i]->position;
    ballArray[i]->prev_frame_x = ballArray[i]->frame_x;
    ballArray[i]->prev_frame_y = ballArray[i]->frame_y;
    ballArray[i]->prev_frame_z = ballArray[i]->frame_z;
  }
}

void reset_ball_states()
{
  for(int i=0;i<numBalls;i++)
  {
    ballArray[i]->position = ballArray[i]->prev_position;
    ballArray[i]->frame_x = ballArray[i]->prev_frame_x;
    ballArray[i]->frame_y = ballArray[i]->prev_frame_y;
    ballArray[i]->frame_z = ballArray[i]->prev_frame_z;
    if(ballArray[i]->position.y > pottedHeightThreshold)
      ballArray[i]->potted = false;
  }
}

bool examine_balls()
{
  bool foul = false;

  for(int i=0;i<numBalls && !foul;i++)
  {
    if(!ballArray[i]->potted && ballArray[i]->position.y <= pottedHeightThreshold)
    {
      ballArray[i]->potted = true;

      if(ballArray[i]->type == cueball)
      {
        printf("Potted the Cue Ball!\n");
        foul = true;
      }
      else if(ballArray[i]->type == eightball && numPotted+numPottedDuringHit < 7)
      {
        printf("Potted Eight Ball too early!\n");
        foul = true;
      }
      else if(ballArray[i]->type == eightball && numPotted+numPottedDuringHit == 7)
      {
        printf("Potted Eight Ball, you win!\n");
        won = true;
        return false;
      }
      else if(breaking && scoringType == cueball)
      {
        scoringType = ballArray[i]->type;

        if(scoringType == solid)
          printf("First ball potted was a solid ball. Must pot the remaining solid balls before the Eight Ball!\n");
        else if(scoringType == solid)
          printf("First ball potted was a striped ball. Must pot the remaining striped balls before the Eight Ball!\n");
      }
      else if(breaking && scoringType == scoringType)
        numPottedDuringHit++;
      else if(!breaking && ballArray[i]->type != scoringType)
      {
        if(scoringType == solid)
          printf("Potted wrong ball type! You must pot solid balls.\n");
        else if(scoringType == striped)
          printf("Potted wrong ball type! You must pot striped balls.\n");
        foul = true;
      }
      else if(!breaking && ballArray[i]->type == scoringType)
        numPottedDuringHit++;
    }
  }

  return foul;
}
