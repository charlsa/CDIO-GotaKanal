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

#define SIZECommand 	6
#define SIZEResponse 	2

///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/

const char ATtest[] 		  = "AT\r";
const char ATecho[] 		  = "ATE0\r";
const char ATsetNetworkReg[]  = "AT+CREG=1\r";
const char ATshowNetworkReg[] = "AT+CREG?\r";
const char ATtextMode[] 	  = "AT+CMGF=1\r";
const char ATsendSMS[] 		  = "AT+CMGS=\"+46735082283\"\r";
const char ATsetSmsStorage[]  = "AT+CPMS=\"SM\"\r";
const char ATreadUnreadSms[]  = "AT+CMGL=\"REC UNREAD\"\r";
const char ATdeleteSms[]   = "AT+CMGDA=\"DEL READ\"\r";


///////////////////////////////////////////////////////////////
/*AT Responses setup definitions*/

const char responseOK[] 			= "OK\r\n";
//const char responseNetworkReg[] 	= "\r\n+CREG: 1,1\r\n\r\nOK\r\n";

///////////////////////////////////////////////////////////////

char startGSM[2];
char stopGSM[3];
char stopAlarm[5];
char status[6];
char newLevel[9];
char newTel[11];
char newThreshold[17];

char startGSMResp[] 	= "ON";
char stopGSMResp[] 		= "OFF";
char stopAlarmResp[] 	= "STOPP";
char statusResp[] 		= "STATUS";
char newLevelResp[] 	= "KONFIG N:";
char newTelResp[] 		= "KONFIG TEL:";
char newThresholdResp[] = "KONFIG TOLERANS:";

const int SIZEphoneBook = 8;

char phoneNumber[] = "000000000000";

const int SIZEThresholds = 2;
char tempThresholdsUp[2];
char tempThresholdsDown[2]; // Lokal in ...
int thresholds[2];

char tempLevel[2];
int level;

/*
 * 	Sets direction for the Power on/off pin
*/
void pinGSM()
{
	P4DIR |= BIT2;
	P8DIR &= ~BIT4;
}

// Power on/off the GSM module
void pwrOnOff(void)
{
	P4OUT &= ~BIT2;
	int i = 0;

	while(i < 40)
	{
		Delay();
		i++;
	}
}


