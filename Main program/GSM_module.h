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
void pwrOnOff();

//Initiating the GSM module
void initGSM();

//Sends AT command to Tx buffer
void sendATCommand(const char *command);

//Check if GSM returns "OK"
void checkOK();

//Check if GSM has registered on network
char checkRegistration(char *message);

char checkAT();

#endif /* GSM_MODULE_H_ */
