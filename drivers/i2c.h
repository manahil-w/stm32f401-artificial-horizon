#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c1_init(void);

int i2c1_write_reg(uint8_t address,
                   uint8_t reg,
                   uint8_t value);

int i2c1_read_regs(uint8_t address,
                   uint8_t reg,
                   uint8_t *data,
                   uint16_t length);

int i2c1_write_buf(uint8_t address,
                   uint8_t control,
                   const uint8_t *data,
                   uint16_t length);

#endif