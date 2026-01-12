# Detect real GCC vs Apple's gcc alias
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS: gcc is actually clang, use explicit paths if available
    GCC := $(shell which gcc-14 gcc-13 gcc-12 2>/dev/null | head -1)
    ifeq ($(GCC),)
        GCC := gcc
        GCC_NOTE := "(Apple Clang)"
    endif
else
    GCC := gcc
endif

CLANG := clang
CFLAGS_COMMON = -Wall -Wextra -pedantic
SRC = src/chatbot.c

# Default targets
all: gcc-builds clang-builds

# GCC builds
gcc-builds: chat-gcc-c17 chat-gcc-c23

chat-gcc-c17:
	$(GCC) -std=c17 $(CFLAGS_COMMON) $(SRC) -o chat-gcc-c17

chat-gcc-c23:
	$(GCC) -std=c23 $(CFLAGS_COMMON) $(SRC) -o chat-gcc-c23 2>/dev/null || \
	$(GCC) -std=c2x $(CFLAGS_COMMON) $(SRC) -o chat-gcc-c23

# Clang builds
clang-builds: chat-clang-c17 chat-clang-c23

chat-clang-c17:
	$(CLANG) -std=c17 $(CFLAGS_COMMON) $(SRC) -o chat-clang-c17

chat-clang-c23:
	$(CLANG) -std=c23 $(CFLAGS_COMMON) $(SRC) -o chat-clang-c23 2>/dev/null || \
	$(CLANG) -std=c2x $(CFLAGS_COMMON) $(SRC) -o chat-clang-c23

# Legacy targets for backwards compatibility
chat: chat-gcc-c17
	cp chat-gcc-c17 chat

chat-c17: chat-gcc-c17
	cp chat-gcc-c17 chat-c17

chat-c23: chat-gcc-c23
	cp chat-gcc-c23 chat-c23

clean:
	rm -rf *.o chat chat-c17 chat-c23 chat-gcc-* chat-clang-*

info:
	@echo "GCC: $(GCC) $(GCC_NOTE)"
	@echo "Clang: $(CLANG)"
