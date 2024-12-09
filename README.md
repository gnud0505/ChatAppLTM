# ChatAppLTM
// gui
sudo apt update
sudo apt install libsdl2-dev libsdl2-ttf-dev

// sql connect
sudo apt-get install libmysqlclient-dev

// sql
sudo apt update
sudo apt install mysql-server

// tao database
sudo mysql -u root -p
SOURCE ./create_database.sql

//test database
gcc ./tests/test.c ./include/db.h ./src/db.c -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -o ./tests/test.exe

gcc ./tests/test.c ./src/db.c -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -o ./tests/test.exe

// run group
gcc ./src/group/server/server.c ./src/db.c -o server_exec -lmysqlclient -lpthread
gcc client.c -o client

then, run file server and client

// run module_file
make
then, run file server and client