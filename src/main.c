
#include "raylib.h"
#include <stdio.h>
#include "raymath.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Movement constants
#define GRAVITY         32.0f
#define MAX_SPEED       20.0f
#define CROUCH_SPEED     5.0f
#define JUMP_FORCE      12.0f
#define MAX_ACCEL      150.0f
// Grounded drag
#define FRICTION         0.86f
// Increasing air drag, increases strafing speed
#define AIR_DRAG         0.98f
// Responsiveness for turning movement direction to looked direction
#define CONTROL         15.0f
#define CROUCH_HEIGHT    0.5f
#define STAND_HEIGHT     1.5f

#define FIXED_TIMESTAMP 0.01f
#define DELTA_CAP 0.25f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
  float time;
  float delta;
  float accumulator;
  float alpha;
} TimeState;

typedef struct {
  Vector2 sensitivity;
  int windowWidth;
  int windowHeight;
  bool vsync;
  int fps;
  char language;
  float audioLevel;
} Settings;

typedef struct {
  Vector3 prevPosition;
  Vector3 position;
  Vector3 prevVelocity;
  Vector3 velocity;
  bool isGrounded;
  float prevWalk;
  float walk;
  float prevHeight;
  float height;
  float headTimer;
  char selectedItem;
  bool crouching;
  bool jumpPressed;
  float side;
  float forward;
  Vector3 prevDesiredDir;
  Vector3 desiredDir;
  Vector3 dir;
} Player;

typedef struct {
  Vector2 lookRotation;
  Vector2 lean;
} CameraProps;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void DrawLevel(void);
static void UpdateCameraFPS(Camera *camera, CameraProps *camProps, Player *player);
static void UpdateCameraFPSPhysics(Camera *camera, TimeState *timeState);
static void UpdateCameraFPSRender(Camera *camera, TimeState *timeState);

static void UpdatePlayer(Player *player, float rot);
static void UpdatePlayerPhysics(TimeState *time, Player *player, CameraProps *camProps, Camera *camera);
static void UpdatePlayerRender();


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
  Player player = {
    .velocity = (Vector3) {0.0, 0.0, 0.0},
    .prevVelocity = (Vector3) {0.0, 0.0, 0.0}
  };
  // TODO: Create JSON based settings save system
  Settings settings = {
    .sensitivity = (Vector2) { 0.002f, 0.002f },
  };
  TimeState timeState;
  CameraProps cameraProps;

  // Initialization
  //--------------------------------------------------------------------------------------
  settings.windowWidth = 800;
  settings.windowHeight = 600;

  InitWindow(settings.windowWidth, settings.windowHeight, "Game Project 1");

  // Initialize camera variables
  // NOTE: UpdateCameraFPS() takes care of the rest
  Camera camera = { 0 };
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
  camera.position = (Vector3){
      player.position.x,
      player.position.y + player.height,
      player.position.z,
  };

  UpdateCameraFPS(&camera, &cameraProps, &player); // Update camera parameters
  DisableCursor(); // Limit cursor to relative movement inside the window
  SetTargetFPS(60);

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
      if (IsKeyPressed(KEY_Z)) SetTargetFPS(144);
      if (IsKeyPressed(KEY_O)) SetTargetFPS(30);

      timeState.delta = GetFrameTime();
      if (timeState.delta > DELTA_CAP) timeState.delta = DELTA_CAP;
      timeState.accumulator += timeState.delta; 

      // Update
      //----------------------------------------------------------------------------------
      Vector2 mouseDelta = GetMouseDelta();
      cameraProps.lookRotation.x -= mouseDelta.x*settings.sensitivity.x;
      cameraProps.lookRotation.y += mouseDelta.y*settings.sensitivity.y;


      UpdatePlayer(&player, cameraProps.lookRotation.x);
    

      while (timeState.accumulator >= FIXED_TIMESTAMP) {
        UpdatePlayerPhysics(&timeState, &player, &cameraProps, &camera);
        timeState.accumulator -= FIXED_TIMESTAMP;
        timeState.time += FIXED_TIMESTAMP;
      }

      timeState.alpha = timeState.accumulator / FIXED_TIMESTAMP;

      camera.position = (Vector3){
          player.position.x,
          player.position.y + player.height,
          player.position.z,
      };

      
      UpdateCameraFPS(&camera, &cameraProps, &player);
      //----------------------------------------------------------------------------------

      // Draw
      //----------------------------------------------------------------------------------
      BeginDrawing();
          ClearBackground(RAYWHITE);

          BeginMode3D(camera);
              DrawLevel();
          EndMode3D();

          // Draw info box
          DrawRectangle(5, 5, 330, 75, Fade(SKYBLUE, 0.5f));
          DrawRectangleLines(5, 5, 330, 75, BLUE);

          DrawText("Camera controls:", 15, 15, 10, BLACK);
          DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
          DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
          DrawText(TextFormat("- Velocity Len: (%06.3f)", Vector2Length((Vector2){ player.velocity.x, player.velocity.z })), 15, 60, 10, BLACK);

      EndDrawing();
      //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Update body considering current world state
