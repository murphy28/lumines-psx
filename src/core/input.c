#include "input.h"
#include "../drivers/pad.h"
#include "libapi.h"

static GamePad _pad;

void Input_Init() {
    Pad_Init(&_pad, 0);
    InitPAD((char *)_pad.rawBuffer, 34, 0, 0);
    StartPAD();
}

void Input_Update() {
    Pad_Update(&_pad);
}

static unsigned short getMaskForAction(GameBinding bind) {
    switch(bind) {
        case MOVE_LEFT: return PAD_LEFT;
        case MOVE_RIGHT: return PAD_RIGHT;
        case SLAM: return PAD_DOWN;
        case ROTATE_CW: return PAD_CROSS | PAD_UP;
        case ROTATE_CCW: return PAD_CIRCLE;
        case CONFIRM: return PAD_START | PAD_CROSS;
        case CANCEL: return PAD_TRIANGLE;
        default: return 0;
    }
}

int Input_IsActionDown(GameBinding bind) {
    unsigned short mask = getMaskForAction(bind);
    return Pad_GetButtonDown(&_pad, mask);
}

int Input_IsActionHeld(GameBinding bind) {
    unsigned short mask = getMaskForAction(bind);
    return Pad_GetButton(&_pad, mask);
}

int Input_IsActionUp(GameBinding bind) {
    unsigned short mask = getMaskForAction(bind);
    return Pad_GetButtonUp(&_pad, mask);
}
