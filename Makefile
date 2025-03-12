CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
SQLITE_CFLAGS = $(shell pkg-config --cflags sqlite3)
SQLITE_LIBS = $(shell pkg-config --libs sqlite3)
JANSSON_CFLAGS = $(shell pkg-config --cflags jansson)
JANSSON_LIBS = $(shell pkg-config --libs jansson)

CFLAGS += $(SQLITE_CFLAGS) $(JANSSON_CFLAGS)
LDFLAGS = $(SQLITE_LIBS) $(JANSSON_LIBS)

TARGET = chat
SEEDER = seed_db
SRCS = src/chatbot.c
SEEDER_SRCS = src/seed_db.c

.PHONY: all clean seed

all: $(TARGET) seed

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(SEEDER): $(SEEDER_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

seed: $(SEEDER)
	./$(SEEDER)

clean:
	rm -f $(TARGET) $(SEEDER)
	rm -f chatbot.db
	rm -f chatbot.log
