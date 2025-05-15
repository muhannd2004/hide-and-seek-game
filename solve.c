#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>

#define MAX_N 100

void generate_game_matrix(int N, const char* difficulty[], int matrix[MAX_N][MAX_N], bool is_hider) {
    if(is_hider){
        for (int h = 0; h < N; h++) {
            for (int s = 0; s < N; s++) {
                if (h == s) {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = -3;
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = -1;
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = -1;
                } else {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = 1;
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = 1;
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = 2;
                }
            }
        }
    }else{
        for (int h = 0; h < N; h++) {
            for (int s = 0; s < N; s++) {
                if (h == s) {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = 3;
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = 1;
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = 1;
                } else {
                    if (strcmp(difficulty[h], "hard") == 0)
                        matrix[h][s] = -1;
                    else if (strcmp(difficulty[h], "neutral") == 0)
                        matrix[h][s] = -1;
                    else if (strcmp(difficulty[h], "easy") == 0)
                        matrix[h][s] = -2;
                }
            }
        }
    }
}

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
    printf("\nOptimal hider game value: %f\n", v);

    // Allocate memory for the probabilities
    double* probabilities = (double*)malloc(N * sizeof(double));
    if (probabilities == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    printf("Hider strategy probabilities:\n");
    for (int i = 1; i <= N; i++) {
        probabilities[i - 1] = glp_get_col_prim(lp, i);
        printf("Place %d: %.3f\n", i - 1, probabilities[i - 1]);
    }

    glp_delete_prob(lp);
    return probabilities;
}

double* probability_calculate_dual(int N, int matrix[MAX_N][MAX_N]) {
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
    printf("\nOptimal seeker game value: %f\n", v);

    double* probabilities = malloc(N * sizeof(double));
    printf("Seeker strategy probabilities:\n");
    for (int j = 1; j <= N; j++) {
        probabilities[j - 1] = glp_get_col_prim(lp, j);
        printf("Place %d: %.3f\n", j - 1, probabilities[j - 1]);
    }

    glp_delete_prob(lp);
    return probabilities;
}




int computer_turn(int N, double* probabilities) {
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


void simulate(int N, const char* difficulty[]) {
    int matrix_hider[MAX_N][MAX_N];
    int matrix_seeker[MAX_N][MAX_N];
    int hider_score = 0, seeker_score = 0;
    int rounds = 100;
    
    // Generate game matrices for both perspectives
    generate_game_matrix(N, difficulty, matrix_hider, true);  // Hider's perspective
    generate_game_matrix(N, difficulty, matrix_seeker, false); // Seeker's perspective
    
    printf("Hider's Game Matrix:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%3d ", matrix_hider[i][j]);
        printf("\n");
    }
    
    printf("\nSeeker's Game Matrix:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%3d ", matrix_seeker[i][j]);
        printf("\n");
    }
    
    // Calculate optimal strategies
    double* hider_probs = probability_calculate(N, matrix_hider);
    double* seeker_probs = probability_calculate_dual(N, matrix_hider);
    
    printf("\nStarting %d round simulation...\n", rounds);
    printf("------------------------------------\n");
    
    for (int round = 1; round <= rounds; round++) {
        // Get moves for both players based on their optimal probabilities
        int hider_place = computer_turn(N, hider_probs);
        int seeker_place = computer_turn(N, seeker_probs);
        
        // Determine outcome and scores
        int round_score = matrix_hider[hider_place][seeker_place];
        
        if (hider_place == seeker_place) {
            // Seeker found the hider
            seeker_score += matrix_seeker[hider_place][seeker_place];
            hider_score -= matrix_seeker[hider_place][seeker_place];
            
            printf("Round %3d: Hider chose place %d, Seeker chose place %d - Seeker wins! (Score: %+d)\n", 
                   round, hider_place, seeker_place, matrix_seeker[hider_place][seeker_place]);
        } else {
            // Hider escaped
            hider_score += matrix_hider[hider_place][seeker_place];
            seeker_score -= matrix_hider[hider_place][seeker_place];
            
            printf("Round %3d: Hider chose place %d, Seeker chose place %d - Hider wins! (Score: %+d)\n", 
                   round, hider_place, seeker_place, matrix_hider[hider_place][seeker_place]);
        }
    }
    
    printf("\nSimulation Results:\n");
    printf("------------------------------------\n");
    printf("Total rounds: %d\n", rounds);
    printf("Hider's final score: %d (avg per round: %.2f)\n", 
           hider_score, (float)hider_score / rounds);
    printf("Seeker's final score: %d (avg per round: %.2f)\n", 
           seeker_score, (float)seeker_score / rounds);
    
    if (hider_score > seeker_score) {
        printf("Overall winner: Hider\n");
    } else if (seeker_score > hider_score) {
        printf("Overall winner: Seeker\n");
    } else {
        printf("The game ended in a tie\n");
    }
    
    // Distribution analysis
    int hider_distribution[MAX_N] = {0};
    int seeker_distribution[MAX_N] = {0};
    
    // Reset random seed for consistent analysis
    srand(time(NULL));
    
    for (int i = 0; i < 1000; i++) {
        hider_distribution[computer_turn(N, hider_probs)]++;
        seeker_distribution[computer_turn(N, seeker_probs)]++;
    }
    
    printf("\nMove Distribution Analysis (1000 simulated choices):\n");
    printf("------------------------------------\n");
    
    printf("Hider's move distribution:\n");
    for (int i = 0; i < N; i++) {
        printf("Place %d: %d times (%.1f%%) - Expected: %.1f%%\n", 
               i, hider_distribution[i], 
               (float)hider_distribution[i] / 10.0,
               hider_probs[i] * 100);
    }
    
    printf("\nSeeker's move distribution:\n");
    for (int i = 0; i < N; i++) {
        printf("Place %d: %d times (%.1f%%) - Expected: %.1f%%\n", 
               i, seeker_distribution[i], 
               (float)seeker_distribution[i] / 10.0,
               seeker_probs[i] * 100);
    }
    
    // Calculate expected game value
    double expected_value = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            expected_value += hider_probs[i] * seeker_probs[j] * matrix_hider[i][j];
        }
    }
    printf("\nTheoretical expected value per round: %.3f\n", expected_value);
    printf("Actual average value per round: %.3f\n", (float)hider_score / rounds);
    
    // Free allocated memory
    free(hider_probs);
    free(seeker_probs);
}



int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    int N = 3;
    const char* difficulty[] = {"neutral", "easy", "hard"};
    
    // Run the simulation
    simulate(N, difficulty);
    
    return 0;
}
