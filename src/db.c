#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "chatbot.h"

/* DESIGN
 * ------
 * The database layer uses SQLite for persistent storage of both responses and
 * conversation history. We use two main tables:
 * 
 * 1. responses: Stores pattern-response pairs with context
 * 2. conversations: Logs all interactions for history and analysis
 *
 * Pattern matching uses SQLite's LIKE operator for fuzzy matching, with
 * patterns stored in a way that allows partial matches. This provides
 * more natural conversations without exact word matching requirements.
 */

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

/* Initialize the database and create necessary tables.
 * Returns 1 on success, 0 on failure. */
int init_database(Context *ctx) {
    int rc = sqlite3_open(DB_FILE, &ctx->db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(ctx->db));
        return 0;
    }

    char *err_msg = 0;
    rc = sqlite3_exec(ctx->db, CREATE_RESPONSES_TABLE, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }

    rc = sqlite3_exec(ctx->db, CREATE_CONVERSATIONS_TABLE, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }

    return 1;
}

/* Add a new response pattern to the database */
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

/* Get response using fuzzy pattern matching */
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
            sqlite3_finalize(stmt);
            return response;
        }
    }
    sqlite3_finalize(stmt);
    return NULL;
}

/* Log conversation to database */
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

/* Close database connection */
void close_database(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
} 
