#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8083
#define BUFFER_SIZE 1024
#define MAX_GROUP_NAME 50
#define MAX_MESSAGE_LENGTH 900 // Dành khoảng trống cho "SEND " và group_name

void print_menu()
{
    printf("\nOptions:\n");
    printf("1. Create group\n");
    printf("2. Join group\n");
    printf("3. Leave group\n");
    printf("4. Send message\n");
    printf("5. Add member\n");
    printf("6. Remove member\n");
    printf("7. Exit\n");
    printf("Choose an option: ");
}

int main()
{
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char username[50];
    char password[50];
    char email[100];
    char group_name[MAX_GROUP_NAME];
    char message[MAX_MESSAGE_LENGTH];

    // Tạo socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Địa chỉ server
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    // Kết nối đến server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        return -1;
    }

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0; // Loại bỏ ký tự newline

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0; // Loại bỏ ký tự newline

    printf("Enter your email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0; // Loại bỏ ký tự newline

    // Gửi username, password, và email đến server
    snprintf(buffer, sizeof(buffer), "%s %s %s", username, password, email);
    send(client_socket, buffer, strlen(buffer), 0);

    while (1)
    {
        print_menu();
        int option;
        scanf("%d", &option);
        getchar(); // Xóa newline từ input buffer

        switch (option)
        {
        case 1:
            printf("Enter group name to create: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            snprintf(buffer, sizeof(buffer), "CREATE_GROUP %s", group_name);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 2:
            printf("Enter group name to join: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            snprintf(buffer, sizeof(buffer), "JOIN_GROUP %s", group_name);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 3:
            printf("Enter group name to leave: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            snprintf(buffer, sizeof(buffer), "LEAVE_GROUP %s", group_name);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 4:
            printf("Enter group name: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            printf("Enter your message: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;

            if (strlen(group_name) + strlen(message) + 5 >= BUFFER_SIZE)
            {
                printf("Message too long. Please try again.\n");
                break;
            }

            snprintf(buffer, sizeof(buffer), "SEND %s %s", group_name, message);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 5:
            printf("Enter group name to add member: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            printf("Enter username of member to add: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;

            if (strlen(group_name) + strlen(message) + 11 >= BUFFER_SIZE)
            {
                printf("Input too long. Please try again.\n");
                break;
            }

            snprintf(buffer, sizeof(buffer), "ADD_MEMBER %s %s", group_name, message);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 6:
            printf("Enter group name to remove member: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;
            printf("Enter username of member to remove: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;

            if (strlen(group_name) + strlen(message) + 14 >= BUFFER_SIZE)
            {
                printf("Input too long. Please try again.\n");
                break;
            }

            snprintf(buffer, sizeof(buffer), "REMOVE_MEMBER %s %s", group_name, message);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 7:
            printf("Exiting...\n");
            close(client_socket);
            exit(0);

        default:
            printf("Invalid option. Please try again.\n");
        }

        // Nhận phản hồi từ server
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            printf("Server: %s\n", buffer);
        }
    }

    close(client_socket);
    return 0;
}
