#include "grid.h"
#include "theme.h"
#include "../core/gpu_prims.h"

// Assets
#include "../assets/bg_left.h"
#include "../assets/bg_right.h"

#define TOTAL_CELLS (GRID_W * GRID_H)
#define COLOR_GRID_LINES  40, 42, 44
#define COLOR_GRID_BG 0, 0, 0
#define GRAVITY_DELAY_FRAMES 3

typedef struct {
    unsigned char type;   // 0: Empty; 1: Color A; 2: Color B;
    unsigned char marked; // 0: Unmarked; 1: Marked;
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

void Grid_SetTheme(int themeIndex) {
    if (themeIndex < 0) themeIndex = TOTAL_THEMES - 1;
    if (themeIndex >= TOTAL_THEMES) themeIndex = 0;

    currentThemeIndex = themeIndex;
    const Theme* t = &THEME_LIBRARY[currentThemeIndex];

    BLOCK_PALETTE_LIGHT[0] = (CVECTOR){0,0,0,0};
    BLOCK_PALETTE_DARK[0] = (CVECTOR){0,0,0,0};
    BLOCK_PALETTE_LIGHT[1] = t->block_a;
    BLOCK_PALETTE_DARK[1] = t->block_a_dark;
    BLOCK_PALETTE_LIGHT[2] = t->block_b;
    BLOCK_PALETTE_DARK[2] = t->block_b_dark;
}

static void clearBoard(void) {
    BlockData *ptr = (BlockData*)board;
    for (int i = 0; i < TOTAL_CELLS; i++) {
        ptr[i].type = 0;
        ptr[i].marked = 0;
    }
}

void Grid_Init(void) {
    clearBoard();
    Grid_SetTheme(10);

    // Texture Loading
    LoadTexture((u_long*)bg_left_tim, &bgLeftInfo, 320, 0);
    LoadTexture((u_long*)bg_right_tim, &bgRightInfo, 512, 0);
    bgLeftInfo.mode = getTPage(2, 0, 320, 0);
    bgRightInfo.mode = getTPage(2, 0, 512, 0);

    Player_Init();
}

// Collision Logic
int Grid_IsMoveValid(int nextX, int nextY) {
    // Bounds Check
    if (nextX < 0) return 0;
    if (nextX > GRID_W - 2) return 0;
    if (nextY > GRID_H - 2) return 0;

    // Ceiling Check (allow spawning above grid)
    if (nextY < -1) return 1;

    // "Portal" entry check (entering top of grid)
    if (nextY == -1) {
        if (board[0][nextX].type != 0) return 0;
        if (board[0][nextX + 1].type != 0) return 0;
        return 1;
    }

    // Standard Grid Collision
    if (board[nextY + 1][nextX].type != 0) return 0;     // Bottom Left
    if (board[nextY + 1][nextX + 1].type != 0) return 0; // Bottom Right
    if (board[nextY][nextX].type != 0) return 0;         // Top Left
    if (board[nextY][nextX + 1].type != 0) return 0;     // Top Right

    return 1;
}

void Grid_PlaceBlock(ActivePiece* p) {
    int x = p->gridX;
    int y = p->gridY;

    // Safety check for array bounds
    if (y < 0 || y >= GRID_H - 1 || x < 0 || x >= GRID_W - 1) return;

    board[y][x].type = p->cells[0];
    board[y][x+1].type = p->cells[1];
    board[y+1][x].type = p->cells[2];
    board[y+1][x+1].type = p->cells[3];

    p->active = 0;
    doPhysicsUpdate = 1;
    gravityTimer = 0;
}

// Physics: Gravity for settled blocks
static void Grid_UpdatePhysics(void) {
    doPhysicsUpdate = 0;
    for (int y = GRID_H - 2; y >= 0; y--) {
        for (int x = 0; x < GRID_W; x++) {
            // If block exists and space below is empty
            if (board[y][x].type != 0 && board[y + 1][x].type == 0) {
                board[y + 1][x].type = board[y][x].type;
                board[y][x].type = 0;

                // If the block moved, check if it can move again next frame
                if(y + 2 < GRID_H && board[y + 2][x].type == 0) {
                    doPhysicsUpdate = 1;
                }
            }
        }
    }
}

void Grid_Update(void) {
    // Update Player Logic
    Player_Update();

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
    if (type <= 0) return;
    CVECTOR *cLight = &BLOCK_PALETTE_LIGHT[type];
    CVECTOR *cDark = &BLOCK_PALETTE_DARK[type];

    if (marked) {
        Draw_Rect(x + 1, y + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cLight->r, cLight->g, cLight->b, z_index);
    } else {
        Draw_Rect(x + 2, y + 2, BLOCK_SIZE - 3, BLOCK_SIZE - 3, cLight->r, cLight->g, cLight->b, z_index);
        Draw_Rect(x + 1, y + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cDark->r, cDark->g, cDark->b, z_index);
    }
}

static void drawActiveBlock(int z_index) {
    if(!player.active) return;

    int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
    int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
    int originX = gridOffsetX - gridExtentX;
    int originY = gridOffsetY - gridExtentY;

    int baseX = originX + (player.gridX * BLOCK_SIZE);
    int baseY = originY + (player.gridY * BLOCK_SIZE);

    Draw_RawBlock(baseX, baseY, player.cells[0], 0, z_index);
    Draw_RawBlock(baseX + BLOCK_SIZE, baseY, player.cells[1], 0, z_index);
    Draw_RawBlock(baseX, baseY + BLOCK_SIZE, player.cells[2], 0, z_index);
    Draw_RawBlock(baseX + BLOCK_SIZE, baseY + BLOCK_SIZE, player.cells[3], 0, z_index);
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
}
