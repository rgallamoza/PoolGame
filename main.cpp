// Include standard headers

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>

// Include GLEW

#include <GL/glew.h>

// Include GLFW

#include <GLFW/glfw3.h>

// Include GLM

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Bullet-specific stuff

#include "Bullet_Utils.hh"

// for loading GLSL shaders

#include <common/shader.hpp>

// creature-specific stuff

#include "Ball.hh"
#include "Table.hh"
#include "Cue.hh"

//----------------------------------------------------------------------------

// to avoid gimbal lock issues...

#define MAX_LATITUDE_DEGS     89.0
#define MIN_LATITUDE_DEGS     25.0

#define CAMERA_MODE_ORBIT    1
#define CAMERA_MODE_CREATURE 2

#define MIN_ORBIT_CAM_RADIUS    (7.0)
#define MAX_ORBIT_CAM_RADIUS    (25.0)

#define DEFAULT_ORBIT_CAM_RADIUS            15.0
#define DEFAULT_ORBIT_CAM_LATITUDE_DEGS     25.0
#define DEFAULT_ORBIT_CAM_LONGITUDE_DEGS    90.0

//----------------------------------------------------------------------------

// some convenient globals

GLFWwindow* window;

GLuint ballProgramID;
GLuint tableProgramID;
GLuint cueProgramID;
GLuint lineProgramID;

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;

GLuint ballMatrixID;
GLuint ballModelMatrixID;
GLuint ballViewMatrixID;
vector<GLuint> ballTextureIDs;
GLuint ballLightID;

GLuint tableMatrixID;
GLuint tableModelMatrixID;
GLuint tableViewMatrixID;
GLuint tableTextureID;
GLuint tableWoodTextureID;
GLuint tableLightID;

GLuint cueMatrixID;
GLuint cueModelMatrixID;
GLuint cueViewMatrixID;
GLuint cueTextureID;
GLuint cueLightID;

GLuint VertexArrayID;

// used for obj files

vector<glm::vec3> ballVertices;
vector<glm::vec2> ballUvs;
vector<glm::vec3> ballNormals;

vector<GLuint> ballTextures;
vector<unsigned short> ballIndices;

vector<glm::vec3> ballIndexedVertices;
vector<glm::vec2> ballIndexedUvs;
vector<glm::vec3> ballIndexedNormals;

GLuint ballVertexbuffer;
GLuint ballUvbuffer;
GLuint ballNormalbuffer;
GLuint ballElementbuffer;

vector<glm::vec3> tableVertices;
vector<glm::vec2> tableUvs;
vector<glm::vec3> tableNormals;

GLuint tableTexture;
GLuint tableWoodTexture;
vector<unsigned short> tableIndices;

vector<glm::vec3> tableIndexedVertices;
vector<glm::vec2> tableIndexedUvs;
vector<glm::vec3> tableIndexedNormals;

GLuint tableVertexbuffer;
GLuint tableUvbuffer;
GLuint tableNormalbuffer;
GLuint tableElementbuffer;

vector<glm::vec3> cueVertices;
vector<glm::vec2> cueUvs;
vector<glm::vec3> cueNormals;

GLuint cueTexture;
vector<unsigned short> cueIndices;

vector<glm::vec3> cueIndexedVertices;
vector<glm::vec2> cueIndexedUvs;
vector<glm::vec3> cueIndexedNormals;

GLuint cueVertexbuffer;
GLuint cueUvbuffer;
GLuint cueNormalbuffer;
GLuint cueElementbuffer;

// these along with Model matrix make MVP transform

glm::mat4 ProjectionMat;
glm::mat4 ViewMat;

// sim-related globals

double target_FPS = 60.0;

bool using_obj_program = false;

bool is_paused = false;
bool is_physics_active = false;

double orbit_cam_radius = DEFAULT_ORBIT_CAM_RADIUS;
double orbit_cam_delta_radius = 0.1;

double orbit_cam_latitude_degs = DEFAULT_ORBIT_CAM_LATITUDE_DEGS;
double orbit_cam_longitude_degs = DEFAULT_ORBIT_CAM_LONGITUDE_DEGS;
double orbit_cam_delta_theta_degs = 2.0;

int win_scale_factor = 300;
int win_w = win_scale_factor * 4.0;
int win_h = win_scale_factor * 3.0;

int camera_mode = CAMERA_MODE_ORBIT;

double mouseXPos, mouseYPos;

extern vector <Ball *> ballArray;
extern Table *table;
extern Cue *cue;

int numBalls = 16;
extern double ballInitPos[];

int cueMod = 0;
double force = 100.0;

float pottedHeightThreshold = -1.0f;
BallType scoringType = cueball;

