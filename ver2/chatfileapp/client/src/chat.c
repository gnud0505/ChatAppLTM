#include "../include/chat.h"
#include "../include/common.h"
#include "../include/utils.h"

void handle_chat(int sock, const char *username) {
    // Implement chat functionality: sending and receiving messages 1-1
    // For example, prompt user to enter recipient and message, then send to server
    char recipient[50];
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    printf("Enter recipient username: ");
    scanf("%s", recipient);
    getchar(); // Consume newline

    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0'; // Remove newline

    // Send chat command
    snprintf(buffer, sizeof(buffer), "SEND_MSG %s %s", recipient, message);
    send_all(sock, buffer, strlen(buffer));

    // Wait for acknowledgment
    memset(buffer, 0, BUFFER_SIZE);
    recv_all(sock, buffer, sizeof(buffer));
    printf("Server: %s\n", buffer);
}
