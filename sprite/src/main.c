#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"

/* #define SOUND_ON */
#define SPRITE_FLAGS    SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_FAST_AUTO_VISIBILITY | SPR_FLAG_AUTO_VRAM_ALLOC | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD

#define SFX_JUMP        64
#define SFX_ROLL        65
#define SFX_STOP        66

#define ANIM_STAND      0
#define ANIM_WAIT       1
#define ANIM_WALK       2
#define ANIM_RUN        3
#define ANIM_BRAKE      4
#define ANIM_UP         5
#define ANIM_CROUNCH    6
#define ANIM_ROLL       7

#define MAX_SPEED       FIX32(8)
#define RUN_SPEED       FIX32(6)
#define BRAKE_SPEED     FIX32(2)

#define JUMP_SPEED      FIX32(-8)
#define GRAVITY         FIX32(0.4)
#define ACCEL           FIX32(0.1)
#define DE_ACCEL        FIX32(0.15)

#define PLY_POSX		FIX32(88)
#define MIN_POSX        FIX32(10)
#define MAX_POSX        FIX32(10000)
#define MAX_POSY        FIX32(156)



// forward
static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);

static void updatePhysic();
static void updateAnim();
static void updateCamera(fix32 x, fix32 y);

// Array of sprites
Sprite* sprites[3];

u16 scrollPos;
fix32 camposx;
fix32 camposy;
fix32 posx;
fix32 posy;
fix32 movx;
fix32 movy;
s16 xorder;
s16 yorder;

fix32 enemyPosx[2];
fix32 enemyPosy[2];
s16 enemyXorder[2];

int main() {
    u16 palette[64];
    u16 ind;

    // disable interrupt when accessing VDP
    SYS_disableInts();

    // initialization
    VDP_setScreenWidth320();

    // init SFX
#ifdef SOUND_ON
    SND_setPCM_XGM(SFX_JUMP, sonic_jump_sfx, sizeof(sonic_jump_sfx));
    SND_setPCM_XGM(SFX_ROLL, sonic_roll_sfx, sizeof(sonic_roll_sfx));
    SND_setPCM_XGM(SFX_STOP, sonic_stop_sfx, sizeof(sonic_stop_sfx));

    // start music
    SND_startPlay_XGM(sonic_music);
#endif

    // init sprites engine
    SPR_init(16, 256, 256);

    // set plan size and all palette to black
    VDP_setPlanSize(64, 32);
    VDP_setPaletteColors(0, (u16*) palette_black, 64);

    // load and draw foreground
	ind = TILE_USERINDEX;
	Map *map = unpackMap(bga_image.map, NULL);
	VDP_loadTileSet(bga_image.tileset, ind, TRUE);
	VDP_setMapEx(PLAN_A, map, TILE_ATTR_FULL(PAL1, 1, FALSE, FALSE, ind), 0, 0, 0, 0, 320 >> 3, 32);
	ind += bga_image.tileset->numTile;

    // load and draw background
	VDP_drawImageEx(PLAN_B, &bgb_image, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);

    // VDP process done, we can reenable interrupts
    SYS_enableInts();

    camposx = -1;
    camposy = -1;
    posx = PLY_POSX;
    posy = MAX_POSY;
    movx = FIX32(0);
    movy = FIX32(0);
    xorder = 0;
    yorder = 0;

    enemyPosx[0] = FIX32(10);
    enemyPosy[0] = FIX32(164);
    enemyPosx[1] = FIX32(300);
    enemyPosy[1] = FIX32(84);
    enemyXorder[0] = 1;
    enemyXorder[1] = -1;

    // set camera
    updateCamera(FIX32(0), FIX32(0));

    // init sonic sprite
    sprites[0] = SPR_addSpriteEx(&sonic_sprite, fix32ToInt(posx - camposx), fix32ToInt(posy - camposy), TILE_ATTR(PAL2, 0, FALSE, FALSE), 0, SPRITE_FLAGS);

    // init enemies sprites
    sprites[1] = SPR_addSpriteEx(&enemies_sprite, fix32ToInt(enemyPosx[0] - camposx), fix32ToInt(enemyPosy[0] - camposy), TILE_ATTR(PAL3, 1, FALSE, FALSE), 1, SPRITE_FLAGS);
    sprites[2] = SPR_addSpriteEx(&enemies_sprite, fix32ToInt(enemyPosx[1] - camposx), fix32ToInt(enemyPosy[1] - camposy), TILE_ATTR(PAL3, 1, FALSE, FALSE), 2, SPRITE_FLAGS);

    // select enemy for each sprite
    SPR_setAnim(sprites[1], 1);
    SPR_setAnim(sprites[2], 0);

    // sort sprites
	SPR_setYSorting(sprites[0], 1);
	SPR_setYSorting(sprites[1], 0);
	SPR_setYSorting(sprites[2], 0);
    SPR_sort(NULL);

	SPR_update();

    // prepare palettes
	memcpy(&palette[0], bgb_image.palette->data, 16 * 2);
	memcpy(&palette[16], bga_image.palette->data, 16 * 2);
    memcpy(&palette[32], sonic_sprite.palette->data, 16 * 2);
    memcpy(&palette[48], enemies_sprite.palette->data, 16 * 2);

    // fade in
    VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    JOY_setEventHandler(joyEvent);

    while(TRUE) {
        handleInput();

        updatePhysic();
        updateAnim();

        if (movx > 0 && posx > FIX32(240)) {
            scrollPos = ((fix32ToInt(posx + FIX32(80)) >> 3)) & 63;
            VDP_setMapEx(PLAN_A, map, TILE_ATTR_FULL(PAL1, 1, FALSE, FALSE, TILE_USERINDEX), scrollPos, 0, scrollPos, 0, 1, 32);
        }


        // update sprites
        SPR_update();

        VDP_waitVSync();
    }

    MEM_free(map);
    return 0;
}

