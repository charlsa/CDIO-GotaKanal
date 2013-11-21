/*
 * powerControl.c
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */
#include "powerControl.h"

void powerPinSetup(){
	P8DIR |= BIT0 + BIT1 + BIT5; 	// Bit0 = 4.1V, Bit1 = 5V and Bit5 = second GSM
	P3DIR |= BIT0;					// SHND Charger

	P6DIR &= 0x00;					// DIP1 = P6.0
}

void chargerStart(){
	P3OUT |= BIT0;
}

void stopCharger(){
	P3OUT &= ~BIT0;
}

void V4Start(){
	P4OUT |= BIT0; // enable 4.1 V
}

void V4Stop(){
	P4OUT &= ~BIT0;
}

void tmpvV4Start(){
	P8OUT |= BIT5;
}

void tmpvV4Stop(){
	P8OUT &= ~BIT5;
}

void V5Start(){
	P4OUT |= BIT1; // enable 5 V
}

void V5Stop(){
	P4OUT &= ~BIT1;
}

char readDIP(){
	char DIP = P6IN;
	return DIP;
}
