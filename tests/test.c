#include <stdio.h>
#include <mysql/mysql.h>

int main()
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        printf("mysql_init() failed\n");
        return 1;
    }

   if (mysql_real_connect(conn, "localhost", "root", "transyhieu123", "chat_app", 0, NULL, 0) == NULL)
{
    printf("mysql_real_connect() failed: %s\n", mysql_error(conn));
    mysql_close(conn);
    return 1;
}


    printf("Connected to MySQL database successfully!\n");
    mysql_close(conn);

    return 0;
}
