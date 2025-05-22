#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>
#include "solve.h"





void shuffle(const char** array, int N) {
    // First pass of shuffling
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        const char* temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
    
    // Second pass of shuffling to further randomize and avoid clustering
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        const char* temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void difficulty_create(int N, const char* difficulty[]) {
    const char* levels[] = {"easy", "neutral", "hard"};
    
    // Handle special cases for small N
    if (N <= 0) return;
    
    if (N == 1) {
        difficulty[0] = levels[rand() % 3]; // Pick one random difficulty
        return;
    }
    
    if (N == 2) {
        difficulty[0] = levels[rand() % 3];
        difficulty[1] = levels[rand() % 3];
        return;
    }
    
    // For N >= 3, distribute difficulties evenly
    int count_per_level = N / 3;
    int remainder = N % 3;
    int index = 0;
    
    // Distribute the base counts first
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < count_per_level; j++) {
            difficulty[index++] = levels[i];
        }
    }
    
    // Distribute the remainder
    // For example, if N=8, we have 2 "easy", 2 "neutral", 2 "hard", 
    // and 2 remainders which would go to "easy" and "neutral"
    for (int i = 0; i < remainder; i++) {
        difficulty[index++] = levels[((rand() % 3)+1)%3];//reduces number of easy places
    }
    
    // Shuffle the array to randomize placement
    shuffle(difficulty, N);
    
    // Debug: print the distribution
    int counts[3] = {0};
    for (int i = 0; i < N; i++) {
        if (strcmp(difficulty[i], "easy") == 0) counts[0]++;
        else if (strcmp(difficulty[i], "neutral") == 0) counts[1]++;
        else if (strcmp(difficulty[i], "hard") == 0) counts[2]++;
    }
    printf("Difficulty distribution: easy=%d, neutral=%d, hard=%d\n", 
           counts[0], counts[1], counts[2]);
}

void generate_game_matrix(int N, const char* difficulty[], int matrix[MAX_N][MAX_N], bool is_hider) {
int keeper = (int)sqrt(N);
while (N % keeper != 0) keeper--;

int m = keeper;
int n = N / keeper;

if (n == 1 || m == 1) {
    for (int h = 0; h < N; h++) {
        for (int s = 0; s < N; s++) {
            int dist = abs(h - s);
            int base = 0;

            if (strcmp(difficulty[h], "hard") == 0) {
                if (dist == 0)
                    base = -12;
                else if (dist == 1)
                    base = 2;
                else if (dist == 2)
                    base = 3;
                else
                    base = 4;
            } else if (strcmp(difficulty[h], "neutral") == 0) {
                if (dist == 0)
                    base = -4;
                else if (dist == 1)
                    base = 2;
                else if (dist == 2)
                    base = 3;
                else
                    base = 4;
            } else { // "easy"
                if (dist == 0)
                    base = -4; 
                else if (dist == 1)
                    base = 4;
                else if (dist == 2)
                    base = 6;
                else
                    base = 8;
            }

            // Flip sign if computing from seeker's perspective
            if (!is_hider) base *= -1;

            matrix[h][s] = base;
        }
    }
    if (!is_hider) {
            for (int h = 0; h < N; h++) {
                for (int s = h + 1; s < N; s++) {
                    int temp = matrix[h][s];
                    matrix[h][s] = matrix[s][h];
                    matrix[s][h] = temp;
                }
            }
        }


    }else{
        const int sign[2] = {-1,1};
        for (int h = 0; h < N; h++) {
            for (int s = 0; s < N; s++) {
                if (h == s) {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = -3 * sign[is_hider];
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = -1 * sign[is_hider];
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = -1 * sign[is_hider];
                } else {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = 1 * sign[is_hider];
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = 1 * sign[is_hider];
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = 2 * sign[is_hider];
                }
            }
        }
        if (!is_hider) {
            for (int h = 0; h < N; h++) {
                for (int s = h + 1; s < N; s++) {
                    int temp = matrix[h][s];
                    matrix[h][s] = matrix[s][h];
                    matrix[s][h] = temp;
                }
            }
        }
    }
}

// void generate_game_matrix(int N, const char* difficulty[], int matrix[MAX_N][MAX_N], bool is_hider) {
//     const int sign[2] = {-1, 1};

//     // Derive n x m grid from N
//     int keeper = (int)sqrt(N);
//     while (N % keeper != 0) keeper--;

//     int m= keeper;
//     int n= N/keeper;

//     for (int h = 0; h < N; h++) {
//         int hi = h / m, hj = h % m;
//         for (int s = 0; s < N; s++) {
//             int si = s / m, sj = s % m;
//             int dist = abs(hi - si) + abs(hj - sj);

//             int base;
//             if (h == s) {
//                 // Scaled base payoffs (multiplied by 4)
//                 if      (strcmp(difficulty[h], "hard")    == 0) base = -12 * sign[is_hider];
//                 else if (strcmp(difficulty[h], "neutral") == 0) base = -4 * sign[is_hider];
//                 else                                           base = -4 * sign[is_hider];
//             } else {
//                 if      (strcmp(difficulty[h], "hard")    == 0) base =  4 * sign[is_hider];
//                 else if (strcmp(difficulty[h], "neutral") == 0) base =  4 * sign[is_hider];
//                 else                                           base =  8 * sign[is_hider];
//             }

//             // Apply distance penalty
//             if (dist == 1) {
//                 base = (base * 3) / 4;  // 0.75
//             } else if (dist == 2) {
//                 base = base / 2;       // 0.5
//             } else if (dist > 2) {
//                 base = 0;              // Beyond 2 tiles, no reward or penalty
//             }

//             matrix[h][s] = base;
//         }
//     }
// }



