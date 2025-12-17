#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "../core/system.h"
#include "player.h" // Needed for PlaceBlock

#define BLOCK_SIZE  16
#define GRID_W      16
#define GRID_H      10

void Grid_Init(void);
void Grid_Update(void);
void Grid_Draw(void);
void Grid_SetTheme(int themeIndex);

int Grid_IsMoveValid(int x, int y);
void Grid_PlaceBlock(ActivePiece* p);

int GetScore(void);

#endif
