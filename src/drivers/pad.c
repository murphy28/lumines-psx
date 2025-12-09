#include "pad.h"

void Pad_Init(GamePad *pad, int port) {
    pad->port = port;
    pad->btnState = 0;
    pad->lastBtnState = 0;
    pad->connected = 0;
}

void Pad_Update(GamePad *pad) {
    pad->lastBtnState = pad->btnState;

    if (pad->rawBuffer[0] != 0x00) {
        pad->connected = 0;
        pad->btnState = 0;
        return;
    }

    pad->connected = 1;

    unsigned short raw = pad->rawBuffer[2] | (pad->rawBuffer[3] << 8);
    pad->btnState = ~raw;pad->lx =

    pad->rawBuffer[4]; // Left X
    pad->ly = pad->rawBuffer[5]; // Left Y
    pad->rx = pad->rawBuffer[6]; // Right X
    pad->ry = pad->rawBuffer[7]; // Right Y
}

int Pad_GetButton(GamePad *pad, unsigned short mask) {
    return (pad->btnState & mask) ? 1 : 0;
}

int Pad_GetButtonDown(GamePad *pad, unsigned short mask) {
    return ((pad->btnState & mask) && !(pad->lastBtnState & mask)) ? 1 : 0;
}

int Pad_GetButtonUp(GamePad *pad, unsigned short mask) {
    return (!(pad->btnState & mask) && (pad->lastBtnState & mask)) ? 1 : 0;
}
