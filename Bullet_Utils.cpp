#include "Bullet_Utils.hh"
#include "Ball.hh"
#include "Table.hh"
#include "Cue.hh"

using namespace std;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// created in main.cpp
extern GLFWwindow* window;
extern GLuint VertexArrayID;
extern GLuint objprogramID;
extern GLuint objMatrixID;
extern GLuint objViewMatrixID;
extern GLuint objModelMatrixID;
extern GLuint objTextureID;
extern GLuint objLightID;

extern vector<glm::vec3> obj_vertices;
extern vector<glm::vec2> obj_uvs;
extern vector<glm::vec3> obj_normals;
extern GLuint obj_Texture;
extern vector<unsigned short> obj_indices;
extern vector<glm::vec3> obj_indexed_vertices;
extern vector<glm::vec2> obj_indexed_uvs;
extern vector<glm::vec3> obj_indexed_normals;
extern GLuint obj_vertexbuffer;
extern GLuint obj_uvbuffer;
extern GLuint obj_normalbuffer;
extern GLuint obj_elementbuffer;

extern vector<glm::vec3> ballVertices;
extern vector<glm::vec3> tableVertices;
extern vector<glm::vec3> tableIndexedVertices;
extern vector<unsigned short> tableIndices;

extern int numBalls;
extern const double ballRadius;
extern vector <Ball *> ballArray;
extern Table *table;
extern Cue *cue;

extern float box_width;
extern float box_height;
extern float box_depth;

btDiscreteDynamicsWorld* bullet_dynamicsWorld;
btSequentialImpulseConstraintSolver* bullet_solver;
btBroadphaseInterface* bullet_broadphase;
btDefaultCollisionConfiguration* bullet_collisionConfiguration;
btCollisionDispatcher* bullet_dispatcher;
vector<btRigidBody*> bullet_rigidbodies;   // bullet side

vector<glm::vec3> xyz_positions;        // ogl side
vector<glm::quat> quat_orientations;     // ogl side
vector<glm::vec3> xyz_velocities;       // ogl side

vector<glm::vec3> prev_xyz_positions;
vector<glm::quat> prev_quat_orientations;

extern double target_FPS;
extern float box_width;

float initialCameraZ = 25.0;

// cut these out of common/controls.cpp

glm::mat4 myViewMatrix;
glm::mat4 myProjectionMatrix;

glm::vec3 myPosition = glm::vec3( 0, 0, initialCameraZ ); ;
float myHorizontalAngle = 3.14f;
float myVerticalAngle = 0.0f;
float myInitialFoV = 45.0f;

float scaleFactor = 0.1;

bool isRestarting = false;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// get information FROM flocker objects
// note that orientations are NOT currently taken from flocker -- they are random

void copy_ball_states_to_graphics_objects()
{
  xyz_positions.resize(numBalls);
  quat_orientations.resize(numBalls);

  for (int i=0;i<numBalls;i++)
  {
    xyz_positions[i] = glm::vec3(ballArray[i]->position.x, ballArray[i]->position.y, ballArray[i]->position.z);

    glm::mat4 R = glm::mat4(glm::vec4(ballArray[i]->frame_x, 0),
                            glm::vec4(ballArray[i]->frame_y, 0),
                            glm::vec4(ballArray[i]->frame_z, 0),
                            glm::vec4(0, 0, 0, 1));
    quat_orientations[i] = glm::quat(R);
  }
}

//----------------------------------------------------------------------------

// move information TO ball objects

void copy_graphics_objects_to_ball_states()
{
  for(int i=0;i<numBalls;i++)
  {
    ballArray[i]->position.x = xyz_positions[i].x;
    ballArray[i]->position.y = xyz_positions[i].y;
    ballArray[i]->position.z = xyz_positions[i].z;

    // Copy orientation to ball
    glm::mat4 R = glm::toMat4(quat_orientations[i]);
    ballArray[i]->frame_x = glm::vec3(R[0]);
    ballArray[i]->frame_y = glm::vec3(R[1]);
    ballArray[i]->frame_z = glm::vec3(R[2]);
  }
}

