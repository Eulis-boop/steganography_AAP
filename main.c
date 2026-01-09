// Projet Avancé - Algorithmique Avancée et Programmation
// Auteur : Eunice Saraí CASTILLO TURRUBIARTES
// Date : Janvier 2026

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png_utils.h"
#include "steg_text.h"
#include "steg_image.h"
#include "display_sdl.h"

static void usage(const char *prog) {
    fprintf(stderr,
    "Usage:\n"
    " %s open-and-save <input.png> <output.png>\n"
    " %s show-hex <input.png> [max_rows]\n"
    " %s hide-text <input.png> <output.png> <message>\n"
    " %s extract-text <stego.png>\n"
    " %s hide-image <cover.png> <secret.png> <output.png>\n"
    " %s extract-image <stego.png> <output_secret.png>\n"
    " %s show-image <display.png>\n",
    prog, prog, prog, prog, prog, prog, prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]); 
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "open-and-save") == 0) {
        if (argc != 4) {
            usage(argv[0]);
            return 1;
        }
        PNG_IMAGE img = {0};
        if (load_png(argv[2], &img) != 0) return 1;
        if (save_png(argv[3], &img) != 0) {
            free_png(&img);
            return 1;
        }
        free_png(&img);
        printf("Image saved in %s.\n", argv[3]);
        return 0;
    }

    if (strcmp(cmd, "show-hex") == 0) {
        if (argc != 3 && argc != 4) {
            usage(argv[0]);
            return 1;
        }
        int max_rows = 10;
        if (argc == 4) max_rows = atoi(argv[3]);

        PNG_IMAGE img = {0};
        if (load_png(argv[2], &img) != 0) return 1;

        print_png_hex(&img, max_rows);

        free_png(&img);
        return 0;
    }

    if (strcmp(cmd, "hide-text") == 0) {
        if (argc != 5) { usage(argv[0]); return 1; }
        PNG_IMAGE img = {0};
        if (load_png(argv[2], &img) != 0) return 1;

        const unsigned char *msg = (const unsigned char*)argv[4];
        size_t len = strlen(argv[4]);

        if (steg_hide_text(&img, msg, len) != 0) { free_png(&img); return 1; }
        if (save_png(argv[3], &img) != 0) { free_png(&img); return 1; }

        free_png(&img);
        printf("Hidden text in %s\n", argv[3]);
        return 0;
    }

    if (strcmp(cmd, "extract-text") == 0) {
        if (argc != 3) { usage(argv[0]); return 1; }
        PNG_IMAGE img = {0};
        if (load_png(argv[2], &img) != 0) return 1;

        unsigned char *msg = NULL;
        size_t len = 0;
        if (steg_extract_text(&img, &msg, &len) != 0) {
            free_png(&img);
            fprintf(stderr, "Text could not be extracted.\n");
            return 1;
        }

        printf("Message (%zu bytes):\n%s\n", len, msg);

        free(msg);
        free_png(&img);
        return 0;
    }

    if (strcmp(cmd, "hide-image") == 0) {
        if (argc != 5) { usage(argv[0]); return 1; }
        PNG_IMAGE cover = {0}, secret = {0};

        if (load_png(argv[2], &cover) != 0) return 1;
        if (load_png(argv[3], &secret) != 0) { free_png(&cover); return 1; }

        if (steg_hide_image(&cover, &secret) != 0) {
            free_png(&cover); free_png(&secret); return 1;
        }
        if (save_png(argv[4], &cover) != 0) {
            free_png(&cover); free_png(&secret); return 1;
        }

        free_png(&cover);
        free_png(&secret);
        printf("Hidden image in %s\n", argv[4]);
        return 0;
    }

    if (strcmp(cmd, "extract-image") == 0) {
        if (argc != 4) { usage(argv[0]); return 1; }
        PNG_IMAGE stego = {0};
        if (load_png(argv[2], &stego) != 0) return 1;

        PNG_IMAGE secret = {0};
        if (steg_extract_image(&stego, &secret) != 0) {
            free_png(&stego);
            fprintf(stderr, "Image could not be extracted.\n");
            return 1;
        }

        if (save_png(argv[3], &secret) != 0) {
            free_png(&stego);
            free_png(&secret);
            return 1;
        }

        free_png(&stego);
        free_png(&secret);
        printf("Image extracted %s\n", argv[3]);
        return 0;
    }

    if (strcmp(cmd, "show-image") == 0) {
        if (argc != 3) { usage(argv[0]); return 1; }

        PNG_IMAGE img = {0};
        if (load_png(argv[2], &img) != 0) return -1;

        int display = display_png_sdl(&img, argv[2]);

        free_png(&img);
        return (display == 0) ? 0 : 1;
    }

    usage(argv[0]);
    return 1;
}