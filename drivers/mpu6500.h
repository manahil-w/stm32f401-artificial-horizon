#ifndef MPU6500_H
#define MPU6500_H

#include <stdint.h>

typedef struct
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t temperature;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

} mpu6500_data_t;

int mpu6500_init(void);

int mpu6500_read_raw(mpu6500_data_t *data);

#endif