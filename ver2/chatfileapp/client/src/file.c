#include "../include/file.h"
#include "../include/common.h"
#include "../include/utils.h"

void handle_file_transfer(int sock) {
    int choice;
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    
    printf("\nFile Transfer Options:\n");
    printf("1. Upload File\n2. Download File\n3. Search File\n4. Back\nChoose an option: ");
    scanf("%d", &choice);
    getchar(); // Consume newline

    switch (choice) {
        case 1:
            printf("Enter filename to upload: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "UPLOAD_FILE %s", filename);
            send_all(sock, buffer, strlen(buffer));
            // Implement file upload logic
            break;
        case 2:
            printf("Enter filename to download: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "DOWNLOAD_FILE %s", filename);
            send_all(sock, buffer, strlen(buffer));
            // Implement file download logic
            break;
        case 3:
            printf("Enter filename to search: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';
            snprintf(buffer, sizeof(buffer), "SEARCH_FILE %s", filename);
            send_all(sock, buffer, strlen(buffer));
            // Implement file search logic
            break;
        case 4:
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
