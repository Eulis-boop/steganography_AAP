#ifndef STEG_TEXT_H
#define STEG_TEXT_H

#include "png_utils.h"
#include <stddef.h>

// Hides a message (bytes) inside of an image (modify img). Returns 0 if it succeed, -1 if there are an error.
int steg_hide_text(PNG_IMAGE *img, const unsigned char *msg, size_t msg_len);

// Extracts a hidden message from img. Assigns *out_msg with malloc and *out_len. Returns 0 if it succeed, -1 if there is no valid message or an error.
int steg_extract_text(const PNG_IMAGE *img, unsigned char **out_msg, size_t *out_len);

#endif