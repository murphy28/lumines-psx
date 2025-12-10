#include "core/system.h"
#include "drivers/pad.h"
#include "game/grid.h"
#include "game/player.h"

// Controller Instance
GamePad player1;

int main(void)
{
    // 1. Initialize Hardware (GPU, GTE, etc.)
    System_Init();

    // 2. Initialize Drivers (Controller)
    Pad_Init(&player1, 0);
    InitPAD((char *)player1.rawBuffer, 34, 0, 0);
    StartPAD();

    // 3. Initialize Game Systems
    // Grid_Init() will internally initialize the Player and Theme
    Grid_Init();

    // 4. Main Loop
    while (1)
    {
        // Update Game Logic
        // Grid_Update() now internally calls Player_Update() for gravity/physics
        Grid_Update();

        // Poll Input
        Pad_Update(&player1);

        // Process Controller Commands
        if(Pad_GetButtonDown(&player1, PAD_LEFT)) {
            Player_MoveLeft();
        }

        if(Pad_GetButtonDown(&player1, PAD_RIGHT)) {
            Player_MoveRight();
        }

        if(Pad_GetButton(&player1, PAD_DOWN)) {
            // "Slam" logic (fast drop)
            Player_SlamBlock();
        }

        if(Pad_GetButtonUp(&player1, PAD_DOWN)) {
            // Release slam lock if button is released
            Player_UnlockDrop();
        }

        if(Pad_GetButtonDown(&player1, PAD_CIRCLE)) {
            Player_RotateCounterClockwise();
        }

        if(Pad_GetButtonDown(&player1, PAD_CROSS) || Pad_GetButtonDown(&player1, PAD_UP)) {
            Player_RotateClockwise();
        }

        // Render Frame
        System_ClearOT();
        Grid_Draw();
        System_Display();
    }
    return 0;
}
