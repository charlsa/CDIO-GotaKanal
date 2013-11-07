/*
 * powerControl.c
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */
#include "powerControl.h"

void powerPinSetup(){
	P4DIR |= BIT0 + BIT1; 	// Bit1 5V and Bit0 4.1V
	P3DIR |= BIT0;			// SHND Charger
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

void V5Start(){
	P4OUT |= BIT1; // enable 5 V
}

void V5Stop(){
	P4OUT &= ~BIT1;
}
