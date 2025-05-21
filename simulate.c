#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>
#include "raylib.h"
#include "solve.h"
#include "simulate.h"


    
// Log storage
char log_lines[MAX_LOG_LINES][MAX_LOG_LENGTH];
int log_count = 0;    
    
    
    

    void add_log(const char* format, ...) {
        if (log_count >= MAX_LOG_LINES) return;
        va_list args;
        va_start(args, format);
        vsnprintf(log_lines[log_count], MAX_LOG_LENGTH, format, args);
        va_end(args);
        log_count++;
    }

    void simulate(int N, const char* difficulty[]) {
        int matrix_hider[MAX_N][MAX_N];
        int matrix_seeker[MAX_N][MAX_N];
        int hider_score = 0, seeker_score = 0;
        int rounds = 100;
        int scrollOffset = 0;
        int horizontalScrollOffset = 0;
        bool hider_showMatrix = false;
        bool seeker_showMatrix = false;
        bool showSteps = false;

        Rectangle hiderScoreBox = {50, 20, 200, 50};
        Rectangle seekerScoreBox = {GetScreenWidth() - 250, 20, 200, 50};
        Rectangle hiderBtn = {500, 100, 150, 40};
        Rectangle seekerBtn = {700, 100, 150, 40};
        Rectangle stepsBtn = {900, 100, 150, 40};
        Rectangle centerPopup = {GetScreenWidth()/2 - 300, GetScreenHeight()/2 - 200, 600, 400};


        generate_game_matrix(N, difficulty, matrix_hider, true);
        generate_game_matrix(N, difficulty, matrix_seeker, false);

        

        double* seeker_probs = seeker_probability_calculate(N, matrix_hider);
        double* hider_probs = hider_probability_calculate(N, matrix_hider);


        add_log("Starting %d round simulation...", rounds);

        for (int round = 1; round <= rounds; round++) {
            int hider_place = computer_turn(N, hider_probs);
            int seeker_place = computer_turn(N, seeker_probs);
            int round_score = matrix_hider[hider_place][seeker_place];

            if (hider_place == seeker_place) {
                seeker_score += matrix_seeker[hider_place][seeker_place];
                hider_score -= matrix_seeker[hider_place][seeker_place];
                add_log("Round %3d: H:%d, S:%d - Seeker wins (%+d)", 
                    round, hider_place, seeker_place, matrix_seeker[hider_place][seeker_place]);
            } else {
                hider_score += matrix_hider[hider_place][seeker_place];
                seeker_score -= matrix_hider[hider_place][seeker_place];
                add_log("Round %3d: H:%d, S:%d - Hider wins (%+d)", 
                    round, hider_place, seeker_place, matrix_hider[hider_place][seeker_place]);
            }
        }

        // === Render loop ===
        while (!WindowShouldClose()) {
            int visibleHeight = GetScreenHeight() - 220; 
            int totalLogHeight = log_count * 20;
            int maxScroll =10000;
            int maxHorizontalScroll = (N > 8) ? (N - 8) * 40 : 0;  


            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                    // Hold Shift for horizontal scrolling
                    horizontalScrollOffset -= (int)(wheel * 40);
                } else {
                    // Normal vertical scrolling
                    scrollOffset -= (int)(wheel * 40); // Scroll faster (40px per wheel tick)
                }
            }

            // Optional: Arrow key scroll
            if (IsKeyDown(KEY_DOWN)) scrollOffset += 10;
            if (IsKeyDown(KEY_UP)) scrollOffset -= 10;
            if (IsKeyDown(KEY_RIGHT)) horizontalScrollOffset += 10;
            if (IsKeyDown(KEY_LEFT)) horizontalScrollOffset -= 10;


            // Clamp scrollOffset
            if (scrollOffset < 0) scrollOffset = 0;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;
            if (horizontalScrollOffset < 0) horizontalScrollOffset = 0;
            if (horizontalScrollOffset > maxHorizontalScroll) horizontalScrollOffset = maxHorizontalScroll;



            // Handle button clicks
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, hiderBtn)) {
                hider_showMatrix = !hider_showMatrix;
            }

                if (CheckCollisionPointRec(mouse, seekerBtn)) {
                    seeker_showMatrix = !seeker_showMatrix;
                }
                if (CheckCollisionPointRec(mouse, stepsBtn)) {
                    showSteps = !showSteps;
                    if (showSteps) {
                        scrollOffset = 0; 
                    }
                }
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);

            // --- Fixed Top Section: Buttons & Scores ---
            DrawRectangle(0, 0, GetScreenWidth(), 160, LIGHTGRAY);

            // Score Display
            DrawRectangleRec(hiderScoreBox, LIGHTGRAY);
            DrawRectangleLinesEx(hiderScoreBox, 2, BLACK);
            DrawText("Hider Score", hiderScoreBox.x + 10, hiderScoreBox.y + 5, 20, BLACK);
            DrawText(TextFormat("%d", hider_score), hiderScoreBox.x + 10, hiderScoreBox.y + 25, 20, DARKBLUE);

            DrawRectangleRec(seekerScoreBox, LIGHTGRAY);
            DrawRectangleLinesEx(seekerScoreBox, 2, BLACK);
            DrawText("Seeker Score", seekerScoreBox.x + 10, seekerScoreBox.y + 5, 20, BLACK);
            DrawText(TextFormat("%d", seeker_score), seekerScoreBox.x + 10, seekerScoreBox.y + 25, 20, DARKGREEN);

            // Buttons
            DrawRectangleRec(hiderBtn, DARKGOLD);
            DrawText("Hider Matrix", hiderBtn.x + 10, hiderBtn.y + 10, 16, BLACK);

            DrawRectangleRec(seekerBtn, DARKGOLD);
            DrawText("Seeker Matrix", seekerBtn.x + 10, seekerBtn.y + 10, 16, BLACK);

            DrawRectangleRec(stepsBtn, DARKGOLD);
            DrawText("Steps Log", stepsBtn.x + 20, stepsBtn.y + 10, 16, BLACK);

            // --- Scrollable Content Section ---
            int scrollY = 180;  // Start of scrollable area
            BeginScissorMode(0, scrollY, GetScreenWidth(), GetScreenHeight() - scrollY);
            int y = scrollY - scrollOffset;

            // Steps log
            if (showSteps) {
                DrawText("Steps Log:", 20, y, 20, BLACK);
                y += 30;
                for (int i = 0; i < log_count; i++) {
                    if (y >= scrollY - 20 && y <= GetScreenHeight()) {
                        DrawText(log_lines[i], 40, y, 16, BLACK);
                    }
                    y += 20;
                }
            }

            // Hider matrix & probabilities
            if (hider_showMatrix) {
                y += 20;
                DrawText("Hider Matrix:", 20, y, 20, DARKBLUE);
                y += 30;
                for (int i = 0; i < N; i++) {
                    char row[128] = "";
                    for (int j = 0; j < N; j++) {
                        char num[8];
                        sprintf(num, "%3d ", matrix_hider[i][j]);
                        strcat(row, num);
                    }
                    DrawText(row, 40 - horizontalScrollOffset, y, 16, DARKBLUE);
                    y += 20;
                }
                y += 10;
                DrawText("Hider Probabilities:", 20, y, 20, MAROON);
                y += 30;
                for (int i = 0; i < N; i++) {
                    DrawText(TextFormat("H%d: %.2f", i, hider_probs[i]), 40, y, 16, MAROON);
                    y += 20;
                }
            }

            // Seeker matrix & probabilities
            if (seeker_showMatrix) {
                y += 20;
                DrawText("Seeker Matrix:", 20, y, 20, DARKGREEN);
                y += 30;
                for (int i = 0; i < N; i++) {
                    char row[128] = "";
                    for (int j = 0; j < N; j++) {
                        char num[8];
                        sprintf(num, "%3d ", matrix_seeker[i][j]);
                        strcat(row, num);
                    }
                    DrawText(row, 40 - horizontalScrollOffset, y, 16, DARKGREEN);
                    y += 20;
                }
                y += 10;
                DrawText("Seeker Probabilities:", 20, y, 20, DARKGREEN);
                y += 30;
                for (int i = 0; i < N; i++) {
                    DrawText(TextFormat("S%d: %.2f", i, seeker_probs[i]), 40, y, 16, DARKGREEN);
                    y += 20;
                }
            }
            EndScissorMode();
            EndDrawing();
                }

        free(hider_probs);
        free(seeker_probs);
    }