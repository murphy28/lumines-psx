#include "grid.h"
#include "../core/gpu_prims.h"
#include "player.h"
#include "theme.h"

// Assets
#include "../assets/bg_left.h"
#include "../assets/bg_right.h"
#include "../core/vram_map.h"

#define TOTAL_CELLS (GRID_W * GRID_H)
#define COLOR_GRID_LINES 40, 42, 44
#define COLOR_GRID_BG 0, 0, 0
#define GRAVITY_DELAY_FRAMES 3

typedef struct {
  unsigned char type;   // 0: Empty; 1: Color A; 2: Color B;
  unsigned char marked; // 0: Unmarked; 1: Marked;
  unsigned char protected; // 0: Not protected for timeline; 1: Protected for timeline;
} BlockData;

// State
static BlockData board[GRID_H][GRID_W];
static int gridOffsetX = CENTERX;
static int gridOffsetY = CENTERY;
static TIM_IMAGE bgLeftInfo;
static TIM_IMAGE bgRightInfo;
static int currentThemeIndex = 0;

static CVECTOR BLOCK_PALETTE_LIGHT[3];
static CVECTOR BLOCK_PALETTE_DARK[3];

// Internal Physics State
static int gravityTimer = 0;
static int doPhysicsUpdate = 0;

// Timeline State
static int timelineGridX = 0;
static int timelinePixelX = 0;
#define TIMELINE_SPEED 1
#define TIMELINE_WIDTH 2

// Score State
static int score = 0;
static int currentTimelineBlockCount = 0;
#define BLOCK_SCORE_VALUE 1

int GetScore() {
    return score;
}

void Grid_SetTheme(int themeIndex) {
  if (themeIndex < 0)
    themeIndex = TOTAL_THEMES - 1;
  if (themeIndex >= TOTAL_THEMES)
    themeIndex = 0;

  currentThemeIndex = themeIndex;
  const Theme *t = &THEME_LIBRARY[currentThemeIndex];

  BLOCK_PALETTE_LIGHT[0] = (CVECTOR){0, 0, 0, 0};
  BLOCK_PALETTE_DARK[0] = (CVECTOR){0, 0, 0, 0};
  BLOCK_PALETTE_LIGHT[1] = t->block_a;
  BLOCK_PALETTE_DARK[1] = t->block_a_dark;
  BLOCK_PALETTE_LIGHT[2] = t->block_b;
  BLOCK_PALETTE_DARK[2] = t->block_b_dark;
}

static void clearBoard(void) {
  BlockData *ptr = (BlockData *)board;
  for (int i = 0; i < TOTAL_CELLS; i++) {
    ptr[i].type = 0;
    ptr[i].marked = 0;
  }
}

void Grid_Init(void) {
  clearBoard();
  Grid_SetTheme(10);

  // Texture Loading
  LoadTexture((u_long *)bg_left_tim, &bgLeftInfo, TEX_BG_LEFT_X, TEX_BG_LEFT_Y);
  LoadTexture((u_long *)bg_right_tim, &bgRightInfo, TEX_BG_RIGHT_X, TEX_BG_RIGHT_Y);
  bgLeftInfo.mode = getTPage(2, 0, TEX_BG_LEFT_X, TEX_BG_LEFT_Y);
  bgRightInfo.mode = getTPage(2, 0, TEX_BG_RIGHT_X, TEX_BG_RIGHT_Y);

  score = 0;

  Player_Init();
}

// Timeline Logic

static void checkSweptColumn(int col) {
    int blocksCleared = 0;

    for (int y = 0; y < GRID_H; y++) {
        if (board[y][col].marked) {
            // Destroy Block
            board[y][col].type = 0;
            board[y][col].marked = 0;

            // Protect Right Neighbor
            // Neighbor will wait for timeline to be updated
            if (col < GRID_W - 1) {
                board[y][col + 1].protected = 1;
            }

            currentTimelineBlockCount += 1;
            blocksCleared = 1;
        }

        // Always remove protection from the block
        board[y][col].protected = 0;
    }

    if (blocksCleared) {
        doPhysicsUpdate = 1;
        gravityTimer = 0;
    }
}

static void UpdateTimeline(void) {
  timelinePixelX += TIMELINE_SPEED;

  if (timelinePixelX >= BLOCK_SIZE) {
    timelinePixelX = 0;

    checkSweptColumn(timelineGridX);

    timelineGridX++;

    if (timelineGridX >= GRID_W) {
      timelineGridX = 0;

      // Divide blocks by 4 to get the amount of squares
      score += (currentTimelineBlockCount >> 2) * BLOCK_SCORE_VALUE;

      currentTimelineBlockCount = 0;
    }
  }
}

