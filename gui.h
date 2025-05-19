#ifndef GUI_H
#define GUI_H
#include "raylib.h"

typedef struct {
    Vector3 pos;
    BoundingBox box;
} Cell;

extern float angle ;
extern float radius;
extern Vector3 center;
extern Camera camera;
extern Model idleHuman;
extern Model runningHuman;
extern char humanState;
extern Vector3 position;
extern Vector3 rotationAxis;
extern Vector3 scale;
extern float rotationAngle;
extern int idleAnimsCount;
extern int runningAnimsCount;
extern unsigned int animIndex;
extern unsigned int animCurrentFrame;
extern ModelAnimation *idleHumanAnimations;
extern ModelAnimation *runningHumanAnimations;
extern bool hasAnimations;
extern int n;
extern int m;
extern float spacing;
extern float cubeSize;
extern Cell *grid;
extern Vector3 playerPos;


void guiInit();
void runGame(int screenWidth, int screenHeight);

#endif