double* probability_calculate(int N, int matrix[MAX_N][MAX_N]) {
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MAX);

    // Variables: x1..xN (hider strategy), v (game value)
    glp_add_cols(lp, N + 1);
    for (int i = 1; i <= N; i++) {
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // x_i >= 0
    }
    glp_set_col_bnds(lp, N + 1, GLP_FR, 0.0, 0.0); // v free

    // Objective: maximize v
    glp_set_obj_coef(lp, N + 1, 1.0);

    // Add N + 1 constraints: N for seeker's strategies, 1 for normalization
    glp_add_rows(lp, N + 1);

    int ia[1 + MAX_N * (MAX_N + 2)];
    int ja[1 + MAX_N * (MAX_N + 2)];
    double ar[1 + MAX_N * (MAX_N + 2)];
    int idx = 1;

    // Constraints: sum a_ij * x_i ≥ v   for each seeker strategy j
    for (int j = 0; j < N; j++) {
        glp_set_row_bnds(lp, j + 1, GLP_LO, 0.0, 0.0); // LHS ≥ 0

        for (int i = 0; i < N; i++) {
            ia[idx] = j + 1;
            ja[idx] = i + 1;
            ar[idx] = matrix[i][j];
            idx++;
        }
        ia[idx] = j + 1;
        ja[idx] = N + 1;
        ar[idx] = -1.0;
        idx++;
    }

    // Constraint: x1 + x2 + ... + xN = 1 (normalization)
    glp_set_row_bnds(lp, N + 1, GLP_FX, 1.0, 1.0);
    for (int i = 0; i < N; i++) {
        ia[idx] = N + 1;
        ja[idx] = i + 1;
        ar[idx] = 1.0;
        idx++;
    }

    glp_load_matrix(lp, idx - 1, ia, ja, ar);

    int status = glp_simplex(lp, NULL);
    if (status != 0) {
        fprintf(stderr, "GLPK simplex solver failed with code %d\n", status);
        exit(1);
    }

    int sol_status = glp_get_status(lp);
    if (sol_status != GLP_OPT) {
        fprintf(stderr, "No optimal solution found (status: %d)\n", sol_status);
        exit(1);
    }

    double v = glp_get_obj_val(lp);
    printf("\nOptimal game value for the seeker: %f\n", v);

    // Allocate memory for the probabilities
    double* probabilities = (double*)malloc(N * sizeof(double));
    if (probabilities == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    printf("Strategy probabilities:\n");
    for (int i = 1; i <= N; i++) {
        probabilities[i - 1] = glp_get_col_prim(lp, i);
    }

    glp_delete_prob(lp);
    return probabilities;
}

double* dual_probability_calculate(int N, int matrix[MAX_N][MAX_N]) {
    glp_prob *lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MIN); // Seeker minimizes game value

    glp_add_cols(lp, N + 1); // y1..yN (seeker strategy), v
    for (int j = 1; j <= N; j++) {
        glp_set_col_bnds(lp, j, GLP_LO, 0.0, 0.0); // y_j ≥ 0
    }
    glp_set_col_bnds(lp, N + 1, GLP_FR, 0.0, 0.0); // v free
    glp_set_obj_coef(lp, N + 1, 1.0); // minimize v

    glp_add_rows(lp, N + 1); // N constraints + 1 normalization

    int ia[1 + MAX_N * (MAX_N + 2)];
    int ja[1 + MAX_N * (MAX_N + 2)];
    double ar[1 + MAX_N * (MAX_N + 2)];
    int idx = 1;

    // Constraints: sum a_ij * y_j ≤ v  for each hider strategy i
    for (int i = 0; i < N; i++) {
        glp_set_row_bnds(lp, i + 1, GLP_UP, 0.0, 0.0); // LHS ≤ 0

        for (int j = 0; j < N; j++) {
            ia[idx] = i + 1;
            ja[idx] = j + 1;
            ar[idx] = matrix[i][j]; // Note: matrix is a_ij
            idx++;
        }
        ia[idx] = i + 1;
        ja[idx] = N + 1;
        ar[idx] = -1.0;
        idx++;
    }

    // Constraint: sum y_j = 1
    glp_set_row_bnds(lp, N + 1, GLP_FX, 1.0, 1.0);
    for (int j = 0; j < N; j++) {
        ia[idx] = N + 1;
        ja[idx] = j + 1;
        ar[idx] = 1.0;
        idx++;
    }

    glp_load_matrix(lp, idx - 1, ia, ja, ar);

    if (glp_simplex(lp, NULL) != 0 || glp_get_status(lp) != GLP_OPT) {
        fprintf(stderr, "Failed to solve dual problem\n");
        exit(1);
    }
    double v = glp_get_obj_val(lp);


    double* probabilities = malloc(N * sizeof(double));
    if (probabilities == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    printf("Strategy probabilities:\n");
    for (int j = 1; j <= N; j++) {
        probabilities[j - 1] = glp_get_col_prim(lp, j);
    }

    glp_delete_prob(lp);
    return probabilities;
}

int computer_turn(int N, double* probabilities) {
    // Check if probabilities sum to zero
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += probabilities[i];
    }
    
    if (sum <= 0.0) {
        // If all probabilities are zero, choose randomly
        return rand() % N;
    }
    
    // Generate a random number between 0 and 1
    double random_value = (double)rand() / RAND_MAX;
    double cumulative_probability = 0.0;
    
    // Find which interval the random value falls into
    for (int i = 0; i < N; i++) {
        cumulative_probability += probabilities[i];
        if (random_value <= cumulative_probability) {
            return i;
        }
    }
    
    // Fallback (should rarely happen due to floating point precision)
    return N - 1;
}

