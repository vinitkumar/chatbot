CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -I./include
LDFLAGS = -lsqlite3

# Source files
SRCS = src/chatbot.c src/db.c src/context.c src/ui.c
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = chat

.PHONY: all clean

all: clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c include/chatbot.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f chatbot.db
	rm -f chatbot.log
