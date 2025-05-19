#ifndef MENU_H
#define MENU_H


#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define MAX_INPUT_CHARS 3
extern int firstMenu;
extern int activeOrSimulated;
extern int hideOrSeek;
extern int inputActive;
extern int letterCount;
extern char worldSizeInput[];
extern int sizeOfWorld;
extern int *theWorld;
extern Color darkBrown;

// Input boxes and buttons
extern Rectangle inputBox;
extern Rectangle startButton;
extern Rectangle active;
extern Rectangle Simulated;
extern Rectangle hideChoose;
extern Rectangle seekChoose;

// Background image
extern Texture2D mainMenuBackGround;


//menu function
void DrawModernButton(Rectangle bounds, const char *text, bool hovered, Color baseColor);
void menuInit();
void showFirstMenu(int screenWidth, int screenHeight);

#endif