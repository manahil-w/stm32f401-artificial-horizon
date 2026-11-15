#include "mpu6500.h"
#include "i2c.h"

#define MPU6500_ADDRESS       0x68U
#define MPU6500_WHO_AM_I      0x70U

/* MPU6500 registers */
#define REG_SMPLRT_DIV        0x19U
#define REG_CONFIG            0x1AU
#define REG_GYRO_CONFIG       0x1BU
#define REG_ACCEL_CONFIG      0x1CU
#define REG_ACCEL_CONFIG2     0x1DU

#define REG_ACCEL_XOUT_H      0x3BU
#define REG_PWR_MGMT_1        0x6BU
#define REG_WHO_AM_I          0x75U


/*
 * Combine two bytes into a signed 16-bit value.
 */
static int16_t make_int16(uint8_t high, uint8_t low)
{
    uint16_t value;

    value = ((uint16_t)high << 8) | (uint16_t)low;

    return (int16_t)value;
}


/*
 * Initialize MPU6500.
 *
 * Returns:
 *  0 = success
 * -1 = I2C communication error
 * -2 = incorrect WHO_AM_I value
 */
int mpu6500_init(void)
{
    uint8_t id = 0U;

    /* Read WHO_AM_I */
    if (i2c1_read_regs(MPU6500_ADDRESS,
                       REG_WHO_AM_I,
                       &id,
                       1U) != 0) {
        return -1;
    }

    /* Check chip identity */
    if (id != MPU6500_WHO_AM_I) {
        return -2;
    }

    /* Wake up and select X-axis gyroscope PLL clock */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_PWR_MGMT_1,
                       0x01U) != 0) {
        return -1;
    }

    /* Gyroscope DLPF: approximately 42 Hz bandwidth */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_CONFIG,
                       0x03U) != 0) {
        return -1;
    }

    /* Sample rate = 1000 / (1 + 4) = 200 Hz */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_SMPLRT_DIV,
                       0x04U) != 0) {
        return -1;
    }

    /* Gyroscope range: +/-250 degrees/second */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_GYRO_CONFIG,
                       0x00U) != 0) {
        return -1;
    }

    /* Accelerometer range: +/-2g */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_ACCEL_CONFIG,
                       0x00U) != 0) {
        return -1;
    }

    /* Accelerometer DLPF: approximately 41 Hz bandwidth */
    if (i2c1_write_reg(MPU6500_ADDRESS,
                       REG_ACCEL_CONFIG2,
                       0x03U) != 0) {
        return -1;
    }

    return 0;
}



int mpu6500_read_raw(mpu6500_data_t *data)
{
    uint8_t raw[14];

    if (data == 0) {
        return -1;
    }

    
    if (i2c1_read_regs(MPU6500_ADDRESS,
                       REG_ACCEL_XOUT_H,
                       raw,
                       14U) != 0) {
        return -1;
    }

    data->accel_x = make_int16(raw[0], raw[1]);
    data->accel_y = make_int16(raw[2], raw[3]);
    data->accel_z = make_int16(raw[4], raw[5]);

    data->temperature = make_int16(raw[6], raw[7]);

    data->gyro_x = make_int16(raw[8], raw[9]);
    data->gyro_y = make_int16(raw[10], raw[11]);
    data->gyro_z = make_int16(raw[12], raw[13]);

    return 0;
}