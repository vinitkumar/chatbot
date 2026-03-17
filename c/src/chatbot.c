// Enable POSIX/GNU extensions (strdup, strncasecmp, etc.)
// _GNU_SOURCE is needed because -std=c17 disables extensions by default
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "chatbot.h"

// hash table implementation from here
// https://gist.githubusercontent.com/tonious/1377667/raw/c814d0833c8699dc017871931a5c5bee11af0f64/hash.c

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
    free( hashtable );
    return NULL;
  }
  for( i = 0; i < size; i++ ) {
    hashtable->table[i] = NULL;
  }

  hashtable->size = size;

  return hashtable;
}

/* FNV-1a hash: fast, well-distributed, single pass, no strlen call. */
int ht_hash( hashtable_t *hashtable, char *key ) {

  uint64_t h = UINT64_C(14695981039346656037);
  for( const unsigned char *p = (const unsigned char *)key; *p; ++p ) {
    h ^= *p;
    h *= UINT64_C(1099511628211);
  }

  return (int)(h % (unsigned)hashtable->size);
}

/* Create a key-value pair. */
entry_t *ht_newpair( char *key, char *value ) {
  entry_t *newpair;

  if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
    return NULL;
  }

  if( ( newpair->key = strdup( key ) ) == NULL ) {
    free( newpair );
    return NULL;
  }

  if( ( newpair->value = strdup( value ) ) == NULL ) {
    free( newpair->key );
    free( newpair );
    return NULL;
  }

  newpair->next = NULL;

  return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, char *key, char *value ) {
  int bin = ht_hash( hashtable, key );

  /* Walk the chain, update if key already exists. */
  for( entry_t *e = hashtable->table[ bin ]; e != NULL; e = e->next ) {
    if( strcmp( key, e->key ) == 0 ) {
      char *new_value = strdup( value );
      if( new_value == NULL ) return;
      free( e->value );
      e->value = new_value;
      return;
    }
  }

  /* Key not found, prepend new entry to chain. */
  entry_t *newpair = ht_newpair( key, value );
  if( newpair == NULL ) return;
  newpair->next = hashtable->table[ bin ];
  hashtable->table[ bin ] = newpair;
}

/* Retrieve a key-value pair from a hash table. */
char *ht_get( hashtable_t *hashtable, char *key ) {
  int bin = ht_hash( hashtable, key );

  for( entry_t *e = hashtable->table[ bin ]; e != NULL; e = e->next ) {
    if( strcmp( key, e->key ) == 0 ) return e->value;
  }
  return NULL;
}

int main(void) {

  char line[LINELENGTH];
  char *word;
  printf("$ Chatbot v1.0.0!\n");

  hashtable_t *hashtable = ht_create(65536);
  ht_set(hashtable, "hi", "hello");
  ht_set(hashtable, "hey", "hello");
  ht_set(hashtable, "hear", "What you heard is right");
  ht_set(hashtable, "python", "Yo, I love Python");
  ht_set(hashtable, "light", "I like light");
  ht_set(hashtable, "What", "It is clear, ain't it?");

  int running = 1;
  while(running) {
    printf("\n$ (user) ");

    if (!fgets(line, LINELENGTH, stdin) || line[0] == '\n') break;

    word = strtok(line, SEPCHARS);

    while (word != NULL) {
      if (strncasecmp(word, "exit", 150) == 0) {
        running = 0;
        break;
      }

      char *reply = ht_get(hashtable, word);
      printf("\n$ (chatbot) %s\n",
             reply ? reply : "Sorry, I don't know what to say about that");

      word = strtok(NULL, SEPCHARS);
    }
  }
  return 0;
}
