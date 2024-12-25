#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "gui.h"
#include "db.h"

int main() {
    // Khởi tạo SDL
    printf("INFO: Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Khởi tạo SDL_ttf
    printf("INFO: Initializing SDL_ttf...\n");
    if (TTF_Init() < 0) {
        SDL_Log("Failed to initialize SDL_ttf: %s", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Bắt đầu giao diện người dùng
    start_gui();

    // Dọn dẹp tài nguyên
    TTF_Quit();
    SDL_Quit();
    return 0;
}
