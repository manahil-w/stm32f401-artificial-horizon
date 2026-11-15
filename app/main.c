#include "stm32f4xx.h"

int main(void)
{
    /* 1. Enable the GPIOC peripheral clock (bit 2 of AHB1ENR) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    /* 2. Set PC13 to output mode (01) */
    GPIOC->MODER &= ~(3UL << (13 * 2));
    GPIOC->MODER |=  (1UL << (13 * 2));

    /* 3. Set PC13 output type to push-pull (0) */
    while (1)
{
    for (int n = 0; n < 4; n++) {              /* 4 toggles = 2 full blinks */
        GPIOC->ODR ^= (1UL << 13);
        for (volatile uint32_t i = 0; i < 100000; i++) { }
    }
    for (volatile uint32_t i = 0; i < 2000000; i++) { }   /* long pause */
}
}