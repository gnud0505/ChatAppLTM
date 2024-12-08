#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../../include/db.h"

#define PORT 8084
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct
{
    int socket;
    int id;
    char username[50];
    char password[50];
    char email[100];
} Client;

Client *clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
DBConnection db;

// Kiểm tra xem một nhóm có tồn tại không, trả về group_id nếu có
int get_group_id(const char *group_name)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    int group_id = -1;

    snprintf(query, sizeof(query),
             "SELECT id FROM chat_groups WHERE group_name='%s'", group_name);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error querying group_id: %s\n", mysql_error(db.conn));
        return -1;
    }

    res = mysql_store_result(db.conn);
    if (res && (row = mysql_fetch_row(res)))
    {
        group_id = atoi(row[0]);
    }
    mysql_free_result(res);
    return group_id;
}

// Kiểm tra xem một người dùng có tồn tại không, trả về user_id nếu có
int get_user_id(const char *username)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    int user_id = -1;

    snprintf(query, sizeof(query),
             "SELECT id FROM users WHERE username='%s'", username);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error querying user_id: %s\n", mysql_error(db.conn));
        return -1;
    }

    res = mysql_store_result(db.conn);
    if (res && (row = mysql_fetch_row(res)))
    {
        user_id = atoi(row[0]);
    }
    mysql_free_result(res);
    return user_id;
}

// Kiểm tra xem một người dùng có phải admin của nhóm không
int is_group_admin(int user_id, int group_id)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    int is_admin = 0;

    snprintf(query, sizeof(query),
             "SELECT role FROM group_members WHERE user_id=%d AND group_id=%d",
             user_id, group_id);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error querying admin role: %s\n", mysql_error(db.conn));
        return 0;
    }

    res = mysql_store_result(db.conn);
    if (res && (row = mysql_fetch_row(res)))
    {
        if (strcmp(row[0], "admin") == 0)
        {
            is_admin = 1;
        }
    }
    mysql_free_result(res);
    return is_admin;
}

// Kiểm tra xem một người dùng có phải thành viên của nhóm không
int is_group_member(int user_id, int group_id)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    int is_member = 0;

    snprintf(query, sizeof(query),
             "SELECT 1 FROM group_members WHERE user_id=%d AND group_id=%d",
             user_id, group_id);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error querying group membership: %s\n", mysql_error(db.conn));
        return 0;
    }

    res = mysql_store_result(db.conn);
    if (res && mysql_num_rows(res) > 0)
    {
        is_member = 1;
    }
    mysql_free_result(res);
    return is_member;
}

void broadcast_message(int group_id, char *message, Client *sender)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;

    pthread_mutex_lock(&mutex);

    // Truy vấn lấy danh sách các thành viên trong nhóm
    snprintf(query, sizeof(query),
             "SELECT users.id FROM users "
             "JOIN group_members ON users.id = group_members.user_id "
             "WHERE group_members.group_id=%d",
             group_id);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error querying group members: %s\n", mysql_error(db.conn));
        pthread_mutex_unlock(&mutex);
        return;
    }
    res = mysql_store_result(db.conn);
    if (res == NULL)
    {
        fprintf(stderr, "Error storing result: %s\n", mysql_error(db.conn));
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Lưu tin nhắn vào bảng messages
    snprintf(query, sizeof(query),
             "INSERT INTO messages (sender_id, message, group_id) "
             "VALUES (%d, '%s', %d)",
             sender->id, message, group_id);
    if (mysql_query(db.conn, query) != 0)
    {
        fprintf(stderr, "Error inserting message: %s\n", mysql_error(db.conn));
    }

    // Giải phóng kết quả truy vấn
    mysql_free_result(res);

    pthread_mutex_unlock(&mutex);
}

int insert_group(DBConnection *db, const char *group_name, int creator_id)
{
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO chat_groups (group_name, creator_id) VALUES ('%s', %d)",
             group_name, creator_id);

    if (mysql_query(db->conn, query))
    {
        fprintf(stderr, "Error inserting group: %s\n", mysql_error(db->conn));
        return -1; // Trả về -1 nếu xảy ra lỗi
    }

    return (int)mysql_insert_id(db->conn); // Trả về ID của nhóm vừa được tạo
}

