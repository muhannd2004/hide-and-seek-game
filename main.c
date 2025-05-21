#include "menu.h"
#include "gui.h"
#include "solve.h"
#include "simulate.h"
#include <time.h>
// gcc main.c menu.c gui.c simulate.c solve.c -o out -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU -lglpk && ./out
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
            int N = sizeOfWorld; // Use the world size from menu or gui
            const char* difficulty[MAX_N];
            srand(time(NULL)); // Seed the random number generator
            difficulty_create(N, difficulty); // Generate random difficulties
            
            simulate(N, difficulty);    
        }
    }
    return 0;
}

