#include "core/system.h"
#include "drivers/pad.h"
#include "game/grid.h"

// Controller Instance
GamePad player1;

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
        // Poll Input
        Pad_Update(&player1);

        // Update Game Logic
        if(Pad_GetButton(&player1, PAD_LEFT))  Grid_UpdateOffset(-2, 0);
        if(Pad_GetButton(&player1, PAD_RIGHT)) Grid_UpdateOffset(2, 0);
        if(Pad_GetButton(&player1, PAD_UP))    Grid_UpdateOffset(0, -2);
        if(Pad_GetButton(&player1, PAD_DOWN))  Grid_UpdateOffset(0, 2);

        // Render Frame
        System_ClearOT(); // Prepare Ordering Table
        Grid_Draw();      // Link primitives to OT
        System_Display(); // Push OT to GPU
    }
    return 0;
}
