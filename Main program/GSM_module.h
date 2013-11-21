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

extern const char ATResponseOK[];
// Initiating the GSM module
void pinGSM();

void initGSM(void);

// Sends AT command to Tx buffer
void sendATCommand(int length, const char *command);

//Send stop bit to GSM module, to end task
void sendCtrlZ(void);

//Compare response from GSM to wanted response
int compareEasy(char *response, const char *wantedResponse);

char readSMS();

int searchForSMS(char *message);

void doSomethingDummy();

void pwrOn(void);
void checkAT();
void whatIsTheMessage();

void sendSMS(char *message);

#endif /* GSM_MODULE_H_ */
