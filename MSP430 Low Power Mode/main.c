#include <msp430.h> 

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
    P4SEL |= 0x80;							// P4.7 for debugging freq.

    P11DIR |= BIT0;							// ACLK
    P11SEL |= BIT0;							// P11.0

	UCSCTL3 |= SELREF_2;                    // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_4;                      // Set ACLK = REFO
	__bis_SR_register(SCG0);                // Disable the FLL control loop
	UCSCTL0 = 0x0000;                       // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                    // Select DCO range 16MHz operation
	UCSCTL2 = FLLD_1 + 122;  	//122	        // Set DCO Multiplier for 8MHz
	UCSCTL5 = DIVA__4 + DIVS__4; // test 2MHz
	                                        // (N + 1) * FLLRef = Fdco
											// (244 + 1) * 32768 = 8MHz
											// Set FLL Div = fDCOCLK/2
	__bic_SR_register(SCG0);                // Enable the FLL control loop

	__delay_cycles(80000);					// Test for avoid clk prob

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
}

void setUpRTC(){
	RTCCTL01 |= RTCTEV_3 + RTCTEVIE;	// 32 bits overflow ACLK default, Interrupt Enable
	// clear registers
	RTCTIM0 = 0;
	RTCTIM1 = 0xFF00;
}

void startRTC(){
	//if(RTCTIM0 != 0) RTCTIM0 = 0;	// if not reseted, reset
	//if(RTCTIM1 != 0) RTCTIM1 = 0;

	RTCCTL01 &= ~RTCHOLD; 			// Start RTC (Hold is writen to Zero)
	__bis_SR_register(LPM3_bits + GIE);
	LPM3;							// Sets the MCU into LPM3 => only (ACLK != 0)
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    clockSetup();
    deBuggSetup();
    setUpRTC();
    __enable_interrupt();
    startRTC();

   // while(1){}
	
	//return 0;
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
	RTCCTL01 |= RTCHOLD;
	triggLED();
	RTCTIM0 = 0;
	RTCCTL01 &= ~RTCTEVIFG;
	LPM1;
}
