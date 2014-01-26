//Jacob Yacovelli
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"

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
#define loadCount 6
#define rest 560
#define fire 250
#define center 350
#define USB UART1

unsigned char temp;
int rld = 0;
int shots = loadCount;

enum {wait, play, reload} mode;

int main() {
    int ang = center;  //set pos of pan servo to center
    
    TRISDCLR = BIT_10; PORTDCLR = BIT_10; //set output for gun rotors and turn off

    //TRISFCLR = BIT_12;  //used for communicating to the tower
    //PORTFCLR = BIT_12;

    TRISGSET = BIT_6 | BIT_7; //make buttons inputs

    //Configure USB serial
    UARTConfigure(USB, UART_ENABLE_PINS_TX_RX_ONLY); //set the RX and TX pins
    UARTSetDataRate(USB, PERFCLK, 9600); //Pass in Peripheral Bus Clock and set BAUD rate for Cerebot
    UARTSetLineControl(USB, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1); //Align this information with realterm
    UARTEnable(USB, UART_ENABLE | UART_PERIPHERAL | UART_RX | UART_TX);

    OpenTimer2(T2_ON | T2_IDLE_CON | T2_SOURCE_INT | T2_PS_1_32 | T2_GATE_OFF, 6250); //10M/32/6250 = 100kHz
    OpenOC2(OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);  //pan servo
    //OpenOC3(OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0xFFFF);

    //OpenTimer3(T2_ON | T2_IDLE_CON | T2_SOURCE_INT | T2_PS_1_32 | T2_GATE_OFF, 6250);
    OpenOC4(OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0); //fire servo

    OpenTimer4(T2_ON | T2_IDLE_CON | T2_SOURCE_INT | T2_PS_1_256 | T2_GATE_OFF, 40000); //timer for servo reload

    INTClearFlag(INT_T4);             //timer for firing servo
    INTSetVectorPriority(INT_TIMER_4_VECTOR, INT_PRIORITY_LEVEL_5);
    INTEnable(INT_T4, INT_ENABLED);

    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR); //Use multi-vector INT's
    INTEnableInterrupts(); //Enable INT's                                                                 //clear timer2 INT (just in case)
    INTClearFlag(INT_T4);

    SetDCOC4PWM(rest);   //Set fire servo to rest possition
    mode = reload;       //set mode to reload

    while (1)
    {
        switch (mode)
        {
            case wait:
                SetDCOC2PWM(center);  //set position to center
                if(PORTG & BIT_7)  //button 2 for starting game
                {
                    mode = play;
                    while(!UARTTransmitterIsReady(USB));
                    UARTSendDataByte (USB, 'G');   //send go to kinect app
                    while(!UARTTransmissionHasCompleted(USB));
                }
                break;
            case play:
                if (UARTReceivedDataIsAvailable(USB)) //UART1 is the only one with USB
                {
                    temp = UARTGetDataByte(USB); //Get possition and shooting information
                    //PORTFCLR = BIT_12;
                    switch (temp) {
                        case 'L': // 'L' // Turn left
                            if(ang <= 550)
                                ang = ang + 10;
                            SetDCOC2PWM(ang);
                            break;
                        case 82: // 'R' // Turn Right
                            if(ang >= 280)
                                ang = ang - 10;
                            SetDCOC2PWM(ang);
                            break;
                        //case 85: // 'U' // Turn Up
                            //SetDCOC3PWM(250);
                            //break;
                        //case 68: // 'D' // Turn down
                            //SetDCOC3PWM(250);
                            //break;
                        case 83: // 'S' // Shoot! In crosshair.
                            //SetDCOC2PWM(0);
                            //SetDCOC3PWM(0);
                            if ((rld == 0) && (shots != 0)) {
                                SetDCOC4PWM(fire);
                                rld = 1;
                                shots--;
                                //PORTFSET = BIT_12;                 //Talk to tower
                                //while(!(PORTF & BIT_13));
                                //PORTFCLR = BIT_12;
                            }
                            if(shots == 0) //check if all shots have been fired
                            {
                                mode = reload;
                                PORTDCLR = BIT_10;  //if empty, turn off gun rotors
                                while(!UARTTransmitterIsReady(USB));
                                UARTSendDataByte (USB, 'H');          //send halt
                                while(!UARTTransmissionHasCompleted(USB));
                                //SetDCOC3PWM(0);
                            }
                            break;
                        case 78: // 'N' // Do nothing or go to center point. Out of boundary.
                            //SetDCOC2PWM(0);
                            //SetDCOC3PWM(0);
                            break;
                    }
                }
                break;
            case reload:
                if(PORTG & BIT_6) //wait for button one
                {
                    SetDCOC4PWM(rest);
                    mode = wait;
                    shots = loadCount;
                    PORTDSET = BIT_10; //turn on gun rotors
                }
                if (UARTReceivedDataIsAvailable(USB)) //UART1 is the only one with USB
                    temp = UARTGetDataByte(USB); //returns unsigned byte that was received
                break;
        }
    }
}

/*************************************************/
/* T4IntHandler()                                */
/* - ISR for for timer 4                         */
/*************************************************/
void __ISR(_TIMER_4_VECTOR, ipl5) T3IntHandler() {
    if (rld == 1)
        rld = 2;
    else if (rld == 2)
    {
        SetDCOC4PWM(rest);
        rld = 3;
    }
    else if (rld == 3)
        rld = 0;

    INTClearFlag(INT_T4); //clear T4 INT flag
}