int add_user_to_group(DBConnection *db, int group_id, int user_id, const char *role)
{
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO group_members (group_id, user_id, role) VALUES (%d, %d, '%s')",
             group_id, user_id, role);

    if (mysql_query(db->conn, query))
    {
        fprintf(stderr, "Error adding user to group: %s\n", mysql_error(db->conn));
        return -1; // Trả về -1 nếu xảy ra lỗi
    }

    return 0; // Trả về 0 nếu thành công
}

int remove_user_from_group(DBConnection *db, int group_id, int user_id)
{
    char query[256];
    snprintf(query, sizeof(query),
             "DELETE FROM group_members WHERE group_id = %d AND user_id = %d",
             group_id, user_id);

    if (mysql_query(db->conn, query))
    {
        fprintf(stderr, "Error removing user from group: %s\n", mysql_error(db->conn));
        return -1; // Trả về -1 nếu xảy ra lỗi
    }

    return 0; // Trả về 0 nếu thành công
}

void list_user_groups(DBConnection *db, int user_id, char *result, size_t result_size)
{
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT g.name FROM groups g "
             "INNER JOIN group_members gm ON g.id = gm.group_id "
             "WHERE gm.user_id = %d",
             user_id);

    if (mysql_query(db->conn, query))
    {
        fprintf(stderr, "Error listing groups: %s\n", mysql_error(db->conn));
        snprintf(result, result_size, "Failed to retrieve groups.\n");
        return;
    }

    MYSQL_RES *res = mysql_store_result(db->conn);
    if (!res)
    {
        fprintf(stderr, "Error storing result: %s\n", mysql_error(db->conn));
        snprintf(result, result_size, "Failed to retrieve groups.\n");
        return;
    }

    MYSQL_ROW row;
    size_t offset = 0;
    while ((row = mysql_fetch_row(res)))
    {
        offset += snprintf(result + offset, result_size - offset, "%s\n", row[0]);
        if (offset >= result_size)
            break; // Tránh tràn bộ đệm
    }

    mysql_free_result(res);

    if (offset == 0)
    {
        snprintf(result, result_size, "You are not in any groups.\n");
    }
}

void list_users(DBConnection *db, char *result, size_t result_size)
{
    char query[256];
    snprintf(query, sizeof(query), "SELECT username FROM users");

    if (mysql_query(db->conn, query))
    {
        fprintf(stderr, "Error listing users: %s\n", mysql_error(db->conn));
        snprintf(result, result_size, "Failed to retrieve users.\n");
        return;
    }

    MYSQL_RES *res = mysql_store_result(db->conn);
    if (!res)
    {
        fprintf(stderr, "Error storing result: %s\n", mysql_error(db->conn));
        snprintf(result, result_size, "Failed to retrieve users.\n");
        return;
    }

    MYSQL_ROW row;
    size_t offset = 0;
    while ((row = mysql_fetch_row(res)))
    {
        offset += snprintf(result + offset, result_size - offset, "%s\n", row[0]);
        if (offset >= result_size)
            break; // Tránh tràn bộ đệm
    }

    mysql_free_result(res);

    if (offset == 0)
    {
        snprintf(result, result_size, "No users found.\n");
    }
}

int list_group_messages(DBConnection *db, int group_id, int user_id, int client_socket)
{
    char query[BUFFER_SIZE];
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;

    // Kiểm tra nếu người dùng là thành viên của nhóm
    if (!is_group_member(user_id, group_id))
    {
        send(client_socket, "You are not a member of this group.\n", 37, 0);
        return -1; // Trả về -1 nếu người dùng không phải thành viên
    }

    // Truy vấn tin nhắn từ nhóm
    snprintf(query, sizeof(query),
             "SELECT m.id AS message_id, m.sender_id, m.message "
             "FROM messages m "
             "WHERE m.group_id = %d "
             "ORDER BY m.created_at ASC",
             group_id);

    if (mysql_query(db->conn, query) != 0)
    {
        fprintf(stderr, "Error querying group messages: %s\n", mysql_error(db->conn));
        send(client_socket, "Error retrieving group messages.\n", 33, 0);
        return -1; // Trả về -1 nếu có lỗi
    }

    res = mysql_store_result(db->conn);
    if (res == NULL)
    {
        fprintf(stderr, "Error storing result: %s\n", mysql_error(db->conn));
        send(client_socket, "Error retrieving group messages.\n", 33, 0);
        return -1; // Trả về -1 nếu không có kết quả
    }

    // Gửi danh sách tin nhắn tới client
    char message_buffer[BUFFER_SIZE];
    int message_count = 0;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        snprintf(message_buffer, sizeof(message_buffer), "Message ID: %s | Sender ID: %s | Message: %s\n",
                 row[0],  // message_id
                 row[1],  // sender_id
                 row[2]); // message
        send(client_socket, message_buffer, strlen(message_buffer), 0);
        message_count++;
    }

    if (message_count == 0)
    {
        send(client_socket, "No messages in this group.\n", 26, 0);
    }
    else
    {
        send(client_socket, "END_OF_MESSAGES\n", 17, 0);
    }

    mysql_free_result(res);
    return 0; // Trả về 0 nếu thành công
}

