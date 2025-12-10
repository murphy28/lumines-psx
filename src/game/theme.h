#ifndef GAME_THEME_H
#define GAME_THEME_H

#include <sys/types.h>
#include <libgte.h>

typedef struct {
    char* name;
    CVECTOR block_a;
    CVECTOR block_b;
    CVECTOR block_a_dark;
    CVECTOR block_b_dark;
} Theme;

// Expose the library and count
extern const Theme THEME_LIBRARY[];
extern const int TOTAL_THEMES;

#endif
