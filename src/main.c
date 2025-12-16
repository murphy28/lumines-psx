#include "core/statemanager.h"
#include "core/system.h"
#include "core/input.h"

int main(void)
{
    System_Init();

    StateManager_Init();

    Input_Init();

    while (1)
    {
        Input_Update();

        StateManager_Update();
    }
    return 0;
}
