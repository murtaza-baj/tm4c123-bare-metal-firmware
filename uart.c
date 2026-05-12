#include <stdint.h>
#include "tm4c_reg_defs.h"
#include "uart.h"

static const char hex[] = "0123456789ABCDEF";

void uart0_init(void){
    SYSCTL_RCGCUART_REG |= UART0;
    volatile int delay;
    delay = SYSCTL_RCGCUART_REG;
    (void)delay;
    //Initializing GPIO Port F
    SYSCTL_RCGCGPIO_REG |= SYSCTL_RCGC_GPIOA;           //UART0 TX and RX pins are on Port A
    delay = SYSCTL_RCGCGPIO_REG;
    (void)delay;

    GPIO_PORTA_AFSEL_REG = GPIO_PORTA_AFSEL_REG | GPIO_PORTA_PIN_0 | GPIO_PORTA_PIN_1;      //Enable RX on Port A Pin 0 and TX on Port A Pin 1
    GPIO_PORTA_DEN_REG = GPIO_PORTA_DEN_REG | GPIO_PORTA_PIN_0 | GPIO_PORTA_PIN_1;          //Enabling TX and RX
    GPIO_PORTA_PCTL_REG &= ~0xFF;
    GPIO_PORTA_PCTL_REG |= 0x11;                                                        //Enable alternate function - UART
    UART0_CTL_REG = 0;
    //BRD = 80000000/(16 * 115200) = 43.40278. IBRD = 43, FBRD = round(0.40278 × 64) = 26
    UART0_IBRD_REG = 43;
    UART0_FBRD_REG = 26;
    UART0_LCRH_REG = UART0_LCRH_WLEN | UART0_LCRH_FEN;                    //8N1 FIFO
    UART0_CC_REG = SYSTEM_CLK;
    UART0_CTL_REG = UART0_CTL_REG | UART0EN | UART0_TXE | UART0_RXE;
}

void uart0_send_byte(uint8_t byte){
    while(UART0_FR_REG & UART0_TXFF);
    UART0_DR_REG = (byte & UART_DATA_MASK);
}

void uart0_send_string(const char *s){
    if (!s) return;
    while(*s){
        uart0_send_byte(*s);
        s++;
    }
}

uint8_t uart0_receive_byte(void){
    while(UART0_FR_REG & UART0_RXFE);
    return UART0_DR_REG & UART_DATA_MASK;
}

void uart0_send_hex8(uint8_t val){
    uint8_t upper = (val & 0xF0) >> 4;
    uint8_t lower = (val & 0x0F);
    uart0_send_byte(hex[upper]);
    uart0_send_byte(hex[lower]);
}
/* TODO: move to utils.c in Phase 3 */
char convert_lower_case(char s){
    if (s >= 'A' && s <= 'Z') return s += 32;
    return s;
}

void uart0_send_uint32_hex(uint32_t val){
    for (int8_t i = 3; i >= 0; i--){
        uint8_t byte = (uint8_t)((val >> (i*8)) & 0xFF);
        uart0_send_hex8(byte);
    }
}
