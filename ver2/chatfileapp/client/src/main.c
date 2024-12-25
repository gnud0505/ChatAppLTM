#include "../include/common.h"
#include "../include/chat.h"
#include "../include/group.h"
#include "../include/file.h"
#include "../include/utils.h"

extern void register_user(int sock);
extern int login_user(int sock);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        handle_error("Socket creation failed");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        handle_error("Invalid address/ Address not supported");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        handle_error("Connection failed");

    printf("Connected to server %s:%d\n", server_ip, server_port);

    int choice;
    int authenticated = 0;

    while (1) {
        if (!authenticated) {
            printf("\n1. Register\n2. Login\n3. Exit\nChoose an option: ");
            scanf("%d", &choice);
            getchar(); // Consume newline

            if (choice == 1) {
                register_user(sock);
            } else if (choice == 2) {
                authenticated = login_user(sock);
            } else if (choice == 3) {
                printf("Exiting...\n");
                break;
            } else {
                printf("Invalid choice. Please try again.\n");
            }
        } else {
            printf("\n1. Chat\n2. Group Chat\n3. File Transfer\n4. Logout\nChoose an option: ");
            scanf("%d", &choice);
            getchar(); // Consume newline

            if (choice == 1) {
                handle_chat(sock, "username"); // Replace "username" with actual username
            }
            else if (choice == 2) {
                handle_group(sock, "username");
            }
            else if (choice == 3) {
                handle_file_transfer(sock);
            }
            else if (choice == 4) {
                authenticated = 0;
                printf("Logged out.\n");
            }
            else {
                printf("Invalid choice. Please try again.\n");
            }
        }
    }

    close(sock);
    return 0;
}
