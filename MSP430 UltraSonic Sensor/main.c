#include <msp430.h> 
#include "LevelMeasure.h"

/*
 * main.c
 */
void clockSetup(){
	/*
	 * Need to understand more about this...c
	 *
	 */
    //P2DIR == 0x00;	 // for pulse generating

    P4DIR |= 0x80;							// P4.7 SMCLK
    P4SEL |= 0x80;						    // P4.7 for debugging freq.
	UCSCTL3 |= SELREF_2;                    // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;                      // Set ACLK = REFO
	__bis_SR_register(SCG0);                // Disable the FLL control loop
	UCSCTL0 = 0x0000;                       // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                    // Select DCO range 16MHz operation
	UCSCTL2 = FLLD_1 + 244;                 // Set DCO Multiplier for 8MHz
	                                        // (N + 1) * FLLRef = Fdco
											// (244 + 1) * 32768 = 8MHz
											// Set FLL Div = fDCOCLK/2
	__bic_SR_register(SCG0);                // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
}

int main(void) {


    WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT
    directionSetup();
    clockSetup();

    interuptPin();
    __enable_interrupt();
    timerA0Setup();
	__delay_cycles(250000);
	unsigned int distance;

	while(1){
		while(!(P2IN &= BIT6)){
			triggerPulse();
			echo();
			//		trigPin ^= trigPin_nr;  // toggle output pin
			//		P2IFG &= ~BIT6;  				// clear interruptflag
			__delay_cycles(20000);
			SensorCalc(&distance);
			__delay_cycles(2);
		}


		}
}
