#ifndef TINYRENDER_H_
#define TINYRENDER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

typedef enum {
    TINYRENDER_LOG_DEBUG = 0,
    TINYRENDER_LOG_INFO,
    TINYRENDER_LOG_WARNING,
    TINYRENDER_LOG_ERROR,
    TINYRENDER_LOG_NONE
} TINYRENDER_LOG_LEVEL;

typedef void (*tinyrender_log_handler_fn)(
    TINYRENDER_LOG_LEVEL level,
    const char *fmt,
    va_list args
);

void tinyrender_set_log_handler(tinyrender_log_handler_fn handler);
void tinyrender_log(TINYRENDER_LOG_LEVEL level, const char *fmt, ...);

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