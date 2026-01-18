#ifndef TINYRENDER_H_
#define TINYRENDER_H_

#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
} TinyRenderPixels;

typedef struct {
    uint8_t r, g, b;
} TinyRenderColor;

typedef struct {
    const char *filename;
    int width, height, fps;
} TinyRenderOption;

typedef struct {
    FILE *f;
    TinyRenderOption opt;
    TinyRenderPixels *pixels;
    uint8_t *y_plane;
    uint8_t *u_plane;
    uint8_t *v_plane;
} TinyRenderWriter;

int tinyrender_start(TinyRenderOption opt, TinyRenderWriter *w, TinyRenderPixels *pixels, uint8_t *y, uint8_t *u, uint8_t *v);
int tinyrender_frame(TinyRenderWriter *w);
void tinyrender_end(TinyRenderWriter *w);

void tinyrender_clear_background(TinyRenderWriter *writer, const TinyRenderColor color);

#endif // TINYRENDER_H_