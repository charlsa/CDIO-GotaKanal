/*
 * GSM_module.h
 *
 *  Created on: Oct 9, 2013
 *      Author: Administrator
 */

#ifndef GSM_MODULE_H_
#define GSM_MODULE_H_

// Initiating the GSM module
void initGSM(void);

// Sends AT command to Tx buffer
void sendATCommand(int length, const unsigned char *command);

//Send stop bit to GSM module, to end task
void sendCTRL_Z(void);



#endif /* GSM_MODULE_H_ */
