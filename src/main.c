#include "core/system.h"
#include "core/input.h"
#include "game/session.h"

int main(void)
{
    // 1. Initialize Hardware (GPU, GTE, etc.)
    System_Init();

    // 2. Initialize Controls
    Input_Init();

    // 3. Initialize Game Systems
    GameSession_Init();

    // 4. Main Loop
    while (1)
    {
        Input_Update();

        GameSession_Update();

        // Render Frame
        System_ClearOT();
        GameSession_Draw();
        System_Display();
    }
    return 0;
}
