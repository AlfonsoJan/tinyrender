#include "tinyrender.h"

#define WIDTH 1600
#define HEIGHT 900
#define N WIDTH*HEIGHT
#define FPS 60
#define DURATION_SECONDS 3
#define TOTAL_FRAMES FPS * DURATION_SECONDS

static TinyRenderPixels pixels[N];
static uint8_t Y[N], U[N], V[N];

int main() {
    TinyRenderOption opt = {
        .width = WIDTH,
        .height = HEIGHT,
        .fps = FPS,
        .filename = "output.y4m"
    };
    TinyRenderWriter w = {0};
    if(tinyrender_start(opt, &w, pixels, Y, U, V) != 0) {
        return -1;
    }

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        float t = (float)(frame) / (float)w.opt.fps;

        if (t <= 1.0f) {
            tinyrender_clear_background(&w, (TinyRenderColor){255, 0, 0});
        } else if (t <= 2.0f) {
            tinyrender_clear_background(&w, (TinyRenderColor){0, 255, 0});
        } else {
            tinyrender_clear_background(&w, (TinyRenderColor){0, 0, 255});
        }

        tinyrender_frame(&w);
    }

    tinyrender_end(&w);
    return 0;
}