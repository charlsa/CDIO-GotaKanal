/*
 * GSM_module.c
 *
 *  Created on: Oct 9, 2013
 *      Author: Administrator
 */



#include <msp430.h>
#include "GSM_module.h"
#include "UART.h"
#include "LCD.h"
#include <string.h>


///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/
const unsigned char AT_Test[]		 = "AT\r";
const unsigned char AT_ConfSMS[] 	 = "AT+CMGF=1\r";
const unsigned char AT_ConfJakob[] 	 = "AT+CMGS=\"+46736525723\"\r";

///////////////////////////////////////////////////////////////

// Initiating the GSM module
void initGSM(void)
{
	sendATCommand(strlen(AT_Test),AT_Test);
	Delay();
	sendATCommand(strlen(AT_ConfSMS),AT_ConfSMS);
	sendATCommand(strlen(AT_ConfJakob),AT_ConfJakob); //GET EEPROM FUnction to be created!
}


// Sends AT command to Tx buffer
void sendATCommand(int length, const unsigned char* command)
{
	char com;
	while((length) > 0 )
	{
		length--;
		com=*(command++);
		uart_send(length, com);
	}
}

//Use this to end your message to the GSM module
void sendCTRL_Z(void)
{
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF=26;			// ASCII number for Ctrl+Z
}

/*
void CompareResponse()
{
	while(strcmp(uartRxBuf,) ) //// Skriv en funktion har som hamtar in vardet den bor se, beroende pa vad som skickats
}
*/
