#include "player.h"
#include "grid.h" // Needs to know about grid boundaries
#include <libgte.h>
#include <stdlib.h> // for rand()

ActivePiece player;

static const int BLOCK_PATTERNS[][4] = {
    {1, 1, 1, 1}, {2, 2, 2, 2},
    {1, 2, 1, 2}, {1, 1, 2, 2},
    {1, 2, 2, 1}, {2, 1, 1, 2},
    {1, 1, 1, 2}, {2, 2, 2, 1},
};
#define PATTERN_COUNT (sizeof(BLOCK_PATTERNS) / sizeof(BLOCK_PATTERNS[0]))

void Player_Init(void) {
    player.active = 1;
    player.gridX = (GRID_W / 2) - 1;
    player.gridY = -2; // Start above board
    player.dropTimer = 0;
    player.dropLock = 0;
    player.graceCycles = 3;

    int p = rand() % PATTERN_COUNT;
    player.cells[0] = BLOCK_PATTERNS[p][0];
    player.cells[1] = BLOCK_PATTERNS[p][1];
    player.cells[2] = BLOCK_PATTERNS[p][2];
    player.cells[3] = BLOCK_PATTERNS[p][3];
}

void Player_Update(void) {
    if(!player.active) {
        Player_Init();
        return;
    }

    // Handle Drop Timer
    if (player.dropLock) {
        // Slam logic
        while (Grid_IsMoveValid(player.gridX, player.gridY + 1)) {
            player.gridY++;
        }
        player.dropTimer = DROP_DELAY_FRAMES + 1; // Force landing next check
    } else {
        player.dropTimer++;
    }

    // Handle Gravity Tick
    if (player.dropTimer >= DROP_DELAY_FRAMES) {
        player.dropTimer = 0;

        if (player.graceCycles > 0) {
            player.graceCycles--;
            return;
        }

        if (Grid_IsMoveValid(player.gridX, player.gridY + 1)) {
            player.gridY++;
        } else {
            // Landed
            if (player.gridY < -1) {
                // Game Over logic (reset for now)
                Grid_Init();
                return;
            }
            Grid_PlaceBlock(&player);
        }
    }
}

// Movement Wrappers
void Player_MoveLeft(void) {
    if (Grid_IsMoveValid(player.gridX - 1, player.gridY)) {
        player.gridX--;
    }
}

void Player_MoveRight(void) {
    if (Grid_IsMoveValid(player.gridX + 1, player.gridY)) {
        player.gridX++;
    }
}

void Player_SlamBlock(void) {
    if (player.slamLatch) return;
    player.dropLock = 1;
    player.slamLatch = 1;
}

void Player_UnlockDrop(void) {
    player.slamLatch = 0;
}

// Rotation Logic
void Player_RotateCW(void) {
    int c0 = player.cells[0]; int c1 = player.cells[1];
    int c2 = player.cells[2]; int c3 = player.cells[3];
    player.cells[0] = c2; player.cells[1] = c0;
    player.cells[2] = c3; player.cells[3] = c1;
}

void Player_RotateCCW(void) {
    int c0 = player.cells[0]; int c1 = player.cells[1];
    int c2 = player.cells[2]; int c3 = player.cells[3];
    player.cells[0] = c1; player.cells[1] = c3;
    player.cells[2] = c0; player.cells[3] = c2;
}
