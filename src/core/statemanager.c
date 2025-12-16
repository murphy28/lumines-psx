#include "statemanager.h"
#include <stddef.h>

typedef void (*StateFunc)(void);

static StateFunc _currentInit = NULL;
static StateFunc _currentUpdate = NULL;
static StateFunc _currentExit = NULL;

static GameState _nextState = STATE_BOOT;
static int _pendingChange = 0;

#include "../states/title.h"
#include "../states/arcade.h"

static void setupStatePointers(GameState state) {
    switch(state) {
        case STATE_TITLE:
            _currentInit = StateTitle_Init;
            _currentUpdate = StateTitle_Update;
            _currentExit = StateTitle_Exit;
            break;
        case STATE_ARCADE:
            _currentInit = StateArcade_Init;
            _currentUpdate = StateArcade_Update;
            _currentExit = StateArcade_Exit;
            break;
        default:
            _currentInit = NULL;
            _currentUpdate = NULL;
            _currentExit = NULL;
            break;
    }
}

void StateManager_Init() {
    _nextState = STATE_TITLE;
    _pendingChange = 1;
}

void StateManager_Update() {
    if (_pendingChange) {
        if (_currentExit != NULL) _currentExit();

        setupStatePointers(_nextState);

        if (_currentInit != NULL) _currentInit();

        _pendingChange = 0;
    }

    if (_currentUpdate != NULL) _currentUpdate();
}

void StateManager_ChangeState(GameState newState) {
    _nextState = newState;
    _pendingChange = 1;
}
