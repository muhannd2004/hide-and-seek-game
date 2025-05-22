#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glpk.h>
#include <time.h>
#include "raylib.h"
#include "solve.h"
#include "simulate.h"
#include "menu.h"


    
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
        bool exitToMenu = false;  // Flag to track if we should exit to main menu

        // Load Times New Roman font
        Font timesNewRoman = LoadFontEx("Times_New_Roman.ttf", 20, NULL, 0);
        // Fallback to default font if loading fails
        bool useCustomFont = timesNewRoman.texture.id > 0;

        Rectangle hiderScoreBox = {50, 20, 200, 50};
        Rectangle seekerScoreBox = {GetScreenWidth() - 250, 20, 200, 50};
        Rectangle hiderBtn = {500, 100, 150, 40};
        Rectangle seekerBtn = {700, 100, 150, 40};
        Rectangle stepsBtn = {900, 100, 150, 40};
        Rectangle menuBtn = {700, 20, 150, 40};  
        Rectangle centerPopup = {GetScreenWidth()/2 - 300, GetScreenHeight()/2 - 200, 600, 400};


        generate_game_matrix(N, difficulty, matrix_hider, true);
        generate_game_matrix(N, difficulty, matrix_seeker, false);

        

        double* seeker_probs = probability_calculate(N, matrix_seeker);
        double* hider_probs = probability_calculate(N, matrix_hider);


        add_log("Starting %d round simulation...", rounds);

        // Arrays to track actual probabilities
        int hider_counts[MAX_N] = {0};
        int seeker_counts[MAX_N] = {0};

        for (int round = 1; round <= rounds; round++) {
            int hider_place = computer_turn(N, hider_probs);
            int seeker_place = computer_turn(N, seeker_probs);
            int round_score = matrix_hider[hider_place][seeker_place];
            
            // Count occurrences of each position for calculating actual probabilities
            hider_counts[hider_place]++;
            seeker_counts[seeker_place]++;

            if (hider_place == seeker_place) {
                seeker_score += matrix_seeker[seeker_place][hider_place];
                hider_score -= matrix_seeker[seeker_place][hider_place];
                add_log("Round %3d: H:%d (%s), S:%d (%s) - Seeker wins (%+d)", 
                    round, hider_place, difficulty[hider_place], seeker_place, difficulty[seeker_place], matrix_seeker[hider_place][seeker_place]);
            } else {
                hider_score += matrix_hider[hider_place][seeker_place];
                seeker_score -= matrix_hider[hider_place][seeker_place];
                add_log("Round %3d: H:%d (%s), S:%d (%s) - Hider wins (%+d)", 
                    round, hider_place, difficulty[hider_place], seeker_place, difficulty[seeker_place], matrix_hider[hider_place][seeker_place]);
            }
        }
        
        // Add actual probabilities to log
        add_log("Actual probabilities:");
        for (int i = 0; i < N; i++) {
            double actual_hider_prob = (double)hider_counts[i] / rounds;
            double actual_seeker_prob = (double)seeker_counts[i] / rounds;
            add_log("Position %d: Hider %.4f, Seeker %.4f", i, actual_hider_prob, actual_seeker_prob);
        }
        
        add_log("The winner is: %s", (hider_score > seeker_score) ? "Hider" : "Seeker");

        // === Render loop ===
        while (!WindowShouldClose() && !exitToMenu) {
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
                if (CheckCollisionPointRec(mouse, menuBtn)) {
                    exitToMenu = true;  
                }
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);

            // --- Fixed Top Section: Buttons & Scores ---
            DrawRectangle(0, 0, GetScreenWidth(), 160, LIGHTGRAY);

            // Score Display
            DrawRectangleRec(hiderScoreBox, LIGHTGRAY);
            DrawRectangleLinesEx(hiderScoreBox, 2, BLACK);
            DrawRectangleRec(seekerScoreBox, LIGHTGRAY);
            DrawRectangleLinesEx(seekerScoreBox, 2, BLACK);
            
            // Use the custom font for text if it was loaded successfully
            if (useCustomFont) {
                
                DrawTextEx(timesNewRoman, "Hider Score", 
                    (Vector2){ hiderScoreBox.x + 10, hiderScoreBox.y + 5 }, 20, 1, BLACK);
                DrawTextEx(timesNewRoman, TextFormat("%d", hider_score), 
                    (Vector2){ hiderScoreBox.x + 10, hiderScoreBox.y + 25 }, 20, 1, DARKBLUE);
                    
                DrawTextEx(timesNewRoman, "Seeker Score", 
                    (Vector2){ seekerScoreBox.x + 10, seekerScoreBox.y + 5 }, 20, 1, BLACK);
                DrawTextEx(timesNewRoman, TextFormat("%d", seeker_score), 
                    (Vector2){ seekerScoreBox.x + 10, seekerScoreBox.y + 25 }, 20, 1, DARKGREEN);
                    
                // Buttons
                DrawRectangleRec(hiderBtn, DARKGOLD);
                DrawTextEx(timesNewRoman, "Hider Matrix", 
                    (Vector2){ hiderBtn.x + 10, hiderBtn.y + 10 }, 24, 1, BLACK);
                DrawRectangleRec(seekerBtn, DARKGOLD);
                DrawTextEx(timesNewRoman, "Seeker Matrix", 
                    (Vector2){ seekerBtn.x + 10, seekerBtn.y + 10 }, 24, 1, BLACK);
                DrawRectangleRec(stepsBtn, DARKGOLD);
                DrawTextEx(timesNewRoman, "Steps Log", 
                    (Vector2){ stepsBtn.x + 20, stepsBtn.y + 10 }, 24, 1, BLACK);
                DrawRectangleRec(menuBtn, DARKGOLD);
                DrawTextEx(timesNewRoman, "Main Menu", 
                    (Vector2){ menuBtn.x + 10, menuBtn.y + 10 }, 24, 1, BLACK);
            } else {
                // Fallback to regular DrawText if font failed to load
                DrawText("Hider Score", hiderScoreBox.x + 10, hiderScoreBox.y + 5, 20, BLACK);
                DrawText(TextFormat("%d", hider_score), hiderScoreBox.x + 10, hiderScoreBox.y + 25, 20, DARKBLUE);
                DrawText("Seeker Score", seekerScoreBox.x + 10, seekerScoreBox.y + 5, 20, BLACK);
                DrawText(TextFormat("%d", seeker_score), seekerScoreBox.x + 10, seekerScoreBox.y + 25, 20, DARKGREEN);
                DrawText("Hider Matrix", hiderBtn.x + 10, hiderBtn.y + 10, 16, BLACK);
                DrawText("Seeker Matrix", seekerBtn.x + 10, seekerBtn.y + 10, 16, BLACK);
                DrawText("Steps Log", stepsBtn.x + 20, stepsBtn.y + 10, 16, BLACK);
                DrawText("Main Menu", menuBtn.x + 10, menuBtn.y + 10, 16, BLACK);
            }

            // --- Scrollable Content Section ---
            int scrollY = 180;  // Start of scrollable area
            BeginScissorMode(0, scrollY, GetScreenWidth(), GetScreenHeight() - scrollY);
            int y = scrollY - scrollOffset;

            // Steps log with new font
            if (showSteps) {
                if (useCustomFont) {
                    DrawTextEx(timesNewRoman, "Steps Log:", 
                        (Vector2){ 20, y }, 20, 1, BLACK);
                    y += 30;
                    for (int i = 0; i < log_count; i++) {
                        if (y >= scrollY - 20 && y <= GetScreenHeight()) {
                            DrawTextEx(timesNewRoman, log_lines[i], 
                                (Vector2){ 40, y }, 24, 1, BLACK);
                        }
                        y += 20;
                    }
                } else {
                    // Original DrawText calls
                    DrawText("Steps Log:", 20, y, 20, BLACK);
                    y += 30;
                    for (int i = 0; i < log_count; i++) {
                        if (y >= scrollY - 20 && y <= GetScreenHeight()) {
                            DrawText(log_lines[i], 40, y, 16, BLACK);
                        }
                        y += 20;
                    }
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

        // Unload the font when done
        if (useCustomFont) {
            UnloadFont(timesNewRoman);
        }

        free(hider_probs);
        free(seeker_probs);
        
        if (exitToMenu) {
            firstMenu = true;  // Set firstMenu flag to true when exiting
        }
    }