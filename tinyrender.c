#include "tinyrender.h"

static inline uint8_t clamp_u8(float v) {
    if (v < 0.0f)   return 0;
    if (v > 255.0f) return 255;
    return (uint8_t)v;
}

void tinyrender_log(TINYRENDER_LOG_LEVEL level, const char *fmt, ...) {
    if (level < TINYRENDER_LOG_INFO || level >= TINYRENDER_LOG_NONE) return;
    FILE *out = (level == TINYRENDER_LOG_INFO) ? stdout : stderr;
    switch (level) {
    case TINYRENDER_LOG_INFO:    fprintf(out, "[INFO] "); break;
    case TINYRENDER_LOG_WARNING: fprintf(out, "[WARNING] "); break;
    case TINYRENDER_LOG_ERROR:   fprintf(out, "[ERROR] "); break;
    default: return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    fflush(out);
}

int tinyrender_start(TinyRenderOption opt, TinyRenderWriter *w, TinyRenderPixels *pixels, uint8_t *y, uint8_t *u, uint8_t *v) {
    if (!w) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Writer pointer is NULL\n");
        return 1;
    }
    if (!pixels) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Pixels pointer is NULL\n");
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

    w->f = fopen(opt.filename, "wb");
    if (!w->f) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Failed to open '%s'\n", opt.filename);
        return 1;
    }

    w->opt = opt;
    w->y_plane = y;
    w->u_plane = u;
    w->v_plane = v;
    w->pixels = pixels;

    int wrote = fprintf(w->f, "YUV4MPEG2 W%d H%d F%d:1 Ip A1:1 C444\n", w->opt.width, w->opt.height, w->opt.fps);
    if (wrote < 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Failed to write file header to '%s'\n", opt.filename);
        fclose(w->f);
        w->f = NULL;
        return 1;
    }

    size_t planeN = (size_t)w->opt.width * (size_t)w->opt.height;
    tinyrender_log(TINYRENDER_LOG_INFO, "Opened '%s' (%dx%d @ %dfps, plane bytes=%llu)\n", opt.filename, opt.width, opt.height, opt.fps, planeN);
    return 0;
}

int tinyrender_frame(TinyRenderWriter *w) {
    if (!w || !w->f) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "writer/file is NULL\n");
        return 1;
    }
    if (!w->pixels) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "pixels buffer is NULL\n");
        return 1;
    }
    if (fprintf(w->f, "FRAME\n") < 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "failed to write frame header\n");
        return 1;
    }

    const size_t N = (size_t)w->opt.width * (size_t)w->opt.height;
    for (size_t i = 0; i < N; ++i) {
        uint8_t r = w->pixels[i].r;
        uint8_t g = w->pixels[i].g;
        uint8_t b = w->pixels[i].b;

        float Yf =  0.299f * r + 0.587f * g + 0.114f * b;
        float Uf = -0.169f * r - 0.331f * g + 0.500f * b + 128.0f;
        float Vf =  0.500f * r - 0.419f * g - 0.081f * b + 128.0f;

        w->y_plane[i] = clamp_u8(Yf);
        w->u_plane[i] = clamp_u8(Uf);
        w->v_plane[i] = clamp_u8(Vf);
    }

    size_t wroteY = fwrite(w->y_plane, 1, N, w->f);
    size_t wroteU = fwrite(w->u_plane, 1, N, w->f);
    size_t wroteV = fwrite(w->v_plane, 1, N, w->f);

    if (wroteY != N || wroteU != N || wroteV != N) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "Short write (Y=%llu/%llu, U=%llu/%llu, V=%llu/%llu)\n",
            (unsigned long long)wroteY, (unsigned long long)N,
            (unsigned long long)wroteU, (unsigned long long)N,
            (unsigned long long)wroteV, (unsigned long long)N);
        return 1;
    }
    tinyrender_log(TINYRENDER_LOG_INFO, "Wrote %llu bytes (YUV444)\n", (unsigned long long)3 * N);
    return 0;
}

void tinyrender_end(TinyRenderWriter *w) {
    if (!w) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "File already closed or was never opened\n");
        return;
    }
    if (fclose(w->f) != 0) {
        tinyrender_log(TINYRENDER_LOG_ERROR, "fclose failed\n");
    } else {
        tinyrender_log(TINYRENDER_LOG_INFO, "Closed '%s'\n", w->opt.filename ? w->opt.filename : "(unknown)");
    }
    w->f = NULL;
}

void tinyrender_clear_background(TinyRenderWriter *w, const TinyRenderColor color) {
    if (!w->pixels) return;
    for (size_t i = 0; i < (size_t)w->opt.width * (size_t)w->opt.height; i++) {
        w->pixels[i].r = color.r;
        w->pixels[i].g = color.g;
        w->pixels[i].b = color.b;
    }
}