static void updatePhysic() {
    u16 i;
    fix32 px_scr, py_scr;
    fix32 npx_cam, npy_cam;

    // sonic physic
    if (xorder > 0) {
        movx += ACCEL;
        // going opposite side, quick breaking
        if (movx < 0) movx += ACCEL;
        if (movx >= MAX_SPEED) movx = MAX_SPEED;
    } else if (xorder < 0) {
        movx -= ACCEL;
        // going opposite side, quick breaking
        if (movx > 0) movx -= ACCEL;
        if (movx <= -MAX_SPEED) movx = -MAX_SPEED;
    } else {
        if ((movx < FIX32(0.1)) && (movx > FIX32(-0.1))) movx = 0;
        else if ((movx < FIX32(0.3)) && (movx > FIX32(-0.3))) movx -= movx >> 2;
        else if ((movx < FIX32(1)) && (movx > FIX32(-1))) movx -= movx >> 3;
        else movx -= movx >> 4;
    }

    posx += movx;
    posy += movy;

    if (movy) {
        if (posy > MAX_POSY) {
            posy = MAX_POSY;
            movy = 0;
        } else movy += GRAVITY;
    }

    if (posx >= MAX_POSX) {
        posx = MAX_POSX;
        movx = 0;
    } else if (posx <= MIN_POSX) {
        posx = MIN_POSX;
        movx = 0;
    }

    // enemies physic
    if (enemyXorder[0] > 0) enemyPosx[0] += FIX32(1.5);
    else enemyPosx[0] -= FIX32(1.5);
    if (enemyXorder[1] > 0) enemyPosx[1] += FIX32(1.5);
    else enemyPosx[1] -= FIX32(1.5);

    for(i = 0; i < 2; i++) {
        if ((enemyPosx[i] >= MAX_POSX) || (enemyPosx[i] <= MIN_POSX)) enemyXorder[i] = -enemyXorder[i];
    }

    // get sprite position on screen
    px_scr = posx - camposx;
    py_scr = posy - camposy;

    // calculate new camera position
    if (px_scr > FIX32(240)) npx_cam = posx - FIX32(240);
    else if (px_scr < FIX32(40)) npx_cam = posx - FIX32(40);
    else npx_cam = camposx;

    if (py_scr > FIX32(160)) npy_cam = posy - FIX32(160);
    else if (py_scr < FIX32(100)) npy_cam = posy - FIX32(100);
    else npy_cam = camposy;

    // clip camera position
    if (npx_cam < FIX32(0)) npx_cam = FIX32(0);
    else if (npx_cam > MAX_POSX) npx_cam = MAX_POSX;

    if (npy_cam < FIX32(0)) npy_cam = FIX32(0);
    else if (npy_cam > FIX32(100)) npy_cam = FIX32(100);

    // set camera position
    updateCamera(npx_cam, npy_cam);

    // debugging
	/* char str[12]; */
	/* int pos = fix32ToInt(enemyPosx[0]-camposx); */
	/* sprintf(str, "%d/%d/%d", fix32ToInt(camposx), (pos), fix32ToInt(posx)); */
    /* VDP_drawText(str, 5, 13); */

    // set sprites position
    SPR_setPosition(sprites[0], fix32ToInt(px_scr), fix32ToInt(py_scr));
    SPR_setPosition(sprites[1], fix32ToInt(enemyPosx[0] - camposx), fix32ToInt(enemyPosy[0] - camposy));
    SPR_setPosition(sprites[2], fix32ToInt(enemyPosx[1] - camposx), fix32ToInt(enemyPosy[1] - camposy));
}