int numPotted = 0;
int numPottedDuringHit;

bool breaking = true;
bool won = false;

//----------------------------------------------------------------------------

void save_game_state()
{
  save_ball_states();
  save_rigidbody_states();
}

void reset_game_state()
{
  printf("Reverting to previous game state...\n");
  is_physics_active = false;
  delete_bullet_simulator();
  reset_ball_states();
  copy_ball_states_to_graphics_objects();
  reset_rigidbody_states();
  initialize_bullet_simulator();
}

//----------------------------------------------------------------------------

void end_program()
{
  // Cleanup VBOs and shader
  //glDeleteBuffers(1, &box_vertexbuffer);
  //glDeleteBuffers(1, &box_colorbuffer);

  glDeleteProgram(ballProgramID);
  glDeleteProgram(tableProgramID);
  glDeleteProgram(cueProgramID);

  //glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  exit(1);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// handle key presses

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

  // quit
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    end_program();

  // Hit cue ball
  else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !is_physics_active)
  {
    is_physics_active = true;
    numPottedDuringHit = 0;
    save_game_state();
    hit_cueball(force);
  }

  // Reset to previous gamestate
  else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    reset_game_state();

  // orbit rotate
  else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    orbit_cam_longitude_degs -= orbit_cam_delta_theta_degs;
  else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    orbit_cam_longitude_degs += orbit_cam_delta_theta_degs;
  else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_latitude_degs + orbit_cam_delta_theta_degs <= MAX_LATITUDE_DEGS)
      orbit_cam_latitude_degs += orbit_cam_delta_theta_degs;
  }
  else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_latitude_degs - orbit_cam_delta_theta_degs >= MIN_LATITUDE_DEGS)
      orbit_cam_latitude_degs -= orbit_cam_delta_theta_degs;
  }

  // orbit zoom in/out
  else if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_radius + orbit_cam_delta_radius <= MAX_ORBIT_CAM_RADIUS)
      orbit_cam_radius += orbit_cam_delta_radius;
  }
  else if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_radius - orbit_cam_delta_radius >= MIN_ORBIT_CAM_RADIUS)
      orbit_cam_radius -= orbit_cam_delta_radius;
  }

  // orbit pose reset
  else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    orbit_cam_radius = DEFAULT_ORBIT_CAM_RADIUS;
    orbit_cam_latitude_degs = DEFAULT_ORBIT_CAM_LATITUDE_DEGS;
    orbit_cam_longitude_degs = DEFAULT_ORBIT_CAM_LONGITUDE_DEGS;
  }

  // change cue angle
  else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    cueMod = ++cueMod % 4;
  else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    cueMod = --cueMod % 4;
  else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
  {
    force += 10;
    printf("Force set to %.0f\n", force);
  }
  else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
  {
    force -= 10;
    printf("Force set to %.0f\n", force);
  }
}

//----------------------------------------------------------------------------

// allocate simulation data structures and populate them

void initialize_game_objects()
{
  ballArray.clear();
  for(int i=0;i<numBalls;i++)
    ballArray.push_back(
      new Ball(i,
               ballInitPos[3*i], ballInitPos[3*i+1], ballInitPos[3*i+2],
               glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f)
              ));

  table = new Table(0,0,0);
  cue = new Cue();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// place the camera here