/*
 * Initiating the GSM module:
 * Turns off the echo mode
 * Sets the GSM module in text mode (used for SMS)
*/
void initGSM(void)
{
	//Echo mode off
	sendATCommand(ATecho);
	uartEnable();				//Enable interrupt on UART
	checkAT();					//Check OK from GSM and disable interrupt

	//Text mode
   	sendATCommand(ATtextMode);
   	uartEnable();
   	checkAT();

   	/*sendATCommand("AT+CREG=0\r");
   	uartEnable();
   	checkAT();
*/
   //	deleteSMS();

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

void sendNumber(char *number)
{
	char text[23] = "AT+CMGS=\"";

	int i = 0;
	while (i < 12)
	{
		text[9+i] = number[i];
		i++;
	}
	text[21] = '\"';
	text[22] = '\r';

	int length = strlen(text);
	char c;
	i = 0;
	while(length > 0 )
    {
    	length--;
        c = (text[i++]);
        uartSend(c);
    }
}

char checkNumber(char *phoneList, char *number, int pos)
{
	if (pos > 0)
	{
		int tmp = pos * 13;
		if (phoneList[tmp] != '+') return '0';

		int end = (pos+1) * 13;
		int i = 0;
		while (tmp < end-1)
		{
			number[i] = phoneList[tmp];
			i++; tmp++;
		}
		return '1';
	}
	else
	{
		int tmp = 0;
		if (phoneList[tmp] != '+') return '0';
		while (tmp < 12)
		{
			number[tmp] = phoneList[tmp];
			tmp++;
		}
		return '1';
	}
}
/*
 * Check if the GSM module has sent the whole message to CPU.
 * The GSM module ends the message with an "OK".
 * If the GSM sends "OK", uartRxBuf is ended and the interrupt disabled.
*/
void checkAT()
{
	int j = 0;

	while(id < strlen(responseOK))
	{
		Delay();
		j++;
		if (j > 200)
			break;
	}

	uartRxBuf[uartStart] = '\0';
	uartDisable();
	uartStart = 0;
	id = 0;
}

/*
 * Read the unread SMS in the GSM module's inbox.
 * return the type of the SMS with whatIsTheSMS function
*/
char readSMS()
{
    sendATCommand(ATsetSmsStorage);
    uartEnable();
	checkAT();

    sendATCommand(ATreadUnreadSms);
	uartEnable();
	checkAT();

	char executionType = '0';

	whatIsTheSMS(&executionType);

	//deleteSMS();

	return executionType;
}

/*
 * Compares the message with known messages.
 * Sets c to different letters and executes different things
 * depending on what the message contains.
*/
void whatIsTheSMS(char* c)
{
	int start = searchForSMS(uartRxBuf);
	int startOfSMS = start;
	int endOfSMS = start;
	int lengthOfSMS = 0;
	int j = 0;

	while(start < strlen(uartRxBuf))
	{
		if(uartRxBuf[start] == '#')
		{
			start = strlen(uartRxBuf);
			endOfSMS++;
		}
		else if(uartRxBuf[start] == ':')
		{
			j = start;
			start++;
			endOfSMS++;
		}
		else
		{
			lengthOfSMS++;
			start++;
			endOfSMS++;
		}
	}

	int i = 0;

	if(j != 0)
		lengthOfSMS = j - startOfSMS + 1;

	switch(lengthOfSMS)
	{
		//Enable GSM
	    case 2:
		 	while(i < lengthOfSMS)
		 	{
		 		startGSM[i++] = uartRxBuf[startOfSMS];
		 		startOfSMS++;
		 	}

		 	startGSM[i] = '\0';

		    if (strcmp(startGSM, startGSMResp) != 0)
		    	break;

		    else
		    	*c = 'E';
		    break;

		//Stop GSM
	    case 3:
		 	while(i < lengthOfSMS)
		 	{
		 		stopGSM[i++] = uartRxBuf[startOfSMS];
		 		startOfSMS++;
		 	}

		 	stopGSM[i] = '\0';

		    if (strcmp(stopGSM, stopGSMResp) != 0)
		    	break;

		    else
		    	*c = 'D';

		    //Stop GSM
		    // if STATUS == 1
		    //pwrOnOff();
		    break;

		//Stop alarm
	    case 5:
		 	while(i < lengthOfSMS)
		 	{
		 		stopAlarm[i++] = uartRxBuf[startOfSMS];
		 		startOfSMS++;
		 	}

		 	stopAlarm[i] = '\0';

		    if (strcmp(stopAlarm, stopAlarmResp) != 0)
		    	break;

		    else
		    	*c = 'A';
		    //Stop alarm
		    break;

	    //Send status
		case 6:
		    while(i < lengthOfSMS)
		    {
		    	status[i++] = uartRxBuf[startOfSMS];
		    	startOfSMS++;
		    }

		    status[i] = '\0';

		    if (strcmp(status,statusResp) != 0)
		    	break;

		    else
		    	*c = 'S';

		    // Send status from main
		    break;

		//Config new level
		case 9:
			while(i < lengthOfSMS)
			{
				newLevel[i] = uartRxBuf[startOfSMS];

		    	if(uartRxBuf[startOfSMS] == ':')
		    		startOfSMS = lengthOfSMS;
		    	i++;
		    	startOfSMS++;
			}

			newLevel[i] = '\0';

			if (strcmp(newLevel, newLevelResp) != 0)
		    	break;

			else
			{
				*c = 'L';

				int k = j + 1;
				int l = 0;

				while(k < endOfSMS-1)
				{
		    		tempLevel[l] = uartRxBuf[k];
		    		k++;
		    		l++;
				}

				// Send newLevel to Flash
				level = (tempLevel[0]-'0')*10 + (tempLevel[1]-'0');

				writeFlashSensorOffset(level);
			}

			break;

		//Config new telephone numbers
		case 11:
		    while(i < lengthOfSMS)
		    {
		    	newTel[i] = uartRxBuf[startOfSMS];

		    	if(uartRxBuf[startOfSMS] == ':')
		    		startOfSMS = lengthOfSMS;
		    	i++;
		    	startOfSMS++;
		    }

		    newTel[i] = '\0';

		    if (strcmp(newTel, newTelResp) != 0)
		    	break;
		    else
		    {
		    	*c = 'N';
		    	int k = j + 1;
		    	int l = 0;
		    	int m = 0;
		    	int i = 0;

		    	char clear[] = "XXXXXXXXXXXX\n";
		    	for(i; i<8; i++)
		    	{
	    			writeFlashTele(clear, i);
		    	}
		    	// Send phone numbers to Flash
		    	while(k < endOfSMS)
		    	{	//If a whole number is read, send it to flash and change position in phoneNumber
		    		if(uartRxBuf[k] == ',' || uartRxBuf[k] == '#')
		    		{
		    			writeFlashTele(phoneNumber, l);
		    			k++;
		    			l++;
		    			m = 0;
		    		}
		    		// put the number from the SMS into phoneBook
		    		else
		    		{
		    			phoneNumber[m] = uartRxBuf[k];
		    			k++;
			    		m++;
			    	}
		    	}
		    }
		    break;
		//New threshold
		case 16:
		    while(i < lengthOfSMS)
		    {
		    	newThreshold[i] = uartRxBuf[startOfSMS];

		    	if(uartRxBuf[startOfSMS] == ':')
		    		startOfSMS = lengthOfSMS;
		    	i++;
		    	startOfSMS++;
		    }

		    newThreshold[i] = '\0';

		    if(strcmp(newThreshold, newThresholdResp) != 0)
		    	break;

		    else
		    {
		    	*c = 'T';

		    	int l = 0;
		    	int m = 0;
		    	j++; 	// change offset later
		    	while(j < endOfSMS)
		    	{
		    		if(uartRxBuf[j] == ',')
		   			{
		   				j++;
		   				l++;
		   				m = 0;
	    			}
		    		if(l == 0)
		    		{
		   				tempThresholdsDown[m] = uartRxBuf[j];
		   			}
		   			else if (l == 1)
					{
	    				tempThresholdsUp[m] = uartRxBuf[j];
	    				if(m == 2) l++;
					}
		    		j++;
		    		m++;
		    	}

		    	thresholds[0] = (tempThresholdsDown[0]-'0')*10 + (tempThresholdsDown[1]-'0');
		    	thresholds[1] = (tempThresholdsUp[0]-'0')*10 + (tempThresholdsUp[1]-'0');

		    	writeFlashTolerance(thresholds[0], thresholds[1]);			// Send newThreshold to Flash
		    }
		    break;
		//Do nothing
		default: break;
	}
}

/*
 * Search for the beginning of the SMS in the message from the GSM module
 * (The message contains date, time, number and the actual SMS)
*/
char searchForSMS(char *SMS)
{
	int i = 0;

	while(i < strlen(SMS))
		if(SMS[i++] == '#')
			return i;
	return 0;
}

/*
 * Send AT command for sending SMS.
 * Send SMS.
*/
// SENDSMS
void sendSMS(char *SMS)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < 8)
	{
		sendNumber(number);		//Send the telephone number to SIM900
		pos++;
		int i = 0;
		while(i < 4)
		{
			Delay();
			i++;
		}

		int length = strlen(SMS);
		char *tmp = SMS;
		char c;
		while(length > 0)
		{
			length--;

			c = *(tmp++);
			uartSend(c);
		}

		i = 0;
		while(i < 4)
		{
			Delay();
			i++;
		}
		sendCtrlZ();
		i = 0;
		while(i < 150)
		{
			Delay();
			i++;
		}
	}
}

