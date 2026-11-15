#include <stdint.h>
#include "stm32f4xx.h"          /* for SCB, __DSB, __ISB */

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);

void Reset_Handler(void);
void Default_Handler(void) { while (1) {} }

// weak aliases 
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector"), used))
void (* const vector_table[55])(void) = {
    [0] = (void (*)(void))&_estack,   //Initial Stack Pointer 
    [1] = Reset_Handler,              // 1:  Reset                
    [2] = NMI_Handler,                // 2:  NMI                   
    [3] = HardFault_Handler,          // 3:  Hard Fault            
    [4] = MemManage_Handler,          // 4:  MPU Fault             
    [5] = BusFault_Handler,           // 5:  Bus Fault             
    [6] = UsageFault_Handler,         // 6:  Usage Fault           
    [7] = 0,                          // 7:  Reserved             
    [8] = 0,                          // 8:  Reserved             
    [9] = 0,                          // 9:  Reserved             
    [10] = 0,                         // 10: Reserved             
    [11] = SVC_Handler,               // 11: SVCall                
    [12] = DebugMon_Handler,          // 12: Debug Monitor         
    [13] = 0,                         // 13: Reserved             
    [14] = PendSV_Handler,            // 14: PendSV                
    [15] = SysTick_Handler,           // 15: SysTick   
    [54]= USART2_IRQHandler,           // 54: USART2                
               
};

void Reset_Handler(void)
{
    /* TODO A: copy .data */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* TODO B: zero .bss */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* TODO C: enable FPU + barriers */
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); //Full access to CP10 and CP11 
    __DSB();
    __ISB();

    main();
    while (1) {}
}
