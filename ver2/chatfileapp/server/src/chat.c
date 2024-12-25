#include "../include/chat.h"
#include "../include/common.h"
#include "../include/utils.h"
#include "../include/db.h"

void handle_chat(int client_sock, DBConnection *db, const char *username) {
    char buffer[BUFFER_SIZE];
    while (1) {
        // Receive a message command
        memset(buffer, 0, BUFFER_SIZE);
        if (recv_all(client_sock, buffer, sizeof(buffer)) <= 0) {
            printf("Client %s disconnected from chat.\n", username);
            break;
        }

        if (strncmp(buffer, "SEND_MSG", 8) == 0) {
            // Parse message: "SEND_MSG <recipient_username> <message>"
            char recipient[50];
            char message[BUFFER_SIZE];
            sscanf(buffer + 9, "%s %[^\n]", recipient, message);

            // Lookup recipient's socket
            int recipient_sock = find_user_socket(recipient); // Implement find_user_socket
            if (recipient_sock != -1) {
                // Forward the message
                char formatted_message[BUFFER_SIZE];
                snprintf(formatted_message, sizeof(formatted_message), "MESSAGE_FROM %s: %s", username, message);
                send_all(recipient_sock, formatted_message, strlen(formatted_message));
                send_all(client_sock, "Message sent.", strlen("Message sent."));
            } else {
                send_all(client_sock, "User not online.", strlen("User not online."));
            }
        }
        else if (strncmp(buffer, "EXIT_CHAT", 9) == 0) {
            printf("Client %s exited chat.\n", username);
            break;
        }
        // Handle other chat-related commands
    }
}
