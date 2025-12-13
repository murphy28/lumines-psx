#include "session.h"
#include "grid.h"
#include "player.h"
#include "../core/input.h"

void GameSession_Init() {
    Grid_Init();
}

void GameSession_Update() {
    if(Input_IsActionDown(MOVE_LEFT)) Player_MoveLeft();
    if(Input_IsActionDown(MOVE_RIGHT)) Player_MoveRight();

    if(Input_IsActionDown(SLAM)) Player_SlamBlock();
    if(Input_IsActionUp(SLAM)) Player_UnlockDrop();

    if(Input_IsActionDown(ROTATE_CW)) Player_RotateCW();
    if(Input_IsActionDown(ROTATE_CCW)) Player_RotateCCW();

    Grid_Update();
}

void GameSession_Draw() {
    Grid_Draw();
}