void sendAlarm(char* SMS, int value)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];


	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < 8)
	{
		sendNumber(number);		//Send the telephone number to SIM900
		pos++;
		int i = 0;
		while(i < 4)
		{
			Delay();
			i++;
		}

		int length = strlen(SMS);
		char *tmp = SMS;
		char c;
		while(length > 0)
		{
			length--;
			c = *(tmp++);
			uartSend(c);
		}

		int tmpValue = value;
		if (tmpValue >= 10)
		{	// Convert int to char value larger than 10
			int x = tmpValue/10;
			char a = x + '0';

			tmpValue -= 10*x;
			char b = tmpValue + '0';
			uartSend(a);
			uartSend(b);
		}
		else
		{
			char a = tmpValue + '0';
			uartSend(a);
		}

		i = 0;
		while(i < 4)
		{
			Delay();
			i++;
		}
		sendCtrlZ();
		i = 0;
		while(i < 150)
		{
			Delay();
			i++;
		}
	}

		//
/*	int length = strlen(SMS);

	sendATCommand(ATsendSMS);		//Send the telephone number to SIM900

	int i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	char c;
	while(length > 0 )
	{
	   	length--;

	    c = *(SMS++);
	    uartSend(c);
	}

	if (value >= 10)
	{	// Convert int to char value larger than 10
		int x = value/10;
		char a = x + '0';

		value -= 10*x;
		char b = value + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = value + '0';
		uartSend(a);
	}

	i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	sendCtrlZ(); */
}

