#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "../core/system.h"

void Grid_Init(void);
void Grid_Update(void);
void Grid_UpdateOffset(int dx, int dy);
void Grid_Draw(void);
void Grid_SetTheme(int themeIndex);

void Player_MoveLeft(void);
void Player_MoveRight(void);
void Player_SlamBlock(void);
void Player_RotateClockwise(void);
void Player_RotateCounterClockwise(void);
void Player_UnlockDrop(void);

#endif
