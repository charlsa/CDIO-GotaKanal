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
		text[9+i] = number[i]; i++;
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

		int end = (pos+1) * 13, i = 0;
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
	checkOK();

    sendATCommand(ATreadUnreadSms);
	uartEnable();
	checkOK();

	char executionType = '0';

	whatIsTheSMS(&executionType);

	return executionType;
}

//
void saveMessage(char *message, int startSMS, int lengthSMS)
{
	int i = 0;
	while(i < lengthSMS)
	{
		message[i] = uartRxBuf[startSMS];
    	i++;
    	startSMS++;
	}
	message[i] = '\0';
}

/*
 * Compares the message with known messages.
 * Sets c to different letters and executes different things
 * depending on what the message contains.
*/
void whatIsTheSMS(char* c)
{
	int start = searchForSMS(uartRxBuf);
	int i = start, end = start;
	int config = 0, length = 0;

	if(i == 0) return;

	while(i < strlen(uartRxBuf))
	{
		if(uartRxBuf[i] == '#') break;

		else if(uartRxBuf[i] == ':')
		{
			i++; end++; config = i;

		}
		else
		{
			i++; length++; end++;
		}
	}

	if(config != 0)
		length  = config-start;

	char startGSM[2], stopGSM[3];
	char stopAlarm[5], status[6];
	char newLevel[9], newTel[11], newThreshold[17];

	switch(length)
	{
	    case 2:		//Enable GSM
	    	saveMessage(startGSM, start, length);

		    if (strcmp(startGSM, "ON") != 0) break;
		    else *c = 'E';
		    break;

	    case 3:		//Stop GSM
	    	saveMessage(stopGSM, start, length);

		 	if (strcmp(stopGSM, "OFF") != 0) break;
		    else *c = 'D';
		    break;

	    case 5:		//Stop alarm
	    	saveMessage(stopAlarm, start, length);

		    if (strcmp(stopAlarm, "STOPP") != 0) break;
		    else *c = 'A';
		    break;

		case 6:		//Send status
	    	saveMessage(status, start, length);

		    if (strcmp(status, "STATUS") != 0) break;
		    else *c = 'S';
		    break;

		case 9:		//Config new level
			saveMessage(newLevel, start, length);

			if (strcmp(newLevel, "KONFIG N:") != 0) break;
			else
			{
				*c = 'L';
				i = config;
				char tempLevel[2];
				int j = 0, level = 0;

				while(i < end)
				{
		    		tempLevel[j] = uartRxBuf[i];
		    		i++; j++;
				}
				level = (tempLevel[0]-'0')*10 + (tempLevel[1]-'0');
				writeFlashSensorOffset(level);	// Send new level to Flash
			}
			break;

		case 11:	//Config new telephone numbers
			saveMessage(newTel, start, length);

		    if (strcmp(newTel, "KONFIG TEL:") != 0) break;
		    else
		    {
		    	*c = 'N';
		    	i = config;
		    	int j = 0, k = 0;
		    	char phoneNumber[] = "000000000000";
		    	char clear[] = "XXXXXXXXXXXX\n";

		    	while(j < 8)
		    	{
	    			writeFlashTele(clear, j);
	    			j++;
		    	}
		    	j = 0;
		    	while(i < end+1)
		    	{	//If a whole number is read, send it to flash and change position in phoneNumber
		    		if(uartRxBuf[i] == ',' || uartRxBuf[i] == '#')
		    		{
		    			writeFlashTele(phoneNumber, j);
		    			i++; j++;
		    			k = 0;
		    		}
		    		else
		    		{	// put the number from the SMS into phoneBook
		    			phoneNumber[k] = uartRxBuf[i];
		    			i++; k++;
			    	}
		    	}
		    }
		    break;

		case 16:	//New threshold
			saveMessage(newThreshold, start, length);

		    if(strcmp(newThreshold, "KONFIG TOLERANS:") != 0)
		    	break;
		    else
		    {
		    	*c = 'T';
		    	i = config;
		    	char tempThresholdDown[2];
		    	char tempThresholdUp[2];
		    	int thresholds[2];
		    	int j = 0, k = 0;

		    	// change offset later
		    	while(i < end)
		    	{
		    		if(uartRxBuf[i] == ',')
		   			{
		    			i++; j++; k = 0;
	    			}
		    		if(j == 0)
		    		{
		   				tempThresholdDown[k] = uartRxBuf[i];
		   			}
		   			else if (j == 1)
					{
	    				tempThresholdUp[k] = uartRxBuf[i];
	    				if(k == 2) i++;
					}
		    		i++; k++;
		    	}
		    	thresholds[0] = (tempThresholdDown[0]-'0')*10 + (tempThresholdDown[1]-'0');
		    	thresholds[1] = (tempThresholdUp[0]-'0')*10 + (tempThresholdUp[1]-'0');
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
		if(SMS[i++] == '#') return i;
	return 0;
}

/*
 * Send AT command for sending SMS.
 * Send SMS.
*/
// SENDSMS
void sendSMS(char *SMS)
{
	char phoneList[104], number[12];
	int listLength = 8, pos = 0;
	readFlashTele(phoneList);
	char position[30];
	readFlashPosition(position);

	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		sendGSM(":\n");
		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay(); i++;
		}
	}
}