// Block Match Logic
// Does NOT check bounds, caller beware.
static inline void checkSquareAt(int gridX, int gridY) {
    int type = board[gridY][gridX].type;

    if (type == 0) return; // No block here

    // Check Neighbors
    if (board[gridY][gridX + 1].type == type && board[gridY + 1][gridX].type == type && board[gridY + 1][gridX + 1].type == type) {
        board[gridY][gridX].marked = 1;
        board[gridY][gridX + 1].marked = 1;
        board[gridY + 1][gridX].marked = 1;
        board[gridY + 1][gridX + 1].marked = 1;
    }
}


void Grid_ScanNeighborhood(int targetX, int targetY) {
    // Off grid
    if (targetX < 0 || targetX >= GRID_W || targetY < 0 || targetY >= GRID_H) return;

    // Check Bottom Right Square
    if (targetX < GRID_W - 1 && targetY < GRID_H - 1) {
        checkSquareAt(targetX, targetY);
    }

    // Check Bottom Left Square
    if (targetX > 0 && targetY < GRID_H - 1) {
        checkSquareAt(targetX - 1, targetY);
    }

    // Check Top Right Square
    if (targetX < GRID_W - 1 && targetY > 0) {
        checkSquareAt(targetX, targetY - 1);
    }

    // Check Top Left Square
    if (targetX > 0 && targetY > 0) {
        checkSquareAt(targetX - 1, targetY - 1);
    }

}

void Grid_ValidateMatches(void) {
    // Unmark all
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if(board[y][x].protected) continue;

            board[y][x].marked = 0;
        }
    }

    for (int y = 0; y < GRID_H - 1; y++) {
        for (int x = 0; x < GRID_W - 1; x++) {
        int type = board[y][x].type;

        if (type == 0)
            continue;

        if (board[y][x + 1].type == type && board[y + 1][x].type == type && board[y + 1][x + 1].type == type) {
            board[y][x].marked = 1;
            board[y][x + 1].marked = 1;
            board[y + 1][x].marked = 1;
            board[y + 1][x + 1].marked = 1;
        }
        }
    }
}

// Collision Logic
int Grid_IsMoveValid(int nextX, int nextY) {
  // Bounds Check
  if (nextX < 0)
    return 0;
  if (nextX > GRID_W - 2)
    return 0;
  if (nextY > GRID_H - 2)
    return 0;

  // Ceiling Check (allow spawning above grid)
  if (nextY < -1)
    return 1;

  // "Portal" entry check (entering top of grid)
  if (nextY == -1) {
    if (board[0][nextX].type != 0)
      return 0;
    if (board[0][nextX + 1].type != 0)
      return 0;
    return 1;
  }

  // Standard Grid Collision
  if (board[nextY + 1][nextX].type != 0)
    return 0; // Bottom Left
  if (board[nextY + 1][nextX + 1].type != 0)
    return 0; // Bottom Right
  if (board[nextY][nextX].type != 0)
    return 0; // Top Left
  if (board[nextY][nextX + 1].type != 0)
    return 0; // Top Right

  return 1;
}

void Grid_PlaceBlock(ActivePiece *p) {
  int x = p->gridX;
  int y = p->gridY;

  // Safety check for array bounds
  if (x < 0 || x >= GRID_W - 1)
    return;

  // Place Top Half
  if (y >= 0) {
      board[y][x].type = p->cells[0];
      board[y][x + 1].type = p->cells[1];
      Grid_ScanNeighborhood(x, y);
      Grid_ScanNeighborhood(x + 1, y);
  }

  // Place Bottom Half if Inside Grid
  if (y + 1 >= 0) {
      board[y + 1][x].type = p->cells[2];
      board[y + 1][x + 1].type = p->cells[3];
      Grid_ScanNeighborhood(x, y + 1);
      Grid_ScanNeighborhood(x + 1, y + 1);
  }

  // Reset Player
  p->active = 0;
  doPhysicsUpdate = 1;
  gravityTimer = 0;
}

// Physics: Gravity for settled blocks
static void Grid_UpdatePhysics(void) {
  doPhysicsUpdate = 0;
  int stabilityChanged = 0;
  for (int y = GRID_H - 2; y >= 0; y--) {
    for (int x = 0; x < GRID_W; x++) {
      // If block exists and space below is empty
      if (board[y][x].type != 0 && board[y + 1][x].type == 0) {
        board[y + 1][x].type = board[y][x].type;
        board[y + 1][x].marked = 0;
        board[y + 1][x].protected = 0;

        board[y][x].type = 0;
        board[y][x].marked = 0;
        board[y][x].protected = 0;

        Grid_ScanNeighborhood(x, y + 1); // Scan new position for matches

        // If the block moved, check if it can move again next frame
        if (y + 2 < GRID_H && board[y + 2][x].type == 0) {
          doPhysicsUpdate = 1;
        }
        stabilityChanged = 1;
      }
    }
  }
  if (stabilityChanged) Grid_ValidateMatches();
}

