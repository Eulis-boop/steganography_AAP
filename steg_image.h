#ifndef STEG_IMAGE_H
#define STEG_IMAGE_H

#include "png_utils.h"

// Hides a secret image inside 'cover' (modifies cover). Returns 0 if it succeed, -1 if no capacity or error.
int steg_hide_image(PNG_IMAGE *cover, const PNG_IMAGE *secret);

// Extracts a secret image form 'stego' and returns it in 'out_secret'. Returns 0 if it succeed, -1 if unsuccessful. (*out_secret must be freed with free_png())
int steg_extract_image(const PNG_IMAGE *stego, PNG_IMAGE *out_secret);

#endif