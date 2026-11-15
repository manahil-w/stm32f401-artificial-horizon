#ifndef OLED_H
#define OLED_H

#include <stdint.h>

int oled_init(void);

void oled_clear(void);

void oled_draw_horizon(float pitch_deg,
                       float roll_deg);

int oled_update_page(uint8_t page);

int oled_update(void);

int oled_test_pattern(void);

int oled_test_half_screen(void);

#endif