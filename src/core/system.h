#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libapi.h>

// Configuration
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX    (SCREENXRES/2)
#define CENTERY    (SCREENYRES/2)
#define OTLEN      8

// Globals required by the inline renderer
extern u_long ot[2][OTLEN];
extern char *nextpri;
extern short db;

// System API
void System_Init(void);
void System_ClearOT(void);
void System_Display(void);

#endif
