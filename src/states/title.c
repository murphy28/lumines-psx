#include "title.h"
#include "../core/system.h"
#include "../core/input.h"
#include "../core/statemanager.h"
#include "libgpu.h"

static int titleFrameCount = 0;

void StateTitle_Init() {
    titleFrameCount = 0;
}

void StateTitle_Update() {
    titleFrameCount++;

    if(titleFrameCount > 30) { // Accept inputs after half second warmup
        if (Input_IsActionUp(CONFIRM)) StateManager_ChangeState(STATE_ARCADE);
    }

    System_ClearOT();

    FntPrint("Lumines PSX\n");
    FntPrint("WIP Title Screen\n\n");
    FntPrint("Press X or START");

    System_Display();
}

void StateTitle_Exit() {}
