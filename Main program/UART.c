/*
 * UART.c
 *
 *  Created on: Oct 17, 2013
 *  Author: Amanda
 */

#include <msp430.h>
#include "UART.h"
#include <string.h>

int uartStart = 0;
int id = 0;
char uartRxBuf[UART_RX_BUF_LEN];

#include "GSM_module.h"

void initUART(void)
{
	P9SEL |= BIT4+BIT5;				// Choose bit 3.4 (TX) and 3.5 (RX)

    UCA2CTL1|= UCSWRST;             //Have to set this flag to be able to initiate & modify other registers UCSWRST=1 NOW.
    UCA2CTL0 &= ~UCSYNC;            // 0 UART mode selected contrary to SPI mode

    UCA2CTL1 |= 0xC0;             //CLK=SMCK chosen

    UCA2BR0 = 0x34;                 //19200 baud rate for GPRS Sim900 setup
    UCA2BR1 = 0x00;
    UCA2MCTL = 0;              //Set modulation control register to 0x04 and =1mhz 19200bds

    UCA2CTL1 &= ~UCSWRST;
}


//Delay that satisfy GSM module in certain configuration
void Delay(void)
{
	int i = 0;

	while(i < 4)
	{
		__delay_cycles (100000);
		i++;
	}
}

void uartEnable(void)
{
	UCA2IE |= UCRXIE;
}

// Disable Tx/Rx interrupts
void uartDisable()
{
	UCA2IE &= ~UCRXIE;
}


// Puts a vector in the output buffer and starts the sending process
void uartSend(char vec)
{
	while(!(UCA2IFG & UCTXIFG));
    UCA2TXBUF = vec;
}


// Read Everything from RX that service routine says it to do.
void uartRead(char a)
{
    uartRxBuf[uartStart++] = a;

    if(responseOK[id] == a)
    	id++;
}

char uart_is_tx_clear(void)
{
	return UCA2IFG &= UCTXIFG; 				// Dummy function that might be interesting but I gonna fix this with UCA1IFG &= UCTXIFG
}

#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void)
{
	 switch(UCA2IV)
	 {
	    case 2:                   	// Vector 2 - RXIF
	    	  uartRead(UCA2RXBUF);
	    	  UCA2IFG &= ~UCRXIFG;
	        break;
	    default: break;
	 }
}
