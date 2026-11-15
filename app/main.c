
#include <stdint.h>

#include "stm32f4xx.h"

#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "mpu6500.h"
#include "attitude.h"
#include "oled.h"

#define CALIBRATION_SAMPLES  500U
#define SAMPLE_DELAY_MS       5U
#define SAMPLE_PERIOD_MS     10U
#define OUTPUT_PERIOD_MS    200U

#define GYRO_SENSITIVITY    131.0f


static void uart_write_i32(int32_t value)
{
    char digits[11];
    uint32_t count = 0U;
    int64_t number = value;

    if (number < 0) {
        uart_putc('-');
        number = -number;
    }

    do {
        digits[count++] = (char)('0' + (number % 10));
        number /= 10;
    } while (number > 0);

    while (count > 0U) {
        uart_putc(digits[--count]);
    }
}


static void uart_write_float2(float value)
{
    int32_t scaled = (int32_t)(value * 100.0f);

    if (scaled < 0) {
        uart_putc('-');
        scaled = -scaled;
    }

    uart_write_i32(scaled / 100);
    uart_putc('.');

    int32_t fraction = scaled % 100;

    if (fraction < 10) {
        uart_putc('0');
    }

    uart_write_i32(fraction);
}


static int gyro_calibrate(int32_t *offset_x,
                          int32_t *offset_y,
                          int32_t *offset_z)
{
    int64_t sum_x = 0;
    int64_t sum_y = 0;
    int64_t sum_z = 0;

    mpu6500_data_t sample;

    uart_write("\r\n");
    uart_write("Keep board completely STILL!\r\n");
    uart_write("Gyroscope calibration...\r\n");

    delay_ms(1000U);

    for (uint32_t i = 0U; i < CALIBRATION_SAMPLES; i++) {

        if (mpu6500_read_raw(&sample) != 0) {
            return -1;
        }

        sum_x += sample.gyro_x;
        sum_y += sample.gyro_y;
        sum_z += sample.gyro_z;

        delay_ms(SAMPLE_DELAY_MS);
    }

    *offset_x = (int32_t)(sum_x / CALIBRATION_SAMPLES);
    *offset_y = (int32_t)(sum_y / CALIBRATION_SAMPLES);
    *offset_z = (int32_t)(sum_z / CALIBRATION_SAMPLES);

    return 0;
}


int main(void)
{
    uint32_t hz = (clock_init_84mhz() == 0)
                  ? 84000000U : 16000000U;

    systick_init(hz);
    uart_init();
    i2c1_init();

    uart_write("\r\n");
    uart_write("============================\r\n");
    uart_write(" STM32F401 + MPU6500 + OLED\r\n");
    uart_write("============================\r\n");

    uart_write("Initializing OLED...\r\n");

    if (oled_init() != 0) {
        uart_write("OLED initialization failed!\r\n");

        while (1) {
        }
    }

    uart_write("OLED initialized successfully!\r\n");
    uart_write("Initializing MPU6500...\r\n");

    if (mpu6500_init() != 0) {
        uart_write("MPU6500 initialization failed!\r\n");

        while (1) {
        }
    }

    uart_write("MPU6500 initialization successful!\r\n");

    int32_t gyro_offset_x = 0;
    int32_t gyro_offset_y = 0;
    int32_t gyro_offset_z = 0;

    if (gyro_calibrate(&gyro_offset_x,
                       &gyro_offset_y,
                       &gyro_offset_z) != 0) {

        uart_write("Gyroscope calibration failed!\r\n");

        while (1) {
        }
    }

    uart_write("Calibration complete!\r\n");

    uart_write("Gyro offsets: X=");
    uart_write_i32(gyro_offset_x);

    uart_write(" Y=");
    uart_write_i32(gyro_offset_y);

    uart_write(" Z=");
    uart_write_i32(gyro_offset_z);

    uart_write("\r\n");

    uart_write("Establishing level reference...\r\n");

    delay_ms(100U);

    mpu6500_data_t sample;

    if (mpu6500_read_raw(&sample) != 0) {
        uart_write("Initial sensor read failed!\r\n");

        while (1) {
        }
    }

    attitude_t attitude;

    if (attitude_init(&attitude,
                      sample.accel_x,
                      sample.accel_y,
                      sample.accel_z) != 0) {

        uart_write("Attitude initialization failed!\r\n");

        while (1) {
        }
    }

    uart_write("Level reference established!\r\n");
    uart_write("Artificial horizon started!\r\n");

    uint32_t last_update_ms = millis();
    uint32_t last_output_ms = last_update_ms;

    uint8_t oled_page = 0U;

    while (1) {

        while ((uint32_t)(millis() - last_update_ms)
               < SAMPLE_PERIOD_MS) {
            __NOP();
        }

        if (mpu6500_read_raw(&sample) != 0) {

            uart_write("MPU6500 read error!\r\n");

            last_update_ms = millis();

            continue;
        }

        uint32_t now = millis();

        float dt =
            (float)(uint32_t)(now - last_update_ms) * 0.001f;

        last_update_ms = now;

        int32_t gyro_y =
            (int32_t)sample.gyro_y - gyro_offset_y;

        int32_t gyro_z =
            (int32_t)sample.gyro_z - gyro_offset_z;

        float gyro_pitch =
            (float)gyro_y / GYRO_SENSITIVITY;

        float gyro_roll =
            (float)gyro_z / GYRO_SENSITIVITY;

        if (attitude_update(&attitude,
                            sample.accel_x,
                            sample.accel_y,
                            sample.accel_z,
                            gyro_pitch,
                            gyro_roll,
                            dt) != 0) {
            continue;
        }

        oled_draw_horizon(attitude.pitch_deg,
                          attitude.roll_deg);

        if (oled_update_page(oled_page) != 0) {

            uart_write("OLED I2C transfer error!\r\n");

            while (1) {
            }
        }

        oled_page++;

        if (oled_page >= 8U) {
            oled_page = 0U;
        }

        if ((uint32_t)(now - last_output_ms)
            >= OUTPUT_PERIOD_MS) {

            last_output_ms = now;

            uart_write("PITCH: ");
            uart_write_float2(attitude.pitch_deg);

            uart_write(" deg | ROLL: ");
            uart_write_float2(attitude.roll_deg);

            uart_write(" deg\r\n");
        }
    }
}
