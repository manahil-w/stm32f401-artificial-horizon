#include "systick.h"
#include "stm32f4xx.h"

static volatile uint32_t ms_ticks;

void systick_init(uint32_t sysclk_hz)
{
    SysTick->LOAD = (sysclk_hz / 1000) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
    ms_ticks++;
}

uint32_t millis(void)
{
    return ms_ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((uint32_t)(millis() - start) < ms) { }
}