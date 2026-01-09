#include "steg_image.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t capacity_bits_rgb(const PNG_IMAGE *img) {
    return (size_t)img->width * (size_t)img->height * 3u;
}

static int write_bit_rgb(PNG_IMAGE *img, size_t bit_index, int bit) {
    size_t pixel_index = bit_index / 3u;
    int c = (int)(bit_index % 3u);

    size_t y = pixel_index / (size_t)img->width;
    size_t x = pixel_index % (size_t)img->width;

    if (y >= (size_t)img->height) return -1;

    png_bytep px = &img->rows[y][x * img->channels];
    px[c] = (png_byte)((px[c] & 0xFEu) | (bit & 1));
    return 0;
}

static int read_bit_rgb(const PNG_IMAGE *img, size_t bit_index) {
    size_t pixel_index = bit_index / 3u;
    int c = (int)(bit_index % 3u);

    size_t y = pixel_index / (size_t)img->width;
    size_t x = pixel_index % (size_t)img->width;

    if (y >= (size_t)img->height) return -1;

    png_bytep px = &img->rows[y][x * img->channels];
    return (px[c] & 1u);
}

static void write_u32_be(PNG_IMAGE *img, size_t *bit_pos, uint32_t v) {
    for (int i = 31; i >= 0; --i) {
        write_bit_rgb(img, (*bit_pos)++, (v >> i) & 1u);
    }
}

static uint32_t read_u32_be(const PNG_IMAGE *img, size_t *bit_pos, int *ok) {
    uint32_t v = 0;
    for (int i = 0; i < 32; ++i) {
        int b = read_bit_rgb(img, (*bit_pos)++);
        if (b < 0) { *ok = 0; return 0; }
        v = (v << 1) | (uint32_t)(b & 1);
    }
    *ok = 1;
    return v;
}

static int write_bits(PNG_IMAGE *img, size_t *bit_pos, uint32_t value, int n_bits) {
    for (int i = n_bits - 1; i >= 0; --i) {
        int b = (value >> i) & 1u;
        if (write_bit_rgb(img, (*bit_pos)++, b) != 0) return -1;
    }
    return 0;
}

static int read_bits(const PNG_IMAGE *img, size_t *bit_pos, uint32_t *value, int n_bits) {
    uint32_t v = 0;
    for (int i = 0; i < n_bits; ++i) {
        int b = read_bit_rgb(img, (*bit_pos)++);
        if (b < 0) return -1;
        v = (v << 1) | (uint32_t)(b & 1);
    }
    *value = v;
    return 0;
}

int steg_hide_image(PNG_IMAGE *cover, const PNG_IMAGE *secret) {
    if (!cover || !cover->rows || cover->channels < 3) return -1;
    if (!secret || !secret->rows || secret->channels < 3) return -1;

    // Encodes 'secret' in 4 bits per RGBA channel (16 bits per secret pixel)
    // Header: secret_w (32) + secret_h (32)
    size_t header_bits = 64u;
    size_t secret_pixels = (size_t)secret->width * (size_t)secret->height;
    size_t payload_bits = secret_pixels * 16u;
    size_t needed = header_bits + payload_bits;
    size_t cap = capacity_bits_rgb(cover);

    fprintf(stderr, "[hide-image] cover=%dx%d ch=%d cap=%zu bits\n",
        cover->width, cover->height, cover->channels, cap);
    fprintf(stderr, "[hide-image] secret=%dx%d ch=%d needed=%zu bits\n",
        secret->width, secret->height, secret->channels, needed);


    if (needed > cap) {
        fprintf(stderr, "There is no ability to hide the image: It needs %zu bits, it has %zu bits.\n", needed, cap);
        return -1;
    }
    
    size_t bit_pos = 0;
    write_u32_be(cover, &bit_pos, (uint32_t)secret->width);
    write_u32_be(cover, &bit_pos, (uint32_t)secret->height);

    for (int y = 0; y < secret->height; ++y) {
        png_bytep row = secret->rows[y];
        for (int x = 0; x < secret->width; ++x) {
            png_bytep px = &row[x * secret->channels];

            // Takes the 4 MSBs from ach channel
            uint32_t r4 = (uint32_t)(px[0] >> 4);
            uint32_t g4 = (uint32_t)(px[1] >> 4);
            uint32_t b4 = (uint32_t)(px[2] >> 4);
            uint32_t a4 = (secret->channels >= 4) ? (uint32_t)(px[3] >> 4) : 0xFu;

            if (write_bits(cover, &bit_pos, r4, 4) != 0) return -1;
            if (write_bits(cover, &bit_pos, g4, 4) != 0) return -1;
            if (write_bits(cover, &bit_pos, b4, 4) != 0) return -1;
            if (write_bits(cover, &bit_pos, a4, 4) != 0) return -1;
        }
    }
    return 0;
}

int steg_extract_image(const PNG_IMAGE *stego, PNG_IMAGE *out_secret) {
    if (!stego || !stego->rows || stego->channels < 3) return -1;
    if (!out_secret) return -1;

    memset(out_secret, 0, sizeof(*out_secret));

    size_t cap = capacity_bits_rgb(stego);
    if (cap < 64u) return -1;

    size_t bit_pos = 0;
    int ok = 0;
    uint32_t w = read_u32_be(stego, &bit_pos, &ok);
    if (!ok) return -1;
    uint32_t h = read_u32_be(stego, &bit_pos, &ok);
    if (!ok) return -1;

    fprintf(stderr, "[extract-image] header w=%u h=%u\n", w, h);

    if (w == 0 || h == 0) return -1;

    size_t needed = 64u + (size_t)w * (size_t)h * 16u;
    if (needed > cap) {
        fprintf(stderr, "Invalid header or secret image not included.\n");
        return -1;
    }

    // Creates an RGBA 8-bit image
    out_secret->width = (int)w;
    out_secret->height = (int)h;
    out_secret->channels = 4;

    out_secret->rows = (png_bytep *)malloc(h * sizeof(png_bytep));
    if (!out_secret->rows) return -1;

    size_t row_bytes = (size_t)w * 4u;
    for (uint32_t y = 0; y < h; ++y) {
        out_secret->rows[y] = (png_bytep)malloc(row_bytes);
        if (!out_secret->rows[y]) {
            for (uint32_t k = 0; k < y; ++k) free(out_secret->rows[k]);
            free(out_secret->rows);
            memset(out_secret, 0, sizeof(*out_secret));
            return -1;
        }
    }

    for (uint32_t y = 0; y < h; ++y) {
        png_bytep row = out_secret->rows[y];
        for (uint32_t x = 0; x < w; ++x) {
            uint32_t r4, g4, b4, a4;
            if (read_bits(stego, &bit_pos, &r4, 4) != 0) return -1;
            if (read_bits(stego, &bit_pos, &g4, 4) != 0) return -1;
            if (read_bits(stego, &bit_pos, &b4, 4) != 0) return -1;
            if (read_bits(stego, &bit_pos, &a4, 4) != 0) return -1;

            png_bytep px = &row[x * 4u];
            // We re-expand: nibble -> byte (xxxx0000)
            px[0] = (png_byte)(r4 << 4);
            px[1] = (png_byte)(g4 << 4);
            px[2] = (png_byte)(b4 << 4);
            px[3] = (png_byte)(a4 << 4);
        }
    }
    return 0;
}