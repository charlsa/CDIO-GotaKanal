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

// Reads Dip and stores position into Fash//
void readDip(){

		P6DIR = 0x00;	// Dip input
	 	P10DIR = 0xFF; 	// Dip Power
	 	P10OUT = 0xFF;	// Turn on Dip power

		char *positioner[10];
		positioner[0] = "Projektrummet";
		positioner[1] = "Borensberg";
		positioner[2] = "Heda";
		positioner[3] = "Ruda";
		positioner[4] = "Soderkoping";
		positioner[5] = "Boren";
		positioner[6] = "Roxen";
		positioner[7] = "karlsborg";
		positioner[8] = "Bergsgasthamn";
		positioner[9] = "BergBorn";


	if(P6IN == 0x00){		//(00000000)  Vänster bit = bit1 på dip switch, (Läser Dip)
		writeFlashPosition(positioner[0]);
	}
	if(P6IN == 0x01){		//(10000000)
		writeFlashPosition(positioner[1]);
	}
	if(P6IN == 0x02){		//(01000000)
		writeFlashPosition(positioner[2]);
	}
	if(P6IN == 0x04){		//(00100000)
		writeFlashPosition(positioner[3]);
	}
	if(P6IN == 0x08){		//(00010000)
		writeFlashPosition(positioner[4]);
	}
	if(P6IN == 0x30){		//(00001000)
		writeFlashPosition(positioner[5]);
	}
	if(P6IN == 0x20){		//(00000100)
		writeFlashPosition(positioner[6]);
	}
	if(P6IN == 0x40){		//(00000010)
		writeFlashPosition(positioner[7]);
	}
	if(P6IN == 0x80){		//(00000001)
		writeFlashPosition(positioner[8]);
	}
	if(P6IN == 0x03){		//(11000000)
		writeFlashPosition(positioner[9]);
	}
		//etc, kommer behövas fylla på med positioner
 	P10OUT = 0x00;		// Turn off dip powe
}
