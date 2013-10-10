/*
 * LevelMeasure.c
 *
 *  Created on: 3 okt 2013
 *      Author: hej
 */
#include "LevelMeasure.h"

void directionSetup(){
	 trigPin_DIR |= trigPin_nr;					// Set output direction
	 trigPin &= ~trigPin_nr;					// Set pin low

	 P1DIR &= ~ECHO; 							// Echo pin
	 P1SEL = ECHO;								// set P1.2 to TA0
}

void timerA0Setup(){
	 TA0CTL = TASSEL_2 + MC_0 + ID_3 + TACLR; 			// SMCLK, stop, div8, clearTAR, interrupt enable
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
	__delay_cycles(80);
	trigPin ^= trigPin_nr;
	__delay_cycles(80);
}

void echo(){
	EdgeCount = 0;
	SonicEcho = 0;
	TA0CCTL1 &= ~CCIFG;
	TA0CTL |= MC_2; 			// start continius mode
	TA0CTL &= ~TAIFG;			// Reset TA1 interrupt/owerflowflag
	TA0CCTL1 |= CCIE;
	TA0R = 0x0000; 				// Reset timer

	while(!(TA0CTL & TAIFG)); // Wait for timer owerflow

	TA0CCTL1 &= ~CCIE;			// Disable catch interrupt
}

void SensorCalc(int* dist){
	/*
	 * Converts the pulse width of the measuring echo
	 *
	 */
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
