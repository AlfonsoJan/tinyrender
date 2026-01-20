#include "tinyrender.h"

#define WIDTH 1600
#define HEIGHT 900
#define N WIDTH*HEIGHT
#define FPS 60
#define DURATION_SECONDS 3
#define TOTAL_FRAMES FPS * DURATION_SECONDS

static TinyRenderPixels pixels[N];
static uint8_t Y[N], U[N], V[N], Z[N];

int main() {
    // If you want to disable all logging or create your own handler
    // tinyrender_set_log_handler(NULL);
    // if no logger then you can check for errors
    TinyRenderResult result;
    TinyRenderCtx ctx = {0};
    result = tinyrender_init_ctx(&ctx, pixels, Y, U, V, Z);
    if (result != TINYRENDER_OK) {
        printf("%s\n", tinyrender_strerror(result));
        return 1;
    }

    TinyRenderOption opt = {
        .width = WIDTH,
        .height = HEIGHT,
        .fps = FPS,
        .filename = "output.y4m"
    };

    result = tinyrender_start(opt, &ctx);
    if (result != TINYRENDER_OK) {
        printf("%s\n", tinyrender_strerror(result));
        return 1;
    }

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        float t = (float)(frame) / (float)ctx.opt.fps;

        if (t <= 1.0f) {
            tinyrender_clear_background(&ctx, (TinyRenderColor){255, 0, 0});
        } else if (t <= 2.0f) {
            tinyrender_clear_background(&ctx, (TinyRenderColor){0, 255, 0});
        } else {
            tinyrender_clear_background(&ctx, (TinyRenderColor){0, 0, 255});
        }


        result = tinyrender_frame(&ctx);
        if (result != TINYRENDER_OK) {
            printf("%s\n", tinyrender_strerror(result));
            return 1;
        }
    }

    tinyrender_end(&ctx);
    return 0;
}