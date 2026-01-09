#include "steg_text.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t capacity_bits_rgb(const PNG_IMAGE *img) {
    // Use only RGB (3 channels), 1 bit per channel
    return (size_t)img->width * (size_t)img->height * 3u;
}

static int write_bit_rgb(PNG_IMAGE *img, size_t bit_index, int bit) {
    // bit_index
    size_t pixel_index = bit_index / 3u;
    int c = (int)(bit_index % 3u);

    size_t y = pixel_index / (size_t)img->width;
    size_t x = pixel_index % (size_t)img->width;

    if (y >= (size_t)img->height) return -1;

    png_bytep px = &img->rows[y][x * img->channels];
    px[c] = (png_byte)(((px[c]) & 0xFEu) | (bit & 1));
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

static void write_u32_be_bits(PNG_IMAGE *img, size_t *bit_pos, uint32_t v) {
    // writes 32 bits big-endian (MSB first)
    for (int i = 31; i >= 0; --i) {
        int b = (v >> i) & 1u;
        write_bit_rgb(img, (*bit_pos)++, b);
    }
}

static uint32_t read_u32_be_bits(const PNG_IMAGE *img, size_t *bit_pos, int *ok) {
    uint32_t v= 0;
    for (int i = 0; i < 32; ++i) {
        int b = read_bit_rgb(img, (*bit_pos)++);
        if (b < 0) { *ok = 0; return 0; }
        v = (v << 1) | (uint32_t)(b & 1);
    }
    *ok = 1;
    return v;
}

int steg_hide_text(PNG_IMAGE *img, const unsigned char *msg, size_t msg_len) {
    if (!img || !img->rows || img->channels < 3) return -1;

    size_t cap = capacity_bits_rgb(img);
    size_t needed = 32u + msg_len * 8u; // 32 bits for length + message
    if (needed > cap) {
        fprintf(stderr, "There is no capacity: it only need %zu bits, it has %zu bits.\n", needed, cap);
        return -1;
    }

    size_t bit_pos = 0;
    if (msg_len > UINT32_MAX) return -1;

    write_u32_be_bits(img, &bit_pos, (uint32_t)msg_len);

    for (size_t i = 0; i < msg_len; ++i) {
        unsigned char c = msg[i];
        for (int b = 7; b >=0; --b) {
            int bit = (c >> b) & 1u;
            write_bit_rgb(img, bit_pos++, bit);
        }
    }

    return 0;
}

int steg_extract_text(const PNG_IMAGE *img, unsigned char **out_msg, size_t *out_len) {
    if (!img || !img->rows || img->channels < 3 || !out_msg || !out_len) return -1;

    size_t cap = capacity_bits_rgb(img);
    if (cap < 32u) return -1;

    size_t bit_pos = 0;
    int ok = 0;
    uint32_t msg_len = read_u32_be_bits(img, &bit_pos, &ok);
    if (!ok) return -1;

    size_t needed = 32u + (size_t)msg_len * 8u;
    if (needed > cap) {
        fprintf(stderr, "Invalid length (probably no hidden message): %u bytes.\n", msg_len);
        return -1;
    }

    unsigned char *buf = (unsigned char *)malloc((size_t)msg_len + 1u);
    if (!buf) return -1;

    for (uint32_t i = 0; i < msg_len; ++i) {
        unsigned char c = 0;
        for (int b = 0; b < 8; ++b) {
            int bit = read_bit_rgb(img, bit_pos++);
            if (bit < 0) { free(buf); return -1; }
            c = (unsigned char)((c << 1) | (bit & 1));
        }
        buf[i] = c;
    }
    buf[msg_len] = '\0';

    *out_msg = buf;
    *out_len = (size_t)msg_len;
    return 0;
}