# Makefile_client

CC = gcc
CFLAGS = -Wall
DEPS = common.h
OBJ = client.o functions.o

all: client

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o client
