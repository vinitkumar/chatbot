#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sqlite3.h>
#include "chatbot.h"

// ANSI Color codes for prettier output
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"

#define MAX_RESPONSE_LENGTH 1024
#define MAX_CONTEXT_LENGTH 5
#define CONFIG_FILE "chatbot_responses.txt"
#define LOG_FILE "chatbot.log"
#define TYPING_DELAY 50000 // 50ms delay between characters
#define DB_FILE "chatbot.db"
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

// Forward declarations of structures
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

// Context structure to maintain conversation state
typedef struct {
    char *messages[MAX_CONTEXT_LENGTH];
    int current;
    sqlite3 *db;  // SQLite database connection
} Context;

// Function prototypes
hashtable_t *ht_create(int size);
int ht_hash(hashtable_t *hashtable, char *key);
entry_t *ht_newpair(char *key, char *value);
void ht_set(hashtable_t *hashtable, char *key, char *value);
char *ht_get(hashtable_t *hashtable, char *key);
void ht_destroy(hashtable_t *hashtable);

void init_context(Context *ctx);
void add_to_context(Context *ctx, const char *message);
void free_context(Context *ctx);
void load_responses_from_file(hashtable_t *hashtable, const char *filename);
void log_conversation(const char *user_input, const char *response);
void print_with_typing_effect(const char *text);
char* get_contextual_response(hashtable_t *hashtable, Context *ctx, const char *input);
void save_conversation_history(const char *filename);
void print_help(void);
void print_welcome_message(void);
char* generate_response(hashtable_t *hashtable, const char *input);
void init_database(sqlite3 **db);
void add_response(sqlite3 *db, const char *pattern, const char *response, const char *context);
char* get_response_from_db(sqlite3 *db, const char *input);
void log_conversation_to_db(sqlite3 *db, const char *user_input, const char *response);
void close_database(sqlite3 *db);

// Helper function to print welcome message
void print_welcome_message(void) {
    printf("\n%s==================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s       Chatbot v1.0.0%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s==================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("\nWelcome! I'm here to chat with you.\n");
    printf("Commands:\n");
    printf("  - Type %sexit%s to quit\n", COLOR_YELLOW, COLOR_RESET);
    printf("  - Press %sEnter%s twice to quit\n", COLOR_YELLOW, COLOR_RESET);
    printf("  - Just type naturally to chat with me\n\n");
}

// Helper function to process input and generate response
char* generate_response(hashtable_t *hashtable, const char *input) {
    static char response[MAX_RESPONSE_LENGTH];
    char input_copy[LINELENGTH];
    char *word;
    int found = 0;

    strncpy(input_copy, input, LINELENGTH - 1);
    input_copy[LINELENGTH - 1] = '\0';

    // Try to match each word in the input
    word = strtok(input_copy, SEPCHARS);
    while (word != NULL) {
        char *word_response = ht_get(hashtable, word);
        if (word_response != NULL) {
            strncpy(response, word_response, MAX_RESPONSE_LENGTH - 1);
            response[MAX_RESPONSE_LENGTH - 1] = '\0';
            found = 1;
            break;
        }
        word = strtok(NULL, SEPCHARS);
    }

    if (!found) {
        strncpy(response, "I'm not sure how to respond to that. Try asking something else!", MAX_RESPONSE_LENGTH - 1);
        response[MAX_RESPONSE_LENGTH - 1] = '\0';
    }

    return response;
}

// hash table implementation from here
// https://gist.githubusercontent.com/tonious/1377667/raw/c814d0833c8699dc017871931a5c5bee11af0f64/hash.c

/* Create a new hashtable. */
hashtable_t *ht_create( int size ) {

  hashtable_t *hashtable = NULL;
  int i;

  if( size < 1 ) return NULL;

  /* Allocate the table itself. */
  if( ( hashtable = malloc( sizeof( hashtable_t ) ) ) == NULL ) {
    return NULL;
  }

  /* Allocate pointers to the head nodes. */
  if( ( hashtable->table = malloc( sizeof( entry_t * ) * size ) ) == NULL ) {
    return NULL;
  }
  for( i = 0; i < size; i++ ) {
    hashtable->table[i] = NULL;
  }

  hashtable->size = size;

  return hashtable;
}

/* Hash a string for a particular hash table. */
int ht_hash( hashtable_t *hashtable, char *key ) {

  unsigned long int hashval = 0;
  unsigned long i = 0;

  /* Convert our string to an integer */
  while( hashval < ULONG_MAX && i < strlen( key ) ) {
    hashval = hashval << 8;
    hashval += key[ i ];
    i++;
  }

  return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_t *ht_newpair( char *key, char *value ) {
  entry_t *newpair;

  if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
    return NULL;
  }

  if( ( newpair->key = strdup( key ) ) == NULL ) {
    return NULL;
  }

  if( ( newpair->value = strdup( value ) ) == NULL ) {
    return NULL;
  }

  newpair->next = NULL;

  return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, char *key, char *value ) {
  int bin = 0;
  entry_t *newpair = NULL;
  entry_t *next = NULL;
  entry_t *last = NULL;

  bin = ht_hash( hashtable, key );

  next = hashtable->table[ bin ];

  while( next != NULL && next->key != NULL && strcmp( key, next->key ) > 0 ) {
    last = next;
    next = next->next;
  }

  /* There's already a pair.  Let's replace that string. */
  if( next != NULL && next->key != NULL && strcmp( key, next->key ) == 0 ) {

    free( next->value );
    next->value = strdup( value );

  /* Nope, could't find it.  Time to grow a pair. */
  } else {
    newpair = ht_newpair( key, value );

    /* We're at the start of the linked list in this bin. */
    if( next == hashtable->table[ bin ] ) {
      newpair->next = next;
      hashtable->table[ bin ] = newpair;

    /* We're at the end of the linked list in this bin. */
    } else if ( next == NULL ) {
      last->next = newpair;

    /* We're in the middle of the list. */
    } else  {
      newpair->next = next;
      last->next = newpair;
    }
  }
}

/* Retrieve a key-value pair from a hash table. */
char *ht_get( hashtable_t *hashtable, char *key ) {
  int bin = 0;
  entry_t *pair;

  bin = ht_hash( hashtable, key );

  /* Step through the bin, looking for our value. */
  pair = hashtable->table[ bin ];
  while( pair != NULL && pair->key != NULL && strcmp( key, pair->key ) > 0 ) {
    pair = pair->next;
  }

  /* Did we actually find anything? */
  if( pair == NULL || pair->key == NULL || strcmp( key, pair->key ) != 0 ) {
    return NULL;

  } else {
    return pair->value;
  }

}

/* Free all resources used by the hashtable */
void ht_destroy(hashtable_t *hashtable) {
    if (hashtable == NULL) return;
    
    // Free all entries
    for (int i = 0; i < hashtable->size; i++) {
        entry_t *current = hashtable->table[i];
        while (current != NULL) {
            entry_t *next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
    }
    
    // Free the table and the hashtable struct
    free(hashtable->table);
    free(hashtable);
}

void init_context(Context *ctx) {
    for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
        ctx->messages[i] = NULL;
    }
    ctx->current = 0;
}

void add_to_context(Context *ctx, const char *message) {
    // Free the oldest message if we're at capacity
    if (ctx->messages[ctx->current] != NULL) {
        free(ctx->messages[ctx->current]);
    }
    
    // Add new message
    ctx->messages[ctx->current] = strdup(message);
    ctx->current = (ctx->current + 1) % MAX_CONTEXT_LENGTH;
}

void free_context(Context *ctx) {
    for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
        if (ctx->messages[i] != NULL) {
            free(ctx->messages[i]);
            ctx->messages[i] = NULL;
        }
    }
}

