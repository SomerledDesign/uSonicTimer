#include "U8g2lib.h"

static uint8_t byte_cb(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }
static uint8_t gpio_cb(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }

U8G2::U8G2()
{
    u8g2_Setup_pcd8544_84x48_f(&u8g2, U8G2_R0, byte_cb, gpio_cb);
    u8g2_InitDisplay(&u8g2);
}

bool U8G2::getPixel(int x, int y)
{
    // PCD8544 full buffer: one byte per column per 8-row tile, bit 0 = top row
    const uint8_t *buf = u8g2_GetBufferPtr(&u8g2);
    const int stride = u8g2_GetBufferTileWidth(&u8g2) * 8;
    return (buf[(y / 8) * stride + x] >> (y % 8)) & 1;
}
