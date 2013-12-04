/*
 * SMS.c
 *
 *  Created on: 4 dec 2013
 *      Author: Amanda
 */

#include <msp430.h>
#include "SMS.h"
#include "GSM_module.h"
#include "UART.h"
#include "Flash.h"

///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/

const char ATsetSmsStorage[]  = "AT+CPMS=\"SM\"\r";
const char ATreadUnreadSms[]  = "AT+CMGL=\"REC UNREAD\"\r";
const char ATdeleteSMS[]   = "AT+CMGDA=\"DEL READ\"\r";
const char ATsendSMS[] 		  = "AT+CMGS=\"+46735082283\"\r";

///////////////////////////////////////////////////////////////

// Lägg till kommentar
void sendPhonenumber(char *number)
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

// Lägg till kommentar
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

	char startGSM[2];
	char stopGSM[3];
	char stopAlarm[5];
	char status[6];
	char newLevel[9];
	char newTel[11];
	char newThreshold[17];

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

		    if (strcmp(startGSM, "ON") != 0)
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

		 	if (strcmp(stopGSM, "OFF") != 0)
		    	break;
		    else
		    	*c = 'D';
		    break;

		//Stop alarm
	    case 5:
		 	while(i < lengthOfSMS)
		 	{
		 		stopAlarm[i++] = uartRxBuf[startOfSMS];
		 		startOfSMS++;
		 	}
		 	stopAlarm[i] = '\0';

		    if (strcmp(stopAlarm, "STOPP") != 0)
		    	break;
		    else
		    	*c = 'A';
		    break;

	    //Send status
		case 6:
		    while(i < lengthOfSMS)
		    {
		    	status[i++] = uartRxBuf[startOfSMS];
		    	startOfSMS++;
		    }
		    status[i] = '\0';

		    if (strcmp(status, "STATUS") != 0)
		    	break;
		    else
		    	*c = 'S';
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

			if (strcmp(newLevel, "KONFIG N:") != 0)
		    	break;
			else
			{
				*c = 'L';

				char tempLevel[2];
				int level = 0;
				int k = 0;
				j++;

				while(j < endOfSMS-1)
				{
		    		tempLevel[k] = uartRxBuf[j];
		    		j++;
		    		k++;
				}
				level = (tempLevel[0]-'0')*10 + (tempLevel[1]-'0');

				writeFlashSensorOffset(level);	// Send new level to Flash
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

		    if (strcmp(newTel, "KONFIG TEL:") != 0)
		    	break;
		    else
		    {
		    	*c = 'N';
		    	j++;
		    	int k = 0;
		    	int l = 0;
		    	int i = 0;

		    	char phoneNumber[] = "000000000000";
		    	char clear[] = "XXXXXXXXXXXX\n";
		    	for(i; i < 8; i++)
		    	{
	    			writeFlashTele(clear, i);
		    	}

		    	while(j < endOfSMS)
		    	{
		    		//If a whole number is read, send it to flash and change position in phoneNumber
		    		if(uartRxBuf[j] == ',' || uartRxBuf[j] == '#')
		    		{
		    			writeFlashTele(phoneNumber, k);
		    			j++;
		    			k++;
		    			l = 0;
		    		}
		    		else
		    		{
		    			// put the number from the SMS into phoneBook
		    			phoneNumber[l] = uartRxBuf[j];
		    			j++;
			    		l++;
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
		    char newThresholdResp[] = "KONFIG TOLERANS:";

		    if(strcmp(newThreshold, newThresholdResp) != 0)
		    	break;
		    else
		    {
		    	*c = 'T';

		    	char tempThresholdsDown[2];
		    	char tempThresholdsUp[2];
		    	int thresholds[2];
		    	int k = 0;
		    	int l = 0;
		    	j++; 								// change offset later
		    	while(j < endOfSMS)
		    	{
		    		if(uartRxBuf[j] == ',')
		   			{
		   				j++;
		   				k++;
		   				l = 0;
	    			}
		    		if(k == 0)
		    		{
		   				tempThresholdsDown[l] = uartRxBuf[j];
		   			}
		   			else if (k == 1)
					{
	    				tempThresholdsUp[l] = uartRxBuf[j];
	    				if(l == 2) k++;
					}
		    		j++;
		    		l++;
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
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();

		sendGSM(SMS);

		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay();
			i++;
		}
	}
}

// Lägg till kommentar
void sendAlarm(char* SMS, int value)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();

		sendGSM(SMS);
		sendNumber(value);

		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimera!!!
		{
			Delay();
			i++;
		}
	}
}

void responseStatus(char *SMS, int sensor)
{
	int lower = readFlashLowTolerance();
	int upper = readFlashHighTolerance();
	int normal = readFlashSensorOffset();

	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();
		sendGSM(SMS);

		// Sensornivå
		sendGSM("Niva:");
		sendNumber(sensor);

		// TH ned
		sendGSM("\nTroskel ner:");
		sendNumber(lower);

		// TH up
		sendGSM("\nTroskel upp:");
		sendNumber(upper);

		// Normal lvl
		sendGSM("\nNormal:");
		sendNumber(normal);

		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay();
			i++;
		}
	}
}

void responseNrChange(char *SMS)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();

		sendGSM(SMS);
		sendGSM(phoneList);

		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay();
			i++;
		}
	}
}
// LVL
void responseLvlChange(char *SMS, int offset)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();

		sendGSM(SMS);
		sendNumber(offset);

		Delay();
		sendCtrlZ();
		int i = 0;

		while(i < 37)			// Optimeras!!!!
		{
			Delay();
			i++;
		}
	}
}

void responseThChange(char *SMS, int lower, int upper)
{
	char phoneList[104];
	int listLength = 8;
	int pos = 0;
	char number[12];

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;

		Delay();
		sendGSM(SMS);

		// TH low
		sendGSM("Nedre:");
		sendNumber(lower);

		// TH up
		sendGSM("\nOvre:");
		sendNumber(upper);

		Delay();
		sendCtrlZ();

		int i = 0;

		while(i < 37)			// Optimeras!!!!
		{
			Delay();
			i++;
		}
	}
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
	sendATCommand(ATdeleteSMS);
}

void sendNumber(int x)
{
	if (x >= 10)
	{	// Convert int to char value larger than 10
		int y = x/10;
		char a = y + '0';

		x -= 10*y;

		char b = x + '0';
		uartSend(a);
		uartSend(b);
	}
	else
	{
		char a = x + '0';		// < 10
		uartSend(a);
	}
}

void sendGSM(const char *message)
{
	char c;
	int length = strlen(message);

	while(length > 0 )
	{
	   	length--;

	    c = *(message++);
	    uartSend(c);
	}
}
