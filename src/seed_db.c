#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <jansson.h>

#define DB_FILE "chatbot.db"
#define QA_FILE "data/qa_pairs.json"

// SQL to create tables
#define CREATE_RESPONSES_TABLE "\
    CREATE TABLE IF NOT EXISTS responses ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        pattern TEXT NOT NULL, \
        response TEXT NOT NULL, \
        context TEXT, \
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP \
    );"

// Function to initialize database
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
}

// Function to add a response
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

// Function to load and parse JSON data
json_t* load_json_file(const char *filename) {
    json_error_t error;
    json_t *root = json_load_file(filename, 0, &error);
    
    if (!root) {
        fprintf(stderr, "Error loading JSON file: %s\n", error.text);
        return NULL;
    }
    
    return root;
}

// Seed database from JSON file
void seed_from_json(sqlite3 *db, const char *filename) {
    json_t *root = load_json_file(filename);
    if (!root) return;

    json_t *categories = json_object_get(root, "categories");
    const char *category_name;
    json_t *category_data;

    json_object_foreach(categories, category_name, category_data) {
        size_t index;
        json_t *qa_pair;
        
        json_array_foreach(category_data, index, qa_pair) {
            json_t *patterns = json_object_get(qa_pair, "patterns");
            const char *response = json_string_value(json_object_get(qa_pair, "response"));
            const char *context = json_string_value(json_object_get(qa_pair, "context"));
            
            if (json_is_array(patterns)) {
                size_t pattern_index;
                json_t *pattern;
                
                json_array_foreach(patterns, pattern_index, pattern) {
                    const char *pattern_str = json_string_value(pattern);
                    add_response(db, pattern_str, response, context);
                }
            }
        }
    }

    json_decref(root);
    printf("Database seeded from JSON file successfully!\n");
}

int main(void) {
    sqlite3 *db = NULL;
    
    printf("Initializing database...\n");
    init_database(&db);
    
    if (!db) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }

    seed_from_json(db, QA_FILE);
    
    if (db) {
        sqlite3_close(db);
    }
    
    return 0;
} 
