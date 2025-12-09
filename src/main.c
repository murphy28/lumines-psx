#include "core/system.h"
#include "drivers/pad.h"
#include "game/grid.h"

// Controller Instance
GamePad player1;

int activeTheme = 10;

int main(void)
{
    // 1. Initialize Hardware
    System_Init();

    // 2. Initialize Drivers
    Pad_Init(&player1, 0);
    InitPAD((char *)player1.rawBuffer, 34, 0, 0);
    StartPAD();

    // 3. Initialize Game
    Grid_Init();

    // 4. Main Loop
    while (1)
    {
        Grid_Update();

        // Poll Input
        Pad_Update(&player1);

        // Update Game Logic
        if(Pad_GetButtonDown(&player1, PAD_LEFT))    {
            Player_MoveLeft();
        }
        if(Pad_GetButtonDown(&player1, PAD_RIGHT))  {
            Player_MoveRight();
        }

        if(Pad_GetButton(&player1, PAD_DOWN))  {
            Player_SlamBlock();
        }

        if(Pad_GetButtonUp(&player1, PAD_DOWN))  {
            Player_UnlockDrop();
        }

        if(Pad_GetButtonDown(&player1, PAD_CIRCLE))  {
            Player_RotateCounterClockwise();
        }

        if(Pad_GetButtonDown(&player1, PAD_CROSS) || Pad_GetButtonDown(&player1, PAD_UP))  {
            Player_RotateClockwise();
        }

        // Render Frame
        System_ClearOT(); // Prepare Ordering Table
        Grid_Draw();      // Link primitives to OT
        System_Display(); // Push OT to GPU
    }
    return 0;
}
