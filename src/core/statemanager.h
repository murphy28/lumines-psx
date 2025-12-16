#ifndef CORE_STATEMANAGER_H
#define CORE_STATEMANAGER_H

typedef enum {
    STATE_BOOT,
    STATE_TITLE,
    STATE_ARCADE,
    STATE_GAMEOVER,
    STATE_PAUSE
} GameState;

void StateManager_Init(void);
void StateManager_Update(void);
void StateManager_ChangeState(GameState newState);

#endif
