#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "chatbot.h"

/* Portable strdup replacement: pure ISO C, no _GNU_SOURCE needed. */
static char *dup_str( const char *s ) {
  size_t n = strlen( s ) + 1;
  char *copy = malloc( n );
  if( copy ) memcpy( copy, s, n );
  return copy;
}

/* ASCII-only in-place lowercasing. */
static void ascii_lower( char *s ) {
  for( unsigned char *p = (unsigned char *)s; *p; ++p ) {
    if( *p >= 'A' && *p <= 'Z' ) *p = (unsigned char)(*p + ('a' - 'A'));
  }
}

typedef struct entry {
  char         *key;
  char         *value;
  struct entry *next;
} entry_t;

struct hashtable {
  size_t    capacity;
  size_t    count;
  entry_t **buckets;
};


/* FNV-1a hash. */
static uint64_t hash_string( const char *s ) {
  uint64_t h = UINT64_C(14695981039346656037);
  for( const unsigned char *p = (const unsigned char *)s; *p; ++p ) {
    h ^= *p;
    h *= UINT64_C(1099511628211);
  }
  return h;
}

static size_t ht_index( const hashtable_t *ht, const char *key ) {
  return (size_t)(hash_string( key ) % ht->capacity);
}

static entry_t *entry_create( const char *key, const char *value ) {
  entry_t *e = malloc( sizeof *e );
  if( !e ) return NULL;

  e->key = dup_str( key );
  if( !e->key ) { free( e ); return NULL; }

  e->value = dup_str( value );
  if( !e->value ) { free( e->key ); free( e ); return NULL; }

  e->next = NULL;
  return e;
}

static void entry_destroy( entry_t *e ) {
  free( e->key );
  free( e->value );
  free( e );
}

hashtable_t *ht_create( size_t capacity ) {
  if( capacity < 8 ) capacity = 8;

  hashtable_t *ht = malloc( sizeof *ht );
  if( !ht ) return NULL;

  ht->buckets = calloc( capacity, sizeof *ht->buckets );
  if( !ht->buckets ) { free( ht ); return NULL; }

  ht->capacity = capacity;
  ht->count    = 0;
  return ht;
}

void ht_destroy( hashtable_t *ht ) {
  if( !ht ) return;
  for( size_t i = 0; i < ht->capacity; ++i ) {
    entry_t *e = ht->buckets[i];
    while( e ) {
      entry_t *next = e->next;
      entry_destroy( e );
      e = next;
    }
  }
  free( ht->buckets );
  free( ht );
}

bool ht_put( hashtable_t *ht, const char *key, const char *value ) {
  size_t idx = ht_index( ht, key );

  /* Walk the chain, update if key already exists. */
  for( entry_t *e = ht->buckets[idx]; e; e = e->next ) {
    if( strcmp( key, e->key ) == 0 ) {
      char *new_val = dup_str( value );
      if( !new_val ) return false;
      free( e->value );
      e->value = new_val;
      return true;
    }
  }

  /* Key not found, prepend new entry to chain. */
  entry_t *e = entry_create( key, value );
  if( !e ) return false;
  e->next = ht->buckets[idx];
  ht->buckets[idx] = e;
  ht->count++;
  return true;
}

const char *ht_get( const hashtable_t *ht, const char *key ) {
  size_t idx = ht_index( ht, key );

  for( entry_t *e = ht->buckets[idx]; e; e = e->next ) {
    if( strcmp( key, e->key ) == 0 ) return e->value;
  }
  return NULL;
}

int main(void) {
  static const char sep[] = " *.,\"\n";

  hashtable_t *ht = ht_create(16);
  if( !ht ) { fputs("out of memory\n", stderr); return 1; }

  ht_put(ht, "hi",     "hello");
  ht_put(ht, "hey",    "hello");
  ht_put(ht, "hear",   "What you heard is right");
  ht_put(ht, "python", "Yo, I love Python");
  ht_put(ht, "light",  "I like light");
  ht_put(ht, "what",   "It is clear, ain't it?");

  puts("$ Chatbot v1.0.0!");

  char line[CHATBOT_LINE_CAP];
  bool running = true;

  while(running) {
    fputs("\n$ (user) ", stdout);

    if( !fgets(line, sizeof line, stdin) || line[0] == '\n' ) break;

    for( char *word = strtok(line, sep); word; word = strtok(NULL, sep) ) {
      ascii_lower(word);

      if( strcmp(word, "exit") == 0 ) {
        running = false;
        break;
      }

      const char *reply = ht_get(ht, word);
      printf("\n$ (chatbot) %s\n",
             reply ? reply : "Sorry, I don't know what to say about that");
    }
  }

  ht_destroy(ht);
  return 0;
}
