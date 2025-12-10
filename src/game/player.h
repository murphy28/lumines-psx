#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

// Helper constants
#define DROP_DELAY_FRAMES 30

typedef struct {
    int gridX;
    int gridY;
    int cells[4]; // 0:TL; 1:TR; 2:BL; 3:BR;
    int active;
    int dropTimer;
    int dropLock;    // If 1, pull to floor instantly
    int slamLatch;   // If 1, ignore slam button
    int graceCycles; // Frames to ignore gravity at spawn
} ActivePiece;

// Accessor for drawing
extern ActivePiece player;

void Player_Init(void);
void Player_Update(void); // New consolidated update function

// Input Commands
void Player_MoveLeft(void);
void Player_MoveRight(void);
void Player_SlamBlock(void);
void Player_RotateClockwise(void);
void Player_RotateCounterClockwise(void);
void Player_UnlockDrop(void);

#endif
