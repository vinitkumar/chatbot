#include <stdlib.h>
#include <string.h>
#include "chatbot.h"

/* DESIGN
 * ------
 * The context management system maintains a circular buffer of recent
 * messages to provide conversation context. This allows the chatbot
 * to maintain some awareness of the conversation flow and potentially
 * provide more contextually appropriate responses.
 *
 * The system uses a fixed-size array to avoid unbounded memory growth,
 * automatically freeing older messages as new ones are added. This
 * creates a sliding window of conversation context.
 */

/* Initialize a new context structure.
 * This function must be called before using any other context functions.
 * It sets up the circular buffer and initializes all pointers to NULL. */
void init_context(Context *ctx) {
    /* Why: We initialize all message pointers to NULL so that
     * add_to_context can safely free any existing message when
     * overwriting positions in the circular buffer. */
    for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
        ctx->messages[i] = NULL;
    }
    ctx->current = 0;
}

/* Add a new message to the context.
 * This function:
 * 1. Frees the oldest message if the current position is occupied
 * 2. Copies the new message into the context
 * 3. Updates the current position in the circular buffer
 * 
 * The message is copied rather than stored by reference to ensure
 * the context owns its data and the original message can be freed. */
void add_to_context(Context *ctx, const char *message) {
    /* Free the oldest message if we're at capacity */
    if (ctx->messages[ctx->current] != NULL) {
        free(ctx->messages[ctx->current]);
    }
    
    /* Add new message */
    ctx->messages[ctx->current] = strdup(message);
    
    /* Advance the current position, wrapping around if necessary */
    ctx->current = (ctx->current + 1) % MAX_CONTEXT_LENGTH;
}

/* Free all resources used by the context.
 * This function should be called before the context structure
 * is destroyed to prevent memory leaks. It frees all stored
 * messages and resets the context to its initial state. */
void free_context(Context *ctx) {
    for (int i = 0; i < MAX_CONTEXT_LENGTH; i++) {
        if (ctx->messages[i] != NULL) {
            free(ctx->messages[i]);
            ctx->messages[i] = NULL;
        }
    }
} 
