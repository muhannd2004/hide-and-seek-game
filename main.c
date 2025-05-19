#include "menu.h"
#include "gui.h"
// cc main.c menu.c gui.c -o out -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU && ./out

int main(void)
{

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Welcome To The Hide & Seek Game");
    menuInit();

    SetTargetFPS(30);
    
    while (!WindowShouldClose())
    {
            if (firstMenu)
        {
            showFirstMenu(screenWidth, screenHeight);
        }
        else
        {
         runGame(screenWidth, screenHeight);
    
        }
        
    }
    return 0;
}

