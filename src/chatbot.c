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

/* DESIGN
 * ------
 * This is the main chatbot logic module that ties together all other components.
 * It handles the main chat loop, command processing, and orchestrates the
 * interaction between the database, context management, and UI components.
 *
 * The chatbot uses a command-driven interface where special commands start with '/'.
 * Normal input is processed as conversation and matched against the database
 * for appropriate responses.
 */

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

// Function prototypes for hash table operations
hashtable_t *ht_create(int size);
int ht_hash(hashtable_t *hashtable, char *key);
entry_t *ht_newpair(char *key, char *value);
void ht_set(hashtable_t *hashtable, char *key, char *value);
char *ht_get(hashtable_t *hashtable, char *key);
void ht_destroy(hashtable_t *hashtable);

/* void init_context(Context *ctx); */
/* void add_to_context(Context *ctx, const char *message); */
/* void free_context(Context *ctx); */
void load_responses_from_file(hashtable_t *hashtable, const char *filename);
void log_conversation(const char *user_input, const char *response);
/* void print_with_typing_effect(const char *text); */
char* get_contextual_response(hashtable_t *hashtable, Context *ctx, const char *input);
void save_conversation_history(const char *filename);
/* void print_help(void); */
/* void print_welcome_message(void); */
char* generate_response(hashtable_t *hashtable, const char *input);
/* int init_database(Context *ctx); */
/* void add_response(sqlite3 *db, const char *pattern, const char *response, const char *context); */
/* char* get_response_from_db(sqlite3 *db, const char *input); */
/* void log_conversation_to_db(sqlite3 *db, const char *user_input, const char *response); */
/* vovid close_database(sqlite3 *db); */

/* Function prototypes for local functions */
static int handle_command(const char *input, Context *ctx);
static void chat_loop(Context *ctx);

/* Process special commands starting with '/'.
 * Returns 1 if the program should exit, 0 otherwise. */
static int handle_command(const char *input, Context *ctx) {
    if (!input || !ctx) return 0;

    if (strcmp(input, "/exit") == 0) {
        printf("\nGoodbye! Have a great day!\n");
        return 1;
    } else if (strcmp(input, "/help") == 0) {
        print_help();
    } else if (strcmp(input, "/clear") == 0) {
        system("clear");
        print_welcome_message();
    } else if (strcmp(input, "/history") == 0) {
        printf("\n%sRecent Conversation:%s\n", COLOR_CYAN, COLOR_RESET);
        for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
            if (ctx->messages[i] != NULL) {
                printf("%s%s%s\n",
                    (i % 2 == 0) ? COLOR_GREEN : COLOR_YELLOW,
                    ctx->messages[i],
                    COLOR_RESET);
            }
        }
    } else {
        printf("Unknown command. Type /help for available commands.\n");
    }
    return 0;
}

/* Main chat loop that processes user input and generates responses. */
static void chat_loop(Context *ctx) {
    char input[MAX_INPUT_LENGTH];
    char *response;
    int empty_lines = 0;

    print_welcome_message();

    while (1) {
        printf("\n%sYou:%s ", COLOR_GREEN, COLOR_RESET);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            if (++empty_lines >= 2) {
                printf("\nGoodbye! (detected double Enter)\n");
                break;
            }
            continue;
        }
        empty_lines = 0;

        if (input[0] == '/') {
            if (handle_command(input, ctx)) {
                break;
            }
            continue;
        }

        add_to_context(ctx, input);

        response = get_response_from_db(ctx->db, input);
        if (response != NULL) {
            printf("%sBot:%s ", COLOR_YELLOW, COLOR_RESET);
            print_with_typing_effect(response);
            printf("\n");

            add_to_context(ctx, response);
            log_conversation_to_db(ctx->db, input, response);
        } else {
            printf("Sorry, I couldn't find a suitable response.\n");
        }
    }
}

/* Main entry point for the chatbot application */
int main(void) {
    Context ctx;

    init_context(&ctx);

    if (!init_database(&ctx)) {
        fprintf(stderr, "Failed to initialize database\n");
        free_context(&ctx);
        return 1;
    }

    chat_loop(&ctx);

    close_database(ctx.db);
    free_context(&ctx);

    return 0;
}

