#ifndef GROUP_H
#define GROUP_H

#include "db.h"

void handle_group(int client_sock, DBConnection *db, const char *username);

#endif // GROUP_H
