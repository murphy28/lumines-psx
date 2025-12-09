#include "grid.h"
#include "../core/gpu_prims.h"

// Assets
#include "../assets/bg_left.h"
#include "../assets/bg_right.h"

// Constants
#define BLOCK_SIZE  16
#define GRID_W      16
#define GRID_H      10
#define TOTAL_CELLS (GRID_W * GRID_H)

// Colors
#define COLOR_BLOCK_A 255, 153, 36
#define COLOR_BLOCK_B 219, 219, 219
#define COLOR_BLOCK_A_DARK 184, 105, 15
#define COLOR_BLOCK_B_DARK 143, 143, 143
#define COLOR_GRID_LINES  40, 42, 44
#define COLOR_GRID_BG 0, 0, 0

// Data Structures
typedef struct {
    unsigned char type;   // 0: Empty; 1: Color A; 2: Color B;
    unsigned char marked; // 0: Unmarked; 1: Marked;
} BlockData;

// Palettes
static CVECTOR BLOCK_PALETTE_LIGHT[] = {
    {0, 0, 0, 0},
    {COLOR_BLOCK_A, 0},
    {COLOR_BLOCK_B, 0}
};

static CVECTOR BLOCK_PALETTE_DARK[] = {
    {0, 0, 0, 0},
    {COLOR_BLOCK_A_DARK, 0},
    {COLOR_BLOCK_B_DARK, 0}
};

// State
static BlockData board[GRID_H][GRID_W];
static int gridOffsetX = CENTERX;
static int gridOffsetY = CENTERY;
static TIM_IMAGE bgLeftInfo;
static TIM_IMAGE bgRightInfo;

// Internal Helper
static void clearBoard(void) {
    BlockData *ptr = (BlockData*)board;
    for (int i = 0; i < TOTAL_CELLS; i++) {
        ptr[i].type = 0;
        ptr[i].marked = 0;
    }
}

// Setup Test Data
static void setupTestData(void) {
    clearBoard();
    board[9][2].type = 1;
    board[8][2].type = 2;
    board[7][2].type = 1;
    board[6][2].type = 1;

    board[9][3].type = 1;

    board[9][4].type = 1;
    board[8][4].type = 2;

    board[9][5].type = 2;
    board[8][5].type = 1;

    board[9][6].type = 1;
    board[8][6].type = 2;
    board[7][6].type = 1;

    board[9][7].type = 1;
    board[8][7].type = 2;
    board[7][7].type = 1;
    board[6][7].type = 2;

    board[9][8].type = 1;
    board[8][8].type = 2;

    board[9][9].type = 1;
    board[8][9].type = 2;
    board[7][9].type = 2;
    board[8][9].marked = 1;
    board[7][9].marked = 1;
    board[6][9].type = 1;

    board[9][10].type = 1;
    board[8][10].type = 2;
    board[7][10].type = 2;
    board[8][10].marked = 1;
    board[7][10].marked = 1;

    board[9][11].type = 1;

    board[9][12].type = 2;
    board[8][12].type = 2;

}

void Grid_Init(void) {
    setupTestData();

    // Load Textures
    LoadTexture((u_long*)bg_left_tim, &bgLeftInfo, 320, 0);
    LoadTexture((u_long*)bg_right_tim, &bgRightInfo, 512, 0);
    bgLeftInfo.mode = getTPage(2, 0, 320, 0);
    bgRightInfo.mode = getTPage(2, 0, 512, 0);
}

void Grid_UpdateOffset(int dx, int dy) {
    gridOffsetX += dx;
    gridOffsetY += dy;
}

static void drawBlock(int col, int row, BlockData *data, int z_index) {
    int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
    int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
    int originX = gridOffsetX - gridExtentX;
    int originY = gridOffsetY - gridExtentY;
    int blockX = originX + (col * BLOCK_SIZE);
    int blockY = originY + (row * BLOCK_SIZE);

    if (data->type <= 0 || data->type >= 3) return;

    CVECTOR *cLight = &BLOCK_PALETTE_LIGHT[data->type];
    CVECTOR *cDark = &BLOCK_PALETTE_DARK[data->type];

    if (data->marked) {
        Draw_Rect(blockX + 1, blockY + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cLight->r, cLight->g, cLight->b, z_index);
    } else {
        Draw_Rect(blockX + 2, blockY + 2, BLOCK_SIZE - 3, BLOCK_SIZE - 3, cLight->r, cLight->g, cLight->b, z_index);
        Draw_Rect(blockX + 1, blockY + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1, cDark->r, cDark->g, cDark->b, z_index);
    }
}

static void drawBackground(int z_index) {
    // Left half
    Draw_Sprite(0, 0, 0, 0, 160, 240, bgLeftInfo.mode, z_index);
    // Right half
    Draw_Sprite(160, 0, 0, 0, 160, 240, bgRightInfo.mode, z_index);
}

static void drawGridLines(int z_lines, int z_bg) {
    int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
    int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;

    // Background Rect
    Draw_Rect_SemiTrans(gridOffsetX - gridExtentX, gridOffsetY - gridExtentY,
                        gridExtentX << 1, gridExtentY << 1, COLOR_GRID_BG, z_bg);

    // Vertical Lines
    int currentX = gridOffsetX + gridExtentX;
    int topY = gridOffsetY - gridExtentY;
    int botY = gridOffsetY + gridExtentY;

    for (int i = 0; i <= GRID_W; i++) {
        if (currentX >= 0 && currentX <= SCREENXRES) {
            Draw_Line(currentX, topY, currentX, botY, COLOR_GRID_LINES, z_lines);
        }
        currentX -= BLOCK_SIZE;
    }

    // Horizontal Lines
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
    drawBackground(7); // Deepest Z

    // Draw active blocks
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (board[y][x].type != 0) {
                drawBlock(x, y, &board[y][x], 4);
            }
        }
    }

    drawGridLines(3, 5);
}
