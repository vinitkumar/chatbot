#ifndef CHATBOT_H
#define CHATBOT_H

#include <sqlite3.h>

/* Hash table structures */
struct entry_s {
    char *key;
    char *value;
    struct entry_s *next;
};
typedef struct entry_s entry_t;

struct hashtable_s {
    int size;
    struct entry_s **table;
};
typedef struct hashtable_s hashtable_t;

/* ANSI Color codes for prettier output */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"

/* Configuration constants */
#define MAX_RESPONSE_LENGTH 1024
#define MAX_INPUT_LENGTH 1024
#define LINELENGTH 1024
#define MAX_CONTEXT_LENGTH 5
#define CONFIG_FILE "chatbot_responses.txt"
#define LOG_FILE "chatbot.log"
#define TYPING_DELAY 50000  /* 50ms delay between characters */
#define DB_FILE "chatbot.db"
#define SEPCHARS " \t\r\n,."

/* SQL statements for table creation */
#define CREATE_RESPONSES_TABLE "\
    CREATE TABLE IF NOT EXISTS responses ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        pattern TEXT NOT NULL, \
        response TEXT NOT NULL, \
        context TEXT, \
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP \
    );"

#define CREATE_CONVERSATIONS_TABLE "\
    CREATE TABLE IF NOT EXISTS conversations ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        user_input TEXT NOT NULL, \
        bot_response TEXT NOT NULL, \
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP \
    );"

/* Context structure to maintain conversation state.
 * This structure holds both the conversation history and database connection.
 * The messages array works as a circular buffer, with 'current' pointing
 * to the next position to write. This allows us to maintain a fixed-size
 * sliding window of the most recent conversation context. */
typedef struct {
    char *messages[MAX_CONTEXT_LENGTH];  /* Circular buffer of messages */
    int current;                         /* Current position in buffer */
    sqlite3 *db;                         /* SQLite database connection */
} Context;

/* Function prototypes */
/* Database operations */
int init_database(Context *ctx);
void close_database(sqlite3 *db);
char* get_response_from_db(sqlite3 *db, const char *input);
void log_conversation_to_db(sqlite3 *db, const char *user_input, const char *response);
void add_response(sqlite3 *db, const char *pattern, const char *response, const char *context);

/* Context management */
void init_context(Context *ctx);
void add_to_context(Context *ctx, const char *message);
void free_context(Context *ctx);

/* UI functions */
void print_welcome_message(void);
void print_help(void);
void print_with_typing_effect(const char *text);

/* Response generation */
char* generate_response(hashtable_t *hashtable, const char *input);

#endif /* CHATBOT_H */ 
