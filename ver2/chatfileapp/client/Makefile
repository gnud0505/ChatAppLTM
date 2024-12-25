# client/Makefile

CC = gcc
CFLAGS = -Wall -pthread
TARGET = client

SRC = main.c chat.c group.c file.c utils.c
OBJ = $(SRC:.c=.o)

INCLUDE = -I../include

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