static void updateAnim() {
    // jumping
    if (movy) SPR_setAnim(sprites[0], ANIM_ROLL);
    else {
        if (((movx >= BRAKE_SPEED) && (xorder < 0)) || ((movx <= -BRAKE_SPEED) && (xorder > 0))) {
            if (sprites[0]->animInd != ANIM_BRAKE) {
#ifdef SOUND_ON
                SND_startPlayPCM_XGM(SFX_STOP, 1, SOUND_PCM_CH2);
#endif
                SPR_setAnim(sprites[0], ANIM_BRAKE);
            }
        } else if ((movx >= RUN_SPEED) || (movx <= -RUN_SPEED)) {
            SPR_setAnim(sprites[0], ANIM_RUN);
        } else if (movx != 0) {
            SPR_setAnim(sprites[0], ANIM_WALK);
        } else {
            if (yorder < 0) SPR_setAnim(sprites[0], ANIM_UP);
            else if (yorder > 0) SPR_setAnim(sprites[0], ANIM_CROUNCH);
            else SPR_setAnim(sprites[0], ANIM_STAND);
        }
    }

    if (movx > 0) SPR_setHFlip(sprites[0], FALSE);
    else if (movx < 0) SPR_setHFlip(sprites[0], TRUE);

    // enemies
    if (enemyXorder[1] > 0) SPR_setHFlip(sprites[2], TRUE);
    else SPR_setHFlip(sprites[2], FALSE);
}

static void updateCamera(fix32 x, fix32 y) {
    if ((x != camposx) || (y != camposy)) {
        camposx = x;
        camposy = y;
        VDP_setHorizontalScroll(PLAN_A, fix32ToInt(-camposx));
        VDP_setHorizontalScroll(PLAN_B, fix32ToInt(-camposx) >> 3);
        VDP_setVerticalScroll(PLAN_A, fix32ToInt(camposy));
        VDP_setVerticalScroll(PLAN_B, fix32ToInt(camposy) >> 3);
    }
}

static void handleInput() {
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_UP) yorder = -1;
    else if (value & BUTTON_DOWN) yorder = +1;
    else yorder = 0;

    if (value & BUTTON_LEFT) xorder = -1;
    else if (value & BUTTON_RIGHT) xorder = +1;
    else xorder = 0;
}

static void joyEvent(u16 joy, u16 changed, u16 state) {
    if (changed & state & (BUTTON_A | BUTTON_B | BUTTON_C)) {
        if (movy == 0) {
            movy = JUMP_SPEED;
#ifdef SOUND_ON
            SND_startPlayPCM_XGM(SFX_JUMP, 1, SOUND_PCM_CH2);
#endif
        }
    }
}
