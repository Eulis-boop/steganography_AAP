#include "png_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int load_png(const char *file_name, PNG_IMAGE *img) {
    FILE *f = fopen(file_name, "rb");
    if (!f) {
        perror("Error: can not open the PNG file.\n");
        return -1;
    }

    // Read the PNG's signature (8 octets)
    png_byte header[8];
    if (fread(header, 1, 8, f) != 8) {
        fprintf(stderr, "Error: can not read the PNG's signature.\n");
        fclose(f);
        return -1;
    }

    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "%s is not a valid PNG file.\n", file_name);
        fclose(f);
        return -1;
    }

    // Creation of libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_read_struct.\n");
        fclose(f);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: png_create_info_struct.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return -1;
    }

    // Error handling via setjmp (standard libpng mechanism)
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error reading PNG (setjmp).\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return -1;
    }

    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8); // we have already read 8 bytes

    // Reading the header info
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

    // Normalization: everything is converted to 8 bits per channel, and to RGB or RGBA
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr); // converts 16 bits into 8 bits
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    // Adds an alpha channel if not already present (useful for manipulating pixels)
    int channels;
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); // adds alpha = 255
        channels = 4; // RGBA
    } else if (color_type == PNG_COLOR_TYPE_RGBA) {
        channels = 4;
    } else {
        // Unusual case
        fprintf(stderr, "PNG color type not generated: %d\n", color_type);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return -1;
    }

    // Applies transformations
    png_read_update_info(png_ptr, info_ptr);

    // Row table allocation
    png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));
    if (!row_pointers) {
        fprintf(stderr, "Error malloc row_pointers.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return -1;
    }

    for (png_uint_32 y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep)malloc(row_bytes);
        if (!row_pointers[y]) {
            fprintf(stderr, "Error malloc row_pointers[%u].\n", y);
            // Free up already allocated lines
            for (png_uint_32 k = 0; k < y; k++) {
                free(row_pointers[k]);
            }
            free(row_pointers);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(f);
            return -1;
        }
    }

    // Image reading
    png_read_image(png_ptr, row_pointers);

    // Cleaning structures libpng
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);

    // Filling the PNG_IMAGE structure
    img->width = (int)width;
    img->height = (int)height;
    img->channels = channels;
    img->rows = row_pointers;

    return 0;
}

int save_png(const char *file_name, const PNG_IMAGE *img) {
    FILE *f = fopen(file_name, "wb");
    if(!f) {
        perror("Error");
        return -1;
    }

    // Creating libpng structures
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error png_create_write_struct.\n");
        fclose(f);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error png_create_info_struct (write).\n");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(f);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(f);
        return -1;
    }

    png_init_io(png_ptr, f);

    int color_type = PNG_COLOR_TYPE_RGB;
    if (img->channels == 4) {
        color_type = PNG_COLOR_TYPE_RGBA;
    }

    png_set_IHDR(png_ptr, info_ptr, img->width, img->height, 
        8, // 8 bits per channel
        color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    png_write_info(png_ptr, info_ptr);

    // Writing lines
    png_write_image(png_ptr, img->rows);

    // End of file
    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(f);
    return 0;
}

void free_png(PNG_IMAGE *img) {
    if (!img || !img->rows) return;
    for (int y = 0; y < img->height; y++) {
        free(img->rows[y]);
    }
    free(img->rows);
    img->rows = NULL;
    img->width = img->height = img->channels = 0;
}

void print_png_hex(const PNG_IMAGE *img, int max_rows) {
    if (!img || !img->rows) {
        fprintf(stderr, "Empty or uninitialized image.\n");
        return;
    }

    int rows_to_print = img->height;
    if (max_rows > 0 && max_rows < img->height) {
        rows_to_print = max_rows;
    }

    printf("Image %dx%d, %d channels.\n", img->width, img->height, img->channels);

    for (int y = 0; y < rows_to_print; y++) {
        png_bytep row = img->rows[y];
        int row_bytes = img->width * img->channels;
        printf("Line %d: ", y);
        for (int x = 0; x < row_bytes; x++) {
            printf("%02X ", row[x]);
        }
        printf("\n");
    }

    if (rows_to_print < img->height) {
        printf("... (%d lines ).\n", img->height - rows_to_print);
    }
}