#ifndef PNG_UTILS_H
#define PNG_UTILS_H

#include <png.h>

typedef struct {
    int width;
    int height;
    int channels; //3 = RGB, 4 = RGBA
    png_bytep *rows; //
} PNG_IMAGE;

// Load a PNG file in memory on PNG_IMAGE structure. Returns 0 if succeed, -1 if there are an error.
int load_png(const char *file_name, PNG_IMAGE *img);
// Save a PNG image (already in memory on img) within a file. Returns 0 if succeed, -1 if there are an error.
int save_png(const char *file_name, const PNG_IMAGE *img);
// Free the memory related to a PNG_IMAGE;
void free_png(PNG_IMAGE *img);
// Show the image's content brute in hexadecimal. max_rows = # of lines maximum to show (to limit the output). If max_rows <= 0, shows all the lines.
void print_png_hex(const PNG_IMAGE *img, int max_rows);

#endif