#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "gui.h"
#include "db.h" // Thêm phần header của db.h (nếu có)

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Hàm hiển thị văn bản lên màn hình
void display_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, white);
    if (!surface) {
        SDL_Log("Error creating surface: %s", TTF_GetError());
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        SDL_Log("Error creating texture: %s", SDL_GetError());
        return;
    }

    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

// Hàm nhập liệu
void handle_input(SDL_Event *event, char *input, int max_length) {
    if (event->type == SDL_TEXTINPUT) {
        if (strlen(input) < max_length - 1) {
            strcat(input, event->text.text);
        }
    } else if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_BACKSPACE && strlen(input) > 0) {
            input[strlen(input) - 1] = '\0';
        }
    }
}

// Hàm thêm người dùng vào cơ sở dữ liệu
void add_user_to_db(const char *username, const char *email, const char *password) {
    // Giả sử bạn có một hàm kết nối và chèn dữ liệu vào cơ sở dữ liệu
    // Đây chỉ là ví dụ sử dụng MySQL (bạn cần link thư viện MySQL hoặc SQLite và chỉnh sửa cho phù hợp)
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];

    // Kết nối với cơ sở dữ liệu
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    if (mysql_real_connect(conn, "localhost", "root", "", "chat_app", 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return;
    }

    // Câu lệnh SQL để chèn người dùng
    const char *query = "INSERT INTO users (username, email, password) VALUES (?, ?, ?)";

    // Tạo đối tượng prepared statement
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        mysql_close(conn);
        return;
    }

    // Chuẩn bị câu lệnh SQL
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    // Gắn tham số vào câu lệnh
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)username;
    bind[0].buffer_length = strlen(username);

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *)email;
    bind[1].buffer_length = strlen(email);

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)password;
    bind[2].buffer_length = strlen(password);

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    // Thực thi câu lệnh
    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
    } else {
        SDL_Log("User added successfully!");
    }

    // Dọn dẹp
    mysql_stmt_close(stmt);
    mysql_close(conn);
}

void start_gui()
{
    printf("INFO: Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return;
    }

    printf("INFO: Initializing SDL_ttf...\n");
    if (TTF_Init() < 0) {
        SDL_Log("Failed to initialize SDL_ttf: %s", TTF_GetError());
        SDL_Quit();
        return;
    }

    SDL_Window *window = SDL_CreateWindow("Chat Application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);  // Sử dụng SDL_RENDERER_SOFTWARE
    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    TTF_Font *font = TTF_OpenFont("assets/fonts/OpenSans.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    char username[50] = "";
    char email[100] = "";
    char password[255] = "";

    bool running = true;
    SDL_Event event;
    int current_field = 0;  // Biến theo dõi trường nhập liệu hiện tại
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (current_field == 0) {
                handle_input(&event, username, sizeof(username));
            } else if (current_field == 1) {
                handle_input(&event, email, sizeof(email));
            } else if (current_field == 2) {
                handle_input(&event, password, sizeof(password));
            }

            if (event.type == SDL_KEYDOWN) {
                // Kiểm tra nếu người dùng nhấn Enter
                if (event.key.keysym.sym == SDLK_RETURN) {
                    if (current_field == 0) {
                        current_field = 1;  // Chuyển đến trường email
                    } else if (current_field == 1) {
                        current_field = 2;  // Chuyển đến trường password
                    } else if (current_field == 2) {
                        // Gọi hàm thêm người dùng vào database
                        add_user_to_db(username, email, password);
                        SDL_Log("User added to database");
                    }
                }

                // Chuyển trường nhập liệu khi nhấn Tab
                if (event.key.keysym.sym == SDLK_TAB) {
                    current_field = (current_field + 1) % 3;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Đảm bảo có màu nền
        SDL_RenderClear(renderer);

        display_text(renderer, font, "Create Account", 50, 50);
        display_text(renderer, font, "Username:", 50, 100);
        display_text(renderer, font, username, 200, 100);

        display_text(renderer, font, "Email:", 50, 150);
        display_text(renderer, font, email, 200, 150);

        display_text(renderer, font, "Password:", 50, 200);
        display_text(renderer, font, password, 200, 200);

        SDL_RenderPresent(renderer);  // Đảm bảo có cập nhật màn hình
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
