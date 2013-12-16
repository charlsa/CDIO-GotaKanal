/*
 * powerControl.c
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */
#include "powerControl.h"
#include "Flash.h"
#include "GSM_module.h"

void boardSetup(){
	P8DIR |= BIT0 + BIT2 + BIT3 + BIT5; 	// Bit0 = 4.1V, Bit1 = 5V and Bit5 = second GSM
	P8DIR &= ~BIT4;
	P3DIR |= BIT0;					// SHND Charger

	P6DIR = 0x00;					// DIP1 = P6.0
	P10DIR = 0xFF;
}

void chargerStart(){
	P3OUT |= BIT0;
}

void stopCharger(){
	P3OUT &= ~BIT0;
}

void V4Start(){
	P8OUT |= BIT5; // enable 4.1 V
}

void V4Stop(){
	P8OUT &= ~BIT5;
}

void tmpvV4Start(){
	P4OUT |= BIT5;
}

void tmpvV4Stop(){
	P4OUT &= ~BIT5;
}

void V5Start(){
	P8OUT |= BIT0; // enable 5 V
}

void V5Stop(){
	P8OUT &= ~BIT0;
}


void readDip(){

 	P10OUT = 0xFF;	// Turn on Dip power
	char *positions[12];
	positions[0] = "Odefinerad";
	positions[1] = "Borensberg";
	positions[2] = "Heda";
	positions[3] = "Ruda";
	positions[4] = "Soderkoping";
	positions[5] = "Boren";
	positions[6] = "Roxen";
	positions[7] = "Karlsborg";
	positions[8] = "Bergs gasthamn";
	positions[9] = "Bergbron";


	if(P6IN == 0x00)
	{		//(00000000)  V�nster bit = bit1 p� dip switch, (L�ser Dip)
		writeFlashPosition(positions[0]);
	}
	else if(P6IN == 0x01)
	{		//(10000000)
		writeFlashPosition(positions[1]);
	}
	else if(P6IN == 0x02)
	{		//(01000000)
		writeFlashPosition(positions[2]);
	}
	else if(P6IN == 0x04)
	{		//(00100000)
		writeFlashPosition(positions[3]);
	}
	else if(P6IN == 0x08)
	{		//(00010000)
		writeFlashPosition(positions[4]);
	}
	else if(P6IN == 0x30)
	{		//(00001000)
		writeFlashPosition(positions[5]);
	}
	else if(P6IN == 0x20)
	{		//(00000100)
		writeFlashPosition(positions[6]);
	}
	else if(P6IN == 0x40)
	{		//(00000010)
		writeFlashPosition(positions[7]);
	}
	else if(P6IN == 0x80)
	{		//(00000001)
		writeFlashPosition(positions[8]);
	}
	else
	{
	writeFlashPosition(positions[0]);
	}

 	P10OUT = 0x00;		// Turn off dip powe
}

void startGSMmodule()
{
	V5Start();
	V4Start(); 	// Enable power to GSM
	pwrOnOff(); 	// if GSM off
}

