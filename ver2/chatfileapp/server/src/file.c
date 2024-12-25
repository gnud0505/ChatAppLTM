#include "../include/file.h"
#include "../include/common.h"
#include "../include/utils.h"
#include "../include/db.h"

void handle_file(int client_sock, DBConnection *db, const char *username) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv_all(client_sock, buffer, sizeof(buffer)) <= 0) {
            printf("Client %s disconnected from file transfer.\n", username);
            break;
        }

        if (strncmp(buffer, "UPLOAD_FILE", 11) == 0) {
            // Parse and handle file upload
            char filename[BUFFER_SIZE];
            sscanf(buffer + 12, "%s", filename);
            // Implement file upload logic
        }
        else if (strncmp(buffer, "DOWNLOAD_FILE", 13) == 0) {
            // Parse and handle file download
            char filename[BUFFER_SIZE];
            sscanf(buffer + 14, "%s", filename);
            // Implement file download logic
        }
        else if (strncmp(buffer, "SEARCH_FILE", 11) == 0) {
            // Parse and handle file search
            char query[BUFFER_SIZE];
            sscanf(buffer + 12, "%s", query);
            // Implement file search logic
        }
        else if (strncmp(buffer, "EXIT_FILE", 9) == 0) {
            printf("Client %s exited file transfer.\n", username);
            break;
        }
    }
}
