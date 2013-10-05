/*
 * GSMmodul.c
 *
 *  Created on: 5 okt 2013
 *      Author: Challe
 */

#include "GSMmodul.h"

// Global variabels for testing UART
unsigned char send = 'e';
unsigned char trans = 'v';

void initUART() {
    P3SEL = BIT4+BIT5;  	// Pin selection for UART1 communication @ Pin 3.4 (TX) & Pin 3.5 (RX)

    UCA0CTL1 |= UCMSB;  	// Datapackage for UART-mode with [Parity off, MSB first, STOP bit = 1]

    UCA0CTL1|= UCSSEL1; 	//Select boudrate CLK sourse to SCMLK 8 MHz, UCSWRST is set by default

    UCA0BR0 = 0x34;        	//9600 baudrate, values from tabel 1.5 withe 8 MHz clk
    UCA0BR1 = 0x00; 		// BR_value = 52

//    UCA1STAT |= 0x80;		// Loopfeed back enabel for debuging

    UCA0MCTL |= UCOS16;		// Oversampling mode selected if changed the boudrate needs to be changed

    UCA0CTL1 &= ~UCSWRST;	// Enables the UART
}

void triggLED(){
	/*
	 * Trig the LED for debugging
	 */
	P1OUT ^= BIT0;
	__delay_cycles(80000);
	P1OUT ^= BIT0;
}

void deBuggSetup(){
	// debuging button with interrupt
	P1DIR |= BIT0; 		// test led2

	P2DIR |= BIT6;
	P2OUT |= BIT6;		// Set pullup for button

	// Interrupt button
    P2IFG &= ~BIT6;		// Clear interruptflag
    P2IE |= BIT6;
    P2IES |= BIT6; 		// select interrupt edge rising
}

void transmittUART(){
	while (!(UCA0IFG & UCTXIFG)); 	// USCI0 TX buffer ready
	UCA0TXBUF = send;				// Wright to TX buffer
}

void reciveUART(){
	trans = UCA0RXBUF; 				// read from RX buffer
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	__delay_cycles(80000);
	triggLED();
	transmittUART();
	__delay_cycles(1000);
	reciveUART();
	P2IFG &= ~BIT6;				// Clear interruptflag
}

