
TARGET   = build/firmware
LDSCRIPT = linker/stm32f401.ld

SRC = $(wildcard startup/*.c) \
      $(wildcard drivers/*.c) \
      $(wildcard app/*.c)

OBJ = $(SRC:%.c=build/%.o)

CC   = arm-none-eabi-gcc
SIZE = arm-none-eabi-size

CPU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

INC = -Ithird_party/cmsis_5/CMSIS/Core/Include \
      -Ithird_party/cmsis_device_f4/Include \
      -Idrivers

CFLAGS = $(CPU) -DSTM32F401xE $(INC) \
         -Wall -Wextra -Og -g3 \
         -ffunction-sections -fdata-sections \
         -ffreestanding -MMD -MP

LDFLAGS = $(CPU) -T$(LDSCRIPT) \
          -nostartfiles \
          -specs=nano.specs \
          -specs=nosys.specs \
          -Wl,--gc-sections \
          -Wl,-Map=$(TARGET).map

all: $(TARGET).elf

$(TARGET).elf: $(OBJ) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) $(LDFLAGS) -lm -o $@
	$(SIZE) $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	  -c "program $< verify reset exit"

clean:
	rm -rf build

-include $(OBJ:.o=.d)

.PHONY: all flash clean
