#include"tm4c_reg_defs.h"
#include <stdint.h>
#include "gpio.h"

void gpio_init(void){
    //Initializing all GPIO registers to use APB
    SYSCTL_GPIOHBCTL_REG = 0;

    //Initializing GPIO Port F
    SYSCTL_RCGCGPIO_REG |= SYSCTL_RCGC_GPIOF;
    volatile int delay = SYSCTL_RCGCGPIO_REG;
    (void)delay;

    //Initializing Port F registers to enable the Red LED at PF1
    GPIO_PORTF_DIR_REG |= GPIO_PORTF_PIN_1;                //Setting PF1 as output
    GPIO_PORTF_AFSEL_REG &= ~GPIO_PORTF_PIN_1;
    GPIO_PORTF_DR2R_REG |= GPIO_PORTF_PIN_1;               //Setting drive strength of PF1 to 2mA
    GPIO_PORTF_DEN_REG |= GPIO_PORTF_PIN_1;                //Enabling LED

    //Initializing Port F registers to enable the Blue LED at PF2
    GPIO_PORTF_DIR_REG |= GPIO_PORTF_PIN_2;                //Setting PF2 as output
    GPIO_PORTF_AFSEL_REG &= ~GPIO_PORTF_PIN_2;
    GPIO_PORTF_DR2R_REG |= GPIO_PORTF_PIN_2;               //Setting drive strength of PF2 to 2mA
    GPIO_PORTF_DEN_REG |= GPIO_PORTF_PIN_2;                //Enabling LED

    //Initializing Port F registers to enable the Green LED at PF3
    GPIO_PORTF_DIR_REG |= GPIO_PORTF_PIN_3;                //Setting PF3 as output
    GPIO_PORTF_AFSEL_REG &= ~GPIO_PORTF_PIN_3;
    GPIO_PORTF_DR2R_REG |= GPIO_PORTF_PIN_3;               //Setting drive strength of PF3 to 2mA
    GPIO_PORTF_DEN_REG |= GPIO_PORTF_PIN_3;                //Enabling LED

    //Initializing Port F registers to enable the SW1 as input at PF4
    GPIO_PORTF_DIR_REG &= ~GPIO_PORTF_PIN_4;                //Setting PF4 as input
    GPIO_PORTF_AFSEL_REG &= ~GPIO_PORTF_PIN_4;
    GPIO_PORTF_PUR_REG |= GPIO_PORTF_PIN_4;
    GPIO_PORTF_DEN_REG |= GPIO_PORTF_PIN_4;                //Enabling SW

    //Configure Interrupt for SW1 at PF4
    GPIO_PORTF_IM_REG &= ~GPIO_PORTF_PIN_4;
    GPIO_PORTF_IS_REG &= ~GPIO_PORTF_PIN_4;
    GPIO_PORTF_IBE_REG &= ~GPIO_PORTF_PIN_4;
    GPIO_PORTF_IEV_REG &= ~GPIO_PORTF_PIN_4;
    GPIO_PORTF_ICR_REG |= GPIO_PORTF_PIN_4;
    GPIO_PORTF_IM_REG |= GPIO_PORTF_PIN_4;
    NVIC_EN0_REG |= GPIO_PORTF_IRQ;
}

void gpio_write(uint32_t pin_mask, uint8_t val){
    if (val) GPIO_PORTF_DATA_REG = (GPIO_PORTF_DATA_REG & ~ALL_LEDS) | pin_mask;
    else GPIO_PORTF_DATA_REG &= ~pin_mask;
}