void setup_camera()
{
  ProjectionMat = glm::perspective(50.0f, (float) win_w / (float) win_h, 0.1f, 35.0f);

  if (camera_mode == CAMERA_MODE_ORBIT) {

    double orbit_cam_azimuth = glm::radians(orbit_cam_longitude_degs);
    double orbit_cam_inclination = glm::radians(90.0 - orbit_cam_latitude_degs);

    double x_cam = ballArray[0]->position.x + orbit_cam_radius * sin(orbit_cam_inclination) * cos(orbit_cam_azimuth); // 0.5 * box_width;
    double z_cam = ballArray[0]->position.z + orbit_cam_radius * sin(orbit_cam_inclination) * sin(orbit_cam_azimuth); // 0.5 * box_height;
    double y_cam = orbit_cam_radius * cos(orbit_cam_inclination) - 1; // 15.0;

    glm::vec3 cam_look = ballArray[0]->position;
    cam_look.x *= 0.75;
    cam_look.y *= 0.75;
    cam_look.z *= 0.75;
    ViewMat = glm::lookAt(glm::vec3(x_cam, y_cam, z_cam),   // Camera location in World Space
        cam_look,
			  glm::vec3(0,-1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
			  );
  }
  else {

    printf("only orbit camera mode currently supported\n");
    exit(1);

  }
}

//----------------------------------------------------------------------------

// no error-checking -- you have been warned...

void load_objects_and_textures()
{
  /* Ball */
  loadOBJ("objs/ball.obj", ballVertices, ballUvs, ballNormals);
  for(int i=0;i<ballUvs.size();i++)
    ballUvs[i].y = 1.0f-ballUvs[i].y;
  ballTextures.resize(numBalls);
  ballTextures[0] = loadBMP_custom("textures/ball0.bmp");
  ballTextures[1] = loadBMP_custom("textures/ball1.bmp");
  ballTextures[2] = loadBMP_custom("textures/ball2.bmp");
  ballTextures[3] = loadBMP_custom("textures/ball3.bmp");
  ballTextures[4] = loadBMP_custom("textures/ball4.bmp");
  ballTextures[5] = loadBMP_custom("textures/ball5.bmp");
  ballTextures[6] = loadBMP_custom("textures/ball6.bmp");
  ballTextures[7] = loadBMP_custom("textures/ball7.bmp");
  ballTextures[8] = loadBMP_custom("textures/ball8.bmp");
  ballTextures[9] = loadBMP_custom("textures/ball9.bmp");
  ballTextures[10] = loadBMP_custom("textures/ball10.bmp");
  ballTextures[11] = loadBMP_custom("textures/ball11.bmp");
  ballTextures[12] = loadBMP_custom("textures/ball12.bmp");
  ballTextures[13] = loadBMP_custom("textures/ball13.bmp");
  ballTextures[14] = loadBMP_custom("textures/ball14.bmp");
  ballTextures[15] = loadBMP_custom("textures/ball15.bmp");

  indexVBO(ballVertices, ballUvs, ballNormals, ballIndices, ballIndexedVertices, ballIndexedUvs, ballIndexedNormals);

  glGenBuffers(1, &ballVertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, ballVertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, ballIndexedVertices.size() * sizeof(glm::vec3), &ballIndexedVertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &ballUvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, ballUvbuffer);
  glBufferData(GL_ARRAY_BUFFER, ballIndexedUvs.size() * sizeof(glm::vec2), &ballIndexedUvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &ballNormalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, ballNormalbuffer);
  glBufferData(GL_ARRAY_BUFFER, ballIndexedNormals.size() * sizeof(glm::vec3), &ballIndexedNormals[0], GL_STATIC_DRAW);

  glGenBuffers(1, &ballElementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballElementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ballIndices.size() * sizeof(unsigned short), &ballIndices[0] , GL_STATIC_DRAW);

  /* Table */
  loadOBJ("objs/pooltable.obj", tableVertices, tableUvs, tableNormals);
  for(int i=0;i<tableUvs.size();i++)
    tableUvs[i].y = 1.0f-tableUvs[i].y;
  tableTexture = loadBMP_custom("textures/pooltable.bmp");
  tableWoodTexture = loadBMP_custom("textures/pooltable_wood.bmp");

  indexVBO(tableVertices, tableUvs, tableNormals, tableIndices, tableIndexedVertices, tableIndexedUvs, tableIndexedNormals);

  glGenBuffers(1, &tableVertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tableVertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, tableIndexedVertices.size() * sizeof(glm::vec3), &tableIndexedVertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &tableUvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tableUvbuffer);
  glBufferData(GL_ARRAY_BUFFER, tableIndexedUvs.size() * sizeof(glm::vec2), &tableIndexedUvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &tableNormalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tableNormalbuffer);
  glBufferData(GL_ARRAY_BUFFER, tableIndexedNormals.size() * sizeof(glm::vec3), &tableIndexedNormals[0], GL_STATIC_DRAW);

  glGenBuffers(1, &tableElementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tableElementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tableIndices.size() * sizeof(unsigned short), &tableIndices[0] , GL_STATIC_DRAW);

  /* Cue */
  loadOBJ("objs/cue.obj", cueVertices, cueUvs, cueNormals);
  for(int i=0;i<cueUvs.size();i++)
    cueUvs[i].y = 1.0f-cueUvs[i].y;
  cueTexture = loadBMP_custom("textures/cue.bmp");

  indexVBO(cueVertices, cueUvs, cueNormals, cueIndices, cueIndexedVertices, cueIndexedUvs, cueIndexedNormals);

  glGenBuffers(1, &cueVertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cueVertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, cueIndexedVertices.size() * sizeof(glm::vec3), &cueIndexedVertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &cueUvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cueUvbuffer);
  glBufferData(GL_ARRAY_BUFFER, cueIndexedUvs.size() * sizeof(glm::vec2), &cueIndexedUvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &cueNormalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cueNormalbuffer);
  glBufferData(GL_ARRAY_BUFFER, cueIndexedNormals.size() * sizeof(glm::vec3), &cueIndexedNormals[0], GL_STATIC_DRAW);

  glGenBuffers(1, &cueElementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cueElementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, cueIndices.size() * sizeof(unsigned short), &cueIndices[0] , GL_STATIC_DRAW);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Initialise GLFW

  if( !glfwInit() )
    {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      getchar();
      return -1;
    }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

  // Open a window and create its OpenGL context

  window = glfwCreateWindow(win_w, win_h, "PoolGame", NULL, NULL);
  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW

  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  // background color, depth testing

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  //  glEnable(GL_CULL_FACE);  // we don't want this enabled when drawing the creatures as floating triangles

  // vertex arrays

  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Create and compile our GLSL program from the shaders

  // obj rendering mode uses shaders that came with bullet demo program

  ballProgramID = LoadShaders("shaders/BallShading.vertexshader", "shaders/BallShading.fragmentshader");

  tableProgramID = LoadShaders("shaders/TableShading.vertexshader", "shaders/TableShading.fragmentshader");

  cueProgramID = LoadShaders("shaders/CueShading.vertexshader", "shaders/CueShading.fragmentshader");

  lineProgramID = LoadShaders("shaders/SolidColor.vertexshader", "shaders/SolidColor.fragmentshader");

  // Get handles for our uniform variables

  ballMatrixID = glGetUniformLocation(ballProgramID, "MVP");
  ballViewMatrixID = glGetUniformLocation(ballProgramID, "V");
  ballModelMatrixID = glGetUniformLocation(ballProgramID, "M");
  for(int i=0;i<numBalls;i++)
    ballTextureIDs.push_back(glGetUniformLocation(ballProgramID, "tex_ball"+i));
  ballLightID = glGetUniformLocation(ballProgramID, "LightPosition_worldspace");

  tableMatrixID = glGetUniformLocation(tableProgramID, "MVP");
  tableViewMatrixID = glGetUniformLocation(tableProgramID, "V");
  tableModelMatrixID = glGetUniformLocation(tableProgramID, "M");
  tableTextureID = glGetUniformLocation(tableProgramID, "tex_pooltable");
  tableWoodTextureID = glGetUniformLocation(tableProgramID, "tex_wood");
  tableLightID = glGetUniformLocation(tableProgramID, "LightPosition_worldspace");

  cueMatrixID = glGetUniformLocation(cueProgramID, "MVP");
  cueViewMatrixID = glGetUniformLocation(cueProgramID, "V");
  cueModelMatrixID = glGetUniformLocation(cueProgramID, "M");
  cueTextureID = glGetUniformLocation(cueProgramID, "tex_cue");
  cueLightID = glGetUniformLocation(cueProgramID, "LightPosition_worldspace");

  // Use our shader
  glUseProgram(ballProgramID);

  // register all callbacks
  glfwSetKeyCallback(window, key_callback);

  // objects, textures
  load_objects_and_textures();

  // simulation
  initialize_game_objects();

  copy_ball_states_to_graphics_objects();
  initialize_bullet_simulator();

  // timing stuff
  double currentTime, lastTime;
  double target_period = 1.0 / target_FPS;

  lastTime = glfwGetTime();

  save_game_state();
  // enter simulate-render loop (with event handling)
  do {
    // STEP THE SIMULATION -- EITHER FLOCKING OR PHYSICS
    if(is_physics_active)
      update_physics_simulation(target_period);
    // RENDER IT
    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set model transform and color for each triangle and draw it offscreen
    setup_camera();

    // a centering translation for viewing
    glm::mat4 M = glm::mat4(1.0);

    // draw game objects
    glUseProgram(tableProgramID);
    table->draw(M);

    glUseProgram(ballProgramID);
    for(int i=0;i<numBalls;i++)
      if(!ballArray[i]->potted)
        ballArray[i]->draw(M);

    if(!is_physics_active)
    {
      glUseProgram(cueProgramID);
      glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
      cue->update();
      cue->draw(M);

      glUseProgram(lineProgramID);
      cue->draw_lines(M);
    }
    else
    {
      if(is_balls_stopped())
      {
        if(breaking && numPottedDuringHit == 0)
        {
          printf("No balls potted during break! Resetting...\n");
          reset_game_state();
        }
        else
        {
          is_physics_active = false;
          if(breaking)
            breaking = false;
          numPotted += numPottedDuringHit;
        }
      }
      else if(examine_balls() && !won)
        reset_game_state();
    }

    // busy wait if we are going too fast
    do {
      currentTime = glfwGetTime();
    } while (currentTime - lastTime < target_period);

    lastTime = currentTime;

    // Swap buffers

    glfwSwapBuffers(window);
    glfwPollEvents();

  } while ( glfwWindowShouldClose(window) == 0 );

  end_program();

  return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
