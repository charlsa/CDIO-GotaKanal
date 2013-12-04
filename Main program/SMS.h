/*
 * SMS.h
 *
 *  Created on: 4 dec 2013
 *      Author: Amanda
 */

#ifndef SMS_H_
#define SMS_H_

char checkNumber(char *phoneList, char *number, int pos);

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

void sendAlarm(char *SMS, int value);

void responseStatus(char *SMS, int sensor);

void responseLvlChange(char *SMS, int offset);

void responseThChange(char *SMS, int lower, int upper);

void responseNrChange(char *SMS);

void deleteSMS();

void sendGSM(const char *message);

void sendNumber(int x);

#endif /* SMS_H_ */
