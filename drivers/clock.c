#include "clock.h"
#include "stm32f4xx.h"


#define HSE_STARTUP_TIMEOUT 50000U
#define PLL_STARTUP_TIMEOUT 50000U

#define PLL_M  25U
#define PLL_N  336U
#define PLL_P  1U    
#define PLL_Q  7U

int clock_init_84mhz(void)
{
    uint32_t timeout;

    RCC->CR |= RCC_CR_HSEON;
    timeout = HSE_STARTUP_TIMEOUT;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {
        if (--timeout == 0U) {
            return -1;     
        }
    }


    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR = (PWR->CR & ~PWR_CR_VOS) | PWR_CR_VOS_1;


    FLASH->ACR = FLASH_ACR_LATENCY_2WS |
                 FLASH_ACR_PRFTEN      |
                 FLASH_ACR_ICEN        |
                 FLASH_ACR_DCEN;

    RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_Pos) |
                   (PLL_N << RCC_PLLCFGR_PLLN_Pos) |
                   (PLL_P << RCC_PLLCFGR_PLLP_Pos) |
                   RCC_PLLCFGR_PLLSRC_HSE          |
                   (PLL_Q << RCC_PLLCFGR_PLLQ_Pos);


    RCC->CR |= RCC_CR_PLLON;
    timeout = PLL_STARTUP_TIMEOUT;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
        if (--timeout == 0U) {
            RCC->CR &= ~RCC_CR_PLLON;   
            return -1;
        }
    }

    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |
                  RCC_CFGR_PPRE1_DIV2 |
                  RCC_CFGR_PPRE2_DIV1);


    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    timeout = PLL_STARTUP_TIMEOUT;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
        if (--timeout == 0U) {
            return -1;
        }
    }

    return 0;
}