//----------------------------------------------------------------------------

// only the ground plane for now

void bullet_add_obstacles()
{
  // add the ground plane (not drawn)
  btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), -3);

  btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
  btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
  btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
  bullet_dynamicsWorld->addRigidBody(groundRigidBody);

  // add the ground plane (not drawn)
  btCollisionShape* ceilShape = new btStaticPlaneShape(btVector3(0, -1, 0), -0.21);

  btDefaultMotionState* ceilMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
  btRigidBody::btRigidBodyConstructionInfo ceilRigidBodyCI(0, ceilMotionState, ceilShape, btVector3(0, 0, 0));
  btRigidBody* ceilRigidBody = new btRigidBody(ceilRigidBodyCI);
  bullet_dynamicsWorld->addRigidBody(ceilRigidBody);
}

//----------------------------------------------------------------------------

// add all of the moveable objects.
// quat_orientations, xyz_positions should already be set

void bullet_add_dynamic_objects()
{
  bullet_rigidbodies.clear();
  btSphereShape* ballShape = new btSphereShape(ballRadius);

  btScalar ballShapeMass = 0.17;
  btVector3 simplifiedShapeInertia(0, 0, 0);

  ballShape->calculateLocalInertia(ballShapeMass, simplifiedShapeInertia);

  // put objects into physics simulation
  for (int i=0;i<numBalls;i++) {
    btDefaultMotionState* motionstate = new btDefaultMotionState(btTransform(btQuaternion(quat_orientations[i].x, quat_orientations[i].y, quat_orientations[i].z, quat_orientations[i].w),
									     btVector3(xyz_positions[i].x, xyz_positions[i].y, xyz_positions[i].z)));

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(ballShapeMass,
							 motionstate,
							 ballShape,
							 simplifiedShapeInertia);
    rigidBodyCI.m_restitution = 1.0f;

    btRigidBody *rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->activate(true);
    if(ballArray[i]->type != cueball) rigidBody->setDamping(0.40f,0.40f);
    else rigidBody->setDamping(0.25f,0.25f);
    rigidBody->setSleepingThresholds(1.0f,1.0f);
    if(ballArray[i]->type == cueball) rigidBody->setAngularFactor(0.0f);

    bullet_rigidbodies.push_back(rigidBody);
    bullet_dynamicsWorld->addRigidBody(rigidBody);
  }

  btTriangleMesh *tableMesh = new btTriangleMesh();
  for(int i=0;i<tableVertices.size()/3;i++)
  {
    glm::vec3 v0 = tableVertices[3*i];
    glm::vec3 v1 = tableVertices[3*i+1];
    glm::vec3 v2 = tableVertices[3*i+2];
    btVector3 V0(v0.x,v0.y,v0.z);
    btVector3 V1(v1.x,v1.y,v1.z);
    btVector3 V2(v2.x,v2.y,v2.z);
    tableMesh->addTriangle(V0,V1,V2);
  }
  btCollisionShape *tableShape = new btBvhTriangleMeshShape(tableMesh, true, true);

  btScalar tableShapeMass = 0;  // in kg.  0 -> static object aka never moves
  tableShape->calculateLocalInertia(tableShapeMass, simplifiedShapeInertia);

  // put objects into physics simulation
  glm::mat4 R = glm::mat4(glm::vec4(table->frame_x, 0),
                          glm::vec4(table->frame_y, 0),
                          glm::vec4(table->frame_z, 0),
                          glm::vec4(0, 0, 0, 1));
  glm::quat tableQuat = glm::quat(R);

  btDefaultMotionState* motionstate = new btDefaultMotionState(btTransform(btQuaternion(tableQuat.x, tableQuat.y, tableQuat.z, tableQuat.w),
								       btVector3(table->position.x, table->position.y, table->position.z)));

  btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(tableShapeMass,
						 motionstate,
						 tableShape,
						 simplifiedShapeInertia);
 rigidBodyCI.m_restitution = 0.1f;

  btRigidBody *rigidBody = new btRigidBody(rigidBodyCI);
  bullet_rigidbodies.push_back(rigidBody);
  bullet_dynamicsWorld->addRigidBody(rigidBody);
}

