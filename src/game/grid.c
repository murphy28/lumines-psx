#include "grid.h"
#include "../core/gpu_prims.h"

// Assets
#include "../assets/bg_left.h"
#include "../assets/bg_right.h"
#include "libgte.h"
#include <stdlib.h>

// Constants
#define BLOCK_SIZE  16
#define GRID_W      16
#define GRID_H      10
#define TOTAL_CELLS (GRID_W * GRID_H)

// Colors
#define COLOR_GRID_LINES  40, 42, 44
#define COLOR_GRID_BG 0, 0, 0

// Data Structures
typedef struct {
    unsigned char type;   // 0: Empty; 1: Color A; 2: Color B;
    unsigned char marked; // 0: Unmarked; 1: Marked;
} BlockData;

typedef struct {
    char* name;
    CVECTOR block_a;
    CVECTOR block_b;
    CVECTOR block_a_dark;
    CVECTOR block_b_dark;
} Theme;

// Theme Data
static const Theme THEME_LIBRARY[] = {
    { "Vaporwave", {255,105,180}, {0,255,255},   {199,20,133}, {0,139,139} },
    { "Sunset",    {255,165,0},   {147,112,219}, {200,80,0},   {75,0,130} },
    { "Forest",    {220,20,60},   {154,205,50},  {139,0,0},    {85,107,47} },
    { "Arcade",    {255,50,50},   {50,100,255},  {150,0,0},    {0,0,139} },
    { "Citrus",    {50,205,50},   {255,215,0},   {0,100,0},    {184,134,11} },
    { "Pastel",    {152,251,152}, {221,160,221}, {46,139,87},  {128,0,128} },
    { "Industrial",{210,105,30},  {176,196,222}, {139,69,19},  {70,130,180} },
    { "Candy",     {255,127,80},  {64,224,208},  {178,34,34},  {0,128,128} },
    { "Royal",     {238,232,170}, {65,105,225},  {184,134,11}, {25,25,112} },
    { "Halloween", {255,140,0},   {127,255,0},   {100,50,0},   {60,120,0} },
    { "Arctic",    {224,255,255}, {70,130,180},  {95,158,160}, {25,25,112} },
    { "Terminal",  {255,191,0},   {0,255,65},    {139,69,0},   {0,100,0} },
    { "Coffee",    {245,222,179}, {210,180,140}, {160,82,45},  {101,67,33} },
    { "Electric",  {138,43,226},  {200,255,0},   {75,0,130},   {100,128,0} },
    { "Rose Sky",  {255,182,193}, {135,206,235}, {219,112,147},{70,130,180} }
};

#define DROP_DELAY_FRAMES 30
#define GRAVITY_DELAY_FRAMES 3

typedef struct {
    int gridX;
    int gridY;
    int cells[4]; // 0:TL; 1:TR; 2:BL; 3:BR;
    int active;

    int dropTimer;

    int dropLock;   // If 1, pull to floor instantly
    int slamLatch;  // If 1, ignore slam button

    int graceCycles; // Begins at 3, the amount of frames to ignore gravity, typically when starting
} ActivePiece;

static ActivePiece player;

static const int BLOCK_PATTERNS[][4] = {
    {1, 1, 1, 1},
    {2, 2, 2, 2},

    {1, 2, 1, 2},
    {1, 1, 2, 2},

    {1, 2, 2, 1},
    {2, 1, 1, 2},

    {1, 1, 1, 2},
    {2, 2, 2, 1},
};

#define PATTERN_COUNT (sizeof(BLOCK_PATTERNS) / sizeof(BLOCK_PATTERNS[0]))

#define TOTAL_THEMES (sizeof(THEME_LIBRARY) / sizeof(Theme))

static CVECTOR BLOCK_PALETTE_LIGHT[3];
static CVECTOR BLOCK_PALETTE_DARK[3];
static int currentThemeIndex = 0;

// State
static BlockData board[GRID_H][GRID_W];
static int gridOffsetX = CENTERX;
static int gridOffsetY = CENTERY;
static TIM_IMAGE bgLeftInfo;
static TIM_IMAGE bgRightInfo;

// Set Theme
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

// Internal Helper
static void clearBoard(void) {
    BlockData *ptr = (BlockData*)board;
    for (int i = 0; i < TOTAL_CELLS; i++) {
        ptr[i].type = 0;
        ptr[i].marked = 0;
    }
}


void Player_Init(void) {
    player.active = 1;
    player.gridX = (GRID_W / 2) - 1;
    player.gridY = -2;
    player.dropTimer = 0;
    player.dropLock = 0;

    player.graceCycles = 3;

    int p = rand() % PATTERN_COUNT;

    player.cells[0] = BLOCK_PATTERNS[p][0];
    player.cells[1] = BLOCK_PATTERNS[p][1];
    player.cells[2] = BLOCK_PATTERNS[p][2];
    player.cells[3] = BLOCK_PATTERNS[p][3];
}

void Grid_Init(void) {
    clearBoard();

    Grid_SetTheme(10);

    // Load Textures
    LoadTexture((u_long*)bg_left_tim, &bgLeftInfo, 320, 0);
    LoadTexture((u_long*)bg_right_tim, &bgRightInfo, 512, 0);
    bgLeftInfo.mode = getTPage(2, 0, 320, 0);
    bgRightInfo.mode = getTPage(2, 0, 512, 0);

    Player_Init();
}

