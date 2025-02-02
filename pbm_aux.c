#include "pbm.h"
#include <stdlib.h>

PPMImage *new_ppmimage(unsigned int w, unsigned int h, unsigned int m) {
    PPMImage *ppm = (PPMImage *)malloc(sizeof(PPMImage));
    if (!ppm) return NULL;

    ppm->width = w;
    ppm->height = h;
    ppm->max = m;

    for (int c = 0; c < 3; c++) {
        ppm->pixmap[c] = (unsigned int **)malloc(h * sizeof(unsigned int *));
        if (!ppm->pixmap[c]) {
            for (int i = 0; i < c; i++) {
                free(ppm->pixmap[i]);
            }
            free(ppm);
            return NULL;
        }
        for (int i = 0; i < h; i++) {
            ppm->pixmap[c][i] = (unsigned int *)malloc(w * sizeof(unsigned int));
            if (!ppm->pixmap[c][i]) {
                for (int j = 0; j < i; j++) free(ppm->pixmap[c][j]);
                free(ppm->pixmap[c]);
                for (int k = 0; k < c; k++) {
                    for (int j = 0; j < h; j++) free(ppm->pixmap[k][j]);
                    free(ppm->pixmap[k]);
                }
                free(ppm);
                return NULL;
            }
        }
    }
    return ppm;
}

PGMImage *new_pgmimage(unsigned int w, unsigned int h, unsigned int m) {
    PGMImage *pgm = (PGMImage *)malloc(sizeof(PGMImage));
    if (!pgm) return NULL;

    pgm->width = w;
    pgm->height = h;
    pgm->max = m;

    pgm->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *));
    if (!pgm->pixmap) {
        free(pgm);
        return NULL;
    }

    for (int i = 0; i < h; i++) {
        pgm->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int));
        if (!pgm->pixmap[i]) {
            for (int j = 0; j < i; j++) free(pgm->pixmap[j]);
            free(pgm->pixmap);
            free(pgm);
            return NULL;
        }
    }
    return pgm;
}

PBMImage *new_pbmimage(unsigned int w, unsigned int h) {
    PBMImage *pbm = (PBMImage *)malloc(sizeof(PBMImage));
    if (!pbm) return NULL;

    pbm->width = w;
    pbm->height = h;

    pbm->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *));
    if (!pbm->pixmap) {
        free(pbm);
        return NULL;
    }

    for (int i = 0; i < h; i++) {
        pbm->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int));
        if (!pbm->pixmap[i]) {
            for (int j = 0; j < i; j++) free(pbm->pixmap[j]);
            free(pbm->pixmap);
            free(pbm);
            return NULL;
        }
    }
    return pbm;
}

void del_ppmimage(PPMImage *p) {
    if (!p) return;
    for (int c = 0; c < 3; c++) {
        if (p->pixmap[c]) {
            for (int i = 0; i < p->height; i++) free(p->pixmap[c][i]);
            free(p->pixmap[c]);
        }
    }
    free(p);
}

void del_pgmimage(PGMImage *p) {
    if (!p) return;
    for (int i = 0; i < p->height; i++) free(p->pixmap[i]);
    free(p->pixmap);
    free(p);
}

void del_pbmimage(PBMImage *p) {
    if (!p) return;
    for (int i = 0; i < p->height; i++) free(p->pixmap[i]);
    free(p->pixmap);
    free(p);
}