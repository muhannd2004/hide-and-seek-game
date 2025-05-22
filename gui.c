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


Color color1 = {168, 101, 35, 255};  // #A86523
Color color2 = {233, 163, 25, 255};  // #E9A319
Color color3 = {250, 213, 154, 255}; // #FAD59A
Color color4 = {252, 239, 203, 255}; // #FCEFCB


bool nextRoundFlag = false;

// stat menu 
bool statMenu = false;
Rectangle statMenuRec = { 10, 10, 130, 50};
bool seekerChoiceTurn = false;

float cubeSize = 1.3f;
float spacing = 0.0f; // Will be set in guiInit()
Cell *grid = NULL;
Vector3 playerPos = {0};
bool inMove = false;
int targetMove = 0;

Rectangle hideActionRec = { 10, 100, 100, 50}; 

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

    if (IsKeyDown(KEY_H) && humanState != RUN)
    {
         if(humanState == IDLE)
            {
                humanState = HIDE;
                srand(time(NULL));

                seekerGridChoice =  computer_turn(sizeOfWorld, seekerProbs);
                hiderGridChoice = targetMove;
                nextRound();
            }
            else if(humanState == HIDE)
            {
                humanState = IDLE;
            }

    }
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        seekerChoiceTurn = false;
        // Stat menu open/close logic
        if (CheckCollisionPointRec(mouse, statMenuRec)) {
            statMenu = !statMenu;
        }
        if (CheckCollisionPointRec(mouse, hideActionRec) && (humanState == IDLE || humanState == HIDE) && hideOrSeek) {
            if(humanState == IDLE)
            {
                humanState = HIDE;
                srand(time(NULL));

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
    // Only zoom if mouse is NOT over stats menu
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    if (!(statMenu && mouseX >= screenWidth - screenWidth/3 && mouseY >= 0 && mouseY < screenHeight)) {
        if (wheel != 0.0f) {
            radius -= wheel * zoomSpeed;
            if (radius < 5.0f) radius = 5.0f;         // Prevent zooming in too close
            if (radius > 100.0f) radius = 100.0f;     // Prevent zooming out too far
        }
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
            }else if (hit.hit && !hideOrSeek) {  //seeker is playing once chose is like you start the game
                srand(time(NULL));
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

        float targetAngle = rotationAngle;
        if (fabsf(dx) > step || fabsf(dz) > step) {
            if (fabsf(dx) > fabsf(dz)) {
                if (dx > 0)
                    targetAngle = 90.0f;
                else
                    targetAngle = -90.0f;
            } else {
                if (dz > 0)
                    targetAngle = 0.0f;
                else
                    targetAngle = 180.0f;
            }
            float rotationSpeed = 8.0f; // degrees per frame
            float diff = targetAngle - rotationAngle;
            while (diff > 180.0f) diff -= 360.0f;
            while (diff < -180.0f) diff += 360.0f;
            if (fabsf(diff) < rotationSpeed)
                rotationAngle = targetAngle;
            else
                rotationAngle += (diff > 0 ? rotationSpeed : -rotationSpeed);
            if (rotationAngle > 180.0f) rotationAngle -= 360.0f;
            if (rotationAngle < -180.0f) rotationAngle += 360.0f;
        }

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

    bool statMenuHover =  CheckCollisionPointRec(mouse, statMenuRec);
    DrawModernButton(statMenuRec, "Stats Menu",statMenuHover, darkBrown );


    bool resetHover =  CheckCollisionPointRec(mouse, resetRec);
    DrawModernButton(resetRec, "RESET",resetHover, darkBrown );
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Hider Score: %d", hiderScore);
    char seekerScoreText[64];
    snprintf(seekerScoreText, sizeof(seekerScoreText), "Seeker Score: %d", seekerScore);


    DrawText(scoreText, screenWidth/2 - MeasureText(scoreText, 20)/2, 10, 25, darkBrown);
    DrawText(seekerScoreText, screenWidth/2 - MeasureText(seekerScoreText, 20)/2, 40, 25, darkBrown);

    if (statMenu)
    {
        drawStatsMenu();
    }
    

    BeginMode3D(camera);

    // draw grid cubes
    for (int i = 0; i < n*m; i++) {
        
        Color col = color3;

        if (strcmp(theWorld[i], "hard") == 0)
        {
            col = color1;
        }else if(strcmp(theWorld[i], "easy") == 0)
        {
            col = color4;
        }
        // highlight cube under cursor
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        RayCollision hit = GetRayCollisionBox(ray, grid[i].box);
        if (hit.hit ) {
            col = GRAY;
        }
        
        if (seekerChoiceTurn && seekerGridChoice != -1 && i == seekerGridChoice)
        {
            DrawCube(grid[i].pos, cubeSize*2, cubeSize*2, cubeSize*2, color2);
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

    bool caught = (hiderGridChoice == seekerGridChoice);
    
    if (caught) {
        // Seeker caught the hider
        
        if (hideOrSeek) {
            int catchScore = gameMatrix[hiderGridChoice][seekerGridChoice];
            // Player is hider, computer is seeker
            hiderScore -= abs(catchScore);
            seekerScore += abs(catchScore);
        } else {
            int catchScore = gameMatrix[seekerGridChoice][hiderGridChoice];
            // Player is seeker, computer is hider
            seekerScore += abs(catchScore);
            hiderScore -= abs(catchScore);
        }
        humanState = LOSE;
    } else {
        // Seeker missed the hider
        
        if (hideOrSeek) {
            int missScore = gameMatrix[hiderGridChoice][seekerGridChoice];
            // Player is hider, computer is seeker
            hiderScore += abs(missScore);
            seekerScore -= abs(missScore);
        } else {
            int missScore = gameMatrix[seekerGridChoice][hiderGridChoice];
            // Player is seeker, computer is hider
            hiderScore += abs(missScore);
            seekerScore -= abs(missScore);
        }
        humanState = WIN;
    }

    seekerChoiceTurn = true;
    inMove = false; 
    nextRoundFlag = true;
}

void drawStatsMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int statsWidth = screenWidth / 3;
    int statsX = screenWidth - statsWidth;
    int padding = 20;
    int lineHeight = 24;
    int fontSize = 18;

    int matrixRows = sizeOfWorld;
    int matrixCols = sizeOfWorld;
    int rowHeight = fontSize + 2;
    int scrollBarWidth = 16;

    int contentHeight = 0;
    contentHeight += lineHeight + 8; // Title
    contentHeight += matrixRows * rowHeight;
    contentHeight += lineHeight;
    contentHeight += lineHeight; // "Hider Probabilities:"
    contentHeight += matrixRows * (fontSize + 2);
    contentHeight += lineHeight;
    contentHeight += lineHeight; // "Seeker Probabilities:"
    contentHeight += matrixRows * (fontSize + 2);

    int visibleHeight = screenHeight - 2 * padding;

    int charWidth = fontSize / 2 + 18; 
    int matrixAreaWidth = statsWidth - 2 * padding - scrollBarWidth;
    int matrixContentWidth = matrixCols * charWidth;
    int maxScrollX = (matrixContentWidth > matrixAreaWidth) ? (matrixContentWidth - matrixAreaWidth) : 0;

    static int scrollY = 0;
    static int scrollX = 0;
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    float wheel = GetMouseWheelMove();

    bool mouseOverStats = (mouseX >= statsX && mouseX < statsX + statsWidth && mouseY >= 0 && mouseY < screenHeight);

    // Shift+wheel for horizontal scroll, normal wheel for vertical

    // Draw background
    DrawRectangle(statsX, 0, statsWidth, screenHeight, color3);
    DrawRectangleLines(statsX, 0, statsWidth, screenHeight, DARKGRAY);

    // --- Color legend ---
    int legendY = padding;
    int legendX = statsX + padding;
    int legendBox = 22;
    int legendGap = 10;
    DrawText("Legend:", legendX, legendY, fontSize, BLACK);
    legendY += lineHeight;
    contentHeight += legendY;
    // Neutral
    DrawRectangle(legendX, legendY, legendBox, legendBox, color3);
    DrawRectangleLines(legendX, legendY, legendBox, legendBox, DARKGRAY);
    DrawText("Neutral", legendX + legendBox + legendGap, legendY + 2, fontSize - 2, BLACK);
    legendY += legendBox + 4;
    contentHeight += legendY;

    // Hard
    DrawRectangle(legendX, legendY, legendBox, legendBox, color1);
    DrawRectangleLines(legendX, legendY, legendBox, legendBox, DARKGRAY);
    DrawText("Hard", legendX + legendBox + legendGap, legendY + 2, fontSize - 2, BLACK);
    legendY += legendBox + 4;
    contentHeight += legendY;

    // Easy
    DrawRectangle(legendX, legendY, legendBox, legendBox, color4);
    DrawRectangleLines(legendX, legendY, legendBox, legendBox, DARKGRAY);
    DrawText("Easy", legendX + legendBox + legendGap, legendY + 2, fontSize - 2, BLACK);
    legendY += legendBox + 8;
    contentHeight += legendY;

    int maxScrollY = (contentHeight > visibleHeight) ? (contentHeight - visibleHeight) : 0;

    if (mouseOverStats) {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            if (wheel != 0) {
                scrollX -= (int)(wheel * charWidth * 4);
                if (scrollX < 0) scrollX = 0;
                if (scrollX > maxScrollX) scrollX = maxScrollX;
            }
        } else {
            if (wheel != 0) {
                scrollY -= (int)(wheel * rowHeight * 2);
                if (scrollY < 0) scrollY = 0;
                if (scrollY > maxScrollY) scrollY = maxScrollY;
            }
        }
    }

    // --- Scissor for the whole stats panel content ---
    Rectangle statsClip = {statsX + padding, legendY, matrixAreaWidth, visibleHeight - (legendY - padding)};
    BeginScissorMode(statsClip.x, statsClip.y, statsClip.width, statsClip.height);

    int y = legendY - scrollY;

    DrawText("Game Matrix", statsX + padding, y, fontSize + 4, color1);
    y += lineHeight + 8;

    for (int i = 0; i < matrixRows; i++) {
        int x = statsX + padding - scrollX;
        for (int j = 0; j < matrixCols; j++) {
            // Determine color for cell background
            Color bgCol = color3;
            if (strcmp(theWorld[i], "hard") == 0) bgCol = color1;
            else if (strcmp(theWorld[i], "easy") == 0) bgCol = color4;
            // Draw colored background for each cell
            DrawRectangle(x, y, charWidth, rowHeight, Fade(bgCol, 0.5f));
            DrawRectangleLines(x, y, charWidth, rowHeight, DARKGRAY);

            char cell[12];
            snprintf(cell, sizeof(cell), "%4d", gameMatrix[i][j]);
            DrawText(cell, x-0.2 , y + 1, fontSize, BLACK);
            x += charWidth;
        }
        y += rowHeight;
    }

    y += lineHeight / 2;
    DrawLine(statsX + padding, y, statsX + statsWidth - padding, y, GRAY);
    y += lineHeight;

    DrawText("Hider Probabilities:", statsX + padding, y, fontSize, color1);
    y += lineHeight;
    for (int i = 0; i < matrixRows; i++) {
        char prob[64];
        snprintf(prob, sizeof(prob), "H%d: %.3f", i, hiderProbs ? hiderProbs[i] : 0.0);
        DrawText(prob, statsX + padding, y, fontSize - 2, BLACK);
        y += fontSize + 2;
    }

    y += lineHeight / 2;
    DrawLine(statsX + padding, y, statsX + statsWidth - padding, y, GRAY);
    y += lineHeight;

    DrawText("Seeker Probabilities:", statsX + padding, y, fontSize, color1);
    y += lineHeight;
    for (int i = 0; i < matrixRows; i++) {
        char prob[64];
        snprintf(prob, sizeof(prob), "S%d: %.3f", i, seekerProbs ? seekerProbs[i] : 0.0);
        DrawText(prob, statsX + padding, y, fontSize - 2, BLACK);
        y += fontSize + 2;
    }



    EndScissorMode();

    // Draw vertical scrollbar
    if (maxScrollY > 0) {
        float barHeight = (float)(visibleHeight - (legendY - padding)) * (visibleHeight - (legendY - padding)) / contentHeight;
        float barY = legendY + ((float)scrollY / contentHeight) * (visibleHeight - (legendY - padding));
        DrawRectangle(statsX + statsWidth - scrollBarWidth, legendY, scrollBarWidth, visibleHeight - (legendY - padding), LIGHTGRAY);
        DrawRectangle(statsX + statsWidth - scrollBarWidth, barY, scrollBarWidth, barHeight, GRAY);
    }
    // Draw horizontal scrollbar
    if (maxScrollX > 0) {
        float barWidth = (float)matrixAreaWidth * matrixAreaWidth / matrixContentWidth;
        float barX = statsX + padding + ((float)scrollX / matrixContentWidth) * matrixAreaWidth;
        int barY = legendY + (visibleHeight - (legendY - padding)) - scrollBarWidth;
        DrawRectangle(statsX + padding, barY, matrixAreaWidth, scrollBarWidth, LIGHTGRAY);
        DrawRectangle(barX, barY, barWidth, scrollBarWidth, GRAY);
    }
}