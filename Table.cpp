#include "Table.hh"

////////////////////////////////////////////////////////////////////////////////

extern glm::mat4 ViewMat;
extern glm::mat4 ProjectionMat;

extern GLuint tableLightID;

extern GLuint tableMatrixID;
extern GLuint tableModelMatrixID;
extern GLuint tableViewMatrixID;
extern GLuint tableTextureID;
extern GLuint tableWoodTextureID;

extern GLuint tableTexture;
extern GLuint tableWoodTexture;
extern GLuint tableVertexbuffer;
extern GLuint tableUvbuffer;
extern GLuint tableNormalbuffer;
extern GLuint tableElementbuffer;
extern vector<unsigned short> tableIndices;

Table *table;

////////////////////////////////////////////////////////////////////////////////

Table::Table(double init_x, double init_y, double init_z
            ) : GameObj(init_x, init_y, init_z, glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f))
{

}

void Table::draw(glm::mat4 Model)
{
  // set light position
  glm::vec3 lightPos = glm::vec3(0,6,0);
  glUniform3f(tableLightID, lightPos.x, lightPos.y, lightPos.z);

  glm::mat4 RotationMatrix = glm::mat4(glm::vec4(frame_x, 0),
                                       glm::vec4(frame_y, 0),
                                       glm::vec4(frame_z, 0),
                                       glm::vec4(0, 0, 0, 1));

  glm::mat4 TranslationMatrix = translate(glm::mat4(), glm::vec3(position.x, position.y, position.z));

  glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

  glm::mat4 MVP = ProjectionMat * ViewMat * Model * ModelMatrix;

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(tableMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(tableModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
  glUniformMatrix4fv(tableViewMatrixID, 1, GL_FALSE, &ViewMat[0][0]);

  // Bind this object's texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tableTexture);

  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(tableTextureID, 0);

  // Bind this object's texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0+1);
  glBindTexture(GL_TEXTURE_2D, tableWoodTexture);

  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(tableWoodTextureID, 1);

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, tableVertexbuffer);
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
  glBindBuffer(GL_ARRAY_BUFFER, tableUvbuffer);
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
  glBindBuffer(GL_ARRAY_BUFFER, tableNormalbuffer);
  glVertexAttribPointer(
      2,                                // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

  // Index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tableElementbuffer);

  // Draw the triangles !
  glDrawElements(
     GL_TRIANGLES,           // mode
     tableIndices.size(),    // count
     GL_UNSIGNED_SHORT,      // type
     (void*)0                // element array buffer offset
     );

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}
