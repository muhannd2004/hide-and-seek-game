#include "menu.h"
#include "gui.h"
#include "solve.h"
// gcc main.c menu.c gui.c -o out -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU && ./out

int main(void)
{

    const int screenWidth = 800 * 2;
    const int screenHeight = 450 * 2;

    InitWindow(screenWidth, screenHeight, "Welcome To The Hide & Seek Game");
    menuInit( screenWidth,  screenHeight);

    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
            if (firstMenu)
        {
            showFirstMenu(screenWidth, screenHeight);
        }
        else if (activeOrSimulated)
        
        {
         runGame(screenWidth, screenHeight);
    
        }else if (!activeOrSimulated)
        {
                
        }
        
    }
    return 0;
}