void Grid_Update(void) {
  // Update Player Logic
  Player_Update();

  UpdateTimeline();

  // Update World Physics
  if (doPhysicsUpdate) {
    gravityTimer++;
  }

  if (gravityTimer >= GRAVITY_DELAY_FRAMES) {
    gravityTimer = 0;
    Grid_UpdatePhysics();
  }
}

// ---------------------------------------------------------
// Rendering Helpers (Kept internal to Grid for now)
// ---------------------------------------------------------

static void Draw_RawBlock(int x, int y, int type, int marked, int z_index) {
  if (type <= 0)
    return;
  CVECTOR *cLight = &BLOCK_PALETTE_LIGHT[type];
  CVECTOR *cDark = &BLOCK_PALETTE_DARK[type];

  if (marked) {
    Draw_Rect(x + 1, y + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cLight->r,
              cLight->g, cLight->b, z_index);
  } else {
    Draw_Rect(x + 2, y + 2, BLOCK_SIZE - 3, BLOCK_SIZE - 3, cLight->r,
              cLight->g, cLight->b, z_index);
    Draw_Rect(x + 1, y + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cDark->r, cDark->g,
              cDark->b, z_index);
  }
}

static void drawActiveBlock(int z_index) {
  if (!player.active)
    return;

  int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
  int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
  int originX = gridOffsetX - gridExtentX;
  int originY = gridOffsetY - gridExtentY;

  int baseX = originX + (player.gridX * BLOCK_SIZE);
  int baseY = originY + (player.gridY * BLOCK_SIZE);

  Draw_RawBlock(baseX, baseY, player.cells[0], 0, z_index);
  Draw_RawBlock(baseX + BLOCK_SIZE, baseY, player.cells[1], 0, z_index);
  Draw_RawBlock(baseX, baseY + BLOCK_SIZE, player.cells[2], 0, z_index);
  Draw_RawBlock(baseX + BLOCK_SIZE, baseY + BLOCK_SIZE, player.cells[3], 0,
                z_index);
}

static void drawTimeline(int z_index) {
  int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
  int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
  int originX = gridOffsetX - gridExtentX;
  int originY = gridOffsetY - gridExtentY;

  int lineX = originX + (timelineGridX * BLOCK_SIZE) + timelinePixelX;

  Draw_Rect(lineX, originY - BLOCK_SIZE, TIMELINE_WIDTH,
            (BLOCK_SIZE * GRID_H) + BLOCK_SIZE, 255, 127, 80, z_index);
}

static void drawGridLines(int z_lines, int z_bg) {
  int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
  int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;

  // Background Rect
  Draw_Rect_SemiTrans(gridOffsetX - gridExtentX, gridOffsetY - gridExtentY,
                      gridExtentX << 1, gridExtentY << 1, COLOR_GRID_BG, z_bg);

  int currentX = gridOffsetX + gridExtentX;
  int topY = gridOffsetY - gridExtentY;
  int botY = gridOffsetY + gridExtentY;

  // Vertical
  for (int i = 0; i <= GRID_W; i++) {
    if (currentX >= 0 && currentX <= SCREENXRES) {
      Draw_Line(currentX, topY, currentX, botY, COLOR_GRID_LINES, z_lines);
    }
    currentX -= BLOCK_SIZE;
  }

  // Horizontal
  int currentY = gridOffsetY + gridExtentY;
  int leftX = gridOffsetX - gridExtentX;
  int rightX = gridOffsetX + gridExtentX;
  for (int i = 1; i <= GRID_H; i++) {
    if (currentY >= 0 && currentY <= SCREENYRES) {
      Draw_Line(leftX, currentY, rightX, currentY, COLOR_GRID_LINES, z_lines);
    }
    currentY -= BLOCK_SIZE;
  }
}

void Grid_Draw(void) {
  // 1. Draw Background Sprites
  Draw_Sprite(0, 0, 0, 0, 160, 240, bgLeftInfo.mode, 7);
  Draw_Sprite(160, 0, 0, 0, 160, 240, bgRightInfo.mode, 7);

  // 2. Draw Static Grid
  int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
  int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
  int originX = gridOffsetX - gridExtentX;
  int originY = gridOffsetY - gridExtentY;

  for (int y = 0; y < GRID_H; y++) {
    for (int x = 0; x < GRID_W; x++) {
      if (board[y][x].type != 0) {
        int px = originX + (x * BLOCK_SIZE);
        int py = originY + (y * BLOCK_SIZE);
        Draw_RawBlock(px, py, board[y][x].type, board[y][x].marked, 4);
      }
    }
  }

  // 3. Draw Active Player
  drawActiveBlock(4);

  // 4. Draw Lines/UI
  drawGridLines(3, 5);

  // 5. Draw Timeline
  drawTimeline(2);

  FntPrint("Score: %d\n", score);
}
