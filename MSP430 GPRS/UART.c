/*
 * UART.c
 *
 *  Created on: Oct 9, 2013
 *      Author: Administrator
 */

#include <msp430.h>
#include "UART.h"
#include <string.h>

char uartRxBuf[UART_RX_BUF_LEN];
int uartStart = 0;

#include "GSM_module.h"

void initUART(void) {
				// Stop watchdog timer

	//GPRS initiering
    P3SEL |= BIT4+BIT5; 						// Chosen bit 5.6 and 5.7 @port 56 and 57

    UCA0CTL1|= UCSWRST; 					//Have to set ths flag to be able to initiate&modify other registers UCSWRST=1 NOW.
    UCA0CTL0 &= ~UCSYNC; 					// 0 UART mode selected contrary to SPI mode


    UCA0CTL1 |=UCSSEL1;  					//CLK=SMCK chosen
    //UCA0BR0 = 0x41;                         //9600 baudrate
    //UCA0BR1 = 0x3;

    UCA0BR0 = 0xA0;                         //19200 baudrate for GPRS Sim900 setup
    UCA0BR1 = 0x01;

   // UCA1STAT|=UCLISTEN;						//Internal feedback TX to RX
    UCA0MCTL|=UCBRS_6; 						//Set modulation control register to 0x04 and =8mhz 9600bds
    UCA0CTL1 &= ~UCSWRST;
    uart_enable();
}


//Delay that satisfy GSM module in certain configuration
void Delay(void)
{
	__delay_cycles (1000000);
}
void uart_enable(void)
{
	UCA0IE |= (UCTXIE|UCRXIE);
}


// Disable Tx/Rx interrupts
void uart_disable(void)
{
  UCA1IE &= ~(UCTXIE|UCRXIE);
}


// Puts a vector in the output buffer and starts the sending process
void uart_send(int len, char vec)
{
    	while(!(UCA0IFG & UCTXIFG));
    		UCA0TXBUF = vec;
}


// Read Everything from RX that service routine says it to do. TEST!
void uart_read(char a)
{
	uartRxBuf[uartStart++]=a;
}

char uart_is_tx_clear(void)
{
  return UCA1IFG &= UCTXIFG; // Dummy function that might be interesting but I gonna fix this with UCA1IFG &= UCTXIFG
}

void clockSetup(){
	/*
	 * Need to understand more about this...c
	 *
	 */
    //P2DIR == 0x00;	 // for pulse generating

    P4DIR |= 0x80;							// P4.7 SMCLK
    P4SEL |= 0x80;						    // P4.7 for debugging freq.
	UCSCTL3 |= SELREF_2;                    // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;                      // Set ACLK = REFO
	__bis_SR_register(SCG0);                // Disable the FLL control loop
	UCSCTL0 = 0x0000;                       // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                    // Select DCO range 16MHz operation
	UCSCTL2 = FLLD_1 + 244;                 // Set DCO Multiplier for 8MHz
	                                        // (N + 1) * FLLRef = Fdco
											// (244 + 1) * 32768 = 8MHz
											// Set FLL Div = fDCOCLK/2
	__bic_SR_register(SCG0);                // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
}