void UpdatePlayer(Player *player, float rot)
{
  player->side = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
  player->forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
  player->crouching = IsKeyDown(KEY_LEFT_CONTROL);
  player->jumpPressed = IsKeyPressed(KEY_SPACE);

  Vector2 input = (Vector2){ (float)player->side, (float)-player->forward };

   if (player->isGrounded && player->jumpPressed)
  {
      player->velocity.y = JUMP_FORCE;
      player->isGrounded = false;

      // Sound can be played at this moment
      //SetSoundPitch(fxJump, 1.0f + (GetRandomValue(-100, 100)*0.001));
      //PlaySound(fxJump);
  }

  Vector3 front = (Vector3){ sinf(rot), 0.f, cosf(rot) };
  Vector3 right = (Vector3){ cosf(-rot), 0.f, sinf(-rot) };

  player->prevDesiredDir = player->desiredDir;
  player->desiredDir = (Vector3){ input.x*right.x + input.y*front.x, 0.0f, input.x*right.z + input.y*front.z, };
}

static void UpdatePlayerPhysics(TimeState *time, Player *player, CameraProps *camProps, Camera *camera) {
  if (!player->isGrounded) player->velocity.y -= GRAVITY*0.006946;

  player->dir = Vector3Lerp(player->dir, player->desiredDir, CONTROL*0.06946);

  float decel = (player->isGrounded ? FRICTION : AIR_DRAG);
  Vector3 hvel = (Vector3){ player->velocity.x*decel, 0.0f, player->velocity.z*decel };

  float hvelLength = Vector3Length(hvel); // Magnitude
  if (hvelLength < (MAX_SPEED*0.01f)) hvel = (Vector3){ 0 };

  // This is what creates strafing
  float speed = Vector3DotProduct(hvel, player->dir);
 
  // Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
  // a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
  // More info here: https://youtu.be/v3zT3Z5apaM?t=165
  float maxSpeed = (player->crouching ? CROUCH_SPEED : MAX_SPEED);
  float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL*0.006946);
  hvel.x += player->dir.x*accel;
  hvel.z += player->dir.z*accel;

  player->velocity.x = hvel.x;
  player->velocity.z = hvel.z;

  // Fancy collision system against the floor
  if (player->position.y <= 0.0f)
  {
    player->position.y = 0.0f;
    player->velocity.y = 0.0f;
    player->isGrounded = true; // Enable jumping
  }

  player->position.x += player->velocity.x*0.06946;
  player->position.y += player->velocity.y*0.06946;
  player->position.z += player->velocity.z*0.06946;

  player->prevHeight = player->height;
  player->height = Lerp(player->height, (player->crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 0.3);
 
  if (player->isGrounded && ((player->forward != 0) || (player->side != 0)))
  {
    player->headTimer += time->delta*3.0f;
    player->walk = Lerp(player->walk, 1.0f, 0.3);
    camera->fovy = Lerp(camera->fovy, 55.0f, 0.3);
  }
  else
  {
    player->walk = Lerp(player->walk, 0.0f, 0.3);
    camera->fovy = Lerp(camera->fovy, 60.0f, 0.3);
  }

  camProps->lean.x = Lerp(camProps->lean.x, player->side*0.02f, 0.3);
  camProps->lean.y = Lerp(camProps->lean.y, player->forward*0.015f, 0.3);
}

