#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"

int main(void)
{
    uint32_t hz = (clock_init_84mhz() == 0)
                  ? 84000000U : 16000000U;

    systick_init(hz);
    uart_init();

    uart_write("START\r\n");
    char test[301];

    for (int i = 0; i < 300; i++) {
        test[i] = 'A';
    }

    test[300] = '\0';

    uart_write(test);
    uart_write("\r\n");
    while (1) {
        uart_write("HELLO\r\n");
        delay_ms(1000);
    }
}
