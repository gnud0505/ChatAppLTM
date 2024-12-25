#ifndef CHAT_H
#define CHAT_H

#include "db.h"

void handle_chat(int client_sock, DBConnection *db, const char *username);

#endif // CHAT_H