void log_conversation(const char *user_input, const char *response) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        time_t now;
        time(&now);
        char *date = ctime(&now);
        date[strlen(date) - 1] = '\0'; // Remove newline
        fprintf(log_file, "[%s] User: %s\n", date, user_input);
        fprintf(log_file, "[%s] Bot: %s\n\n", date, response);
        fclose(log_file);
    }
}

void print_with_typing_effect(const char *text) {
    for (size_t i = 0; i < strlen(text); i++) {
        putchar(text[i]);
        fflush(stdout);
        usleep(TYPING_DELAY);
    }
}

void load_responses_from_file(hashtable_t *hashtable, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return;

    char line[MAX_RESPONSE_LENGTH];
    char *key, *value;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        key = strtok(line, "|");
        value = strtok(NULL, "");
        
        if (key && value) {
            ht_set(hashtable, key, value);
        }
    }

    fclose(file);
}

char* get_contextual_response(hashtable_t *hashtable, Context *ctx, const char *input) {
    static char response[MAX_RESPONSE_LENGTH];
    char *basic_response = generate_response(hashtable, input);
    
    // Check context for better responses
    for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
        if (ctx->messages[i] != NULL && strstr(ctx->messages[i], "?") != NULL) {
            // If previous message was a question, make response more contextual
            snprintf(response, MAX_RESPONSE_LENGTH, "Regarding your previous question, %s", basic_response);
            return response;
        }
    }
    
    return basic_response;
}

void print_help(void) {
    printf("\n%sAvailable Commands:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s/help%s     - Show this help message\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/exit%s     - Exit the chatbot\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/clear%s    - Clear the screen\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/history%s  - Show conversation history\n", COLOR_YELLOW, COLOR_RESET);
}

