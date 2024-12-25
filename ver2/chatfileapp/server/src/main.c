// server/src/main.c

#include "../include/db.h"
#include "../include/chat.h"
#include "../include/group.h"
#include "../include/file.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024


#define CHAT_PORT 8085
#define GROUP_PORT 8086
#define FILE_PORT 8087

typedef struct {
    int sock;
    int id;
    char username[50];
} Client;

DBConnection db;

// Hàm xử lý kết nối của từng client
void *handle_client(void *arg) {
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Nhận yêu cầu đăng ký hoặc đăng nhập
    ssize_t bytesReceived = recv(client->sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        close(client->sock);
        free(client);
        pthread_exit(NULL);
    }
    buffer[bytesReceived] = '\0';

    // Xử lý đăng ký hoặc đăng nhập
    if (strncmp(buffer, "REGISTER", 8) == 0) {
        char username[50], password[50], email[100];
        sscanf(buffer + 9, "%s %s %s", username, password, email);

        // Kiểm tra xem người dùng đã tồn tại chưa
        char query[BUFFER_SIZE];
        snprintf(query, sizeof(query), "SELECT id FROM users WHERE username='%s'", username);
        MYSQL_RES *res = execute_query(&db, query);
        if (res != NULL && mysql_num_rows(res) > 0) {
            send_response(client->sock, "UserExists");
            mysql_free_result(res);
        } else {
            mysql_free_result(res);
            // Thêm người dùng vào cơ sở dữ liệu
            snprintf(query, sizeof(query), "INSERT INTO users (username, password, email, status) VALUES ('%s', '%s', '%s', 'offline')", username, password, email);
            if (execute_query(&db, query) == NULL) {
                send_response(client->sock, "RegisterFailed");
            } else {
                // Lấy ID người dùng vừa đăng ký
                int user_id = mysql_insert_id(db.conn);
                client->id = user_id;
                strncpy(client->username, username, sizeof(client->username) - 1);
                send_response(client->sock, "RegisterSuccess");
                printf("User %s đã đăng ký.\n", username);
            }
        }
    } else if (strncmp(buffer, "LOGIN", 5) == 0) {
        char username[50], password[50];
        sscanf(buffer + 6, "%s %s", username, password);

        // Kiểm tra thông tin đăng nhập
        char query[BUFFER_SIZE];
        snprintf(query, sizeof(query), "SELECT id FROM users WHERE username='%s' AND password='%s'", username, password);
        MYSQL_RES *res = execute_query(&db, query);
        if (res != NULL && mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            int user_id = atoi(row[0]);
            client->id = user_id;
            strncpy(client->username, username, sizeof(client->username) - 1);
            send_response(client->sock, "LoginSuccess");

            // Cập nhật trạng thái online
            snprintf(query, sizeof(query), "UPDATE users SET status='online' WHERE id=%d", user_id);
            execute_query(&db, query);

            printf("User %s đã đăng nhập.\n", username);
            mysql_free_result(res);
        } else {
            send_response(client->sock, "LoginFailed");
            mysql_free_result(res);
            close(client->sock);
            free(client);
            pthread_exit(NULL);
        }
    } else {
        send_response(client->sock, "InvalidCommand");
        close(client->sock);
        free(client);
        pthread_exit(NULL);
    }

    // Vòng lặp xử lý các lệnh sau khi đăng nhập hoặc đăng ký thành công
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(client->sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            // Cập nhật trạng thái offline
            char update_query[BUFFER_SIZE];
            snprintf(update_query, sizeof(update_query), "UPDATE users SET status='offline' WHERE id=%d", client->id);
            execute_query(&db, update_query);

            printf("User %s đã ngắt kết nối.\n", client->username);
            close(client->sock);
            free(client);
            pthread_exit(NULL);
        }
        buffer[bytesReceived] = '\0';

        // Xử lý các lệnh
        if (strncmp(buffer, "PRIVATE", 7) == 0) {
            char recipient[50];
            char message[BUFFER_SIZE];
            sscanf(buffer + 8, "%s %[^\n]", recipient, message);
            handle_private_message(&db, client->id, recipient, message, client->sock);
        } else if (strncmp(buffer, "CREATE_GROUP", 12) == 0) {
            char group_name[50];
            sscanf(buffer + 13, "%s", group_name);
            int group_id = create_group(&db, group_name, client->id);
            if (group_id == -1) {
                send_response(client->sock, "CreateGroupFailed");
            } else {
                send_response(client->sock, "GroupCreated");
            }
        } else if (strncmp(buffer, "JOIN_GROUP", 10) == 0) {
            char group_name[50];
            sscanf(buffer + 11, "%s", group_name);
            int group_id = join_group(&db, group_name, client->id);
            if (group_id == -1) {
                send_response(client->sock, "JoinGroupFailed");
            } else if (group_id == -2) {
                send_response(client->sock, "AlreadyMember");
            } else {
                send_response(client->sock, "JoinedGroup");
            }
        } else if (strncmp(buffer, "LEAVE_GROUP", 11) == 0) {
            char group_name[50];
            sscanf(buffer + 12, "%s", group_name);
            if (leave_group(&db, group_name, client->id) == 0) {
                send_response(client->sock, "LeftGroup");
            } else {
                send_response(client->sock, "LeaveGroupFailed");
            }
        } else if (strncmp(buffer, "ADD_MEMBER", 10) == 0) {
            char group_name[50], member_username[50];
            sscanf(buffer + 11, "%s %s", group_name, member_username);
            if (add_member_to_group(&db, group_name, member_username) == 0) {
                send_response(client->sock, "AddMemberSuccess");
            } else if (add_member_to_group(&db, group_name, member_username) == -2) {
                send_response(client->sock, "MemberAlreadyExists");
            } else {
                send_response(client->sock, "AddMemberFailed");
            }
        } else if (strncmp(buffer, "REMOVE_MEMBER", 13) == 0) {
            char group_name[50], member_username[50];
            sscanf(buffer + 14, "%s %s", group_name, member_username);
            if (remove_member_from_group(&db, group_name, member_username) == 0) {
                send_response(client->sock, "RemoveMemberSuccess");
            } else {
                send_response(client->sock, "RemoveMemberFailed");
            }
        } else if (strncmp(buffer, "SEND", 4) == 0) {
            char group_name[50];
            char message[BUFFER_SIZE];
            sscanf(buffer + 5, "%s %[^\n]", group_name, message);
            if (send_group_message(&db, client->id, group_name, message) == 0) {
                send_response(client->sock, "GroupMessageSent");
            } else {
                send_response(client->sock, "GroupMessageFailed");
            }
        } else if (strncmp(buffer, "LIST_GROUPS", 11) == 0) {
            char groups[BUFFER_SIZE];
            if (list_user_groups(&db, client->id, groups, sizeof(groups)) == 0) {
                send(client->sock, groups, strlen(groups), 0);
            } else {
                send_response(client->sock, "ListGroupsFailed");
            }
        } else if (strncmp(buffer, "LIST_USERS", 10) == 0) {
            char users[BUFFER_SIZE];
            if (list_all_users(&db, users, sizeof(users)) == 0) {
                send(client->sock, users, strlen(users), 0);
            } else {
                send_response(client->sock, "ListUsersFailed");
            }
        } else if (strncmp(buffer, "SEARCH", 6) == 0) {
            char query_str[BUFFER_SIZE];
            sscanf(buffer + 7, "%s", query_str);
            char search_results[BUFFER_SIZE];
            if (search_files(&db, query_str, search_results, sizeof(search_results)) == 0) {
                send(client->sock, search_results, strlen(search_results), 0);
                send_response(client->sock, "END_OF_RESULTS");
            } else {
                send_response(client->sock, "SearchFailed");
            }
        } else {
            send_response(client->sock, "UnknownCommand");
        }
    }

    close(client->sock);
    free(client);
    pthread_exit(NULL);
}

int main() {
    // Kết nối đến cơ sở dữ liệu
    if (connect_db(&db, "localhost", "root", "", "chat_app") != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to connect to database.\n");
        exit(EXIT_FAILURE);
    }

    int server_sock, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Tạo socket TCP cho chat
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        handle_error("Socket failed");
    }

    // Thiết lập địa chỉ
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CHAT_PORT);

    // Gắn socket với địa chỉ
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        handle_error("Bind failed");
    }

    // Lắng nghe kết nối
    if (listen(server_sock, 10) < 0) {
        handle_error("Listen failed");
    }

    printf("Server đang lắng nghe trên cổng %d...\n", CHAT_PORT);

    while (1) {
        if ((new_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Tạo client mới
        Client *client = (Client *)malloc(sizeof(Client));
        client->sock = new_sock;
        client->id = -1;
        memset(client->username, 0, sizeof(client->username));

        // Tạo thread để xử lý client
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client);
        pthread_detach(thread_id);
    }

    close(server_sock);
    close_db(&db);
    return 0;
}
