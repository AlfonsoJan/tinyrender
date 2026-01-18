#include "tinyrender.h"

static inline uint8_t clamp_u8(float v) {
    if (v < 0.0f)   return 0;
    if (v > 255.0f) return 255;
    return (uint8_t)v;
}

int tinyrender_start(TinyRenderOption opt, TinyRenderWriter *w, uint8_t *y, uint8_t *u, uint8_t *v) {
    if (!w) {
        fprintf(stderr, "Writer pointer is NULL\n");
        return 1;
    }
    if (opt.width <= 0 || opt.height <= 0) {
        fprintf(stderr, "Invalid options (w=%d h=%d)\n", opt.width, opt.height);
        return 1;
    }
    if (opt.fps <= 0) {
        fprintf(stderr, "FPS cannot be 0 or lower\n");
        return 1;
    }
    if (opt.filename == NULL) {
        fprintf(stderr, "Filename pointer is NULL\n");
        return 1;
    }

    w->f = fopen(opt.filename, "wb");
    if (!w->f) {
        fprintf(stderr, "Failed to open '%s'\n", opt.filename);
        return 1;
    }

    w->opt = opt;
    w->y_plane = y;
    w->u_plane = u;
    w->v_plane = v;

    int wrote = fprintf(w->f, "YUV4MPEG2 W%d H%d F%d:1 Ip A1:1 C444\n", w->opt.width, w->opt.height, w->opt.fps);
    if (wrote < 0) {
        fprintf(stderr, "Failed to write file header to '%s'\n", opt.filename);
        fclose(w->f);
        w->f = NULL;
        return 1;
    }

    size_t planeN = (size_t)w->opt.width * (size_t)w->opt.height;
    fprintf(stdout, "Opened '%s' (%dx%d @ %dfps, plane bytes=%llu)\n", opt.filename, opt.width, opt.height, opt.fps, planeN);
    return 0;
}

int tinyrender_frame(TinyRenderWriter *w, const TinyRenderPixels *pixels) {
    if (!w || !w->f) {
        fprintf(stderr, "writer/file is NULL\n");
        return 1;
    }
    if (!pixels) {
        fprintf(stderr, "pixels buffer is NULL\n");
        return 1;
    }
    if (fprintf(w->f, "FRAME\n") < 0) {
        fprintf(stderr, "failed to write frame header\n");
        return 1;
    }

    const size_t N = (size_t)w->opt.width * (size_t)w->opt.height;
    for (size_t i = 0; i < N; ++i) {
        uint8_t r = pixels[i].r;
        uint8_t g = pixels[i].g;
        uint8_t b = pixels[i].b;

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
        fprintf(stderr, "Short write (Y=%llu/%llu, U=%llu/%llu, V=%llu/%llu)\n",
            (unsigned long long)wroteY, (unsigned long long)N,
            (unsigned long long)wroteU, (unsigned long long)N,
            (unsigned long long)wroteV, (unsigned long long)N);
        return 1;
    }
    fprintf(stdout, "Wrote %llu bytes (YUV444)\n", (unsigned long long)3 * N);
    return 0;
}

void tinyrender_end(TinyRenderWriter *w) {
    if (!w) {
        fprintf(stderr, "file already closed or was never opened\n");
        return;
    }
    if (fclose(w->f) != 0) {
        fprintf(stderr, "fclose failed\n");
    } else {
        fprintf(stdout, "closed '%s'\n", w->opt.filename ? w->opt.filename : "(unknown)");
    }
    w->f = NULL;
}

void tinyrender_clear_background(TinyRenderPixels *pixels, TinyRenderWriter w, TinyRenderColor color) {
    if (!pixels) return;
    for (size_t i = 0; i < (size_t)w.opt.width * (size_t)w.opt.height; i++) {
        pixels[i].r = color.r;
        pixels[i].g = color.g;
        pixels[i].b = color.b;
    }
}
