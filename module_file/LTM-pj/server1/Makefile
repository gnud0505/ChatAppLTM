# Makefile_server

CC = gcc
CFLAGS = -Wall
DEPS = common.h
OBJ = server.o functions.o

all: server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o server
