#include "../include/group.h"
#include "../include/common.h"
#include "../include/utils.h"

void handle_group(int sock, const char *username) {
    int choice;
    char group_name[50];
    char member_username[50];
    char buffer[BUFFER_SIZE];
    
    printf("\nGroup Chat Options:\n");
    printf("1. Create Group\n2. Join Group\n3. Leave Group\n4. Add Member\n5. Remove Member\n6. List Groups\n7. List Users\n8. Back\nChoose an option: ");
    scanf("%d", &choice);
    getchar(); // Consume newline

    switch (choice) {
        case 1:
            printf("Enter group name to create: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "CREATE_GROUP %s", group_name);
            send_all(sock, buffer, strlen(buffer));
            break;
        case 2:
            printf("Enter group name to join: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "JOIN_GROUP %s", group_name);
            send_all(sock, buffer, strlen(buffer));
            break;
        case 3:
            printf("Enter group name to leave: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "LEAVE_GROUP %s", group_name);
            send_all(sock, buffer, strlen(buffer));
            break;
        case 4:
            printf("Enter group name to add member: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = '\0';
            printf("Enter username of member to add: ");
            fgets(member_username, sizeof(member_username), stdin);
            member_username[strcspn(member_username, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "ADD_MEMBER %s %s", group_name, member_username);
            send_all(sock, buffer, strlen(buffer));
            break;
        case 5:
            printf("Enter group name to remove member: ");
            fgets(group_name, sizeof(group_name), stdin);
            group_name[strcspn(group_name, "\n")] = '\0';
            printf("Enter username of member to remove: ");
            fgets(member_username, sizeof(member_username), stdin);
            member_username[strcspn(member_username, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "REMOVE_MEMBER %s %s", group_name, member_username);
            send_all(sock, buffer, strlen(buffer));
            break;
        case 6:
            snprintf(buffer, sizeof(buffer), "LIST_GROUPS");
            send_all(sock, buffer, strlen(buffer));
            break;
        case 7:
            snprintf(buffer, sizeof(buffer), "LIST_USERS");
            send_all(sock, buffer, strlen(buffer));
            break;
        case 8:
            return;
        default:
            printf("Invalid choice.\n");
            return;
    }

    // Receive and display server response
    memset(buffer, 0, BUFFER_SIZE);
    recv_all(sock, buffer, sizeof(buffer));
    printf("Server: %s\n", buffer);
}
