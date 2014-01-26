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

int main() {
    unsigned char temp = 0;
    int i = 0;

    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY); //set the RX and TX pins
    UARTSetDataRate(UART1, PERFCLK, 9600); //setup UART1 at 9600 baud
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1); //use 8 data bits, no parity, and one stop bit
    UARTEnable(UART1, UART_ENABLE | UART_PERIPHERAL | UART_RX | UART_TX); //enable UART1

    TRISGSET = BIT_6;
    TRISDSET = BIT_9;
    TRISDCLR = BIT_1 | BIT_2;
    PORTDCLR = BIT_1 | BIT_2;

    TRISGCLR = BIT_12 | BIT_13;
    PORTGCLR = BIT_12 | BIT_13;

    while (1) {

        while (!(PORTD & BIT_9));
        PORTDSET = BIT_0;
        PORTGSET = BIT_13;
        PORTDCLR = BIT_0;
        if (UARTTransmitterIsReady(UART1)) //returns true if transmitter is ready
        {
            UARTSendDataByte(UART1, 'F'); //Sends the byte stored in temp to realterm
            while (!UARTTransmissionHasCompleted(UART1)); //Do nothing until complete
        }
        for (i = 0; i < 1000; i++);

        while (!UARTReceivedDataIsAvailable(UART1));
        temp = UARTGetDataByte(UART1);
        if (temp == 'M') {
            PORTGSET = BIT_12;
            PORTDSET = BIT_1;
            PORTDCLR = BIT_2;
        }
        else if (temp == 'H') {
            PORTGCLR = BIT_12;
            PORTDSET = BIT_2;
            PORTDCLR = BIT_1;
        }
        PORTGCLR = BIT_13;

        temp = 0;
        for(i=0; i<10000; i++);
    }

    return (EXIT_SUCCESS);
}

