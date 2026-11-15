#ifndef ATTITUDE_H
#define ATTITUDE_H

#include <stdint.h>

typedef struct
{
    float pitch_deg;
    float roll_deg;

    float pitch_zero_deg;
    float roll_zero_deg;

    uint8_t initialized;

} attitude_t;

int attitude_init(attitude_t *attitude,
                  int16_t accel_x,
                  int16_t accel_y,
                  int16_t accel_z);

int attitude_update(attitude_t *attitude,
                    int16_t accel_x,
                    int16_t accel_y,
                    int16_t accel_z,
                    float gyro_pitch_dps,
                    float gyro_roll_dps,
                    float dt);

#endif