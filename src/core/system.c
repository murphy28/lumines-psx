#include "system.h"

#define VMODE 0 // 0: NTSC, 1: PAL

// Double buffered DISPENV and DRAWENV
DISPENV disp[2];
DRAWENV draw[2];

// Double ordering table
u_long ot[2][OTLEN];

// Double primitive buffer
// 32768 bytes is a safe default for 2D games
char primbuff[2][32768];

// Pointer to the next primitive
char *nextpri = primbuff[0];

// Current buffer index
short db = 0;

void System_Init(void) {
    // Reset GPU
    ResetGraph(0);

    // Define display environments
    SetDefDispEnv(&disp[0], 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);

    if (VMODE) {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }

    SetDispMask(1);

    // Set clear color (Dark Grey)
    setRGB0(&draw[0], 19, 19, 19);
    setRGB0(&draw[1], 19, 19, 19);

    // Enable background clearing
    draw[0].isbg = 1;
    draw[1].isbg = 1;

    // Apply initial state
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    // Load standard font
    FntLoad(960, 0);
    FntOpen(32, 20, 260, 200, 0, 512);
}

void System_ClearOT(void) {
    // Clear the Ordering Table for the current buffer
    ClearOTagR(ot[db], OTLEN);
}

void System_Display(void) {
    // Wait for GPU to finish
    DrawSync(0);

    // Wait for V-Blank
    VSync(0);

    // Swap buffers
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    // Send OT to GPU
    DrawOTag(&ot[db][OTLEN - 1]);

    // Flip index
    db = !db;

    // Reset primitive pointer to start of new buffer
    nextpri = primbuff[db];
}
