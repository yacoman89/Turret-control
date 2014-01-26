#include <plib.h>
#include <stdio.h>
#include <stdlib.h>

// Cerebot board configuration
#pragma config ICESEL       = ICS_PGx1  // ICE/ICD Comm Channel Select
#pragma config DEBUG        = OFF       // Debugger Disabled for Starter Kit
#pragma config FNOSC        = PRIPLL	// Oscillator selection
#pragma config POSCMOD      = XT	// Primary oscillator mode
#pragma config FPLLIDIV     = DIV_2	// PLL input divider
#pragma config FPLLMUL      = MUL_20	// PLL multiplier
#pragma config FPLLODIV     = DIV_1	// PLL output divider
#pragma config FPBDIV       = DIV_8	// Peripheral bus clock divider
#pragma config FSOSCEN      = OFF	// Secondary oscillator enable

#define PERFCLK 10000000

int main()
{
    int val = 450;
    int i;

    TRISDCLR = BIT_9;
    PORTDCLR = BIT_9;

    TRISDCLR = BIT_1 | BIT_2;
    PORTDCLR = BIT_1 | BIT_2;

    TRISGSET = BIT_6 | BIT_7;

    //OpenTimer2(T2_ON | T2_IDLE_CON | T2_SOURCE_INT | T2_PS_1_32 | T2_GATE_OFF, 6250); //10M/1/500 = 100kHz
    //OpenOC2(OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);

    while(1)
    {
        if(PORTG & BIT_6)
        {
            PORTDSET = BIT_1;
            PORTDCLR = BIT_2;
        }
        if(PORTG & BIT_7)
        {
            PORTDCLR = BIT_1;
            PORTDSET = BIT_2;
        }
    }
    
    return (EXIT_SUCCESS);
}