void responseStatus(char *SMS, int sensor)
{
	int lower = readFlashLowTolerance();
	int upper = readFlashHighTolerance();
	int normal = readFlashSensorOffset();

	sendATCommand(ATsendSMS);		//Send the telephone number to SIM900

	int i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	char c;
	int length = strlen(SMS);
	while(length > 0 )
	{
	   	length--;

	    c = *(SMS++);
	    uartSend(c);
	}

	sendATCommand("Niva:");
	if (sensor >= 10) // aktuellt
	{	// Convert int to char value larger than 10
		int x = sensor/10;
		char a = x + '0';

		sensor -= 10*x;
		char b = sensor + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = sensor + '0';
		uartSend(a);
	}

	// TH ned
	sendATCommand("\nTroskel ner:");
	if (lower >= 10)
	{	// Convert int to char value larger than 10
		int x = lower/10;
		char a = x + '0';

		lower -= 10*x;
		char b = lower + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = lower + '0';
		uartSend(a);
	}

	// TH up
	sendATCommand("\nTroskel upp:");
	if (upper >= 10)
	{	// Convert int to char value larger than 10
		int x = upper/10;
		char a = x + '0';

		upper -= 10*x;
		char b = upper + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = upper + '0';
		uartSend(a);
	}

	// Normal lvl
	sendATCommand("\nNormal:");
	if (normal >= 10)
	{	// Convert int to char value larger than 10
		int x = normal/10;
		char a = x + '0';

		normal -= 10*x;
		char b = normal + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = normal + '0';
		uartSend(a);
	}

	i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}
	sendCtrlZ();
}

void responseNrChange(char *SMS)
{
	char tmp[104];
	int listLength = 8;

	readFlashTele(tmp);

	sendATCommand(ATsendSMS);		//Send the telephone number to SIM900
	int i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	char c;
	int length = strlen(SMS);
	while(length > 0 )
	{
	   	length--;
	    c = *(SMS++);
	    uartSend(c);
	}
	length = 104;
	i = 0;
	while(i < length)
	{
		uartSend(tmp[i]);
		i++;
	}

	i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}
	sendCtrlZ();
}
// LVL
void responseLvlChange(char *SMS, int offset)
{
	sendATCommand(ATsendSMS);		//Send the telephone number to SIM900

	char c;
	int length = strlen(SMS);

	int i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	while(length > 0 )
	{
	   	length--;

	    c = *(SMS++);
	    uartSend(c);
	}

	if (offset >= 10)
	{	// Convert int to char value larger than 10
		int x = offset/10;
		char a = x + '0';

		offset -= 10*x;
		char b = offset + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = offset + '0';
		uartSend(a);
	}

	i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	sendCtrlZ();
}

void responseThChange(char *SMS, int lower, int upper)
{
	sendATCommand(ATsendSMS);		//Send the telephone number to SIM900

	int i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}

	char c;
	int length = strlen(SMS);

	while(length > 0 )
	{
	   	length--;

	    c = *(SMS++);
	    uartSend(c);
	}
	sendATCommand("Nedre:");
	if (lower >= 10)
	{	// Convert int to char value larger than 10
		int x = lower/10;
		char a = x + '0';

		lower -= 10*x;
		char b = lower + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = lower + '0';
		uartSend(a);
	}

	// TH up
	sendATCommand("\nOvre:");
	if (upper >= 10)
	{	// Convert int to char value larger than 10
		int x = upper/10;
		char a = x + '0';

		upper -= 10*x;
		char b = upper + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = upper + '0';
		uartSend(a);
	}

	i = 0;
	while(i < 4)
	{
		Delay();
		i++;
	}
	sendCtrlZ();
}


/*
 * Use this to end your SMS.
*/
void sendCtrlZ(void)
{
	while(!(UCA1IFG & UCTXIFG));		//TX buffer ready?
    UCA1TXBUF=26;                       //ASCII number for Ctrl+Z
}

/*
 * Delete all received SMS
*/
void deleteSMS()
{
	sendATCommand(ATdeleteSms);
}


