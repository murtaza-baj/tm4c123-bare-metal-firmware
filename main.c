#include <stdint.h>
#include <stdlib.h>
#include "tm4c_reg_defs.h"
#include "system.h"
#include "gpio.h"
#include "uart.h"

volatile uint32_t timer_count = 0;
volatile uint8_t pb_flag = 0;

void init_hw(){
    system_init();
    uart0_init();
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
    if (GPIO_PORTF_MIS_REG & SW1){
        GPIO_PORTF_ICR_REG = SW1;
        if ((now - last) > 50){
            pb_flag = 1;
            last = now;
        }
    }
}

int main(void){
    init_hw();
    uart0_send_string("TM4C123 ready\r\n");
    uint16_t period = 1000;
    uint8_t out;
    while(1){
        if (pb_flag){
            if(period == 1000) period = 200;
            else period = 1000;
            pb_flag = 0;
        }
        uart0_send_string("Enter Command: ");
        out = uart0_receive_byte();
        uart0_send_string("\r\n");
        uart0_send_string("Byte received is: ");
        uart0_send_byte(out);
        uart0_send_string("\r\n");
        out = convert_lower_case(out);
        switch(out){
            case 'r':
                gpio_write(RED_LED, 1);
                uart0_send_string("RED ON\r\n");
                break;
            case 'g':
                gpio_write(GREEN_LED, 1);
                uart0_send_string("GREEN ON\r\n");
                break;
            case 'b':
                gpio_write(BLUE_LED, 1);
                uart0_send_string("BLUE ON\r\n");
                break;
            case 'x':
                gpio_write(ALL_LEDS, 0);
                uart0_send_string("ALL OFF\r\n");
                break;
            case 's':
                uart0_send_string("CLK:80MHz BAUD:115200 UPTIME: ");
                uart0_send_uint32_hex(timer_count);
                uart0_send_string("ms\r\n");
                break;
            default:
                uart0_send_string( "INVALID COMMAND\r\n");
        }
        wait(period);
    }
}
