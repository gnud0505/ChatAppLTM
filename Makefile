CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lSDL2 -lSDL2_ttf
SRC = src/main.c src/gui.c
OUT = chat_app

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)
