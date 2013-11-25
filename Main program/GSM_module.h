/*
 * 	GSM_module.h
 *
 *  Created on: Oct 17, 2013
 *  Author: Amanda
 */

#ifndef GSM_MODULE_H_
#define GSM_MODULE_H_

#include <stdio.h>
#include <string.h>

extern const char responseOK[];

void pinGSM();

//Turn on the GSM module
void pwrOnOff(void);

//Initiating the GSM module
void initGSM(void);

//Sends AT command to Tx buffer
void sendATCommand(const char *command);

//Check if GSM returns "OK"
void checkAT();

//Check if GSM has registered on network
char checkRegistration(char *message);

//Read SMS from GSM module
char readSMS();

//Sifts out the SMS from the total message from the GSM module
void whatIsTheSMS(char* c);

//Search for the beginning of SMS
char searchForSMS(char *SMS);

//Send stop bit to GSM module, to end task (use only when sending SMS)
void sendCtrlZ(void);

//
void sendSMS(char *SMS);

//
void deleteSMS();

//Compare response from GSM to wanted response
int compareEasy(char *response, const char *wantedResponse);

void writeFlashLowTolerance(int lowTreshold);

void writeFlashHighTolerance(int highTreshold);
#endif /* GSM_MODULE_H_ */
