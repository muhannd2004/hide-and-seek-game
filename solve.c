#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>

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

void probabilty_calculate(int N, int matrix[MAX_N][MAX_N]) {
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MAX);

    // Variables: x1..xN (strategy), v (value of the game)
    glp_add_cols(lp, N + 1);
    for (int i = 1; i <= N; i++) {
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // x_i >= 0
    }
    glp_set_col_bnds(lp, N + 1, GLP_FR, 0.0, 0.0); // v free

    // Objective: maximize v
    glp_set_obj_coef(lp, N + 1, 1.0);

    // Constraints: one for each seeker strategy
    glp_add_rows(lp, N);
    int ia[1 + MAX_N * MAX_N], ja[1 + MAX_N * MAX_N];
    double ar[1 + MAX_N * MAX_N];
    int idx = 1;

    for (int j = 0; j < N; j++) {
        glp_set_row_bnds(lp, j + 1, GLP_LO, 0.0, 0.0); // sum a_ij x_i ≥ v

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

    glp_load_matrix(lp, idx - 1, ia, ja, ar);

    glp_simplex(lp, NULL);

    double v = glp_get_obj_val(lp);
    printf("\nOptimal hider game value: %f\n", v);

    printf("Hider strategy probabilities:\n");
    double sum = 0.0;
    for (int i = 1; i <= N; i++) {
        double xi = glp_get_col_prim(lp, i);
        sum += xi;
    }

    for (int i = 1; i <= N; i++) {
        double xi = glp_get_col_prim(lp, i);
        printf("Place %d: %.3f\n", i - 1, xi / sum); // normalized
    }

    glp_delete_prob(lp);
}

void seeker_strategy(int N, int matrix[MAX_N][MAX_N]) {
    
}

int main() {
    int N = 4;
    const char* difficulty[] = {"neutral","easy","hard","easy"};
    int matrix[MAX_N][MAX_N];

    generate_game_matrix(N, difficulty, matrix, false);

    printf("Game Matrix:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%3d ", matrix[i][j]);
        printf("\n");
    }

    probabilty_calculate(N, matrix);

    return 0;
}
