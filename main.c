#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_INPUT_CHARS 3



void DrawModernButton(Rectangle bounds, const char *text, bool hovered, Color baseColor);

int main(void)
{

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Welcome To The Hide & Seek Game");

    int sizeOfWorld = 0;
    char firstMenu = 1 , secondMenu = 1, inputActive = 0;
    char worldSizeInput[MAX_INPUT_CHARS + 1] = "\0";
    int *theWorld = NULL;
    int letterCount = 0;
    Rectangle inputBox = { 300, 270, 200, 40 };
    Rectangle startButton = { 350, 340, 100, 40 };
    
    Rectangle active = { 250, 50, 140, 50 };
    Rectangle Simulated = { 430, 50, 140, 50 };

    Rectangle hideChoose = {260, 140, 120, 50};
    Rectangle seekChoose = {440, 140, 120, 50};
    Color darkBrown = (Color){ 121, 85, 61, 255 };  
    // 1 for interactive 0 for simulted
    //1 for hide 0 for seek
    char activeOrSimulated = 1 , hideOrSeek = 1; 

    Texture2D mainMenuBackGround = LoadTexture("resources/MainMenuBackGround.png");

    
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        
        if (firstMenu)
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
                            theWorld = (int*)malloc(sizeOfWorld * sizeof(int));
                            memset(theWorld, 0, sizeOfWorld * sizeof(int));
                            firstMenu = 0;

                            for(int i = 0; i < sizeOfWorld; i++){
                                theWorld[i] = (rand() %5) - 2;
                            }
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
       

            // --- SIMULATION CHOICE --
            bool hoveredSim = CheckCollisionPointRec(GetMousePosition(), Simulated);
            DrawModernButton(Simulated, "simulation", hoveredSim, activeOrSimulated ? GRAY : darkBrown);
     

            // --- HIDE/SEEK if INTERACTIVE ---
            if (activeOrSimulated) {
                
                bool hoveredHide = CheckCollisionPointRec(GetMousePosition(), hideChoose);
                DrawModernButton(hideChoose, "Hide", hoveredHide, hideOrSeek ? darkBrown : GRAY);

                bool hoveredSeek = CheckCollisionPointRec(GetMousePosition(), seekChoose);
                DrawModernButton(seekChoose, "Seek", hoveredSeek, hideOrSeek ? GRAY : darkBrown);
                 
                
            }

            //   // Draw input box
            // DrawRectangleRec(inputBox, inputActive ? darkBrown : GRAY);
            // DrawRectangleLines((int)inputBox.x, (int)inputBox.y, (int)inputBox.width, (int)inputBox.height, DARKGRAY);
            // DrawText(worldSizeInput, (int)inputBox.x + 10, (int)inputBox.y + 10, 20, BLACK);

            DrawModernButton(inputBox, worldSizeInput, false,inputActive ? darkBrown : GRAY );
            // --- START BUTTON ---
            bool hoveredStart = CheckCollisionPointRec(GetMousePosition(), startButton);
            DrawModernButton(startButton, "START", hoveredStart, darkBrown);


            EndDrawing();
        }
        else
        {
           

    
        }

        
        
    }
    

    return 0;
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