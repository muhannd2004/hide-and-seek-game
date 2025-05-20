#include "menu.h"
#include "solve.h"

// 1 for interactive 0 for simulted
//1 for hide,  0 for seek
int firstMenu = 1, activeOrSimulated = 1, hideOrSeek = 1, inputActive = 0, letterCount = 0;
char worldSizeInput[MAX_INPUT_CHARS + 1] = "\0";


int sizeOfWorld = 0;
const char** theWorld = NULL;
int gameMatrix[MAX_N][MAX_N];
double* hiderProbs = NULL;
double* seekerProbs = NULL;

Color darkBrown = {121, 85, 61, 255};


    Rectangle inputBox;
    Rectangle startButton ;
    Rectangle active ;
    Rectangle Simulated ;
    Rectangle hideChoose ;
    Rectangle seekChoose ;


Texture2D mainMenuBackGround;

void menuInit(int screenWidth, int screenHeight) {
    mainMenuBackGround = LoadTexture("resources/MainMenuBackGround.png");

    // Centered horizontally, positioned vertically relative to screen height
    float centerX = screenWidth / 2.0f;
    float inputBoxWidth = 200, inputBoxHeight = 40;
    float buttonWidth = 100, buttonHeight = 40;
    float modeButtonWidth = 140, modeButtonHeight = 50;
    float chooseButtonWidth = 120, chooseButtonHeight = 50;

     inputBox = (Rectangle){ centerX - inputBoxWidth/2, screenHeight * 0.45f, inputBoxWidth, inputBoxHeight };
     startButton = (Rectangle){ centerX - buttonWidth/2, screenHeight * 0.55f, buttonWidth, buttonHeight };
     active = (Rectangle){ centerX - modeButtonWidth - 10, screenHeight * 0.15f, modeButtonWidth, modeButtonHeight };
     Simulated = (Rectangle){ centerX + 10, screenHeight * 0.15f, modeButtonWidth, modeButtonHeight };
     hideChoose = (Rectangle){ centerX - chooseButtonWidth - 10, screenHeight * 0.30f, chooseButtonWidth, chooseButtonHeight };
     seekChoose = (Rectangle){ centerX + 10, screenHeight * 0.30f, chooseButtonWidth, chooseButtonHeight };
}

void showFirstMenu(int screenWidth, int screenHeight)
{
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, inputBox))
            inputActive = 1;
        else if (CheckCollisionPointRec(mouse, startButton)){
            if (strlen(worldSizeInput) > 0)
            {
                sizeOfWorld = atoi(worldSizeInput);
                if (sizeOfWorld > 0)
                {
                    char* difficulty[] = {"neutral", "easy", "hard"};
                    // Allocate memory for theWorld
                    if (theWorld) free(theWorld); // free previous if any
                    theWorld = malloc(sizeOfWorld * sizeof(const char*));
                    srand(time(NULL));
                    for(int i = 0; i < sizeOfWorld; i++){
                        int r = rand() %3;
                        theWorld[i] = difficulty[r];
                    }
                    
                    generate_game_matrix(sizeOfWorld, theWorld, gameMatrix, true);

                    hiderProbs = hider_probability_calculate(sizeOfWorld, gameMatrix);
                    seekerProbs = seeker_probability_calculate(sizeOfWorld, gameMatrix);

                    guiInit(screenWidth, screenHeight);
                    firstMenu = 0;
                }
            }
        }else if(CheckCollisionPointRec(mouse, active)){
            activeOrSimulated = 1;
        }else if (CheckCollisionPointRec(mouse,Simulated))
        {
            activeOrSimulated = 0;
        }else if (activeOrSimulated && CheckCollisionPointRec(mouse, hideChoose))
        {
            hideOrSeek = 1;
        }else if (activeOrSimulated && CheckCollisionPointRec(mouse, seekChoose))
        {
            hideOrSeek = 0;
        }else
            inputActive = 0;
    }

    // takes the char input
    int key = GetCharPressed();
    while (key > 0)
    {
        //must be less than max_input_chars
        if ((key >= 48) && (key <= 57) && letterCount < MAX_INPUT_CHARS)
        {
            worldSizeInput[letterCount++] = (char)key;
            worldSizeInput[letterCount] = '\0';
        }
        key = GetCharPressed();
    }

    //remove from the input
    if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0)
    {
        letterCount--;
        worldSizeInput[letterCount] = '\0';
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

    bool hoveredActive = CheckCollisionPointRec(GetMousePosition(), active);
    DrawModernButton(active, "interactive", hoveredActive, activeOrSimulated ? darkBrown : GRAY);

    bool hoveredSim = CheckCollisionPointRec(GetMousePosition(), Simulated);
    DrawModernButton(Simulated, "simulation", hoveredSim, activeOrSimulated ? GRAY : darkBrown);

    if (activeOrSimulated) {
        bool hoveredHide = CheckCollisionPointRec(GetMousePosition(), hideChoose);
        DrawModernButton(hideChoose, "Hide", hoveredHide, hideOrSeek ? darkBrown : GRAY);

        bool hoveredSeek = CheckCollisionPointRec(GetMousePosition(), seekChoose);
        DrawModernButton(seekChoose, "Seek", hoveredSeek, hideOrSeek ? GRAY : darkBrown);
    }

    DrawModernButton(inputBox, worldSizeInput, false, inputActive ? darkBrown : GRAY );
    bool hoveredStart = CheckCollisionPointRec(GetMousePosition(), startButton);
    DrawModernButton(startButton, "START", hoveredStart, darkBrown);

    EndDrawing();
}

void DrawModernButton(Rectangle bounds, const char *text, bool hovered, Color baseColor)
{
    // Colors based on the image style
    Color borderColor = (Color){ 226, 151, 93, 255 };   // Orange/tan border
    Color textColor = (Color){ 237, 224, 173, 255 };    // Golden text
    
    // Hover effect (slightly adjusts the base color)
    Color currentColor = hovered ? ColorBrightness(baseColor, 0.2f) : baseColor;
    
    // Draw border (outer rounded rectangle)
    DrawRectangleRounded(
        (Rectangle){ bounds.x - 4, bounds.y - 4, bounds.width + 8, bounds.height + 8 }, 
        0.3f, 15, borderColor);
    
    // Draw button background
    DrawRectangleRounded(bounds, 0.3f, 15, currentColor);
    
    // Draw centered text
    int fontSize = 20;  // Adjust font size based on button height
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    DrawText(text,
             bounds.x + (bounds.width - textSize.x)/2,
             bounds.y + (bounds.height - textSize.y)/2,
             fontSize,
             textColor);
}