void handle_client(void *arg)
{
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];

    // Nhận username, password và email từ client
    recv(client->socket, buffer, sizeof(buffer), 0);
    sscanf(buffer, "%s %s %s", client->username, client->password, client->email);

    // Lưu thông tin người dùng vào cơ sở dữ liệu
    char insert_query[BUFFER_SIZE];
    snprintf(insert_query, sizeof(insert_query),
             "INSERT INTO users (username, password, email, status) "
             "VALUES ('%s', '%s', '%s', 'online')",
             client->username, client->password, client->email);

    pthread_mutex_lock(&mutex);
    if (execute_query(&db, insert_query) == EXIT_SUCCESS)
    {
        printf("User %s registered successfully.\n", client->username);
        client->id = mysql_insert_id(db.conn);
    }
    else
    {
        printf("Failed to register user %s.\n", client->username);
    }
    pthread_mutex_unlock(&mutex);

    while (1)
    {
        int bytes_received = recv(client->socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            printf("Client %s disconnected.\n", client->username);

            // Cập nhật trạng thái người dùng
            snprintf(insert_query, sizeof(insert_query),
                     "UPDATE users SET status='offline' WHERE id='%d'",
                     client->id);

            pthread_mutex_lock(&mutex);
            execute_query(&db, insert_query);
            pthread_mutex_unlock(&mutex);

            close(client->socket);
            free(client);
            pthread_exit(NULL);
        }

        buffer[bytes_received] = '\0';
        printf("Received from %s: %s\n", client->username, buffer);

        if (strncmp(buffer, "CREATE_GROUP", 12) == 0)
        {
            char group_name[50];
            sscanf(buffer + 13, "%s", group_name);

            pthread_mutex_lock(&mutex);
            int group_id = insert_group(&db, group_name, client->id);
            if (group_id == -1)
            {
                send(client->socket, "Failed to create group.\n", 24, 0);
            }
            else if (add_user_to_group(&db, group_id, client->id, "admin") == 0)
            {
                send(client->socket, "Group created successfully.\n", 29, 0);
            }
            else
            {
                send(client->socket, "Failed to assign admin role.\n", 28, 0);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "JOIN_GROUP", 10) == 0)
        {
            char group_name[50];
            sscanf(buffer + 11, "%s", group_name);

            pthread_mutex_lock(&mutex);
            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group not found.\n", 17, 0);
            }
            else if (is_group_member(client->id, group_id))
            {
                send(client->socket, "You are already a member of this group.\n", 39, 0);
            }
            else if (add_user_to_group(&db, group_id, client->id, "member") == 0)
            {
                send(client->socket, "Joined group successfully.\n", 27, 0);
            }
            else
            {
                send(client->socket, "Failed to join group.\n", 22, 0);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "ADD_MEMBER", 10) == 0)
        {
            char group_name[50], username_to_add[50];
            sscanf(buffer + 11, "%s %s", group_name, username_to_add);

            pthread_mutex_lock(&mutex);

            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group does not exist.\n", 22, 0);
                pthread_mutex_unlock(&mutex);
                continue;
            }

            int user_id_to_add = get_user_id(username_to_add);
            if (user_id_to_add == -1)
            {
                send(client->socket, "User does not exist.\n", 21, 0);
                pthread_mutex_unlock(&mutex);
                continue;
            }

            if (add_user_to_group(&db, group_id, user_id_to_add, "member") == 0)
            {
                send(client->socket, "User added to the group successfully.\n", 36, 0);
            }
            else
            {
                send(client->socket, "Failed to add user to the group.\n", 33, 0);
            }

            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "SEND", 4) == 0)
        {
            char group_name[50], message[BUFFER_SIZE];
            sscanf(buffer + 5, "%s %[^\n]", group_name, message);

            // Kiểm tra nếu tên nhóm hoặc tin nhắn không hợp lệ
            if (strlen(group_name) == 0 || strlen(message) == 0)
            {
                send(client->socket, "Invalid group name or message.\n", 32, 0);
                continue; // Quay lại nhận dữ liệu mới
            }

            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group not found.\n", 17, 0);
            }
            else
            {
                // Gọi hàm broadcast_message
                broadcast_message(group_id, message, client);
            }

            // Gửi xác nhận lại cho người gửi
            send(client->socket, "Message sent successfully.\n", 27, 0);
            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "REMOVE_MEMBER", 13) == 0)
        {
            char group_name[50], username_to_remove[50];
            sscanf(buffer + 14, "%s %s", group_name, username_to_remove);

            pthread_mutex_lock(&mutex);

            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group does not exist.\n", 22, 0);
            }
            else if (is_group_admin(client->id, group_id))
            {
                send(client->socket, "You are not an admin of this group.\n", 37, 0);
            }
            else
            {
                int user_id_to_remove = get_user_id(username_to_remove);
                if (user_id_to_remove == -1)
                {
                    send(client->socket, "User does not exist.\n", 21, 0);
                }
                else if (remove_user_from_group(&db, group_id, user_id_to_remove) == 0)
                {
                    send(client->socket, "User removed from the group successfully.\n", 42, 0);
                }
                else
                {
                    send(client->socket, "Failed to remove user from the group.\n", 38, 0);
                }
            }

            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "LEAVE_GROUP", 11) == 0)
        {
            char group_name[50];
            sscanf(buffer + 12, "%s", group_name);

            pthread_mutex_lock(&mutex);

            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group not found.\n", 17, 0);
            }
            else if (!is_group_member(client->id, group_id))
            {
                send(client->socket, "You are not a member of this group.\n", 36, 0);
            }
            else if (remove_user_from_group(&db, group_id, client->id) == 0)
            {
                send(client->socket, "You have left the group successfully.\n", 39, 0);
            }
            else
            {
                send(client->socket, "Failed to leave the group.\n", 27, 0);
            }

            pthread_mutex_unlock(&mutex);
        }
        else if (strncmp(buffer, "LIST_GROUPS", 11) == 0)
        {
            char response[BUFFER_SIZE] = {0};

            pthread_mutex_lock(&mutex);
            list_user_groups(&db, client->id, response, sizeof(response));
            pthread_mutex_unlock(&mutex);

            send(client->socket, response, strlen(response), 0);
        }
        else if (strncmp(buffer, "LIST_USERS", 10) == 0)
        {
            char response[BUFFER_SIZE] = {0};

            pthread_mutex_lock(&mutex);
            list_users(&db, response, sizeof(response));
            pthread_mutex_unlock(&mutex);

            send(client->socket, response, strlen(response), 0);
        }
        else if (strncmp(buffer, "LIST_GROUP_MESSAGES", 19) == 0)
        {
            char group_name[50];
            sscanf(buffer + 20, "%s", group_name);

            pthread_mutex_lock(&mutex);
            int group_id = get_group_id(group_name);
            if (group_id == -1)
            {
                send(client->socket, "Group not found.\n", 17, 0);
            }
            else
            {
                int result = list_group_messages(&db, group_id, client->id, client->socket);
                if (result == -1)
                {
                    send(client->socket, "Failed to retrieve messages.\n", 28, 0);
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    }
}

int main()
{
    int server_socket, new_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    if (connect_db(&db, "localhost", "root", "", "chat_app") != EXIT_SUCCESS)
    {
        fprintf(stderr, "Failed to connect to database.\n");
        exit(EXIT_FAILURE);
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (1)
    {
        new_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        Client *new_client = (Client *)malloc(sizeof(Client));
        new_client->socket = new_socket;

        pthread_mutex_lock(&mutex);
        clients[client_count++] = new_client;
        pthread_mutex_unlock(&mutex);

        pthread_t thread;
        pthread_create(&thread, NULL, (void *)handle_client, (void *)new_client);
    }

    close(server_socket);
    close_db(&db);
    return 0;
}
