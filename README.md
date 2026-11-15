# STM32F401 Artificial Horizon

A bare-metal embedded systems project that implements an artificial horizon using an STM32F401 microcontroller, an MPU-6500 inertial measurement unit, and an SSD1306 OLED display.

The system measures pitch and roll, processes the sensor data using a complementary filter, and displays a live artificial horizon.

## Demo

![Artificial Horizon running on SSD1306 OLED](docs/images/artificial-horizon.jpg)

*Live artificial horizon displayed on a 0.96-inch, 128×64 OLED.*

## Features

- Bare-metal STM32 programming using CMSIS and direct peripheral registers.
- 84 MHz system clock configuration.
- Custom startup code and linker script.
- Millisecond timing using SysTick.
- Interrupt-driven USART2 transmission with a TX ring buffer.
- I2C1 communication using PB8 (SCL) and PB9 (SDA).
- MPU-6500 initialization and raw sensor data acquisition.
- Automatic gyroscope bias calibration at startup.
- Pitch and roll estimation using a complementary filter.
- SSD1306 128×64 OLED framebuffer graphics.
- Live artificial horizon that responds to sensor movement.

## Hardware

| Component | Description |
|---|---|
| Microcontroller | STM32F401 Black Pill |
| IMU | MPU-6500 accelerometer and gyroscope |
| Display | SSD1306 0.96-inch, 128×64 I2C OLED |
| USB-to-UART | CP2102 USB-to-serial adapter |
| Debugger | ST-Link |

## Hardware Connections

### I2C connections

The MPU-6500 and SSD1306 share the same I2C bus.

| STM32F401 pin | MPU-6500 | SSD1306 |
|---|---|---|
| PB8 | SCL | SCL |
| PB9 | SDA | SDA |
| 3.3V | VCC | VCC |
| GND | GND | GND |

I2C device addresses:

| Device | Address |
|---|---|
| MPU-6500 | `0x68` |
| SSD1306 | `0x3C` |

The MPU-6500 identification register (`WHO_AM_I`) returns `0x70`.

Use suitable I2C pull-up resistors to 3.3V. Check whether the breakout modules already include pull-ups.

### UART connections

| STM32F401 pin | USB-to-UART |
|---|---|
| PA2 (USART2_TX) | RX |
| PA3 (USART2_RX) | TX |
| GND | GND |

Serial configuration: **115200 baud, 8 data bits, no parity, 1 stop bit (8N1).**

## Software Architecture

The firmware is organized into application and driver layers.

```text
f401-horizon/
├── app/
│   └── main.c
├── drivers/
│   ├── attitude.c
│   ├── attitude.h
│   ├── clock.c
│   ├── clock.h
│   ├── i2c.c
│   ├── i2c.h
│   ├── mpu6500.c
│   ├── mpu6500.h
│   ├── oled.c
│   ├── oled.h
│   ├── systick.c
│   ├── systick.h
│   ├── uart.c
│   └── uart.h
├── linker/
│   └── stm32f401.ld
├── startup/
│   └── startup.c
├── third_party/
├── Makefile
└── README.md
```

### Sensor processing

The MPU-6500 provides three-axis accelerometer and gyroscope measurements.

At startup, the firmware:

1. Initializes the I2C peripheral and MPU-6500.
2. Collects 500 stationary gyroscope samples.
3. Calculates the average gyroscope bias for each axis.
4. Establishes the initial orientation reference.
5. Starts continuous pitch and roll estimation.

### Complementary filter

The complementary filter combines accelerometer-derived orientation with gyroscope integration.

- The accelerometer provides a gravity-based orientation reference.
- The gyroscope measures angular velocity.
- Gyroscope bias is subtracted before integration.
- The complementary filter combines both measurements to reduce drift and short-term noise.

The current mounting configuration uses the sensor X-axis as the approximately vertical reference, with gyroscope Y and Z measurements used for pitch and roll.

### OLED display

The SSD1306 driver uses a 1024-byte framebuffer for its 128×64 monochrome display.

The graphics include:

- Moving artificial horizon.
- Sky and ground regions.
- Pitch ladder markings.
- Fixed aircraft reference symbol.
- Roll-angle indicator.

## Build Requirements

The development environment uses:

- ARM GNU Toolchain (`arm-none-eabi-gcc`)
- GNU Make
- OpenOCD
- ST-Link debugger
- CMSIS Core
- STM32F4 CMSIS device files
- Picocom for UART monitoring

## Getting Started

Clone the repository and its CMSIS submodules:

```bash
git clone --recurse-submodules https://github.com/manahil-w/stm32f401-artificial-horizon.git

cd stm32f401-artificial-horizon
```

If the submodules were not initialized:

```bash
git submodule update --init --recursive
```

### Compile

```bash
make clean
make
```

### Flash the microcontroller

Connect the ST-Link debugger and run:

```bash
make flash
```

The Makefile uses OpenOCD with the ST-Link interface.

### Monitor UART output

Connect the USB-to-UART adapter and run:

```bash
picocom -b 115200 /dev/ttyUSB0
```

The serial output reports initialization, calibration offsets, pitch, and roll.

## Calibration Notes

The sensor must remain completely stationary during gyroscope calibration.

The initial sensor orientation is used as the zero-pitch and zero-roll reference. Keep the hardware in its intended neutral position while the firmware starts.

## Current Status

- [x] Custom startup and linker configuration
- [x] System clock configuration
- [x] SysTick timing
- [x] Interrupt-driven UART
- [x] I2C1 peripheral and device detection
- [x] MPU-650
