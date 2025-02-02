#include "pbm.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    TRANSFORM_NONE,
    TRANSFORM_BITMAP,
    TRANSFORM_GRAYSCALE,
    TRANSFORM_ISOLATE,
    TRANSFORM_REMOVE,
    TRANSFORM_SEPIA,
    TRANSFORM_MIRROR,
    TRANSFORM_THUMBNAIL,
    TRANSFORM_TILE
} TransformType;

void usage_error() {
    fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
    exit(1);
}

void error_exit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

PBMImage *convert_to_pbm(PPMImage *ppm) {
    PBMImage *pbm = new_pbmimage(ppm->width, ppm->height);
    if (!pbm) error_exit("Memory allocation failed.\n");
    for (int h = 0; h < ppm->height; h++) {
        for (int w = 0; w < ppm->width; w++) {
            unsigned int r = ppm->pixmap[0][h][w];
            unsigned int g = ppm->pixmap[1][h][w];
            unsigned int b = ppm->pixmap[2][h][w];
            unsigned int avg = (r + g + b) / 3;
            pbm->pixmap[h][w] = (avg < (ppm->max / 2)) ? 0 : 1;
        }
    }
    return pbm;
}

PGMImage *convert_to_pgm(PPMImage *ppm, unsigned int max_gray) {
    PGMImage *pgm = new_pgmimage(ppm->width, ppm->height, max_gray);
    if (!pgm) error_exit("Memory allocation failed.\n");
    for (int h = 0; h < ppm->height; h++) {
        for (int w = 0; w < ppm->width; w++) {
            unsigned int r = ppm->pixmap[0][h][w];
            unsigned int g = ppm->pixmap[1][h][w];
            unsigned int b = ppm->pixmap[2][h][w];
            float avg = (r + g + b) / 3.0f;
            unsigned int gray = (avg / ppm->max) * max_gray;
            pgm->pixmap[h][w] = (gray > max_gray) ? max_gray : gray;
        }
    }
    return pgm;
}

PPMImage *isolate_channel(PPMImage *ppm, const char *channel) {
    PPMImage *new_ppm = new_ppmimage(ppm->width, ppm->height, ppm->max);
    if (!new_ppm) error_exit("Memory allocation failed.\n");
    int c = -1;
    if (strcmp(channel, "red") == 0) c = 0;
    else if (strcmp(channel, "green") == 0) c = 1;
    else if (strcmp(channel, "blue") == 0) c = 2;
    else error_exit("Error: Invalid channel specification.\n");

    for (int h = 0; h < ppm->height; h++) {
        for (int w = 0; w < ppm->width; w++) {
            for (int i = 0; i < 3; i++) {
                new_ppm->pixmap[i][h][w] = (i == c) ? ppm->pixmap[i][h][w] : 0;
            }
        }
    }
    return new_ppm;
}

PPMImage *apply_sepia(PPMImage *ppm) {
    PPMImage *sepia = new_ppmimage(ppm->width, ppm->height, ppm->max);
    if (!sepia) error_exit("Memory allocation failed.\n");
    for (int h = 0; h < ppm->height; h++) {
        for (int w = 0; w < ppm->width; w++) {
            float r = ppm->pixmap[0][h][w];
            float g = ppm->pixmap[1][h][w];
            float b = ppm->pixmap[2][h][w];
            float newR = 0.393f * r + 0.769f * g + 0.189f * b;
            float newG = 0.349f * r + 0.686f * g + 0.168f * b;
            float newB = 0.272f * r + 0.534f * g + 0.131f * b;
            newR = (newR > ppm->max) ? ppm->max : (newR < 0) ? 0 : newR;
            newG = (newG > ppm->max) ? ppm->max : (newG < 0) ? 0 : newG;
            newB = (newB > ppm->max) ? ppm->max : (newB < 0) ? 0 : newB;
            sepia->pixmap[0][h][w] = (unsigned int)(newR + 0.5f);
            sepia->pixmap[1][h][w] = (unsigned int)(newG + 0.5f);
            sepia->pixmap[2][h][w] = (unsigned int)(newB + 0.5f);
        }
    }
    return sepia;
}

PPMImage *mirror_image(PPMImage *ppm) {
    PPMImage *mirrored = new_ppmimage(ppm->width, ppm->height, ppm->max);
    if (!mirrored) error_exit("Memory allocation failed.\n");
    for (int h = 0; h < ppm->height; h++) {
        for (int w = 0; w < ppm->width / 2; w++) {
            int mirror_w = ppm->width - 1 - w;
            for (int c = 0; c < 3; c++) {
                mirrored->pixmap[c][h][w] = ppm->pixmap[c][h][w];
                mirrored->pixmap[c][h][mirror_w] = ppm->pixmap[c][h][w];
            }
        }
        if (ppm->width % 2) {
            int mid = ppm->width / 2;
            for (int c = 0; c < 3; c++) {
                mirrored->pixmap[c][h][mid] = ppm->pixmap[c][h][mid];
            }
        }
    }
    return mirrored;
}

