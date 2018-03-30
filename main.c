#include <genesis.h>

int main() {
    VDP_drawText("Hello darkness, my old friend...", 5, 13);

    while (1) {
        VDP_waitVSync();
    }

    return 0;
}
