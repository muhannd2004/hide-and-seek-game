#include "gui.h"
#include "menu.h"

// Variable definitions (matching externs in gui.h)
float angle = DEG2RAD * 45.0f;
float radius = 25.0f;
Vector3 center = { 0.0f, 0.0f, 0.0f };

Camera camera = {
    .position = { 0.0f, 15.0f, 25.0f }, // will be set below
    .target = { 0.0f, 0.0f, 0.0f },
    .up = { 0.0f, 1.0f, 0.0f },
    .fovy = 45.0f,
    .projection = CAMERA_PERSPECTIVE
};

Model idleHuman;
Model runningHuman;
char humanState = 0; 

Vector3 rotationAxis = { 1.0f, 0.0f, 0.0f }; // X axis
Vector3 scale = { 0.02f, 0.02f, 0.02f };
float rotationAngle = 0.0f; // Default: 90 degrees to match Blender's X orientation

int idleAnimsCount = 0;
int runningAnimsCount = 0;
unsigned int animIndex = 0;
unsigned int animCurrentFrame = 0;
ModelAnimation *idleHumanAnimations = NULL;
ModelAnimation *runningHumanAnimations = NULL;
int m, n;


float cubeSize = 1.3f;
float spacing = 0.0f; // Will be set in guiInit()
Cell *grid = NULL;
Vector3 playerPos = {0};
bool inMove = false;
int targetMove = 0;
// Initialization function for GUI module
void guiInit() {
    // Set spacing now that cubeSize is initialized
    spacing = cubeSize * 2.0f;

    // Set camera position based on angle and radius
    camera.position.x = center.x + radius * sinf(angle);
    camera.position.y = 15.0f;
    camera.position.z = center.z + radius * cosf(angle);

    idleHuman = LoadModel("resources/glbs/idleHuman2.glb");
    runningHuman = LoadModel("resources/glbs/runningHuman2.glb");

    idleHumanAnimations = LoadModelAnimations("resources/glbs/idleHuman2.glb", &idleAnimsCount);
    runningHumanAnimations = LoadModelAnimations("resources/glbs/runningHuman2.glb", &runningAnimsCount);

  
    int keeper = (int)sqrt(sizeOfWorld);
    while (sizeOfWorld % keeper != 0) keeper--;

    m= keeper;
    n= sizeOfWorld/keeper;
    
    // Allocate and populate grid cells
    grid = malloc(sizeof(Cell) * n * m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            Cell *c = &grid[i * m + j];
            c->pos = (Vector3){
                (i - n/2 + 0.5f) * spacing,
                cubeSize,
                (j - m/2 + 0.5f) * spacing
            };
            c->box.min = (Vector3){ c->pos.x - cubeSize, c->pos.y - 0.05f, c->pos.z - cubeSize };
            c->box.max = (Vector3){ c->pos.x + cubeSize, c->pos.y + 0.05f, c->pos.z + cubeSize };
        }
    }

    playerPos = grid[0].pos;
}

void runGame(int screenWidth, int screenHeight){
    Vector2 mouse = GetMousePosition();
    float rotateSpeed = 2.0f * DEG2RAD; // radians per frame

    // Camera orbit controls
    if (IsKeyDown(KEY_RIGHT)) angle += rotateSpeed;
    if (IsKeyDown(KEY_LEFT))  angle -= rotateSpeed;
    if (IsKeyDown(KEY_UP))    camera.position.y += 0.3f;
    if (IsKeyDown(KEY_DOWN))  camera.position.y -= 0.3f;

    // Camera zoom controls (scroll wheel)
    float zoomSpeed = 1.0f;
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        radius -= wheel * zoomSpeed;
        if (radius < 5.0f) radius = 5.0f;         // Prevent zooming in too close
        if (radius > 100.0f) radius = 100.0f;     // Prevent zooming out too far
    }

    // Recalculate camera position in orbit
    camera.position.x = center.x + radius * sinf(angle);
    camera.position.z = center.z + radius * cosf(angle);
    camera.target = center;
    UpdateCamera(&camera, 0);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        for (int i = 0; i < n*m; i++) {
            RayCollision hit = GetRayCollisionBox(ray, grid[i].box);
            if (hit.hit) {
                humanState = 1;
                inMove = true;
                targetMove = i;

                float dx = grid[targetMove].pos.x - playerPos.x;
                float dz = grid[targetMove].pos.z - playerPos.z;
                if (fabsf(dx) > fabsf(dz)) {
                
                    if (dx > 0)
                        rotationAngle = 90.0f;  
                    else
                        rotationAngle = -90.0f;  
                } else {
                    if (dz > 0)
                        rotationAngle = 0.0f;    
                    else
                        rotationAngle = 180.0f;  
                }
            }
        }
    }

    if (inMove) {
        float dx = grid[targetMove].pos.x - playerPos.x;
        float dz = grid[targetMove].pos.z - playerPos.z;
        float step = 0.14f;

        // Move in X direction
        if (fabsf(dx) > step) {
            playerPos.x += (dx > 0) ? step : -step;
        } else {
            playerPos.x = grid[targetMove].pos.x;
        }

   
        if (fabsf(dz) > step) {
            playerPos.z += (dz > 0) ? step : -step;
        } else {
            playerPos.z = grid[targetMove].pos.z;
        }

        // Stop moving when both X and Z are close enough to target
        if (fabsf(dx) <= step && fabsf(dz) <= step) {
            playerPos.x = grid[targetMove].pos.x;
            playerPos.z = grid[targetMove].pos.z;
            inMove = false;
            humanState = 0;
            // Reset rotation for idle
            rotationAngle = 0.0f;
        }
    }

    // If not moving, keep last rotationAngle or set to default if you want

    // Update model animation
    if (humanState == 1) {
        ModelAnimation anim = runningHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(runningHuman, anim, animCurrentFrame);
    }
    else
    {
        ModelAnimation anim = idleHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(idleHuman, anim, animCurrentFrame);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
      // Source is the whole texture
    Rectangle source = { 0, 0, (float)mainMenuBackGround.width, (float)mainMenuBackGround.height };

    // Destination is full screen
    Rectangle dest = { 0, 0, (float)screenWidth, (float)screenHeight };

    // Origin is top-left corner, no rotation
    Vector2 origin = { 0, 0 };

    // Draw and scale image to fit window
    DrawTexturePro(mainMenuBackGround, source, dest, origin, 0.0f, WHITE);

    BeginMode3D(camera);

    // draw grid cubes
    for (int i = 0; i < n*m; i++) {
        Color col = darkBrown;
        // highlight cube under cursor
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        RayCollision hit = GetRayCollisionBox(ray, grid[i].box);
        if (hit.hit) {
            col = GRAY;
        }
        DrawCube(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, col);
        DrawCubeWires(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, BLACK);
    }

    Vector3 modelAbove = playerPos;
    modelAbove.y += cubeSize ; // 0.5f offset above the cube
    if(humanState == 1){
        // Rotate only running model around Z axis (Y-up)
        DrawModelEx(runningHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }
    else
    {
        // Idle model: no rotation
        DrawModelEx(idleHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, scale, WHITE);
    }

    DrawGrid(10, 1.0f);
    EndMode3D();

    EndDrawing();
}