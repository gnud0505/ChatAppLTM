#include "../include/common.h"
#include "../include/utils.h"

void register_user(int sock) {
    char username[BUFFER_SIZE], password[BUFFER_SIZE], email[BUFFER_SIZE];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    printf("Enter email: ");
    scanf("%s", email);

    // Send register command
    send_all(sock, "Register", strlen("Register"));
    send_all(sock, username, strlen(username));
    send_all(sock, password, strlen(password));
    send_all(sock, email, strlen(email));

    // Receive response
    char response[BUFFER_SIZE];
    memset(response, 0, BUFFER_SIZE);
    recv_all(sock, response, sizeof(response));

    if (strcmp(response, "RegisterSuccess") == 0) {
        printf("Registration successful!\n");
    } else if (strcmp(response, "UserExists") == 0) {
        printf("Username already exists.\n");
    } else {
        printf("Registration failed: %s\n", response);
    }
}

int login_user(int sock) {
    char username[BUFFER_SIZE], password[BUFFER_SIZE];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    // Send login command
    send_all(sock, "Login", strlen("Login"));
    send_all(sock, username, strlen(username));
    send_all(sock, password, strlen(password));

    // Receive response
    char response[BUFFER_SIZE];
    memset(response, 0, BUFFER_SIZE);
    recv_all(sock, response, sizeof(response));

    if (strcmp(response, "LoginSuccess") == 0) {
        printf("Login successful!\n");
        return 1;
    } else {
        printf("Login failed: %s\n", response);
        return 0;
    }
}