void Grid_UpdateOffset(int dx, int dy) {
    gridOffsetX += dx;
    gridOffsetY += dy;
}

static int CanMove(int dx, int dy) {
    int nextX = player.gridX + dx;
    int nextY = player.gridY + dy;

    if (nextX < 0) return 0; // Left Bounds
    if (nextX > GRID_W - 2) return 0; // Right Bounds
    if (nextY > GRID_H - 2) return 0; // Bottom Bounds

    if (nextY < -1) return 1;

    if (nextY == -1) {
        if (board[0][nextX].type != 0) return 0;     // Bottom Left vs Grid Top Left
        if (board[0][nextX + 1].type != 0) return 0; // Bottom Right vs Grid Top Right
        return 1;
    }

    if (board[nextY + 1][nextX].type != 0) return 0;        // Bottom Left
    if (board[nextY + 1][nextX + 1].type != 0) return 0;    // Bottom Right

    if (board[nextY][nextX].type != 0) return 0;        // Top Left
    if (board[nextY][nextX + 1].type != 0) return 0;    // Top Right

    return 1;
}

static void PlaceBlockIntoGrid(void) {
    int x = player.gridX;
    int y = player.gridY;

    board[y][x].type = player.cells[0]; // Top Left
    board[y][x+1].type = player.cells[1]; // Top Right

    board[y+1][x].type = player.cells[2]; // Bottom Left
    board[y+1][x+1].type = player.cells[3]; // Bottom Right

    player.active = 0;
}

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

    Draw_RawBlock(baseX, baseY, player.cells[0], 0, z_index); // Top Left Block
    Draw_RawBlock(baseX + BLOCK_SIZE, baseY, player.cells[1], 0, z_index); // Top Right Block

    Draw_RawBlock(baseX, baseY + BLOCK_SIZE, player.cells[2], 0, z_index); // Bottom Left Block
    Draw_RawBlock(baseX + BLOCK_SIZE, baseY + BLOCK_SIZE, player.cells[3], 0, z_index); // Bottom Right Block

}

static void drawGridBlock(int col, int row, BlockData *data, int z_index) {
    int gridExtentX = (BLOCK_SIZE * GRID_W) >> 1;
    int gridExtentY = (BLOCK_SIZE * GRID_H) >> 1;
    int originX = gridOffsetX - gridExtentX;
    int originY = gridOffsetY - gridExtentY;
    int blockX = originX + (col * BLOCK_SIZE);
    int blockY = originY + (row * BLOCK_SIZE);

    Draw_RawBlock(blockX, blockY, data->type, data->marked, z_index);
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

    // Draw static blocks
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (board[y][x].type != 0) {
                drawGridBlock(x, y, &board[y][x], 4);
            }
        }
    }

    drawActiveBlock(4);

    drawGridLines(3, 5);
}

void Player_MoveLeft() {
    if (CanMove(-1, 0)) {
        player.gridX--;
    }
}

void Player_MoveRight() {
    if (CanMove(1, 0)) {
        player.gridX++;
    }
}

void Player_SlamBlock() {
    if (player.slamLatch) return;

    player.dropLock = 1;

    player.slamLatch = 1;
}

void Player_RotateClockwise() {
    int c0 = player.cells[0];
    int c1 = player.cells[1];
    int c2 = player.cells[2];
    int c3 = player.cells[3];

    player.cells[0] = c2;
    player.cells[1] = c0;
    player.cells[2] = c3;
    player.cells[3] = c1;
}

void Player_RotateCounterClockwise() {
    int c0 = player.cells[0];
    int c1 = player.cells[1];
    int c2 = player.cells[2];
    int c3 = player.cells[3];

    player.cells[0] = c1;
    player.cells[1] = c3;
    player.cells[2] = c0;
    player.cells[3] = c2;
}

void Player_UnlockDrop() {
    player.slamLatch = 0;
}

int gravityTimer = 0;

int doPhysicsUpdate = 0;


void Grid_UpdatePhysics(void) {
    doPhysicsUpdate = 0;

    for (int y = GRID_H - 2; y >= 0; y--) {
        for (int x = 0; x < GRID_W; x++) {
            if (board[y][x].type != 0 && board[y + 1][x].type == 0) {
                board[y + 1][x].type = board[y][x].type;
                board[y][x].type = 0;

                if(y + 2 < GRID_H && board[y + 2][x].type == 0) {
                    doPhysicsUpdate = 1; // More physics must be done
                }
            }
        }
    }
}
void Grid_Update(void) {
    if(!player.active) {
        Player_Init();
        return;
    }

    if (player.dropLock) {
        while (CanMove(0, 1)) {
            player.gridY++;
        }

        player.dropTimer = DROP_DELAY_FRAMES + 1;
    } else {
        player.dropTimer++;
    }

    if (doPhysicsUpdate) {
        gravityTimer++;
    }

    if (gravityTimer >= GRAVITY_DELAY_FRAMES) {
        gravityTimer = 0;
        Grid_UpdatePhysics();
    }

    if (player.dropTimer >= DROP_DELAY_FRAMES) {
        player.dropTimer = 0;
        if (player.graceCycles > 0) {
            player.graceCycles--;
            return;
        }

        if (CanMove(0, 1)) {
            player.gridY++;
        } else {
            if (player.gridY < -1) {
                Grid_Init();
                return;
            }

            PlaceBlockIntoGrid();
            doPhysicsUpdate = 1;
            gravityTimer = 0;
        }
    }

}
