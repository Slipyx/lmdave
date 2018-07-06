# Detect OS

ifeq ($(OS),Windows_NT)
	ISWIN := 1
endif

ifdef ISWIN
	CC := i686-w64-mingw32-gcc
else
	CC := gcc
endif

RM ?= rm -f
RMDIR ?= rm -rf
MKDIR ?= mkdir -p

WARFLAGS := -Wall #-Wextra -pedantic

CFLAGS_g := $(WARFLAGS) -std=gnu11 -O2 -msse2 -ffast-math -mfpmath=sse -DNDEBUG \
	-MMD

LDFLAGS_g := -s

# object output dir
OBJDIR_g := obj

SRCS := $(wildcard *.c)
SRCS += $(wildcard util/*.c)
OBJS_t := $(SRCS:.c=.o)

# include dirs
CFLAGS_g +=

# libs
ifdef ISWIN
	LDFLAGS_g += 
else
	CFLAGS_g += -I/usr/include/SDL2
	LDFLAGS_g += -lSDL2
endif

# binary target
ifdef ISWIN
	TARG_t := lmdave.exe
else
	TARG_t := lmdave
endif

game: $(TARG_t)
	@echo "*** Target '$@': '$^' is up to date!"

default: game

.PHONY: default clean game

# rewrite OBJS so it outputs object files to seperate dir

OBJS_t := $(patsubst %,$(OBJDIR_g)/%,$(OBJS_t))

# include the dependency rules generated by -MMD

-include $(OBJS_t:.o=.d)

clean:
	@echo "*** Removing target binary and object directory..."
	@$(RM) $(TARG_t)
	@$(RMDIR) $(OBJDIR_g)
	@echo "*** Done."

# Compile

$(OBJDIR_g)/%.o: %.c
	@$(MKDIR) $(@D)
	@echo "*** Compiling '$<' ..."
	@$(CC) $(CFLAGS_g) -c $< -o $@

# Link

$(TARG_t): $(OBJS_t)
	@$(MKDIR) $(@D)
	@echo "\n*** Linking binary target '$@' ...\n"
	@$(CC) $(OBJS_t) -o $@ $(LDFLAGS_g)

