#ifndef SOLVE_H
#define SOLVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>

#define MAX_N 100

/**
 * Generates game matrix for hide and seek based on difficulty levels
 * @param N Number of hiding places
 * @param difficulty Array of difficulty strings for each place
 * @param matrix Output matrix to store payoffs
 * @param is_hider Whether the matrix is from hider's perspective
 */
void generate_game_matrix(int N, const char* difficulty[], int matrix[MAX_N][MAX_N], bool is_hider);

/**
 * Calculate optimal probabilities for seeker's mixed strategy
 * @param N Number of hiding places
 * @param matrix Game matrix
 * @return Array of probabilities for each hiding place
 */
double* seeker_probability_calculate(int N, int matrix[MAX_N][MAX_N]);

/**
 * Calculate optimal probabilities for hider's mixed strategy
 * @param N Number of hiding places
 * @param matrix Game matrix
 * @return Array of probabilities for each hiding place
 */
double* hider_probability_calculate(int N, int matrix[MAX_N][MAX_N]);

/**
 * Determine computer's move based on given probability distribution
 * @param N Number of hiding places
 * @param probabilities Probability distribution for making a move
 * @return Chosen hiding place
 */
int computer_turn(int N, double* probabilities);


#endif /* SOLVE_H */