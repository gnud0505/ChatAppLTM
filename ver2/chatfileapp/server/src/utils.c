#include "../include/common.h"
#include "../include/utils.h"
#include "../include/chat.h"
#include <pthread.h>

extern Client *clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

int find_user_socket(const char *username) {
    int sock = -1;
    pthread_mutex_lock(&clients_mutex);
    for (int i=0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (strcmp(clients[i]->username, username) == 0) {
                sock = clients[i]->socket;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return sock;
}
