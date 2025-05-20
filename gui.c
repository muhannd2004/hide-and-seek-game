#include "gui.h"
#include "menu.h"
#include "solve.h"

// Add these prototypes at the top:
void resetGame(void);
void nextRound(void);

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
Model humanHide;
Model winningHuman;
Model losingHuman;
enum HumanState humanState = IDLE;

Vector3 rotationAxis = { 1.0f, 0.0f, 0.0f }; // X axis
Vector3 scale = { 0.02f, 0.02f, 0.02f };
float rotationAngle = 0.0f; // Default: 90 degrees to match Blender's X orientation

int idleAnimsCount = 0;
int runningAnimsCount = 0;
int hideAnimsCount = 0;
unsigned int animIndex = 0;
unsigned int animCurrentFrame = 0;
ModelAnimation *idleHumanAnimations = NULL;
ModelAnimation *runningHumanAnimations = NULL;
ModelAnimation *humanHideAnimations = NULL;
ModelAnimation *winningHumanAnimations = NULL;
ModelAnimation *losingHumanAnimations = NULL;

int hiderGridChoice = -1,seekerGridChoice = -1;

int m, n;
int hiderScore = 0;
int seekerScore = 0;

Color lightBrown = {161, 125, 101, 255};
Color darkerBrown = {81, 45, 21, 255};

bool nextRoundFlag = false;

float cubeSize = 1.3f;
float spacing = 0.0f; // Will be set in guiInit()
Cell *grid = NULL;
Vector3 playerPos = {0};
bool inMove = false;
int targetMove = 0;
Rectangle hideActionRec = { 10, 5, 100, 50}; 
Rectangle resetRec;
void guiInit(int screenWidth, int screenHeight) {
    // Set spacing now that cubeSize is initialized
    spacing = cubeSize * 2.0f;

    resetRec = (Rectangle){ screenWidth - 120,10, 100, 50};
    // Set camera position based on angle and radius
    camera.position.x = center.x + radius * sinf(angle);
    camera.position.y = 15.0f;
    camera.position.z = center.z + radius * cosf(angle);

    idleHuman = LoadModel("resources/glbs/idleHuman3.glb");
    runningHuman = LoadModel("resources/glbs/runningHuman3.glb");
    humanHide = LoadModel("resources/glbs/humanHide.glb");
    winningHuman = LoadModel("resources/glbs/winningHuman3.glb");
    losingHuman = LoadModel("resources/glbs/losingHuman3.glb");



    idleHumanAnimations = LoadModelAnimations("resources/glbs/idleHuman3.glb", &idleAnimsCount);
    runningHumanAnimations = LoadModelAnimations("resources/glbs/runningHuman2.glb", &runningAnimsCount);
    humanHideAnimations = LoadModelAnimations("resources/glbs/humanHide.glb", &hideAnimsCount);
    winningHumanAnimations = LoadModelAnimations("resources/glbs/winningHuman3.glb", &hideAnimsCount);
    losingHumanAnimations = LoadModelAnimations("resources/glbs/losingHuman3.glb", &hideAnimsCount);

  
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
    humanState = IDLE; 
}

