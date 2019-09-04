#include "Cue.hh"
#include "Ball.hh"

////////////////////////////////////////////////////////////////////////////////

extern glm::mat4 ViewMat;
extern glm::mat4 ProjectionMat;

extern GLuint cueLightID;

extern GLuint cueMatrixID;
extern GLuint cueModelMatrixID;
extern GLuint cueViewMatrixID;
extern GLuint cueTextureID;

extern GLuint cueTexture;
extern GLuint cueVertexbuffer;
extern GLuint cueUvbuffer;
extern GLuint cueNormalbuffer;
extern GLuint cueElementbuffer;
extern vector<unsigned short> cueIndices;

extern double mouseXPos;
extern int win_w;

Cue *cue;

extern vector<Ball*> ballArray;
extern const double ballRadius;

extern int cueMod;
extern double force;

////////////////////////////////////////////////////////////////////////////////

Cue::Cue() : GameObj(0, 0, 0, glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f))
{

}

void Cue::draw(glm::mat4 Model)
{
  // set light position
  glm::vec3 lightPos = glm::vec3(0,6,0);
  glUniform3f(cueLightID, lightPos.x, lightPos.y, lightPos.z);

  glm::mat4 RotationMatrix = glm::mat4(glm::vec4(frame_x, 0),
                                       glm::vec4(frame_y, 0),
                                       glm::vec4(frame_z, 0),
                                       glm::vec4(0, 0, 0, 1));

  glm::mat4 TranslationMatrix = translate(glm::mat4(), glm::vec3(position.x, position.y, position.z));

  glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * translate(glm::mat4(), glm::vec3(0,0,-ballRadius));

  glm::mat4 MVP = ProjectionMat * ViewMat * Model * ModelMatrix;

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(cueMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(cueModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
  glUniformMatrix4fv(cueViewMatrixID, 1, GL_FALSE, &ViewMat[0][0]);

  // Bind this object's texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cueTexture);

  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(cueTextureID, 0);

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, cueVertexbuffer);
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
  glBindBuffer(GL_ARRAY_BUFFER, cueUvbuffer);
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
  glBindBuffer(GL_ARRAY_BUFFER, cueNormalbuffer);
  glVertexAttribPointer(
      2,                                // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

  // Index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cueElementbuffer);

  // Draw the triangles !
  glDrawElements(
     GL_TRIANGLES,           // mode
     cueIndices.size(),      // count
     GL_UNSIGNED_SHORT,      // type
     (void*)0                // element array buffer offset
     );

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void Cue::draw_lines(glm::mat4 Model)
{
  int draw_mode, num_vertices;

  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;

  num_vertices = 6;
  draw_mode = GL_LINES;

  vertex_buffer_data = (GLfloat *) malloc(3 * num_vertices * sizeof(GLfloat));
  color_buffer_data = (GLfloat *) malloc(3 * num_vertices * sizeof(GLfloat));

  double axis_scale = 10.0;

  // vertices

  vertex_buffer_data[0] = position.x;
  vertex_buffer_data[1] = position.y;
  vertex_buffer_data[2] = position.z;

  vertex_buffer_data[3] = position.x + axis_scale * frame_z.x;
  vertex_buffer_data[4] = position.y + axis_scale * frame_z.y;
  vertex_buffer_data[5] = position.z + axis_scale * frame_z.z;

  vertex_buffer_data[6] = position.x + ballRadius * frame_x.x;
  vertex_buffer_data[7] = position.y;
  vertex_buffer_data[8] = position.z + ballRadius * frame_x.z;

  vertex_buffer_data[9] = position.x + ballRadius * frame_x.x + axis_scale * frame_z.x;
  vertex_buffer_data[10] = position.y + axis_scale * frame_z.y;
  vertex_buffer_data[11] = position.z + ballRadius * frame_x.z + axis_scale * frame_z.z;

  vertex_buffer_data[12] = position.x - ballRadius * frame_x.x;
  vertex_buffer_data[13] = position.y;
  vertex_buffer_data[14] = position.z - ballRadius * frame_x.z;

  vertex_buffer_data[15] = position.x - ballRadius * frame_x.x + axis_scale * frame_z.x;
  vertex_buffer_data[16] = position.y + axis_scale * frame_z.y;
  vertex_buffer_data[17] = position.z - ballRadius * frame_x.z + axis_scale * frame_z.z;

  // color

  color_buffer_data[0] = 1.0f;
  color_buffer_data[1] = 0.0f;
  color_buffer_data[2] = 0.0f;

  color_buffer_data[3] = 1.0f;
  color_buffer_data[4] = 0.0f;
  color_buffer_data[5] = 0.0f;

  color_buffer_data[6] = 1.0f;
  color_buffer_data[7] = 0.0f;
  color_buffer_data[8] = 0.0f;

  color_buffer_data[9] = 1.0f;
  color_buffer_data[10] = 0.0f;
  color_buffer_data[11] = 0.0f;

  color_buffer_data[12] = 1.0f;
  color_buffer_data[13] = 0.0f;
  color_buffer_data[14] = 0.0f;

  color_buffer_data[15] = 1.0f;
  color_buffer_data[16] = 0.0f;
  color_buffer_data[17] = 0.0f;

  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP = ProjectionMat * ViewMat * Model;

  // make this transform available to shaders
  glUniformMatrix4fv(cueMatrixID, 1, GL_FALSE, &MVP[0][0]);

  // 1st attribute buffer : vertices
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0,                  // attribute. 0 to match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

  // 2nd attribute buffer : colors
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(1,                                // attribute. 1 to match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

  // Draw the flocker!
  glDrawArrays(draw_mode, 0, num_vertices);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  free(vertex_buffer_data);
  free(color_buffer_data);
}

void Cue::update()
{
  position = ballArray[0]->position;

  double x, sign;
  switch(cueMod)
  {
    case 0:
      x = (mouseXPos-win_w/2)/win_w;
      sign = -1;
      break;
    case 1:
    case -3:
      x = (mouseXPos)/win_w;
      sign = 1;
      break;
    case 2:
    case -2:
      x = (mouseXPos-win_w/2)/win_w;
      sign = 1;
      break;
    case 3:
    case -1:
      x = (mouseXPos)/win_w;
      sign = -1;
      break;
    default:
    break;
  }

  frame_y = glm::normalize(glm::vec3(0,1,0));
  frame_z = glm::normalize(glm::vec3(sign*sin(M_PI*x),0,sign*cos(M_PI*x)));
  frame_x = glm::normalize(glm::cross(frame_y, frame_z));
}
