#include "raylib.h"  
#include <stdlib.h> // For rand and srand  
#include <time.h>   // For seeding randomness  
  
#define MAX_LEVELS 10  
#define MAX_PLATFORMS 20  
#define MAX_OBSTACLES 20  
#define MAX_COINS 40  
  
typedef struct Level {  
    Rectangle platforms[MAX_PLATFORMS];  
    Rectangle obstacles[MAX_OBSTACLES];  
    Vector2 coins[MAX_COINS];  
    int totalPlatforms;  
    int totalObstacles;  
    int totalCoins;  
} Level;  
  
// Consistent elements across all levels  
const Rectangle ground = { -500, 450, 4100, 200 }; // Extended to reach the right wall  
const Rectangle goal = { 3400, 400, 50, 50 };  
const Rectangle leftWall = { -500, -500, 500, 1200 };  
const Rectangle rightWall = { 3600, -500, 500, 1200 };  
  
bool CheckOverlap(Rectangle a, Rectangle b) {  
    return CheckCollisionRecs(a, b);  
}  
  
void InitLevel(Level *level, int index) {  
    for (int i = 0; i < MAX_PLATFORMS; i++) {  
        Rectangle platform;  
        bool overlap;  
        do {  
            platform = (Rectangle){ rand() % 3200 + 100, rand() % 200 + 200, 100, 20 };  
            overlap = CheckOverlap(platform, goal);  
            for (int j = 0; j < i && !overlap; j++) {  
                overlap = CheckOverlap(platform, level->platforms[j]);  
            }  
        } while (overlap);  
        level->platforms[i] = platform;  
    }  
  
    for (int i = 0; i < MAX_OBSTACLES; i++) {  
        Rectangle obstacle;  
        bool overlap;  
        do {  
            obstacle = (Rectangle){ rand() % 3200 + 100, 400, 50, 50 };  
            overlap = CheckOverlap(obstacle, goal);  
            for (int j = 0; j < MAX_PLATFORMS && !overlap; j++) {  
                overlap = CheckOverlap(obstacle, level->platforms[j]);  
            }  
        } while (overlap);  
        level->obstacles[i] = obstacle;  
    }  
  
    for (int i = 0; i < MAX_COINS; i++) {  
        Vector2 coin;  
        bool overlap;  
        do {  
            coin = (Vector2){ rand() % 3200 + 100, rand() % 150 + 200 };  
            overlap = CheckCollisionCircleRec(coin, 10, goal);  
        } while (overlap);  
        level->coins[i] = coin;  
    }  
  
    level->totalPlatforms = MAX_PLATFORMS;  
    level->totalObstacles = MAX_OBSTACLES;  
    level->totalCoins = MAX_COINS;  
}  
  
