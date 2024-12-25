#ifndef FILE_H
#define FILE_H

#include "db.h"

void handle_file(int client_sock, DBConnection *db, const char *username);

#endif // FILE_H
