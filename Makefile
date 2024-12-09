CC = gcc
CFLAGS = -Wall -Iinclude -g
LDFLAGS = -lSDL2 -lSDL2_ttf -lmysqlclient
LIB_DIR = /usr/lib/mysql  # Nếu cần, thay đổi đường dẫn tới thư viện MySQL của bạn
INCLUDE_DIR = /usr/include/mysql  # Đảm bảo rằng thư mục này có file header của MySQL

# Danh sách các thư mục và file nguồn
SRC_DIR = src
OBJ_DIR = obj
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/gui.c $(SRC_DIR)/db.c  # Đảm bảo thêm db.c vào
OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/gui.o $(OBJ_DIR)/db.o

# Tên output
OUT = chat_app

# Quy trình biên dịch
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)  # Tạo thư mục obj nếu chưa tồn tại
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@  # Thêm đường dẫn include của MySQL

clean:
	rm -rf $(OBJ_DIR) $(OUT)

.PHONY: clean