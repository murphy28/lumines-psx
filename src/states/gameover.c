#include "gameover.h"
#include "../core/system.h"
#include "../core/input.h"
#include "../core/statemanager.h"
#include "libgpu.h"
#include "../game/grid.h"

static int titleFrameCount = 0;

void StateGameover_Init() {
    titleFrameCount = 0;
}

void StateGameover_Update() {
    titleFrameCount++;

    if(titleFrameCount > 30) { // Accept inputs after half second warmup
        if (Input_IsActionUp(CONFIRM)) StateManager_ChangeState(STATE_TITLE);
    }

    System_ClearOT();

    FntPrint("Game over!\n");
    FntPrint("Your score: %d\n\n", GetScore());
    FntPrint("Press X or START to return to title");

    System_Display();
}

void StateGameover_Exit() {}
