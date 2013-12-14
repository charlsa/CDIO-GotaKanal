/*
 * 	GSM_module.c
 *
 *  Created on: Oct 17, 2013
 *  Author: Amanda
 */

#include <msp430.h>
#include "GSM_module.h"
#include "UART.h"
#include "Flash.h"


///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/

const char ATtest[] 		  = "AT\r";
const char ATecho[] 		  = "ATE0\r";
const char ATsetNetworkReg[]  = "AT+CREG=1\r";
const char ATshowNetworkReg[] = "AT+CREG?\r";
const char ATtextMode[] 	  = "AT+CMGF=1\r";

///////////////////////////////////////////////////////////////
/*AT Responses setup definitions*/

const char responseOK[] 			= "OK\r\n";

///////////////////////////////////////////////////////////////


/*
 * 	Sets direction for the Power on/off pin
*/
void pinGSM()
{
	P8DIR |= BIT2;
	P8DIR &= ~BIT4;
	P8OUT |= BIT3;
}

// Power on/off the GSM module
void pwrOnOff()
{
	P8OUT &= ~BIT2;
	int i = 0;
	while(i < 15)
	{
		Delay();
		i++;
	}
	P8OUT |= BIT2;
}

/*
 * Initiating the GSM module:
 * Turns off the echo mode
 * Sets the GSM module in text mode (used for SMS)
*/
void initGSM(void)
{
	sendATCommand(ATecho);		//Echo mode off
	uartEnable();				//Enable interrupt on UART
	checkOK();

	sendATCommand(ATtextMode);	//Text mode
	uartEnable();
	checkOK();
}
/*
 * Send AT command to Tx buffer
 * which sends it to the GSM module
*/
void sendATCommand(const char *command)
{
	int length = strlen(command);
	char c;
	while(length > 0 )
    {
    	length--;
        c = *(command++);
        uartSend(c);
    }
}
/*
 * Check if the GSM module has sent the whole message to CPU.
 * The GSM module ends the message with an "OK".
 * If the GSM sends "OK", uartRxBuf is ended and the interrupt disabled.
*/
void checkOK()
{
	int j = 0;
	while(id < strlen(responseOK))
	{
		Delay();
		j++;
		if (j > 30) break;
	}
	uartRxBuf[uartStart] = '\0';
	uartDisable();
	uartStart = 0;
	id = 0;
}

char checkAT()
{
	sendATCommand(ATtest);	//Test if GSM module can communicate.
	uartEnable();
	char c;
	int j = 0;
	while(j < 15)
	{
		Delay();
		if(id != 0) break;
		j++;
	}
	if(id != 0) c = '1';
	else c = '0';

	uartRxBuf[uartStart] = '\0';
	uartDisable();
	uartStart = 0;
	id = 0;
	return c;
}

