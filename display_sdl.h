#ifndef DISPLAY_SDL_H
#define DISPLAY_SDL_H

#include "png_utils.h"

// Displays an image on a window/screen. Closes with ESC. Return 0 if it succeed, -1 if error.
int display_png_sdl(const PNG_IMAGE *img, const char *title);

#endif