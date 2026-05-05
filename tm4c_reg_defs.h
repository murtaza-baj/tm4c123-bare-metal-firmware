#ifndef TM4C_REG_DEFS_H_
#define TM4C_REG_DEFS_H_

#define SYSCTL_RCC_REG          (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC2_REG         (*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RCC2_USERCC2     (1U << 31)  // Use RCC2
#define SYSCTL_RCC_BYPASS      (1U << 11)  // RCC PLL bypass
#define SYSCTL_RCC2_BYPASS2     (1U << 11)  // RCC2 PLL bypass
#define SYSCTL_RCC_USESYSDIV    (1U << 22)  // Enable System Clock Divider
#define SYSCTL_RCC2_DIV400      (1U << 30)  // Divide PLL as 400 MHz vs. 200
#define SYSCTL_RCC_XTAL_MASK    (0x1F << 6)
#define SYSCTL_RCC_XTAL_16MHZ   (0X15 << 6)  // 16 MHz
#define SYSCTL_RCC2_OSCSRC2_OSCSRC2_MASK  (0x7 << 4)  // MOSC
#define SYSCTL_RCC2_PWRDN2      (1U << 13)  // Power-Down PLL 2
#define SYSCTL_RCC2_SYSDIV_MASK (0x7F << 22)
#define SYSCTL_RCC2_SYSDIV_80MHz     (4 << 22) // System Clock Divisor
#define SYSCTL_RIS_REG          (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RIS_PLLLRIS      (1U << 6)  // PLL Lock Raw Interrupt Status
#define SYSCTL_GPIOHBCTL_REG    (*((volatile unsigned long *)0x400FE06C))
#define SYSCTL_RCGCGPIO_REG     (*((volatile unsigned long *)0x400FE608))
#define GPIO_PORTA_AFSEL_REG    (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_PCTL_REG     (*((volatile unsigned long *)0x4000452C))
#define SYSCTL_RCGC_GPIOF       (1U << 5)  // Port F Clock Gating Control
#define SYSCTL_RCGC_GPIOA       (1U << 0)  // Port F Clock Gating Control
#define GPIO_PORTA_PIN_0        (1U << 0)
#define GPIO_PORTA_PIN_1        (1U << 1)
#define GPIO_PORTF_DIR_REG      (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_PIN_1        (1U << 1)
#define GPIO_PORTF_DR2R_REG     (*((volatile unsigned long *)0x40025500))
#define GPIO_PORTF_DEN_REG      (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AFSEL_REG    (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PIN_2        (1U << 2)
#define GPIO_PORTF_PIN_3        (1U << 3)
#define GPIO_PORTF_PIN_4        (1U << 4)
#define GPIO_PORTF_DATA_REG     (*((volatile unsigned long *)0x400253FC))
#define NVIC_ST_CTRL_REG        (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_REG      (*((volatile unsigned long *)0xE000E014))
#define SYSCTL_SYSDIV_5         0x4
#define GPIO_PORTF_IM_REG       (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_IS_REG       (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_REG      (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_REG      (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_PUR_REG      (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_MIS_REG      (*((volatile unsigned long *)0x40025418))
#define GPIO_PORTF_ICR_REG      (*((volatile unsigned long *)0x4002541C))
#define NVIC_EN0_REG            (*((volatile unsigned long *)0xE000E100))
#define ALL_LEDS                (GPIO_PORTF_PIN_1 | GPIO_PORTF_PIN_2 | GPIO_PORTF_PIN_3)
#define SYSCTL_RCGCUART_REG     (*((volatile unsigned long *)0x400FE618))
#define UART0                   (1U << 0)
#define NVIC_ST_CTRL_CLK_SRC    (1U << 2)  // Clock Source
#define NVIC_ST_CTRL_INTEN      (1U << 1)  // Interrupt Enable
#define NVIC_ST_CTRL_ENABLE     (1U << 0)  // Enable
#define GPIO_PORTF_IRQ          (1U << 30) // GPIO Port F = IRQ 30
#define RED_LED                 GPIO_PORTF_PIN_1
#define BLUE_LED                GPIO_PORTF_PIN_2
#define GREEN_LED               GPIO_PORTF_PIN_3
#define SW1                     GPIO_PORTF_PIN_4
#define UART0EN                 (1U << 0)
#define UART0_CTL_REG           (*((volatile unsigned long *)0x4000C030))
#define UART0_IBRD_REG          (*((volatile unsigned long *)0x4000C024))
#define UART0_FBRD_REG          (*((volatile unsigned long *)0x4000C028))
#define UART0_LCRH_REG          (*((volatile unsigned long *)0x4000C02C))
#define UART0_LCRH_WLEN         (0x3 << 5)
#define UART0_LCRH_FEN          (1U << 4)
#define UART0_TXE               (1U << 8)
#define UART0_RXE               (1U << 9)
#define UART0_FR_REG            (*((volatile unsigned long *)0x4000C018))
#define UART0_TXFF              (1U << 5)
#define UART0_RXFE              (1U << 4)
#define UART0_DR_REG            (*((volatile unsigned long *)0x4000C000))
#define GPIO_PORTA_DEN_REG      (*((volatile unsigned long *)0x4000451C))
#define UART_DATA_MASK          0xFF
#define UART0_CC_REG            (*((volatile unsigned long *)0x4000CFC8))
#define SYSTEM_CLK              0x0

#endif /* TM4C_REG_DEFS_H_ */
