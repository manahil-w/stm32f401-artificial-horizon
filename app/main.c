#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
int main(void)
{
    uint32_t hz = (clock_init_84mhz() == 0) ? 84000000U : 16000000U;
    systick_init(hz);

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER &= ~(3UL << (13 * 2));
    GPIOC->MODER |=  (1UL << (13 * 2));

    while (1)
    {
        GPIOC->ODR ^= (1UL << 13);
        delay_ms(500);
    }
}