int main(void) {
    char line[MAX_RESPONSE_LENGTH];
    sqlite3 *db = NULL;
    Context ctx = {0};
    
    // Initialize database
    init_database(&db);
    if (!db) {
        fprintf(stderr, "%sError: Failed to connect to database%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    ctx.db = db;

    // Initialize default responses
    add_response(db, "hi|hey|hello", "Hello! How can I help you today?", "greeting");
    add_response(db, "how are you", "I'm doing well, thanks for asking! How about you?", "greeting");
    add_response(db, "what", "That's an interesting question! Let me think...", "question");
    add_response(db, "why", "That's a good question! I think it's because...", "question");
    add_response(db, "python", "Python is a great programming language! I love its simplicity and power.", "programming");
    add_response(db, "programming", "Programming is fun! I especially enjoy helping people learn to code.", "programming");
    add_response(db, "bye|goodbye|quit", "Goodbye! Have a great day!", "farewell");
    add_response(db, "thanks|thank you", "You're welcome! Let me know if you need anything else.", "gratitude");

    // Initialize context
    init_context(&ctx);

    // Display welcome message
    print_welcome_message();

    // Main chat loop
    while(1) {
        // Print prompt with color
        printf("\n%s➜ %s", COLOR_GREEN, COLOR_RESET);
        
        // Get input with error handling
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n%sGoodbye!%s\n", COLOR_CYAN, COLOR_RESET);
            break;
        }

        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }

        // Handle exit conditions
        if (len == 0) break;
        if (strcasecmp(line, "exit") == 0 || strcasecmp(line, "quit") == 0 || strcasecmp(line, "bye") == 0) {
            printf("\n%sGoodbye! Have a great day!%s\n", COLOR_CYAN, COLOR_RESET);
            break;
        }

        // Handle special commands
        if (line[0] == '/') {
            if (strcmp(line, "/help") == 0) {
                print_help();
                continue;
            } else if (strcmp(line, "/history") == 0) {
                // Show recent conversations
                sqlite3_stmt *stmt;
                const char *sql = "SELECT user_input, bot_response, timestamp FROM conversations "
                                "ORDER BY timestamp DESC LIMIT 5";
                
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                    printf("\n%sRecent Conversations:%s\n", COLOR_CYAN, COLOR_RESET);
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        const char *user_input = (const char*)sqlite3_column_text(stmt, 0);
                        const char *bot_response = (const char*)sqlite3_column_text(stmt, 1);
                        const char *timestamp = (const char*)sqlite3_column_text(stmt, 2);
                        printf("%s[%s]%s\n", COLOR_YELLOW, timestamp, COLOR_RESET);
                        printf("%sUser:%s %s\n", COLOR_GREEN, COLOR_RESET, user_input);
                        printf("%sBot:%s %s\n\n", COLOR_BLUE, COLOR_RESET, bot_response);
                    }
                }
                sqlite3_finalize(stmt);
                continue;
            }
        }

        // Get response from database and display with typing effect
        char *response = get_response_from_db(db, line);
        printf("%s❯ %s", COLOR_BLUE, COLOR_RESET);
        print_with_typing_effect(response);
        printf("\n");

        // Log conversation
        log_conversation_to_db(db, line, response);
        add_to_context(&ctx, line);
    }

    // Cleanup
    free_context(&ctx);
    if (db) {
        sqlite3_close(db);
    }
    return 0;
}

// Initialize database connection
void init_database(sqlite3 **db) {
    int rc = sqlite3_open(DB_FILE, db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        return;
    }

    char *err_msg = 0;
    rc = sqlite3_exec(*db, CREATE_RESPONSES_TABLE, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    rc = sqlite3_exec(*db, CREATE_CONVERSATIONS_TABLE, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Add a new response pattern to the database
void add_response(sqlite3 *db, const char *pattern, const char *response, const char *context) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO responses (pattern, response, context) VALUES (?, ?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, context, -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

// Get response using fuzzy pattern matching
char* get_response_from_db(sqlite3 *db, const char *input) {
    static char response[MAX_RESPONSE_LENGTH];
    sqlite3_stmt *stmt;
    const char *sql = "SELECT response FROM responses WHERE lower(?) LIKE '%' || lower(pattern) || '%' "
                     "OR lower(pattern) LIKE '%' || lower(?) || '%' "
                     "ORDER BY length(pattern) DESC LIMIT 1";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, input, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strncpy(response, (const char*)sqlite3_column_text(stmt, 0), MAX_RESPONSE_LENGTH - 1);
            response[MAX_RESPONSE_LENGTH - 1] = '\0';
        } else {
            strncpy(response, "I'm not sure how to respond to that. Could you rephrase or ask something else?", MAX_RESPONSE_LENGTH - 1);
        }
    }
    sqlite3_finalize(stmt);
    return response;
}

// Log conversation to database
void log_conversation_to_db(sqlite3 *db, const char *user_input, const char *response) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO conversations (user_input, bot_response) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

// Close database connection
void close_database(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}