// Helper function to print welcome message
/* void print_welcome_message(void) { */
/*     printf("\n%s==================================%s\n", COLOR_CYAN, COLOR_RESET); */
/*     printf("%s       Chatbot v1.0.0%s\n", COLOR_GREEN, COLOR_RESET); */
/*     printf("%s==================================%s\n", COLOR_CYAN, COLOR_RESET); */
/*     printf("\nWelcome! I'm here to chat with you.\n"); */
/*     printf("Commands:\n"); */
/*     printf("  - Type %sexit%s to quit\n", COLOR_YELLOW, COLOR_RESET); */
/*     printf("  - Press %sEnter%s twice to quit\n", COLOR_YELLOW, COLOR_RESET); */
/*     printf("  - Just type naturally to chat with me\n\n"); */
/* } */

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

// Hash table implementation
/* Create a new hashtable. */
hashtable_t *ht_create(int size) {
    hashtable_t *hashtable = NULL;
    int i;

    if (size < 1) return NULL;

    if ((hashtable = malloc(sizeof(hashtable_t))) == NULL) {
        return NULL;
    }

    if ((hashtable->table = malloc(sizeof(entry_t *) * size)) == NULL) {
        free(hashtable);
        return NULL;
    }

    for (i = 0; i < size; i++) {
        hashtable->table[i] = NULL;
    }

    hashtable->size = size;
    return hashtable;
}

/* Hash a string for a particular hash table. */
int ht_hash(hashtable_t *hashtable, char *key) {
    unsigned long int hashval = 0;
    unsigned long i = 0;

    while (hashval < ULONG_MAX && i < strlen(key)) {
        hashval = hashval << 8;
        hashval += key[i];
        i++;
    }

    return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_t *ht_newpair(char *key, char *value) {
    entry_t *newpair;

    if ((newpair = malloc(sizeof(entry_t))) == NULL) {
        return NULL;
    }

    if ((newpair->key = strdup(key)) == NULL) {
        free(newpair);
        return NULL;
    }

    if ((newpair->value = strdup(value)) == NULL) {
        free(newpair->key);
        free(newpair);
        return NULL;
    }

    newpair->next = NULL;
    return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set(hashtable_t *hashtable, char *key, char *value) {
    int bin = 0;
    entry_t *newpair = NULL;
    entry_t *next = NULL;
    entry_t *last = NULL;

    bin = ht_hash(hashtable, key);
    next = hashtable->table[bin];

    while (next != NULL && next->key != NULL && strcmp(key, next->key) > 0) {
        last = next;
        next = next->next;
    }

    if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0) {
        free(next->value);
        next->value = strdup(value);
    } else {
        newpair = ht_newpair(key, value);

        if (next == hashtable->table[bin]) {
            newpair->next = next;
            hashtable->table[bin] = newpair;
        } else if (next == NULL) {
            last->next = newpair;
        } else {
            newpair->next = next;
            last->next = newpair;
        }
    }
}

/* Retrieve a key-value pair from a hash table. */
char *ht_get(hashtable_t *hashtable, char *key) {
    int bin = 0;
    entry_t *pair;

    bin = ht_hash(hashtable, key);
    pair = hashtable->table[bin];

    while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
        pair = pair->next;
    }

    if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
        return NULL;
    }

    return pair->value;
}

/* Free all resources used by the hashtable */
void ht_destroy(hashtable_t *hashtable) {
    if (hashtable == NULL) return;

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

    free(hashtable->table);
    free(hashtable);
}

/* void init_context(Context *ctx) { */
/*     for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) { */
/*         ctx->messages[i] = NULL; */
/*     } */
/*     ctx->current = 0; */
/* } */

/* void add_to_context(Context *ctx, const char *message) { */
/*     // Free the oldest message if we're at capacity */
/*     if (ctx->messages[ctx->current] != NULL) { */
/*         free(ctx->messages[ctx->current]); */
/*     } */

/*     // Add new message */
/*     ctx->messages[ctx->current] = strdup(message); */
/*     ctx->current = (ctx->current + 1) % MAX_CONTEXT_LENGTH; */
/* } */

/* void free_context(Context *ctx) { */
/*     for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) { */
/*         if (ctx->messages[i] != NULL) { */
/*             free(ctx->messages[i]); */
/*             ctx->messages[i] = NULL; */
/*         } */
/*     } */
/* } */

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

/* void print_with_typing_effect(const char *text) { */
/*     for (size_t i = 0; i < strlen(text); i++) { */
/*         putchar(text[i]); */
/*         fflush(stdout); */
/*         usleep(TYPING_DELAY); */
/*     } */
/* } */

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

