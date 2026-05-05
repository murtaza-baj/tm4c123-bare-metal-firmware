#include"tm4c_reg_defs.h"
#include "system.h"

void system_init(void){

    SYSCTL_RCC_REG = (SYSCTL_RCC_REG |SYSCTL_RCC_BYPASS) & ~SYSCTL_RCC_USESYSDIV;
    SYSCTL_RCC2_REG = SYSCTL_RCC2_REG |SYSCTL_RCC2_USERCC2 | SYSCTL_RCC2_BYPASS2;
    SYSCTL_RCC_REG &= ~SYSCTL_RCC_XTAL_MASK;
    SYSCTL_RCC_REG |= SYSCTL_RCC_XTAL_16MHZ;     //Select Crystal value ->XTAL 16 MHz
    SYSCTL_RCC2_REG = (SYSCTL_RCC2_REG & ~SYSCTL_RCC2_OSCSRC2_MASK) & ~SYSCTL_RCC2_PWRDN2;
    SYSCTL_RCC2_REG |= SYSCTL_RCC2_DIV400;
    SYSCTL_RCC2_REG &= ~SYSCTL_RCC2_SYSDIV_MASK;
    SYSCTL_RCC2_REG |= SYSCTL_RCC2_SYSDIV_80MHz;
    SYSCTL_RCC_REG |= SYSCTL_RCC_USESYSDIV;
    while ((SYSCTL_RIS_REG & SYSCTL_RIS_PLLLRIS) == 0);   //Blocking function to wait for PLL to lock by polling PLLLRIS in Raw Interrupt Status (RIS) register
    SYSCTL_RCC2_REG &= (~SYSCTL_RCC2_BYPASS2);                //Enable use of PLL by clearing BYPASS bit in RCC

    //Configure SYSTICK
    NVIC_ST_CTRL_REG = NVIC_ST_CTRL_REG | NVIC_ST_CTRL_ENABLE | NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_CLK_SRC;                                                      //Initializing the SYSTICK Timer's interrupt
    NVIC_ST_RELOAD_REG = 79999;                                                  //Reload value for the interrupt to go off every 1ms
}

