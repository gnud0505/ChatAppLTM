#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 4040
#define BUFFER_SIZE 1024
#define MAX_USERS 100
#define MAX_USERNAME_LEN 50

int client_count = 0;
int clients[MAX_USERS];
char usernames[MAX_USERS][MAX_USERNAME_LEN];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg);
void send_message_to_all(char* message);
void send_message_to_client(int client_socket, char* message);
void remove_client(int client_socket);
void send_user_list(int client_socket);

int main() {
    int server_socket, new_client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t tid;

    // Tạo socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Lắng nghe
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        new_client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (new_client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Thêm client vào danh sách
        pthread_mutex_lock(&clients_mutex);
        clients[client_count] = new_client_socket;
        client_count++;
        pthread_mutex_unlock(&clients_mutex);

        // Tạo thread xử lý client
        pthread_create(&tid, NULL, handle_client, (void*)&new_client_socket);
    }

    close(server_socket);
    return 0;
}

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;
    char username[MAX_USERNAME_LEN];

    // Hỏi tên người dùng
    send_message_to_client(client_socket, "Enter your username: ");
    bytes_received = recv(client_socket, username, MAX_USERNAME_LEN, 0);
    if (bytes_received <= 0) {
        close(client_socket);
        return NULL;
    }
    username[bytes_received - 1] = '\0';  // Xóa ký tự '\n'

    // Lưu tên người dùng
    pthread_mutex_lock(&clients_mutex);
    strcpy(usernames[client_socket], username);
    pthread_mutex_unlock(&clients_mutex);

    // Gửi danh sách người dùng đang kết nối
    send_user_list(client_socket);

    // Thông báo người dùng mới đã kết nối cho tất cả người dùng còn lại
    snprintf(buffer, sizeof(buffer), "%s has joined the chat.\n", username);
    send_message_to_all(buffer);

    // Xử lý các tin nhắn gửi tới server
    while (1) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            remove_client(client_socket);
            break;
        }

        buffer[bytes_received] = '\0';  // Đảm bảo chuỗi kết thúc đúng

        // Lệnh để thoát chat
        if (strcmp(buffer, "/exit") == 0) {
            snprintf(buffer, sizeof(buffer), "%s has left the chat.\n", username);
            send_message_to_all(buffer);
            remove_client(client_socket);
            break;
        }

        // Lệnh để chat với người khác
        if (strncmp(buffer, "/chat ", 6) == 0) {
            char target_user[MAX_USERNAME_LEN];
            sscanf(buffer + 6, "%s", target_user);
            
            // Kiểm tra nếu user này đang kết nối
            int target_client_socket = -1;
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (strcmp(usernames[clients[i]], target_user) == 0) {
                    target_client_socket = clients[i];
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            
            if (target_client_socket != -1) {
                send_message_to_client(target_client_socket, "You have a new message.\n");
            } else {
                send_message_to_client(client_socket, "User not found.\n");
            }
        } else {
            send_message_to_all(buffer);
        }
    }

    return NULL;
}

void send_message_to_all(char* message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        send_message_to_client(clients[i], message);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message_to_client(int client_socket, char* message) {
    send(client_socket, message, strlen(message), 0);
}

void send_user_list(int client_socket) {
    char user_list[BUFFER_SIZE] = "Connected users:\n";
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        strcat(user_list, usernames[clients[i]]);
        strcat(user_list, "\n");
    }
    pthread_mutex_unlock(&clients_mutex);
    
    send_message_to_client(client_socket, user_list);
}

void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
                strcpy(usernames[clients[j]], usernames[clients[j + 1]]);
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    close(client_socket);
}
