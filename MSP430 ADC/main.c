#include <msp430.h> 
#include "math.h"

int test = 0;
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

void initADC(){
	/*
	 * Later look if the internal power down can be used
	 */
	  P7SEL |= BIT4;                         // This pin is set to eliminate parasitic current flow and
	  	  	  	  	  	  	  	  	  	  	 // Not necessary for the ADC
	  /* Initialize ADC12 */
	  ADC12CTL1 |= ADC12SHP + 0xC000;         	// Sample-and-hold pulse-mode select, signal sourced from input => (SAMPCON 0)

	  ADC12CTL0 = ADC12ON + ADC12SHT12;     	// Turn on ADC12, set sampling time

	  ADC12CTL2 &= 0xFFCF;						// Sets the resolution to 8 Bit
	  ADC12CTL2 |= ADC12TCOFF;					// Internal temp sens off

	  ADC12MCTL12 = ADC12SREF_0 + ADC12INCH_12; // Vr+=Vref+ and Vr-=AVss

	  __delay_cycles(600);                  	// 75 us delay @ ~8 MHz
}

int readBatteryLvl(){
	/*
	 *  This function reads from ADC A12 and returns the voltage times 10
	 *  to avoid double
	 */
	int voltage = 0;
	ADC12CTL0 |= ADC12SC + ADC12ENC;        // Start conversion and enabel the ADC
	while (!(ADC12IFG & BITC));				// wait until finnished converson
	voltage = ADC12MEM12;
	ADC12CTL0 &= !(ADC12SC + ADC12ENC);		// Disable the ADC

	voltage = voltage*0.129; 				// to convert from bits to Voltage [ADC bites]*[Vote/Bits]
	return voltage;
}

void deBuggSetup(){
	// debuging button with interrupt
	P1DIR |= BIT0; 		// test led2

	P2DIR |= BIT6;
	P2OUT |= BIT6;		// Set pullup for button

	// Interrupt button
    P2IFG &= ~BIT6;		// Clear interruptflag
    P2IE |= BIT6;
    P2IES |= BIT6; 		// select interrupt edge rising
}

void triggLED(){
	/*
	 * Trig the LED for debugging
	 */
	P1OUT ^= BIT0;
	__delay_cycles(80000);
	P1OUT ^= BIT0;
}

int main(void){
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    clockSetup();				// MCLK 8 MHz
    deBuggSetup();				// for debuging
    initADC();
    __enable_interrupt();

    while(1){}
	return 0;
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	__delay_cycles(8000);
	triggLED();
	__delay_cycles(1000);
	test = readBatteryLvl();
	P2IFG &= ~BIT6;				// Clear interruptflag
}
