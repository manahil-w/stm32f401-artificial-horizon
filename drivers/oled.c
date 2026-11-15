#include <stdint.h>
#include <string.h>
#include <math.h>

#include "i2c.h"
#include "oled.h"

#define OLED_ADDRESS       0x3CU

#define OLED_WIDTH         128
#define OLED_HEIGHT        64
#define OLED_PAGES         8

#define DEG_TO_RAD         0.0174532925f

#define CENTER_X           63.5f
#define CENTER_Y           31.5f

#define PIXELS_PER_DEG     0.8f

static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];

static float roll_sin;
static float roll_cos;
static float horizon_offset;


static void oled_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= OLED_WIDTH ||
        y < 0 || y >= OLED_HEIGHT) {
        return;
    }

    uint16_t index =
        (uint16_t)x + ((uint16_t)y / 8U) * OLED_WIDTH;

    uint8_t mask =
        (uint8_t)(1U << ((uint16_t)y % 8U));

    if (color) {
        framebuffer[index] |= mask;
    } else {
        framebuffer[index] &= (uint8_t)~mask;
    }
}


static uint8_t oled_is_ground(int x, int y)
{
    float dx = (float)x - CENTER_X;
    float dy = (float)y - CENTER_Y;

    float v = -dx * roll_sin + dy * roll_cos;

    return (v > horizon_offset) ? 1U : 0U;
}


static void oled_contrast_pixel(int x, int y)
{
    uint8_t ground = oled_is_ground(x, y);

    oled_pixel(x, y, ground ? 0U : 1U);
}


static void oled_line(int x0, int y0,
                      int x1, int y1)
{
    int dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 >= y0) ? (y0 - y1) : (y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int error = dx + dy;

    for (int steps = 0; steps < 256; steps++) {

        oled_contrast_pixel(x0, y0);

        if (x0 == x1 && y0 == y1) {
            return;
        }

        int e2 = 2 * error;

        if (e2 >= dy) {
            error += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}


static void oled_rotated_line(float u0,
                              float u1,
                              float v)
{
    float x0 = CENTER_X + roll_cos * u0 - roll_sin * v;
    float y0 = CENTER_Y + roll_sin * u0 + roll_cos * v;

    float x1 = CENTER_X + roll_cos * u1 - roll_sin * v;
    float y1 = CENTER_Y + roll_sin * u1 + roll_cos * v;

    oled_line((int)x0, (int)y0,
              (int)x1, (int)y1);
}


static void oled_draw_horizon_line(void)
{
    oled_rotated_line(-100.0f,
                      100.0f,
                      horizon_offset);
}


static void oled_draw_pitch_ladder(void)
{
    for (int mark = -20; mark <= 20; mark += 10) {

        if (mark == 0) {
            continue;
        }

        float v = horizon_offset -
                  ((float)mark * PIXELS_PER_DEG);

        float half_length =
            (mark == 10 || mark == -10) ? 17.0f : 12.0f;

        oled_rotated_line(-half_length, -5.0f, v);
        oled_rotated_line(5.0f, half_length, v);
    }
}


static void oled_draw_aircraft(void)
{
    int cx = 64;
    int cy = 32;

    oled_line(cx - 20, cy,
              cx - 6, cy);

    oled_line(cx + 6, cy,
              cx + 20, cy);

    oled_line(cx - 20, cy,
              cx - 16, cy - 4);

    oled_line(cx + 20, cy,
              cx + 16, cy - 4);

    oled_line(cx, cy - 8,
              cx, cy + 7);

    oled_line(cx - 3, cy - 4,
              cx, cy - 8);

    oled_line(cx, cy - 8,
              cx + 3, cy - 4);
}


static void oled_draw_roll_indicator(float roll_deg)
{
    for (int angle = -60; angle <= 60; angle += 10) {

        int x = (int)(CENTER_X +
                      ((float)angle * 0.35f));

        int tick_length =
            ((angle % 30) == 0) ? 7 : 4;

        oled_line(x, 3, x, 3 + tick_length);
    }

    if (roll_deg > 60.0f) {
        roll_deg = 60.0f;
    }

    if (roll_deg < -60.0f) {
        roll_deg = -60.0f;
    }

    int pointer_x =
        (int)(CENTER_X + roll_deg * 0.35f);

    oled_line(pointer_x, 1,
              pointer_x - 4, 8);

    oled_line(pointer_x, 1,
              pointer_x + 4, 8);

    oled_line(pointer_x - 4, 8,
              pointer_x + 4, 8);
}


int oled_update_page(uint8_t page)
{
    if (page >= OLED_PAGES) {
        return -1;
    }

    uint8_t commands[3];

    commands[0] = (uint8_t)(0xB0U | page);
    commands[1] = 0x00U;
    commands[2] = 0x10U;

    if (i2c1_write_buf(OLED_ADDRESS,
                       0x00U,
                       commands,
                       3U) != 0) {
        return -1;
    }

    return i2c1_write_buf(
        OLED_ADDRESS,
        0x40U,
        &framebuffer[(uint16_t)page * OLED_WIDTH],
        OLED_WIDTH
    );
}


int oled_update(void)
{
    for (uint8_t page = 0U; page < OLED_PAGES; page++) {

        if (oled_update_page(page) != 0) {
            return -1;
        }
    }

    return 0;
}


void oled_clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}


int oled_init(void)
{
    static const uint8_t commands[] = {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x02,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0x7F,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0x2E,
        0xAF
    };

    oled_clear();

    if (i2c1_write_buf(OLED_ADDRESS,
                       0x00U,
                       commands,
                       sizeof(commands)) != 0) {
        return -1;
    }

    return oled_update();
}


void oled_draw_horizon(float pitch_deg,
                       float roll_deg)
{
    if (pitch_deg > 40.0f) {
        pitch_deg = 40.0f;
    }

    if (pitch_deg < -40.0f) {
        pitch_deg = -40.0f;
    }

    float roll_rad = roll_deg * DEG_TO_RAD;

    roll_sin = sinf(roll_rad);
    roll_cos = cosf(roll_rad);

    horizon_offset = pitch_deg * PIXELS_PER_DEG;

    oled_clear();

    for (int y = 0; y < OLED_HEIGHT; y++) {

        for (int x = 0; x < OLED_WIDTH; x++) {

            oled_pixel(x, y, oled_is_ground(x, y));
        }
    }

    oled_draw_horizon_line();

    oled_draw_pitch_ladder();

    oled_draw_aircraft();

    oled_draw_roll_indicator(roll_deg);
}


int oled_test_pattern(void)
{
    memset(framebuffer, 0xFF, sizeof(framebuffer));

    return oled_update();
}


int oled_test_half_screen(void)
{
    memset(framebuffer, 0x00, sizeof(framebuffer));

    for (uint8_t page = 4U; page < 8U; page++) {

        memset(&framebuffer[(uint16_t)page * OLED_WIDTH],
               0xFF,
               OLED_WIDTH);
    }

    return oled_update();
}