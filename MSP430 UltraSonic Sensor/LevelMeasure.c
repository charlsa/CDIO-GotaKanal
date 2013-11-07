/*
 * LevelMeasure.c
 *
 *  Created on: 3 okt 2013
 *      Author: hej
 */
#include "LevelMeasure.h"


////////////////Funktionen mÃ¤ter 10 vÃ¤rden och sparar i lokala vecktorn "data"
//////////////// datavektorn sorteras efter storleksordning
int measure()
{
	unsigned int data[DataLength];
//	unsigned int data;
	int i = 0, j=0;
	while(i < DataLength){
		triggerPulse();
		echo();
		SensorCalc(&data[i]);
		// Verifie that the data is correct
		if((data[i] < Oveflow && data[i] > Underflow) || j > 9){
			i++;
			j = 0;
		}
		j++;
		__delay_cycles(200);
	}
	sortData(data, DataLength);
	int value = 0;
	value = pickvalue();
	__delay_cycles(20);
	return value;
}

int pickvalue(unsigned int data[], int length)

{
  int countend = 0;
  int countstart = 0;
  int g = 0;
  int d = 0;
    for (d; d<length; d++)
		{
		  if (data[d]< Underflow)
		  {
			countstart++;
		  }

		  if (data[d] > Oveflow)
		  {
			countend++;
		  }
    }

    g = length - countend;
    g = g - countstart;
    g = g/2;
    g = g + countstart;

    if (data[g] <= Oveflow && data[g] >= Underflow)
    {
      return data[g];
    }
  return 0;
}





void sortData(unsigned int data[], int length){
    int i, j, tmp;
    for(j = 1; j < length; j++)    // Start with 1 (not 0)
    {
    	tmp = data[j];
    	for(i = j - 1; (i >= 0) && (data[i] < tmp); i--)   // Smaller values move up
        {
    		data[i+1] = data[i];
        }
        	data[i+1] = tmp;    //Put key into its proper location
        }
}


void directionSetup(){
	 trigPin_DIR |= trigPin_nr;					// Set output direction
	 trigPin &= ~trigPin_nr;					// Set pin low

	 P1DIR &= ~ECHO; 							// Echo pin
	 P1SEL = ECHO;								// set P1.2 to TA0
}

void timerA0Setup(){
	 TA0CTL = TASSEL_2 + MC_0 + ID_0 + TACLR; 			// SMCLK, stop, div8, clearTAR, interrupt enable
	 TA0CCTL1 = CM_3 + CCIS_0 + CAP + SCCI;								// Capturer mode pos/neg flank, P1.2 input
}

void interuptPin(){ // debuging button
	P2DIR |= BIT6;
	P2OUT |= BIT6;		// Set pullup for button

	/* INTERRUPT SETUP
    P2IFG &= ~BIT6;				// Clear interruptflag
    P2IE |= BIT6;
    P2IES |= BIT6; */ 			// select interrupt edge rising
}

void triggerPulse(){
	/*
	 * Generate a 12.2 us pulse to trig the ultrasonic sensor
	 * The cycle delay is calculated by
	 * f_mclk*Pulse_time = (8 MHz)*(10 us) = 80 cyckes
	 */
	trigPin ^= trigPin_nr;
	__delay_cycles(20);
	trigPin ^= trigPin_nr;
}

void echo(){
	/* 
	 * Startar timer i capture mode och vÃ¤ntar pÃ¥ tvÃ¥ interruptflanker	
	 * Tiden mellan flankerna sparas i variabeln sonic echo
	 */
	EdgeCount = 0;
	SonicEcho = 0;
	TA0CCTL1 &= ~CCIFG;
	TA0CTL |= MC_2; 			// start continius mode
	TA0CTL &= ~TAIFG;			// Reset TA1 interrupt/owerflowflag
	TA0CCTL1 |= CCIE;
	TA0R = 0x0000; 				// Reset timer

	while((TA0R < 0x9C40)); 	// Wait for timer to count to 9C40 = 40ms

	TA0CCTL1 &= ~CCIE;			// Disable catch interrupt

}

void SensorCalc(int* dist){
	//Converts the pulse width of the measuring echo
	*dist = (SonicEcho/58);
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void CCR1_ISR(void)
{
	if (EdgeCount == 0)
	{
		SonicEcho = TA0CCR1;
		EdgeCount++;
	}
	else if (EdgeCount == 1)
	{
		SonicEcho = TA0CCR1 - SonicEcho;
	}

	TA0CCTL1 &= ~CCIFG;			// Clear catch interruptflag

}