// Lägg till kommentar
void sendAlarm(char* SMS, int value)
{
	char phoneList[104], number[12];
	int listLength = 8, pos = 0;
	readFlashTele(phoneList);
	char position[30];
	readFlashPosition(position);

	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		sendGSM(":\n");
		sendNumber(value);
		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimera!!!
		{
			Delay(); i++;
		}
	}
}

void responseStatus(char *SMS, int sensor)
{
	//writeFlashPosition
	char position[30];
	readFlashPosition(position);
	int lower = readFlashLowTolerance();
	int upper = readFlashHighTolerance();
	int offset = readFlashSensorOffset();
	char phoneList[104], number[12];
	int listLength = 8, pos = 0;
	readFlashTele(phoneList);

	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		// Normal lvl
		sendGSM("\nOffset: ");
		sendNumber(offset);
		// TH ned
		sendGSM("\nTolerans ner: ");
		sendNumber(lower);
		// TH up
		sendGSM("\nTolerans upp: ");
		sendNumber(upper);
		sendGSM("\nNiva: ");
		if(sensor > 0)
			sendGSM("+");
		else
			sendGSM("-");
		// Sensornivå
		sendNumber(sensor);
		Delay();
		sendCtrlZ();



		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay(); i++;
		}
	}
}

void responseNrChange(char *SMS)
{
	char phoneList[105], number[12];
	int listLength = 8, pos = 0;
	char position[30];
	readFlashPosition(position);

	readFlashTele(phoneList);
	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		sendGSM(":\n");
		int a = strlen(phoneList);
		sendGSM(phoneList);

		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay(); i++;
		}
	}
}
// LVL
void responseLvlChange(char *SMS, int offset)
{
	char phoneList[104], number[12];
	int listLength = 8, pos = 0;
	readFlashTele(phoneList);
	char position[30];
	readFlashPosition(position);


	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		sendGSM(":\n");
		sendNumber(offset);
		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay(); i++;
		}
	}
}

void responseThChange(char *SMS, int lower, int upper)
{
	char phoneList[104], number[12];
	int listLength = 8, pos = 0;
	readFlashTele(phoneList);
	char position[30];
	readFlashPosition(position);

	while(checkNumber(phoneList, number, pos) == '1' && pos < listLength)
	{
		sendPhonenumber(number);		//Send the telephone number to SIM900
		pos++;
		Delay();
		sendGSM(SMS);
		sendGSM(position);
		sendGSM(":\n");
		sendGSM("Ner: "); 		// TH low
		sendNumber(lower);
		sendGSM("\nUpp: ");		// TH up
		sendNumber(upper);
		Delay();
		sendCtrlZ();

		int i = 0;
		while(i < 37)			// Optimeras!!!!
		{
			Delay(); i++;
		}
	}
}

/*
 * Use this to end your SMS.
*/
void sendCtrlZ(void)
{
	while(!(UCA2IFG & UCTXIFG));		//TX buffer ready?
    UCA2TXBUF=26;                       //ASCII number for Ctrl+Z
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
	if(x < 0)
	{
		x = fabs(x);
	}

	if (x >= 10)		// Convert int to char value larger than 10
	{
		int y = x/10;
		char a = y + '0';
		x -= 10*y;
		char b = x + '0';
		uartSend(a); uartSend(b);
	}
	else
	{
		char a = x + '0';		// < 10
		uartSend(a);
	}
}

void sendGSM(char *message)
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