PPMImage *thumbnail(PPMImage *ppm, int scale) {
    int new_w = ppm->width / scale;
    int new_h = ppm->height / scale;
    PPMImage *thumb = new_ppmimage(new_w, new_h, ppm->max);
    if (!thumb) error_exit("Memory allocation failed.\n");
    for (int h = 0; h < new_h; h++) {
        for (int w = 0; w < new_w; w++) {
            int src_h = h * scale;
            int src_w = w * scale;
            for (int c = 0; c < 3; c++) {
                thumb->pixmap[c][h][w] = ppm->pixmap[c][src_h][src_w];
            }
        }
    }
    return thumb;
}

PPMImage *tile_thumbnails(PPMImage *ppm, int scale) {
    int thumb_w = ppm->width / scale;
    int thumb_h = ppm->height / scale;
    PPMImage *tiled = new_ppmimage(ppm->width, ppm->height, ppm->max);
    if (!tiled) error_exit("Memory allocation failed.\n");
    for (int ty = 0; ty < scale; ty++) {
        for (int tx = 0; tx < scale; tx++) {
            for (int h = 0; h < thumb_h; h++) {
                for (int w = 0; w < thumb_w; w++) {
                    int src_h = h * scale;
                    int src_w = w * scale;
                    int dest_h = ty * thumb_h + h;
                    int dest_w = tx * thumb_w + w;
                    if (dest_h >= ppm->height || dest_w >= ppm->width) continue;
                    for (int c = 0; c < 3; c++) {
                        tiled->pixmap[c][dest_h][dest_w] = ppm->pixmap[c][src_h][src_w];
                    }
                }
            }
        }
    }
    return tiled;
}

int main(int argc, char *argv[]) {
    int opt;
    TransformType transform = TRANSFORM_NONE;
    char *input_file = NULL;
    char *output_file = NULL;
    char *channel = NULL;
    unsigned int max_gray = 0;
    int scale = 0;

    while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1) {
        switch (opt) {
            case 'b':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_BITMAP;
                break;
            case 'g':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_GRAYSCALE;
                max_gray = strtoul(optarg, NULL, 10);
                if (max_gray < 1 || max_gray >= 65536) error_exit("Error: Invalid max grayscale pixel value: %u\n", max_gray);
                break;
            case 'i':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_ISOLATE;
                channel = optarg;
                break;
            case 'r':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_REMOVE;
                channel = optarg;
                break;
            case 's':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_SEPIA;
                break;
            case 'm':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_MIRROR;
                break;
            case 't':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_THUMBNAIL;
                scale = atoi(optarg);
                if (scale < 1 || scale > 8) error_exit("Error: Invalid scale factor: %d\n", scale);
                break;
            case 'n':
                if (transform != TRANSFORM_NONE) error_exit("Error: Multiple transformations specified\n");
                transform = TRANSFORM_TILE;
                scale = atoi(optarg);
                if (scale < 1 || scale > 8) error_exit("Error: Invalid scale factor: %d\n", scale);
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                usage_error();
        }
    }

    if (optind >= argc) error_exit("Error: No input file specified\n");
    input_file = argv[optind];
    if (!output_file) error_exit("Error: No output file specified\n");

    PPMImage *input = read_ppmfile(input_file);
    if (!input) error_exit("Error reading input file.\n");

    void *output = NULL;
    switch (transform) {
        case TRANSFORM_BITMAP: {
            PBMImage *pbm = convert_to_pbm(input);
            write_pbmfile(pbm, output_file);
            del_pbmimage(pbm);
            break;
        }
        case TRANSFORM_GRAYSCALE: {
            PGMImage *pgm = convert_to_pgm(input, max_gray);
            write_pgmfile(pgm, output_file);
            del_pgmimage(pgm);
            break;
        }
        case TRANSFORM_ISOLATE: {
            if (!channel || (strcmp(channel, "red") != 0 && strcmp(channel, "green") != 0 && strcmp(channel, "blue") != 0))
                error_exit("Error: Invalid channel specification: %s\n", channel);
            PPMImage *isolated = isolate_channel(input, channel);
            write_ppmfile(isolated, output_file);
            del_ppmimage(isolated);
            break;
        }
        case TRANSFORM_SEPIA: {
            PPMImage *sepia = apply_sepia(input);
            write_ppmfile(sepia, output_file);
            del_ppmimage(sepia);
            break;
        }
        case TRANSFORM_MIRROR: {
            PPMImage *mirrored = mirror_image(input);
            write_ppmfile(mirrored, output_file);
            del_ppmimage(mirrored);
            break;
        }
        case TRANSFORM_THUMBNAIL: {
            PPMImage *thumb = thumbnail(input, scale);
            write_ppmfile(thumb, output_file);
            del_ppmimage(thumb);
            break;
        }
        case TRANSFORM_TILE: {
            PPMImage *tiled = tile_thumbnails(input, scale);
            write_ppmfile(tiled, output_file);
            del_ppmimage(tiled);
            break;
        }
        default:
            error_exit("Error: No transformation specified.\n");
    }

    del_ppmimage(input);
    return 0;
}