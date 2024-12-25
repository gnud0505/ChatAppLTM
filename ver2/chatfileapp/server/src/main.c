#include "../include/common.h"
#include "../include/chat.h"
#include "../include/group.h"
#include "../include/file.h"
#include "../include/db.h"
#include "../include/utils.h"

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
Client *clients[MAX_CLIENTS];
int client_count = 0;

// Function to add client to the list
void add_client(Client *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i=0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to remove client from the list
void remove_client(int client_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i=0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->socket == client_sock) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int main(int argc, char **argv) {
    // Initialize database connection
    DBConnection db;
    if (connect_db(&db, "localhost", "root", "your_password", "chat_app") != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to connect to database.\n");
        exit(EXIT_FAILURE);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("Socket creation failed");

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        handle_error("Bind failed");

    // Listen
    if (listen(server_sock, 10) < 0)
        handle_error("Listen failed");

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New connection accepted\n");

        // Create a thread to handle the client
        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = client_sock;
        if (pthread_create(&tid, NULL, handle_client, pclient) != 0) {
            perror("pthread_create failed");
            free(pclient);
        }
        pthread_detach(tid);
    }

    close(server_sock);
    close_db(&db);
    return 0;
}
