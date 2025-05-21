#ifndef SIMULATE_H
#define SIMULATE_H

#include <stdbool.h>

#define MAX_N 100

#define MAX_LOG_LINES 1000
#define MAX_LOG_LENGTH 128

#define DARKGOLD (Color){184, 134, 11, 255}    

/**
 * Main function to run the hide and seek simulation with visual interface
 * 
 * @param N The number of hiding places
 * @param difficulty Array of difficulty levels for each hiding place ("easy", "neutral", or "hard")
 */
void simulate(int N, const char* difficulty[]);

#endif // SIMULATE_H