static void UpdateCameraFPS(Camera *camera, CameraProps *camProps, Player *player)
{
  const Vector3 up = (Vector3){ 0.0f, 1.0f, 0.0f };
  const Vector3 targetOffset = (Vector3){ 0.0f, 0.0f, -1.0f };

  // Left and right
  Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, camProps->lookRotation.x);

  // Clamp view up
  float maxAngleUp = Vector3Angle(up, yaw);
  maxAngleUp -= 0.001f; // Avoid numerical errors
  if ( -(camProps->lookRotation.y) > maxAngleUp) { camProps->lookRotation.y = -maxAngleUp; }

  // Clamp view down
  float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
  maxAngleDown *= -1.0f; // Downwards angle is negative
  maxAngleDown += 0.001f; // Avoid numerical errors
  if ( -(camProps->lookRotation.y) < maxAngleDown) { camProps->lookRotation.y = -maxAngleDown; }

  // Up and down
  Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

  // Rotate view vector around right axis
  float pitchAngle = -camProps->lookRotation.y - camProps->lean.y;
  pitchAngle = Clamp(pitchAngle, -PI/2 + 0.0001f, PI/2 - 0.0001f); // Clamp angle so it doesn't go past straight up or straight down
  Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);
 
  float headSin = sinf(player->headTimer*PI);
  float headCos = cosf(player->headTimer*PI);
  const float stepRotation = 0.01f;
  camera->up = Vector3RotateByAxisAngle(up, pitch, headSin*stepRotation + camProps->lean.x);

  // Camera BOB
  const float bobSide = 0.1f;
  const float bobUp = 0.15f;
  Vector3 bobbing = Vector3Scale(right, headSin*bobSide);
  bobbing.y = fabsf(headCos*bobUp);

  camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, player->walk));
  camera->target = Vector3Add(camera->position, pitch);
}


static void UpdateCameraFPSPhysics(Camera *camera, TimeState *timeState) {
  // Head animation
  // Rotate up direction around forward axis
 
}

// Draw game level
static void DrawLevel(void)
{
  const int floorExtent = 25;
  const float tileSize = 5.0f;
  const Color tileColor1 = (Color){ 150, 200, 200, 255 };

  // Floor tiles
  for (int y = -floorExtent; y < floorExtent; y++)
  {
      for (int x = -floorExtent; x < floorExtent; x++)
      {
          if ((y & 1) && (x & 1))
          {
              DrawPlane((Vector3){ x*tileSize, 0.0f, y*tileSize}, (Vector2){ tileSize, tileSize }, tileColor1);
          }
          else if (!(y & 1) && !(x & 1))
          {
              DrawPlane((Vector3){ x*tileSize, 0.0f, y*tileSize}, (Vector2){ tileSize, tileSize }, LIGHTGRAY);
          }
      }
  }

  const Vector3 towerSize = (Vector3){ 16.0f, 32.0f, 16.0f };
  const Color towerColor = (Color){ 150, 200, 200, 255 };

  Vector3 towerPos = (Vector3){ 16.0f, 16.0f, 16.0f };
  DrawCubeV(towerPos, towerSize, towerColor);
  DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

  towerPos.x *= -1;
  DrawCubeV(towerPos, towerSize, towerColor);
  DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

  towerPos.z *= -1;
  DrawCubeV(towerPos, towerSize, towerColor);
  DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

  towerPos.x *= -1;
  DrawCubeV(towerPos, towerSize, towerColor);
  DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

  // Red sun
  DrawSphere((Vector3){ 300.0f, 300.0f, 0.0f }, 100.0f, (Color){ 255, 0, 0, 255 });
}
