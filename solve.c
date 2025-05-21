#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>
#include "solve.h"





void difficulty_create(int N, const char* difficulty[]){
    // Function to randomly assign difficulty levels
    const char* levels[] = {"easy", "neutral", "hard"};
    for (int i = 0; i < N; i++) {
        // Randomly select one of the three difficulty levels
        int random_index = rand() % 3;
        difficulty[i] = levels[random_index];
    }
}

void generate_game_matrix(int N, const char* difficulty[], int matrix[MAX_N][MAX_N], bool is_hider) {
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
}

double* seeker_probability_calculate(int N, int matrix[MAX_N][MAX_N]) {
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
        printf("Place %d: %.3f\n", i - 1, probabilities[i - 1]);
    }

    glp_delete_prob(lp);
    return probabilities;
}

double* hider_probability_calculate(int N, int matrix[MAX_N][MAX_N]) {
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
    printf("\nOptimal game value for the hider: %f\n", v);

    double* probabilities = malloc(N * sizeof(double));
    if (probabilities == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    printf("Strategy probabilities:\n");
    for (int j = 1; j <= N; j++) {
        probabilities[j - 1] = glp_get_col_prim(lp, j);
        printf("Place %d: %.3f\n", j - 1, probabilities[j - 1]);
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

