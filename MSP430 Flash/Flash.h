/*
 * GSMmodul.h
 *
 *  Created on: 5 okt 2013
 *      Author: Challe
 */

#ifndef GSMMODUL_H_
#define GSMMODUL_H_

#include <msp430.h>

void initUART();

void triggLED(); // used to indicate interrupt

void deBuggSetup(); // interrupt on button 1 to test

void transmittUART(); // Send via UART needs to be changed to returning char or sending refernce

void reciveUART();	// Receive from uart... Needs to be changed to returining char or sending reference
					// also a buffer cheak needs to be implemented

__interrupt void Port_2(void); // Button interrupt for testing...

#endif /* GSMMODUL_H_ */
