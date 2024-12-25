#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 4040
#define BUFFER_SIZE 1024
#define MAX_USERNAME_LEN 50

int sock;
char username[MAX_USERNAME_LEN];

void* receive_messages(void* arg) {
    char message[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        bytes_received = recv(sock, message, sizeof(message), 0);
        if (bytes_received <= 0) {
            printf("Disconnected from server\n");
            break;
        }

        message[bytes_received] = '\0';
        printf("%s", message); // In ra thông báo từ server
    }

    close(sock);
    exit(0);
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t receive_thread;
    char message[BUFFER_SIZE];

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Kết nối đến server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Nhập tên người dùng
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';  // Xóa ký tự '\n'

    // Gửi tên người dùng đến server
    send(sock, username, strlen(username), 0);

    // Tạo thread nhận tin nhắn từ server
    pthread_create(&receive_thread, NULL, receive_messages, NULL);

    // Nhận danh sách người dùng và chat
    while (1) {
        printf("Enter message or /chat <username> to chat with someone, /exit to quit: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';  // Xóa ký tự '\n'

        // Nếu người dùng muốn thoát
        if (strcmp(message, "/exit") == 0) {
            send(sock, message, strlen(message), 0);
            printf("You have left the chat.\n");
            break;
        }

        // Nếu người dùng muốn bắt đầu chat với người khác
        if (strncmp(message, "/chat ", 6) == 0) {
            send(sock, message, strlen(message), 0);
        } else {
            // Gửi tin nhắn bình thường
            send(sock, message, strlen(message), 0);
        }
    }

    // Đóng kết nối và thoát
    close(sock);
    pthread_join(receive_thread, NULL);
    return 0;
}