//----------------------------------------------------------------------------

// This strictly follows http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World,

void initialize_bullet_simulator()
{
  // Build the broadphase
  bullet_broadphase = new btDbvtBroadphase();

  // Set up the collision configuration and dispatcher
  bullet_collisionConfiguration = new btDefaultCollisionConfiguration();
  bullet_dispatcher = new btCollisionDispatcher(bullet_collisionConfiguration);

  // The actual physics solver
  bullet_solver = new btSequentialImpulseConstraintSolver;

  // The world.
  bullet_dynamicsWorld = new btDiscreteDynamicsWorld(bullet_dispatcher, bullet_broadphase, bullet_solver, bullet_collisionConfiguration);
  bullet_dynamicsWorld->setGravity(btVector3(0,-9.81f,0));

  // the stuff in the world
  bullet_add_obstacles();
  bullet_add_dynamic_objects();
}

//----------------------------------------------------------------------------

// Clean up behind ourselves like good little programmers

void delete_bullet_simulator()
{
  // remove all of the objects

  for (int i = 0; i < bullet_rigidbodies.size(); i++){
    bullet_dynamicsWorld->removeRigidBody(bullet_rigidbodies[i]);
    delete bullet_rigidbodies[i]->getMotionState();
    delete bullet_rigidbodies[i];
  }
  bullet_rigidbodies.clear();

  delete bullet_dynamicsWorld;
  delete bullet_solver;
  delete bullet_collisionConfiguration;
  delete bullet_dispatcher;
  delete bullet_broadphase;
}

//----------------------------------------------------------------------------

void update_physics_simulation(float deltaTime)
{
  // actually simulate motion and collisions
  bullet_dynamicsWorld->stepSimulation(deltaTime, 1);

  // query new positions and orientations
  for(int i=0;i<numBalls;i++)
  {
    btTransform trans;
    bullet_rigidbodies[i]->getMotionState()->getWorldTransform(trans);

    xyz_positions[i].x = trans.getOrigin().getX();
    xyz_positions[i].y = trans.getOrigin().getY();
    xyz_positions[i].z = trans.getOrigin().getZ();

    quat_orientations[i].x = trans.getRotation().getX();
    quat_orientations[i].y = trans.getRotation().getY();
    quat_orientations[i].z = trans.getRotation().getZ();
    quat_orientations[i].w = trans.getRotation().getW();

  }

  // put information back in flocker objects
  copy_graphics_objects_to_ball_states();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void hit_cueball(double force)
{
  printf("Hitting cueball with force %.0f\n", force);

  bullet_rigidbodies[0]->activate(true);
  btVector3 F(force*cue->frame_z.x,0,force*cue->frame_z.z);
  btVector3 p(cue->position.x,cue->position.y,cue->position.z);
  bullet_rigidbodies[0]->applyForce(F,p);
}

bool is_balls_stopped()
{
  bool stopped = true;
  for(int i=0;i<numBalls;i++)
  {
    if(!ballArray[i]->potted)
      stopped = stopped && bullet_rigidbodies[i]->getLinearVelocity().length() == 0.0f;
  }

  return stopped;
}

void save_rigidbody_states()
{
  prev_xyz_positions.resize(numBalls);
  prev_quat_orientations.resize(numBalls);
  for(int i=0;i<numBalls;i++)
  {
    prev_xyz_positions[i] = xyz_positions[i];
    prev_quat_orientations[i] = quat_orientations[i];
  }
}
void reset_rigidbody_states()
{
  for(int i=0;i<numBalls;i++)
  {
    xyz_positions[i] = prev_xyz_positions[i];
    quat_orientations[i] = prev_quat_orientations[i];
  }
}
