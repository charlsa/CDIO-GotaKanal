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

///////////////////////////////////////////////////////////////
/* AT Commands setup definitions*/

const char ATtest[] 		  = "AT\r";
const char ATecho[] 		  = "ATE0\r";
const char ATsetNetworkReg[]  = "AT+CREG=1\r";
const char ATshowNetworkReg[] = "AT+CREG?\r";
const char ATtextMode[] 	  = "AT+CMGF=1\r";
const char ATsendSMS[] 		  = "AT+CMGS=\"+46735082283\"\r";
const char ATsetSmsStorage[]  = "AT+CPMS=\"SM\"\r";
const char ATreadUnreadSms[]  = "AT+CMGR=1\r";
const char ATdeleteSms[]   = "AT+CMGDA=\"DEL INBOX\"\r";


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
char newThreshold[16];

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
char *tempThresholds[2];
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
   	deleteSMS();

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
        uartSend(length, c);
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
			start = strlen(uartRxBuf);

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
		//Start GSM
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

		    //Start GSM
		    // if STATUS == 0
		    //pwrOnOff();
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

				while(k < endOfSMS)
				{
		    		tempLevel[l] = uartRxBuf[j];
		    		k++;
		    		l++;
				}

				// Send newLevel to Flash
				level = (tempLevel[0]-'0')*10 + (tempLevel[1]-'0');

				//writeFlashSensorOffset(level);
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

		    	// Send phone numbers to Flash
		    	while(k < endOfSMS)
		    	{
		    		//If a whole number is read, send it to flash and change position in phoneNumber
		    		if(uartRxBuf[k] == ',' || uartRxBuf[k] == '#')
		    		{
		    			//writeFlashTele(phoneBook, l);
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

		    	while(j < endOfSMS)
		    	{
		    		while(l < SIZEThresholds)
		    		{
		    			if(uartRxBuf[j] == ',')
		    			{
		    				j++;
		    				l++;
		    				m = 0;
		    			}

		    			else
						{
		    				tempThresholds[l][m] = uartRxBuf[j];
		    				j++;
		    				m++;
						}
		    		}
		    	}

		    	thresholds[0] = atoi(tempThresholds[0][0])*10 + atoi(tempThresholds[0][1]);
		    	thresholds[1] = atoi(tempThresholds[1][0])*10 + atoi(tempThresholds[1][1]);

		    	// Send newThreshold to Flash
		    	//writeFlashLowTolerance(thresholds[0], threshold[1]);
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
void sendSMS(char *SMS)
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
	    uartSend(length, c);
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


