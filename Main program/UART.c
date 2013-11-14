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
	P5SEL |= BIT6+BIT7;				// Choose bit 3.4 (TX) and 3.5 (RX)

    UCA1CTL1|= UCSWRST;             //Have to set this flag to be able to initiate & modify other registers UCSWRST=1 NOW.
    UCA1CTL0 &= ~UCSYNC;            // 0 UART mode selected contrary to SPI mode

    UCA1CTL1 |= 0xC0;             //CLK=SMCK chosen

    UCA1BR0 = 0x34;                 //19200 baud rate for GPRS Sim900 setup
    UCA1BR1 = 0x00;
    UCA1MCTL = 0;              //Set modulation control register to 0x04 and =8mhz 19200bds

    UCA1CTL1 &= ~UCSWRST;
}


//Delay that satisfy GSM module in certain configuration
void Delay(void)
{
	__delay_cycles (100000);
}

void uartEnable(void)
{
	UCA1IE |= UCRXIE;
}


// Disable Tx/Rx interrupts
void uartDisable(void)
{
	UCA1IE &= ~UCRXIE;
}


// Puts a vector in the output buffer and starts the sending process
void uartSend(int len, char vec)
{
	while(!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = vec;
}


// Read Everything from RX that service routine says it to do.
void uartRead(char a)
{
    uartRxBuf[uartStart++] = a;

    if(ATResponseOK[id] == a)
    	id++;
}

char uart_is_tx_clear(void)
{
	return UCA1IFG &= UCTXIFG; 				// Dummy function that might be interesting but I gonna fix this with UCA1IFG &= UCTXIFG
}


