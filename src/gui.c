#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "gui.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void start_gui()
{
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return;
    }

    // Khởi tạo SDL_ttf
    if (TTF_Init() < 0)
    {
        SDL_Log("Failed to initialize SDL_ttf: %s", TTF_GetError());
        SDL_Quit();
        return;
    }

    // Tạo cửa sổ
    SDL_Window *window = SDL_CreateWindow("Chat Application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Tạo renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Tải font
    TTF_Font *font = TTF_OpenFont("assets/fonts/OpenSans.ttf", 24);
    if (!font)
    {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Màu sắc
    SDL_Color white = {255, 255, 255, 255};

    // Chạy vòng lặp sự kiện chính
    bool running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        // Xóa màn hình
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Hiển thị tiêu đề "Chat Application"
        SDL_Surface *surface = TTF_RenderText_Blended(font, "Chat Application", white);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dest = {50, 50, surface->w, surface->h};
        SDL_FreeSurface(surface);
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);

        // Vẽ giao diện
        SDL_RenderPresent(renderer);
    }

    // Dọn dẹp tài nguyên
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
