/*
 * 	GSM_module.c
 *
 *  Created on: Oct 17, 2013
 *  Author: Amanda
 */

#include <msp430.h>
#include "GSM_module.h"
#include "UART.h"


#define SIZECommand 	6
#define SIZEResponse 	2
int initCount=0;

///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/

const char ATtest[] = "AT\r";
const char ATecho[] = "ATE0\r";
const char ATtextMode[] = "AT+CMGF=1\r";
const char ATsetPhoneNumber[] = "AT+CMGS=\"+46735082283\"\r";
const char ATsetSmsStorage[] = "AT+CPMS=\"SM\"\r";
const char ATreadUnreadSms[] = "AT+CMGL=\"REC UNREAD\"\r";
const char ATdeleteAllSms[] = "AT+CMGDA=\"DEL ALL\"\r";


///////////////////////////////////////////////////////////////
/*AT Responses setup definitions*/

const char ATResponseOK[] = "OK\r\n";

///////////////////////////////////////////////////////////////

char stopAlarm[2];
char statusReq[6];
char newNumber[19];
char newTolerance[16];

char stopAlarmResp[]="OK";
char statusReqResp[]="STATUS";
char newNumberResp[]="New nr:";
char newToleranceResp[]="New tolerance:";
// Initiating the GSM module
void pinGSM(){
	P4DIR |= BIT2;
}

void initGSM(void)
{

	//pwrOn();

	//Test command
	//sendATCommand(strlen(ATtest), ATtest);
	//Delay();

	//Echo mode off
	sendATCommand(strlen(ATecho), ATecho);
	uartEnable();
	checkAT();
/*
    uartRxBuf[uartStart] = '\0';
    uartDisable();
    uartStart = 0;
    id = 0;
*/
    //Check if OK
    if((compareEasy(uartRxBuf, ATResponseOK)) == 1)
    {
    	sendATCommand(strlen(ATtextMode), ATtextMode);
    	uartEnable();

    	while(id < strlen(ATResponseOK));

    	uartRxBuf[uartStart] = '\0';
    	uartDisable();
    	uartStart = 0;
    	id = 0;
    }
}

// Sending the AT response, that is the received response, wantedResponse is the
//wanted response..
int compareEasy(char *response, const char *wantedResponse)
{
	if(!(strcmp(response, wantedResponse)))
		return 1;
	else
		return -1;
}


// Sends AT command to Tx buffer
void sendATCommand(int length, const char *command)
{
	char com;
    while(length > 0 )
    {
    	length--;

        com = *(command++);
        uartSend(length, com);
    }
}

//Use this to end your SMS
void sendCtrlZ(void)
{
	while(!(UCA1IFG & UCTXIFG));		//TX buffer ready?
    UCA1TXBUF=26;                       //ASCII number for Ctrl+Z
}

//Read the unread SMS
char readSMS()
{
    sendATCommand(strlen(ATsetSmsStorage), ATsetSmsStorage);
	uartEnable();

	checkAT();		//Checks the response from GPRS and disables interrupt

    sendATCommand(strlen(ATreadUnreadSms), ATreadUnreadSms);
	uartEnable();

	checkAT();



	whatIsTheMessage();
}

//Search for the start of the SMS
int searchForSMS(char *message)
{
	int i = 0;

	while(i < strlen(message))
		if(message[i++] == '#')
			return i;
}

void pwrOn(void)
{
	initCount=1;

	//if(!(P4OUT & BIT2))
	//{
		P4OUT &= ~BIT2;
		int i = 0;

		while(i < 40)
		{
			Delay();
			i++;
		}
	//}
}

void checkAT()
{
	//float q = 0;
	//while(q < 1000000){
		while(id < strlen(ATResponseOK));
	//	q++;
	//}

	//while(id < strlen(ATResponseOK));

		uartRxBuf[uartStart] = '\0';
		uartDisable();
		uartStart = 0;
		id = 0;
}

void whatIsTheMessage()
{
	int startSMS = searchForSMS(uartRxBuf);
	int d=startSMS;
	int diff=0;

	while(startSMS < strlen(uartRxBuf))
	{
		startSMS++;
		if(uartRxBuf[startSMS] == '#')
		{
			diff=startSMS-d;
			startSMS = strlen(uartRxBuf);
		}
	}
	//Save the value....
	int startSMS2=searchForSMS(uartRxBuf);
	int i=0;
	switch(diff) {
		    case 2:
		    	while(startSMS2 < strlen(uartRxBuf))
		    			{
		    				stopAlarm[i]=uartRxBuf[startSMS2];
		    				startSMS2++;
		    				i++;
		    				if(uartRxBuf[startSMS] == '#')
		    					startSMS2 = strlen(uartRxBuf);
		    			}
		    	i=0;

		    	//sendSMS(2,"OK");

		    	if (!strcmp(stopAlarm,"OK"))
		    	{
		    		break;
		    	}
		    	//OKflag x=0;
		    	//alarmflag=pr;
		    	break;             //  Stop alarm
		    case 6:                   //Status requested, all info shall be send in uartsend
		    	while(startSMS2 < strlen(uartRxBuf))
		    			{
		    				statusReq[i]=(uartRxBuf[startSMS2]);
		   		    		startSMS2++;
		   		    		i++;
	  			    		if(uartRxBuf[startSMS] == '#')
		    			    	startSMS2 = strlen(uartRxBuf);
		    			}
		    	i=0;
		    	if (!strcmp(statusReq,statusReqResp))
		    	{
		    		break;
		    	}
		    	//
		    	//Go get data, then uartSend
		        break;
		    case 16:			//New tolerance level.
		    	while(startSMS2 < strlen(uartRxBuf))
		    			{
		    				newTolerance[startSMS2]=(uartRxBuf[startSMS2]);
		    				startSMS2++;
		    				if(uartRxBuf[startSMS] == '#')
		    					startSMS2 = strlen(uartRxBuf);
		    			}

		    	break;
		    case 19:			//New number to FLASH.
		    	while(startSMS2 < strlen(uartRxBuf))
		    			{
		    				newNumber[startSMS2]=(uartRxBuf[startSMS2]);
		    				startSMS2++;
		    				if(uartRxBuf[startSMS] == '#')
		    					startSMS2 = strlen(uartRxBuf);
		    			}
		        break;
		    default: break;
		    }

}

void sendSMS(char *message)
{
	sendATCommand(strlen(ATsetPhoneNumber), ATsetPhoneNumber);//Send the telephone number to SIM900
	/*Delay();
	Delay();
	Delay();
	Delay();
*/
	int i = 0;

	while(i < 4)
	{
		Delay();
		i++;
	}

	char com;
	int length = strlen(message);
	while(length > 0 )
	{
	   	length--;

	    com = *(message++);
	    uartSend(length, com);
	}

	i = 0;

	while(i < 4)
	{
		Delay();
		i++;
	}


	//sendATCommand(strlen(message), message);
	sendCtrlZ();

}



