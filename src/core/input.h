#ifndef CORE_INPUT_H
#define CORE_INPUT_H

typedef enum {
    NONE = 0,
    MOVE_LEFT,
    MOVE_RIGHT,
    SLAM,
    ROTATE_CW,
    ROTATE_CCW,
    CONFIRM,
    CANCEL
} GameBinding;

void Input_Init(void);

void Input_Update(void);

int Input_IsActionDown(GameBinding bind);
int Input_IsActionHeld(GameBinding bind);
int Input_IsActionUp(GameBinding bind);

#endif
