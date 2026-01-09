#include "display_sdl.h"
#include <SDL.h>
#include <stdio.h>

int display_png_sdl(const PNG_IMAGE *img, const char *title) {
    if (!img || !img->rows || img->channels < 3) {
        fprintf(stderr, "display_png_sdl: invalid image.\n");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s.\n", SDL_GetError());
        return -1;
    }

    const int w = img->width;
    const int h = img->height;

    SDL_Window *window = SDL_CreateWindow(
        title ? title : "PNG Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s.\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!render) {
        fprintf(stderr, "SDL_CreateRenderer error: %s.\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Since the loader normalizes tp RGBA (channels = 4), it works. If channels = 3, it creates an RGB24 format.
    Uint32 sdl_format = (img->channels == 4) ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24;

    SDL_Texture *texture = SDL_CreateTexture(
        render,
        sdl_format,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );

    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // It copies the rows to the texture. Pitch = bytes per row.
    const int pitch = w * img->channels;

    // SDL_UpdateTexture expects a contiguous buffer, since it has row pointers, it is better to update line by line with a rect.
    for (int y = 0; y < h; ++y) {
        SDL_Rect r = {0, y, w, 1};
        if (SDL_UpdateTexture(texture, &r, img->rows[y], pitch) != 0) {
            fprintf(stderr, "SDL_UpdateTexture error: %s.\n", SDL_GetError());
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(render);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
    }

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = 0;
            }
        }

        SDL_RenderClear(render);
        SDL_RenderCopy(render, texture, NULL, NULL);
        SDL_RenderPresent(render);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}