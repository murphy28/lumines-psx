#ifndef PAD_H
#define PAD_H

#include <sys/types.h>
#include <libetc.h>

#define PAD_SELECT      1
#define PAD_L3          2
#define PAD_R3          4
#define PAD_START       8
#define PAD_UP          16
#define PAD_RIGHT       32
#define PAD_DOWN        64
#define PAD_LEFT        128
#define PAD_L2          256
#define PAD_R2          512
#define PAD_L1          1024
#define PAD_R1          2048
#define PAD_TRIANGLE    4096
#define PAD_CIRCLE      8192
#define PAD_CROSS       16384
#define PAD_SQUARE      32768

typedef struct {
    unsigned char rawBuffer[34];

    unsigned short btnState;
    unsigned short lastBtnState;

    unsigned char lx, ly;
    unsigned char rx, ry;

    int port;
    int connected;
} GamePad;

void Pad_Init(GamePad* pad, int port);

void Pad_Update(GamePad* pad);

int Pad_GetButton(GamePad* pad, unsigned short mask);
int Pad_GetButtonDown(GamePad* pad, unsigned short mask);
int Pad_GetButtonUp(GamePad* pad, unsigned short mask);

#endif
