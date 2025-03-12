#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "chatbot.h"

/* DESIGN
 * ------
 * The UI module handles all user interaction aspects of the chatbot.
 * It provides functions for displaying messages with special effects,
 * showing help information, and formatting output with colors.
 *
 * The typing effect is used to make the chatbot feel more natural
 * by simulating human typing speed rather than instant responses.
 */

/* Display the welcome message with formatting.
 * This function is called once at startup to introduce
 * the chatbot and show basic usage instructions. */
void print_welcome_message(void) {
    printf("\n%s==================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s       Chatbot v1.0.0%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s==================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("\nWelcome! I'm here to chat with you.\n");
    printf("Commands:\n");
    printf("  - Type %s/exit%s to quit\n", COLOR_YELLOW, COLOR_RESET);
    printf("  - Press %sEnter%s twice to quit\n", COLOR_YELLOW, COLOR_RESET);
    printf("  - Type %s/help%s for more commands\n", COLOR_YELLOW, COLOR_RESET);
    printf("  - Just type naturally to chat with me\n\n");
}

/* Display available commands and their descriptions.
 * This function is called when the user types /help. */
void print_help(void) {
    printf("\n%sAvailable Commands:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s/help%s     - Show this help message\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/exit%s     - Exit the chatbot\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/clear%s    - Clear the screen\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s/history%s  - Show conversation history\n", COLOR_YELLOW, COLOR_RESET);
}

/* Print text with a typing effect for more natural interaction.
 * This function prints each character with a small delay,
 * simulating human typing speed. The delay can be adjusted
 * using the TYPING_DELAY constant. */
void print_with_typing_effect(const char *text) {
    if (!text) return;
    
    for (size_t i = 0; i < strlen(text); i++) {
        putchar(text[i]);
        fflush(stdout);
        usleep(TYPING_DELAY);
    }
} 
