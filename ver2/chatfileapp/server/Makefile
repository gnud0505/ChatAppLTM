# server/Makefile

CC = gcc
CFLAGS = -Wall -pthread `mysql_config --cflags`
LIBS = `mysql_config --libs`
TARGET = server

SRC = main.c chat.c group.c file.c db.c utils.c
OBJ = $(SRC:.c=.o)

INCLUDE = -I../include

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