/* void print_help(void) { */
/*     printf("\n%sAvailable Commands:%s\n", COLOR_CYAN, COLOR_RESET); */
/*     printf("  %s/help%s     - Show this help message\n", COLOR_YELLOW, COLOR_RESET); */
/*     printf("  %s/exit%s     - Exit the chatbot\n", COLOR_YELLOW, COLOR_RESET); */
/*     printf("  %s/clear%s    - Clear the screen\n", COLOR_YELLOW, COLOR_RESET); */
/*     printf("  %s/history%s  - Show conversation history\n", COLOR_YELLOW, COLOR_RESET); */
/* } */

/* // Initialize database connection */
/* int init_database(Context *ctx) { */
/*     int rc = sqlite3_open(DB_FILE, &ctx->db); */
/*     if (rc) { */
/*         fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(ctx->db)); */
/*         return 0; */
/*     } */

/*     char *err_msg = 0; */
/*     rc = sqlite3_exec(ctx->db, CREATE_RESPONSES_TABLE, 0, 0, &err_msg); */
/*     if (rc != SQLITE_OK) { */
/*         fprintf(stderr, "SQL error: %s\n", err_msg); */
/*         sqlite3_free(err_msg); */
/*         return 0; */
/*     } */

/*     rc = sqlite3_exec(ctx->db, CREATE_CONVERSATIONS_TABLE, 0, 0, &err_msg); */
/*     if (rc != SQLITE_OK) { */
/*         fprintf(stderr, "SQL error: %s\n", err_msg); */
/*         sqlite3_free(err_msg); */
/*         return 0; */
/*     } */

/*     return 1; */
/* } */

// Add a new response pattern to the database
/* void add_response(sqlite3 *db, const char *pattern, const char *response, const char *context) { */
/*     sqlite3_stmt *stmt; */
/*     const char *sql = "INSERT INTO responses (pattern, response, context) VALUES (?, ?, ?)"; */

/*     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0); */
/*     if (rc == SQLITE_OK) { */
/*         sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_STATIC); */
/*         sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC); */
/*         sqlite3_bind_text(stmt, 3, context, -1, SQLITE_STATIC); */

/*         sqlite3_step(stmt); */
/*     } */
/*     sqlite3_finalize(stmt); */
/* } */

// Get response using fuzzy pattern matching
/* char* get_response_from_db(sqlite3 *db, const char *input) { */
/*     static char response[MAX_RESPONSE_LENGTH]; */
/*     sqlite3_stmt *stmt; */
/*     const char *sql = "SELECT response FROM responses WHERE lower(?) LIKE '%' || lower(pattern) || '%' " */
/*                      "OR lower(pattern) LIKE '%' || lower(?) || '%' " */
/*                      "ORDER BY length(pattern) DESC LIMIT 1"; */

/*     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0); */
/*     if (rc == SQLITE_OK) { */
/*         sqlite3_bind_text(stmt, 1, input, -1, SQLITE_STATIC); */
/*         sqlite3_bind_text(stmt, 2, input, -1, SQLITE_STATIC); */

/*         if (sqlite3_step(stmt) == SQLITE_ROW) { */
/*             strncpy(response, (const char*)sqlite3_column_text(stmt, 0), MAX_RESPONSE_LENGTH - 1); */
/*             response[MAX_RESPONSE_LENGTH - 1] = '\0'; */
/*         } else { */
/*             strncpy(response, "I'm not sure how to respond to that. Could you rephrase or ask something else?", MAX_RESPONSE_LENGTH - 1); */
/*         } */
/*     } */
/*     sqlite3_finalize(stmt); */
/*     return response; */
/* } */

// Log conversation to database
/* void log_conversation_to_db(sqlite3 *db, const char *user_input, const char *response) { */
/*     sqlite3_stmt *stmt; */
/*     const char *sql = "INSERT INTO conversations (user_input, bot_response) VALUES (?, ?)"; */

/*     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0); */
/*     if (rc == SQLITE_OK) { */
/*         sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC); */
/*         sqlite3_bind_text(stmt, 2, response, -1, SQLITE_STATIC); */
/*         sqlite3_step(stmt); */
/*     } */
/*     sqlite3_finalize(stmt); */
/* } */

// Close database connection
/* void close_database(sqlite3 *db) { */
/*     if (db) { */
/*         sqlite3_close(db); */
/*     } */
/* } */
