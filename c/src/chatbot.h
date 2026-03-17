#ifndef CHATBOT_H
#define CHATBOT_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Opaque hash table: separate chaining, power-of-two capacity, FNV-1a.
 * Keys are stored lowercased; lookups are case-insensitive by design.
 */
typedef struct hashtable hashtable_t;

enum { CHATBOT_LINE_CAP = 256 };

hashtable_t *ht_create(size_t capacity);
void         ht_destroy(hashtable_t *ht);
bool         ht_put(hashtable_t *ht, const char *key, const char *value);
const char  *ht_get(const hashtable_t *ht, const char *key);

#endif /* CHATBOT_H */
