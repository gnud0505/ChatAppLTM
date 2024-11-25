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
