#include "uart.h"
#include "stm32f4xx.h"

#define UART_APB1_HZ  42000000U
#define UART_BAUD     115200U

void uart_init(void)
{
  
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
  
    
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3)) | (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
 
    
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~((0xFU << 8) | (0xFU << 12))) | (7U << 8) | (7U << 12);  
\
    USART2->BRR = (UART_APB1_HZ + UART_BAUD / 2) / UART_BAUD;

    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
}

void uart_putc(char c)
{
    /* wait for USART_SR_TXE in USART2->SR, then USART2->DR = c */
    while (!(USART2->SR & USART_SR_TXE)) {}
    USART2->DR = c;
}

void uart_write(const char *s)
{
    /* loop calling uart_putc until '\0' */
    while (*s) {
        uart_putc(*s++);
    }
}