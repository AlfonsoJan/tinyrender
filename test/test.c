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
    TinyRenderWriter writer = {0};
    if(tinyrender_start(opt, &writer, Y, U, V) != 0) {
        return -1;
    }

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        tinyrender_frame(&writer, pixels);
    }

    tinyrender_end(&writer);
    return 0;
}