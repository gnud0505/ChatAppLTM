// client/src/main.c

#include "../include/chat.h"
#include "../include/group.h"
#include "../include/file.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define CHAT_PORT 8085
#define GROUP_PORT 8086
#define FILE_PORT 8087
#define BUFFER_SIZE 1024
#define MESSAGE_BUFFER 1024
#define USERNAME_BUFFER 254

int main() {
    char username[50];
    char password[50];
    char email[100];
    int choice;
    int chat_sock, group_sock, file_sock;

    printf("==== Đăng Ký / Đăng Nhập ====\n");
    printf("1. Đăng ký\n2. Đăng nhập\nLựa chọn: ");
    scanf("%d", &choice);
    getchar(); // Xóa ký tự newline

    // Kết nối đến server để đăng ký hoặc đăng nhập
    int auth_sock;
    struct sockaddr_in server_address;
    auth_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (auth_sock < 0) {
        handle_error("Không thể tạo socket");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CHAT_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        handle_error("Địa chỉ IP không hợp lệ");
    }

    if (connect(auth_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        handle_error("Kết nối đến server thất bại");
    }

    if (choice == 1) {
        // Đăng ký
        printf("Nhập tên người dùng: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;

        printf("Nhập mật khẩu: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;

        printf("Nhập email: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = 0;

        // Gửi yêu cầu đăng ký đến server
        char register_buffer[BUFFER_SIZE];
        snprintf(register_buffer, sizeof(register_buffer), "REGISTER %s %s %s", username, password, email);
        send(auth_sock, register_buffer, strlen(register_buffer), 0);

        // Nhận phản hồi từ server
        char response[BUFFER_SIZE];
        ssize_t bytesReceived = recv(auth_sock, response, sizeof(response) - 1, 0);
        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            printf("Server: %s\n", response);
            if (strcmp(response, "RegisterSuccess") == 0) {
                printf("Đăng ký thành công! Bạn có thể đăng nhập ngay.\n");
            } else {
                printf("Đăng ký thất bại: %s\n", response);
                close(auth_sock);
                exit(0);
            }
        }
    } else if (choice == 2) {
        // Đăng nhập
        printf("Nhập tên người dùng: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;

        printf("Nhập mật khẩu: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;

        // Gửi yêu cầu đăng nhập đến server
        char login_buffer[BUFFER_SIZE];
        snprintf(login_buffer, sizeof(login_buffer), "LOGIN %s %s", username, password);
        send(auth_sock, login_buffer, strlen(login_buffer), 0);

        // Nhận phản hồi từ server
        char response[BUFFER_SIZE];
        ssize_t bytesReceived = recv(auth_sock, response, sizeof(response) - 1, 0);
        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            printf("Server: %s\n", response);
            if (strcmp(response, "LoginSuccess") == 0) {
                printf("Đăng nhập thành công!\n");
            } else {
                printf("Đăng nhập thất bại: %s\n", response);
                close(auth_sock);
                exit(0);
            }
        }
    } else {
        printf("Lựa chọn không hợp lệ.\n");
        close(auth_sock);
        exit(0);
    }

    close(auth_sock);

    // Khởi tạo kết nối cho chat, group, và file
    chat_sock = init_chat(SERVER_IP, CHAT_PORT, username);
    group_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (group_sock < 0) {
        handle_error("Không thể tạo socket cho group");
    }

    struct sockaddr_in group_address;
    group_address.sin_family = AF_INET;
    group_address.sin_port = htons(GROUP_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &group_address.sin_addr) <= 0) {
        handle_error("Địa chỉ IP không hợp lệ cho group");
    }

    if (connect(group_sock, (struct sockaddr *)&group_address, sizeof(group_address)) < 0) {
        handle_error("Kết nối đến server group thất bại");
    }

    // Gửi username đến server group
    send(group_sock, username, strlen(username), 0);

    file_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (file_sock < 0) {
        handle_error("Không thể tạo socket cho file");
    }

    struct sockaddr_in file_address;
    file_address.sin_family = AF_INET;
    file_address.sin_port = htons(FILE_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &file_address.sin_addr) <= 0) {
        handle_error("Địa chỉ IP không hợp lệ cho file");
    }

    if (connect(file_sock, (struct sockaddr *)&file_address, sizeof(file_address)) < 0) {
        handle_error("Kết nối đến server file thất bại");
    }

    // Gửi username đến server file
    send(file_sock, username, strlen(username), 0);

    // Khởi tạo kết nối cho group và file (nếu cần thiết)
    init_group(group_sock, username);
    init_file(file_sock, username);

    int main_choice;
    while (1) {
        printf("\n==== Menu Chính ====\n");
        printf("1. Chat 1-1\n");
        printf("2. Chat Nhóm\n");
        printf("3. Chia Sẻ File\n");
        printf("4. Tìm Kiếm File\n");
        printf("5. Thoát\n");
        printf("Lựa chọn: ");
        scanf("%d", &main_choice);
        getchar(); // Xóa ký tự newline

        if (main_choice == 1) {
            // Chat 1-1
            char recipient[USERNAME_BUFFER];
            char message[MESSAGE_BUFFER];
            printf("Nhập tên người dùng muốn nhắn tin: ");
            fgets(recipient, sizeof(recipient), stdin);
            recipient[strcspn(recipient, "\n")] = 0;

            printf("Nhập tin nhắn: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;

            send_private_message(chat_sock, recipient, message);
        } else if (main_choice == 2) {
            // Chat Nhóm
            int group_choice;
            printf("\n==== Chat Nhóm ====\n");
            printf("1. Tạo nhóm\n");
            printf("2. Tham gia nhóm\n");
            printf("3. Rời nhóm\n");
            printf("4. Gửi tin nhắn nhóm\n");
            printf("5. Thêm thành viên vào nhóm\n");
            printf("6. Xóa thành viên khỏi nhóm\n");
            printf("7. Liệt kê các nhóm\n");
            printf("8. Liệt kê người dùng\n");
            printf("9. Quay lại Menu Chính\n");
            printf("Lựa chọn: ");
            scanf("%d", &group_choice);
            getchar(); // Xóa ký tự newline

            if (group_choice == 1) {
                char group_name[BUFFER_SIZE];
                printf("Nhập tên nhóm: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;
                create_group(group_sock, group_name);
            } else if (group_choice == 2) {
                char group_name[BUFFER_SIZE];
                printf("Nhập tên nhóm để tham gia: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;
                join_group(group_sock, group_name);
            } else if (group_choice == 3) {
                char group_name[BUFFER_SIZE];
                printf("Nhập tên nhóm để rời: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;
                leave_group(group_sock, group_name);
            } else if (group_choice == 4) {
                char group_name[BUFFER_SIZE];
                char message[MESSAGE_BUFFER];
                printf("Nhập tên nhóm: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;

                printf("Nhập tin nhắn: ");
                fgets(message, sizeof(message), stdin);
                message[strcspn(message, "\n")] = 0;

                send_group_message(group_sock, group_name, message);
            } else if (group_choice == 5) {
                char group_name[BUFFER_SIZE];
                char member_username[USERNAME_BUFFER];
                printf("Nhập tên nhóm: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;

                printf("Nhập tên người dùng cần thêm: ");
                fgets(member_username, sizeof(member_username), stdin);
                member_username[strcspn(member_username, "\n")] = 0;

                add_member(group_sock, group_name, member_username);
            } else if (group_choice == 6) {
                char group_name[BUFFER_SIZE];
                char member_username[USERNAME_BUFFER];
                printf("Nhập tên nhóm: ");
                fgets(group_name, sizeof(group_name), stdin);
                group_name[strcspn(group_name, "\n")] = 0;

                printf("Nhập tên người dùng cần xóa: ");
                fgets(member_username, sizeof(member_username), stdin);
                member_username[strcspn(member_username, "\n")] = 0;

                remove_member(group_sock, group_name, member_username);
            } else if (group_choice == 7) {
                list_groups(group_sock);
            } else if (group_choice == 8) {
                list_users(group_sock);
            } else if (group_choice == 9) {
                continue;
            } else {
                printf("Lựa chọn không hợp lệ.\n");
            }
        } else if (main_choice == 3) {
            // Chia Sẻ File
            int file_choice;
            printf("\n==== Chia Sẻ File ====\n");
            printf("1. Tải lên file\n");
            printf("2. Tải xuống file\n");
            printf("3. Tải lên thư mục\n");
            printf("4. Tải xuống thư mục\n");
            printf("5. Quay lại Menu Chính\n");
            printf("Lựa chọn: ");
            scanf("%d", &file_choice);
            getchar(); // Xóa ký tự newline

            if (file_choice == 1) {
                char file_path[BUFFER_SIZE];
                char receiver[USERNAME_BUFFER];
                int is_group;
                printf("1. Gửi tới người dùng\n2. Gửi tới nhóm\nLựa chọn: ");
                scanf("%d", &is_group);
                getchar();

                printf("Nhập tên người dùng hoặc tên nhóm: ");
                fgets(receiver, sizeof(receiver), stdin);
                receiver[strcspn(receiver, "\n")] = 0;

                printf("Nhập đường dẫn file: ");
                fgets(file_path, sizeof(file_path), stdin);
                file_path[strcspn(file_path, "\n")] = 0;

                upload_file(file_sock, file_path, receiver, is_group == 2);
            } else if (file_choice == 2) {
                char file_name[BUFFER_SIZE];
                char save_path[BUFFER_SIZE];
                printf("Nhập tên file cần tải xuống: ");
                fgets(file_name, sizeof(file_name), stdin);
                file_name[strcspn(file_name, "\n")] = 0;

                printf("Nhập đường dẫn để lưu file: ");
                fgets(save_path, sizeof(save_path), stdin);
                save_path[strcspn(save_path, "\n")] = 0;

                download_file(file_sock, file_name, save_path);
            } else if (file_choice == 3) {
                char dir_path[BUFFER_SIZE];
                char receiver[USERNAME_BUFFER];
                int is_group;
                printf("1. Gửi tới người dùng\n2. Gửi tới nhóm\nLựa chọn: ");
                scanf("%d", &is_group);
                getchar();

                printf("Nhập tên người dùng hoặc tên nhóm: ");
                fgets(receiver, sizeof(receiver), stdin);
                receiver[strcspn(receiver, "\n")] = 0;

                printf("Nhập đường dẫn thư mục: ");
                fgets(dir_path, sizeof(dir_path), stdin);
                dir_path[strcspn(dir_path, "\n")] = 0;

                upload_directory(file_sock, dir_path, receiver, is_group == 2);
            } else if (file_choice == 4) {
                char dir_name[BUFFER_SIZE];
                char save_path[BUFFER_SIZE];
                printf("Nhập tên thư mục cần tải xuống: ");
                fgets(dir_name, sizeof(dir_name), stdin);
                dir_name[strcspn(dir_name, "\n")] = 0;

                printf("Nhập đường dẫn để lưu thư mục: ");
                fgets(save_path, sizeof(save_path), stdin);
                save_path[strcspn(save_path, "\n")] = 0;

                download_directory(file_sock, dir_name, save_path);
            } else if (file_choice == 5) {
                continue;
            } else {
                printf("Lựa chọn không hợp lệ.\n");
            }
        } else if (main_choice == 4) {
            // Tìm Kiếm File
            char query[BUFFER_SIZE];
            printf("Nhập từ khóa tìm kiếm: ");
            fgets(query, sizeof(query), stdin);
            query[strcspn(query, "\n")] = 0;

            search_files(file_sock, query);
        } else if (main_choice == 5) {
            printf("Đang thoát...\n");
            close(chat_sock);
            close(group_sock);
            close(file_sock);
            exit(0);
        } else {
            printf("Lựa chọn không hợp lệ.\n");
        }
    }

    close(chat_sock);
    close(group_sock);
    close(file_sock);
    return 0;
}
