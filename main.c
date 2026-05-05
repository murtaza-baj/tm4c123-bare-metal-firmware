#include <stdint.h>
#include <stdlib.h>
#include "tm4c_reg_defs.h"
#include "system.h"
#include "gpio.h"

volatile uint32_t timer_count = 0;
volatile uint8_t pb_flag = 0;
void init_hw(){
    system_init();
    gpio_init();
}
void systick_ISR(void){
    timer_count += 1;
}
uint32_t get_time_ms(){
    return timer_count;
}
void wait(uint32_t val){
    uint32_t current = get_time_ms();
    while((get_time_ms() - current) < val);
}
void pb_ISR(void){
    static uint32_t last = 0;
    uint32_t now = get_time_ms();
    if (GPIO_PORTF_MIS_REG & GPIO_PORTF_PIN_4){
        GPIO_PORTF_ICR_REG = GPIO_PORTF_PIN_4;
        if ((now - last) > 50){
            pb_flag = 1;
            last = now;
        }
    }
}

int main(void){
    init_hw();
    uint16_t period = 1000;
    while(1){
        if (pb_flag){
            if(period == 1000) period = 200;
            else period = 1000;
            pb_flag = 0;
        }
        gpio_write(RED_LED, 1);
        wait(period);
        gpio_write(BLUE_LED, 1);
        wait(period);
        gpio_write(GREEN_LED, 1);
        wait(period);
        gpio_write(ALL_LEDS, 0);
        wait(period);
    }
}