void runGame(int screenWidth, int screenHeight){
    Vector2 mouse = GetMousePosition();
    float rotateSpeed = 2.0f * DEG2RAD; // radians per frame

    // Camera orbit controls
    if (IsKeyDown(KEY_RIGHT)) angle += rotateSpeed;
    if (IsKeyDown(KEY_LEFT))  angle -= rotateSpeed;
    if (IsKeyDown(KEY_UP))    camera.position.y += 0.3f;
    if (IsKeyDown(KEY_DOWN))  camera.position.y -= 0.3f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, hideActionRec) && (humanState == IDLE || humanState == HIDE) && hideOrSeek) {
            if(humanState == IDLE)
            {
                humanState = HIDE;
                seekerGridChoice =  computer_turn(sizeOfWorld, seekerProbs);
                hiderGridChoice = targetMove;
                nextRound();
            }
            else if(humanState == HIDE)
            {
                humanState = IDLE;
            }
        }else if (CheckCollisionPointRec(mouse, resetRec))
        {
            resetGame();
      
        }
        if (nextRoundFlag && 
            !CheckCollisionPointRec(mouse, resetRec) && 
            !CheckCollisionPointRec(mouse, hideActionRec)) 
        {
            nextRoundFlag = false;
            seekerGridChoice = -1;
        }
        
    } 

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
        nextRoundFlag = false;
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        for (int i = 0; i < n*m; i++) {
            RayCollision hit = GetRayCollisionBox(ray, grid[i].box);
            if (hit.hit && hideOrSeek) {
                humanState = RUN;
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
            }else if (hit.hit && !hideOrSeek) {  //seeker is playing once chose is like you start the game
               hiderGridChoice =  computer_turn(sizeOfWorld, hiderProbs);
               playerPos = grid[hiderGridChoice].pos;
               seekerGridChoice = i;
               nextRound();
            }
        }
    }

    if (inMove) {
        float dx = grid[targetMove].pos.x - playerPos.x;
        float dz = grid[targetMove].pos.z - playerPos.z;
        float step = 0.1f;

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
            humanState = IDLE;
            // Reset rotation for idle
            rotationAngle = 0.0f;
        }
    }

    // Update model animation
    if (humanState == RUN) {
        ModelAnimation anim = runningHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(runningHuman, anim, animCurrentFrame);
    }else if(humanState == IDLE)
    {
        ModelAnimation anim = idleHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(idleHuman, anim, animCurrentFrame);
    }else if(humanState == HIDE)
    {
        ModelAnimation anim = humanHideAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(humanHide, anim, animCurrentFrame);
    }else if (humanState == WIN)
    {
        ModelAnimation anim = winningHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(winningHuman, anim, animCurrentFrame);
    }else if(humanState == LOSE)
    {
        ModelAnimation anim = losingHumanAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(losingHuman, anim, animCurrentFrame);
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

    if (hideOrSeek) //hider palying
    {
       bool hideActionHover =  CheckCollisionPointRec(mouse, hideActionRec);
       DrawModernButton(hideActionRec, "HIDE",hideActionHover, darkBrown );
    }

     bool resetHover =  CheckCollisionPointRec(mouse, resetRec);
    DrawModernButton(resetRec, "RESET",resetHover, darkBrown );
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Hide Score: %d", hiderScore);
    char seekerScoreText[64];
    snprintf(seekerScoreText, sizeof(seekerScoreText), "Seeker Score: %d", seekerScore);


    DrawText(scoreText, screenWidth/2 - MeasureText(scoreText, 20)/2, 10, 25, darkBrown);
    DrawText(seekerScoreText, screenWidth/2 - MeasureText(seekerScoreText, 20)/2, 40, 25, darkBrown);

    BeginMode3D(camera);

    // draw grid cubes
    for (int i = 0; i < n*m; i++) {
        
        Color col = darkBrown;

        if (strcmp(theWorld[i], "hard") == 0)
        {
            col = darkerBrown;
        }else if(strcmp(theWorld[i], "easy") == 0)
        {
            col = lightBrown;
        }
        // highlight cube under cursor
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        RayCollision hit = GetRayCollisionBox(ray, grid[i].box);
        if (hit.hit ) {
            col = GRAY;
        }
        
        if (seekerGridChoice != -1 && i == seekerGridChoice)
        {
            DrawCube(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, BLACK);
        } else {
            DrawCube(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, col);
        }
        DrawCubeWires(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, BLACK);
    }
    if (hideOrSeek || nextRoundFlag)
    {
    Vector3 modelAbove = playerPos;
    modelAbove.y += cubeSize ; // 0.5f offset above the cube
    if(humanState == RUN ){
        DrawModelEx(runningHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }
    else if(humanState == IDLE){
        DrawModelEx(idleHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }
    else if(humanState == HIDE)
    {
        DrawModelEx(humanHide, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }else if(humanState == WIN)
    {
        DrawModelEx(winningHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }else if(humanState == LOSE)
    {
        DrawModelEx(losingHuman, modelAbove, (Vector3){0.0f, 1.0f, 0.0f}, rotationAngle, scale, WHITE);
    }
    }

    EndMode3D();

    EndDrawing();
}


void 
resetGame()
{
    hiderScore = 0;
    seekerScore = 0;
    humanState = IDLE;
    inMove = false;
    targetMove = 0;
    playerPos = grid[0].pos;
    animCurrentFrame = 0;
    firstMenu = 1;
    nextRoundFlag = false;
    hiderGridChoice = -1;
    seekerGridChoice = -1;
}

void nextRound()
{
    if (hiderGridChoice == -1 || seekerGridChoice == -1)
    return;
    
        if (hiderGridChoice != seekerGridChoice)
        {
            humanState = WIN;
            hiderScore += 1;
        }
        else
        {
            humanState = LOSE;
            seekerScore += 1;
        }
 
    inMove = false; 
    nextRoundFlag = true;
}