TARGET = lumines

# 1. Directories (Relative to THIS Makefile)
# We are already inside 'src', so we do not prefix it.
CORE_DIR    = core
GAME_DIR    = game
DRIVERS_DIR = drivers

# 2. Source Files
SRCS = main.c \
       $(CORE_DIR)/system.c \
       $(CORE_DIR)/input.c \
       $(GAME_DIR)/grid.c \
       $(GAME_DIR)/player.c \
       $(GAME_DIR)/theme.c \
       $(GAME_DIR)/session.c \
       $(DRIVERS_DIR)/pad.c

# 3. Include Paths
# -I. tells the compiler to look in the current folder (for main.c/headers)
INCLUDES = -I. -I$(CORE_DIR) -I$(GAME_DIR) -I$(DRIVERS_DIR)

# 4. Compiler Flags
CFLAGS += $(INCLUDES)

# 5. The Build Logic
include ../common.mk