int main() {  
    InitWindow(800, 450, "Super Mario Clone");  
    SetTargetFPS(60);  
  
    srand(time(NULL)); // Seed the random number generator  
  
    Level levels[MAX_LEVELS];  
    for (int i = 0; i < MAX_LEVELS; i++) {  
        InitLevel(&levels[i], i);  
    }  
  
    int currentLevel = 0;  
    bool collected[MAX_COINS] = { false };  
    int coinCount = 0;  
    bool gameOver = false;  
    bool isJumping = false;  
  
    // Player variables  
    Rectangle player = { 400, 225, 50, 50 };  
    int playerSpeed = 5;  
    int jumpSpeed = 0;  
    const int gravity = 2;  
    const int jumpForce = 20;  
    int jumpCount = 0;  
  
    // Camera setup  
    Camera2D camera = { 0 };  
    camera.target = (Vector2){ player.x + player.width/2, player.y + player.height/2 };  
    camera.offset = (Vector2){ 400, 225 };  
    camera.zoom = 1.0f;  
  
    while (!WindowShouldClose()) {  
        if (!gameOver) {  
            // Player movement  
            if (IsKeyDown(KEY_RIGHT)) player.x += playerSpeed;  
            if (IsKeyDown(KEY_LEFT)) player.x -= playerSpeed;  
  
            // Collision with left wall  
            if (CheckCollisionRecs(player, leftWall)) {  
                player.x = leftWall.x + leftWall.width;  
            }  
  
            // Collision with right wall  
            if (CheckCollisionRecs(player, rightWall)) {  
                player.x = rightWall.x - player.width;  
            }  
  
            // Jump logic with double jump  
            if (IsKeyPressed(KEY_SPACE) && jumpCount < 2) {  
                jumpSpeed = -jumpForce;  
                isJumping = true;  
                jumpCount++;  
            }  
  
            // Apply physics  
            player.y += jumpSpeed;  
            jumpSpeed += gravity;  
  
            // Check for collision with platforms  
            for (int i = 0; i < levels[currentLevel].totalPlatforms; i++) {  
                if (CheckCollisionRecs(player, levels[currentLevel].platforms[i])) {  
                    if (player.y + player.height >= levels[currentLevel].platforms[i].y &&  
                        player.y + player.height - jumpSpeed <= levels[currentLevel].platforms[i].y) {  
                        player.y = levels[currentLevel].platforms[i].y - player.height;  
                        jumpSpeed = 0;  
                        isJumping = false;  
                        jumpCount = 0;  
                    }  
                }  
            }  
  
            // Ground check  
            if (player.y + player.height >= 450) {  
                player.y = 450 - player.height;  
                jumpSpeed = 0;  
                isJumping = false;  
                jumpCount = 0;  
            }  
  
            // Collision with obstacles - Restart the level  
            for (int i = 0; i < levels[currentLevel].totalObstacles; i++) {  
                if (CheckCollisionRecs(player, levels[currentLevel].obstacles[i])) {  
                    // Restart the level  
                    InitLevel(&levels[currentLevel], currentLevel);  
                    player.x = 400;  
                    player.y = 225;  
                    jumpSpeed = 0;  
                    isJumping = false;  
                    jumpCount = 0;  
                    coinCount = 0;  
                    for (int i = 0; i < MAX_COINS; i++) {  
                        collected[i] = false;  
                    }  
                }  
            }  
  
            // Check for collision with goal  
            if (CheckCollisionRecs(player, goal)) {  
                if (currentLevel < MAX_LEVELS - 1) {  
                    currentLevel++;  
                    InitLevel(&levels[currentLevel], currentLevel);  
                } else {  
                    gameOver = true; // All levels complete  
                }  
                player.x = 400;  
                player.y = 225;  
                coinCount = 0;  
                for (int i = 0; i < MAX_COINS; i++) {  
                    collected[i] = false;  
                }  
            }  
  
            // Check for collision with coins  
            for (int i = 0; i < levels[currentLevel].totalCoins; i++) {  
                if (!collected[i] && CheckCollisionCircleRec(levels[currentLevel].coins[i], 10, player)) {  
                    collected[i] = true;  
                    coinCount++;  
                }  
            }  
  
            // Update camera to follow player  
            camera.target = (Vector2){ player.x + player.width/2, player.y + player.height/2 };  
        }  
  
        // Drawing  
        BeginDrawing();  
        ClearBackground(RAYWHITE);  
  
        BeginMode2D(camera);  
        if (!gameOver) {  
            // Draw player  
            DrawRectangleRec(player, BLUE);  
  
            // Draw platforms  
            for (int i = 0; i < levels[currentLevel].totalPlatforms; i++) {  
                DrawRectangleRec(levels[currentLevel].platforms[i], GREEN);  
            }  
  
            // Draw obstacles  
            for (int i = 0; i < levels[currentLevel].totalObstacles; i++) {  
                DrawRectangleRec(levels[currentLevel].obstacles[i], RED);  
            }  
  
            // Draw goal  
            DrawRectangleRec(goal, GOLD);  
  
            // Draw ground  
            DrawRectangleRec(ground, DARKBROWN);  
  
            // Draw walls  
            DrawRectangleRec(leftWall, GRAY);  
            DrawRectangleRec(rightWall, GRAY);  
  
            // Draw coins  
            for (int i = 0; i < levels[currentLevel].totalCoins; i++) {  
                if (!collected[i]) {  
                    DrawCircleV(levels[currentLevel].coins[i], 10, YELLOW);  
                }  
            }  
        } else {  
            // Check if all levels completed  
            Vector2 cameraCenter = { camera.target.x - camera.offset.x, camera.target.y - camera.offset.y };  
            if (currentLevel == MAX_LEVELS - 1) {  
                DrawText("Congratulations! You Win!", cameraCenter.x + 200.0, cameraCenter.y + 150.0, 20, GREEN);  
            } else {  
                DrawText("Game Over!", cameraCenter.x + 300.0, cameraCenter.y + 150.0, 20, RED);  
            }  
            DrawText("Press R to Restart", cameraCenter.x + 300.0, cameraCenter.y + 180.0, 20, BLACK);  
  
            if (IsKeyPressed(KEY_R)) {  
                // Reset game state  
                currentLevel = 0;  
                InitLevel(&levels[currentLevel], currentLevel);  
                player.x = 400;  
                player.y = 225;  
                jumpSpeed = 0;  
                isJumping = false;  
                jumpCount = 0;  
                coinCount = 0;  
                for (int i = 0; i < MAX_COINS; i++) {  
                    collected[i] = false;  
                }  
                gameOver = false;  
            }  
        }  
        EndMode2D();  
  
        // Draw coin counter and level info  
        DrawText(TextFormat("Coins: %i", coinCount), 10, 10, 20, BLACK);  
        DrawText(TextFormat("Level: %i", currentLevel + 1), 10, 30, 20, BLACK);  
  
        EndDrawing();  
    }  
  
    CloseWindow();  
    return 0;  
}  
