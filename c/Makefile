CC = gcc
CFLAGS_COMMON = -Wall -Wextra -pedantic
SRC = src/chatbot.c

all: chat-c17 chat-c23

chat-c17:
	$(CC) -std=c17 $(CFLAGS_COMMON) $(SRC) -o chat-c17

chat-c23:
	$(CC) -std=c23 $(CFLAGS_COMMON) $(SRC) -o chat-c23 2>/dev/null || \
	$(CC) -std=c2x $(CFLAGS_COMMON) $(SRC) -o chat-c23

# Legacy target for backwards compatibility
chat: chat-c17
	cp chat-c17 chat

clean:
	rm -rf *.o chat chat-c17 chat-c23
