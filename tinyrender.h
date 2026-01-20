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

typedef enum {
    TINYRENDER_OK = 0,

    TINYRENDER_ERR_NULL_CTX,
    TINYRENDER_ERR_NULL_PIXELS,
    TINYRENDER_ERR_NULL_PLANES,
    TINYRENDER_ERR_INVALID_OPTIONS,
    TINYRENDER_ERR_INVALID_FPS,
    TINYRENDER_ERR_NULL_FILENAME,

    TINYRENDER_ERR_FILE_OPEN,
    TINYRENDER_ERR_FILE_WRITE,
    TINYRENDER_ERR_SHORT_WRITE,

    TINYRENDER_ERR_INTERNAL
} TinyRenderResult;

const char *tinyrender_strerror(TinyRenderResult r);

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
    uint8_t *z_buffer;
} TinyRenderCtx;

TinyRenderResult tinyrender_init_ctx(TinyRenderCtx *ctx, TinyRenderPixels *pixels, uint8_t *y, uint8_t *u, uint8_t *v, uint8_t *z);

TinyRenderResult tinyrender_start(TinyRenderOption opt, TinyRenderCtx *ctx);
TinyRenderResult tinyrender_frame(TinyRenderCtx *ctx);
void tinyrender_end(TinyRenderCtx *ctx);

void tinyrender_clear_background(TinyRenderCtx *ctx, const TinyRenderColor color);

#endif // TINYRENDER_H_