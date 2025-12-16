#include "arcade.h"
#include "../game/session.h"
#include "../core/system.h"

void StateArcade_Init() {
    GameSession_Init();
}

void StateArcade_Update() {
    GameSession_Update();

    System_ClearOT();
    GameSession_Draw();
    System_Display();
}

void StateArcade_Exit() {}
