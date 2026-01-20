#include "tinyrender.h"

static void tinyrender_default_log_handler(
    TINYRENDER_LOG_LEVEL level,
    const char *fmt,
    va_list args
) {
    if (level < TINYRENDER_LOG_INFO || level >= TINYRENDER_LOG_NONE) return;

    FILE *out = (level == TINYRENDER_LOG_INFO) ? stdout : stderr;

    switch (level) {
        case TINYRENDER_LOG_DEBUG:   fprintf(out, "[DEBUG] ");    break;
        case TINYRENDER_LOG_INFO:    fprintf(out, "[INFO] ");    break;
        case TINYRENDER_LOG_WARNING: fprintf(out, "[WARNING] "); break;
        case TINYRENDER_LOG_ERROR:   fprintf(out, "[ERROR] ");   break;
        default: return;
    }

    vfprintf(out, fmt, args);
    fflush(out);
}

static tinyrender_log_handler_fn g_log_handler = tinyrender_default_log_handler;

void tinyrender_set_log_handler(tinyrender_log_handler_fn handler) {
    g_log_handler = handler;
}

void tinyrender_log(TINYRENDER_LOG_LEVEL level, const char *fmt, ...) {
    if (!g_log_handler) return;

    va_list args;
    va_start(args, fmt);
    g_log_handler(level, fmt, args);
    va_end(args);
}

static inline uint8_t clamp_u8(float v) {
    if (v < 0.0f)   return 0;
    if (v > 255.0f) return 255;
    return (uint8_t)v;
}

int tinyrender_init_ctx(TinyRenderCtx *ctx, TinyRenderPixels *pixels, uint8_t *y, uint8_t *u, uint8_t *v) {
    if (!ctx) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Writer pointer is NULL\n");
        return 1;
    }
    if (!pixels) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Pixels pointer is NULL\n");
        return 1;
    }
    if (!y || !u || !v) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "YUV plane pointer(s) is NULL\n");
        return 1;
    }

    ctx->y_plane = y;
    ctx->u_plane = u;
    ctx->v_plane = v;
    ctx->pixels = pixels;

    return 0;
}

int tinyrender_start(TinyRenderOption opt, TinyRenderCtx *ctx) {
    if (!ctx) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Writer pointer is NULL\n");
        return 1;
    }
    if (opt.width <= 0 || opt.height <= 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Invalid options (w=%d h=%d)\n", opt.width, opt.height);
        return 1;
    }
    if (opt.fps <= 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "FPS cannot be 0 or lower\n");
        return 1;
    }
    if (opt.filename == NULL) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Filename pointer is NULL\n");
        return 1;
    }

    ctx->f = fopen(opt.filename, "wb");
    if (!ctx->f) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Failed to open '%s'\n", opt.filename);
        return 1;
    }

    ctx->opt = opt;

    int wrote = fprintf(ctx->f, "YUV4MPEG2 W%d H%d F%d:1 Ip A1:1 C444\n", ctx->opt.width, ctx->opt.height, ctx->opt.fps);
    if (wrote < 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Failed to write file header to '%s'\n", opt.filename);
        fclose(ctx->f);
        ctx->f = NULL;
        return 1;
    }

    size_t planeN = (size_t)ctx->opt.width * (size_t)ctx->opt.height;
    tinyrender_log(TINYRENDER_LOG_INFO, "Opened '%s' (%dx%d @ %dfps, plane bytes=%llu)\n", opt.filename, opt.width, opt.height, opt.fps, planeN);
    return 0;
}

int tinyrender_frame(TinyRenderCtx *ctx) {
    if (!ctx || !ctx->f) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "writer/file is NULL\n");
        return 1;
    }
    if (!ctx->pixels) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "pixels buffer is NULL\n");
        return 1;
    }
    if (fprintf(ctx->f, "FRAME\n") < 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "failed to write frame header\n");
        return 1;
    }

    const size_t N = (size_t)ctx->opt.width * (size_t)ctx->opt.height;
    for (size_t i = 0; i < N; ++i) {
        uint8_t r = ctx->pixels[i].r;
        uint8_t g = ctx->pixels[i].g;
        uint8_t b = ctx->pixels[i].b;

        float Yf =  0.299f * r + 0.587f * g + 0.114f * b;
        float Uf = -0.169f * r - 0.331f * g + 0.500f * b + 128.0f;
        float Vf =  0.500f * r - 0.419f * g - 0.081f * b + 128.0f;

        ctx->y_plane[i] = clamp_u8(Yf);
        ctx->u_plane[i] = clamp_u8(Uf);
        ctx->v_plane[i] = clamp_u8(Vf);
    }

    size_t wroteY = fwrite(ctx->y_plane, 1, N, ctx->f);
    size_t wroteU = fwrite(ctx->u_plane, 1, N, ctx->f);
    size_t wroteV = fwrite(ctx->v_plane, 1, N, ctx->f);

    if (wroteY != N || wroteU != N || wroteV != N) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Short write (Y=%llu/%llu, U=%llu/%llu, V=%llu/%llu)\n",
            (unsigned long long)wroteY, (unsigned long long)N,
            (unsigned long long)wroteU, (unsigned long long)N,
            (unsigned long long)wroteV, (unsigned long long)N);
        return 1;
    }
    tinyrender_log(TINYRENDER_LOG_DEBUG, "Wrote %llu bytes (YUV444)\n", (unsigned long long)3 * N);
    return 0;
}

void tinyrender_end(TinyRenderCtx *ctx) {
    if (!ctx) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "File already closed or was never opened\n");
        return;
    }
    if (fclose(ctx->f) != 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "fclose failed\n");
    } else {
        tinyrender_log(TINYRENDER_LOG_INFO, "Closed '%s'\n", ctx->opt.filename ? ctx->opt.filename : "(unknown)");
    }
    ctx->f = NULL;
}

void tinyrender_clear_background(TinyRenderCtx *ctx, const TinyRenderColor color) {
    if (!ctx->pixels) return;
    for (size_t i = 0; i < (size_t)ctx->opt.width * (size_t)ctx->opt.height; i++) {
        ctx->pixels[i].r = color.r;
        ctx->pixels[i].g = color.g;
        ctx->pixels[i].b = color.b;
    }
}
