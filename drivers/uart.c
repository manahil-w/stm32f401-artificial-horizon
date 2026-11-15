#include "uart.h"
#include "stm32f4xx.h"

#define UART_APB1_HZ  42000000U
#define UART_BAUD     115200U
#define TX_BUF_SIZE   128U

static uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint16_t tx_head = 0U;
static volatile uint16_t tx_tail = 0U;

void uart_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3)) |
                   (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);

    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~((0xFU << 8) | (0xFU << 12))) |
                    (7U << 8) | (7U << 12);

    USART2->BRR = (UART_APB1_HZ + (UART_BAUD / 2U)) / UART_BAUD;

    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;

    NVIC_SetPriority(USART2_IRQn, 2);
    NVIC_EnableIRQ(USART2_IRQn);
}

void uart_putc(char c)
{
    uint16_t next_head = (tx_head + 1U) & (TX_BUF_SIZE - 1U);

    while (next_head == tx_tail) {
        __NOP();
    }

    tx_buf[tx_head] = (uint8_t)c;
    __DMB();
    tx_head = next_head;

    USART2->CR1 |= USART_CR1_TXEIE;
}

void uart_write(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

void USART2_IRQHandler(void)
{
    if (((USART2->CR1 & USART_CR1_TXEIE) != 0U) &&
        ((USART2->SR  & USART_SR_TXE)    != 0U)) {

        if (tx_head != tx_tail) {
            USART2->DR = tx_buf[tx_tail];
            tx_tail = (tx_tail + 1U) & (TX_BUF_SIZE - 1U);
            if (tx_head == tx_tail) {
                USART2->CR1 &= ~USART_CR1_TXEIE;
            }
        } 
        else {
            USART2->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}
