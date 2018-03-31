#include <genesis.h>

// resources.h is generated from resources.res
#include "resources.h"

#define TILE1 1

int main() {
    // get the image width and height in pixels, should be 8px aligned
    u16 w = moon.w;
    u16 h = moon.h;

    // get the palette data of moon
    VDP_setPalette(PAL1, moon.palette->data);

    // load bitmap data of moon in VRAM
    // w/8 = width in tiles we want to load
    // h/8 = height in tile we want to load
    // w/8 = width in tiles of the bitamp
    // the 3rd arg is needed because you could load only a part of the bitmap
    // if you want but SGDK needs the width as reference
    VDP_loadBMPTileData((const u32 *)moon.image, 1, w / 8, h / 8, w / 8);

    // draw the moon at (12,12)
    VDP_fillTileMapRectInc(PLAN_B, TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE1), 12, 12, w / 8, h / 8);

    while(1) {
        VDP_waitVSync();
    }

    return 0;
}
