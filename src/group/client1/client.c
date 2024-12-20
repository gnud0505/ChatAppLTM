#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8086
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
    printf("7. List groups\n");
    printf("8. List users\n");
    printf("9. List messages\n");
    printf("10. Exit\n");
    printf("Choose an option: ");
}

int main()
{
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    int valread;
    fd_set read_fds;
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
    snprintf(buffer, sizeof(buffer), "%s:%s:%s", username, password, email);
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

            snprintf(buffer, sizeof(buffer), "SEND %s", group_name);
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
            snprintf(buffer, sizeof(buffer), "LIST_GROUPS");
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 8:
            snprintf(buffer, sizeof(buffer), "LIST_USERS");
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 9: // List Messages
            printf("Enter group name to list messages: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = 0;

            if (strlen(group_name) + 14 >= BUFFER_SIZE) // "LIST_MESSAGES " + group_name
            {
                printf("Group name too long. Please try again.\n");
                break;
            }

            snprintf(buffer, sizeof(buffer), "LIST_MESSAGES %s", group_name);
            send(client_socket, buffer, strlen(buffer), 0);
            break;

        case 10:
            printf("Exiting...\n");
            close(client_socket);
            exit(0);

        default:
            printf("Invalid option. Please try again.\n");
        }

        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_sd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;
        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if (activity > 0)
        {
            // Kiểm tra tin nhắn từ server
            if (FD_ISSET(client_socket, &read_fds))
            {
                valread = recv(client_socket, buffer, sizeof(buffer), 0);
                if (valread <= 0)
                {
                    printf("Mất kết nối với server.\n");
                    close(client_socket);
                    break;
                }
                else
                {
                    buffer[valread] = '\0';
                    printf("%d\n", strncmp(buffer, "SENDING", 7) == 0);
                    if (strncmp(buffer, "SENDING", 7) == 0)
                    {
                        printf("sending mode\n ------------------\n");
                        while (1)
                        {
                            FD_ZERO(&read_fds);
                            FD_SET(client_socket, &read_fds);
                            FD_SET(STDIN_FILENO, &read_fds);

                            int max_sd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;
                            int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

                            if (activity > 0)
                            {
                                // Kiểm tra tin nhắn từ server trước
                                if (FD_ISSET(client_socket, &read_fds))
                                {
                                    valread = recv(client_socket, buffer, sizeof(buffer), 0);
                                    if (valread <= 0)
                                    {
                                        printf("Mất kết nối với server.\n");
                                        close(client_socket);
                                        break;
                                    }
                                    else
                                    {
                                        buffer[valread] = '\0';
                                        printf("%s\n", buffer);
                                        if (strcmp(buffer, "EXIT") == 0)
                                            break;
                                    }
                                }

                                // Kiểm tra tin nhắn từ người dùng
                                if (FD_ISSET(STDIN_FILENO, &read_fds))
                                {
                                    fgets(message, sizeof(message), stdin);
                                    message[strcspn(message, "\n")] = 0;
                                    snprintf(buffer, sizeof(buffer), "MESSAGE %s", message);
                                    send(client_socket, buffer, strlen(buffer), 0);
                                    if (strcmp(message, "EXIT") == 0)
                                        break;
                                }
                            }
                        }
                    }
                    else
                        printf("%s", buffer);
                }
            }
        }
    }

    close(client_socket);
    return 0;
}
