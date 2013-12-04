/*
 * CLK_RTC_setup.c
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */
#include "CLK_RTC_setup.h"
#include "powerControl.h"

int loop1 = 0;
int loop2 = 0;

void clkDebug()
{	// ACLK = P11.0, MCLK = P11.1, SMCLK = P11.2
    P11DIR |= BIT0 + BIT1 + BIT2;
    P11SEL |= BIT0 + BIT1 + BIT2;
}

void clkSetup()
{	// change the Clk to MCLK = 4 MHz, ACLK = SMCLK = 1 MHz
	UCSCTL3 |= SELREF_2;                // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_4;                  // Set ACLK = REFO
	__bis_SR_register(SCG0);            // Disable the FLL control loop
	UCSCTL0 = 0x0000;                   // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                // Select DCO range 16 MHz operation
	UCSCTL2 = FLLD_1 + 121;  		    // (121 + 1) * 32768 = 4 MHz = MCLK
	UCSCTL5 = DIVA__4 + DIVS__4; 		// Divide MCLK with 4 => 1 MHz

	__bic_SR_register(SCG0);			// Enable the FLL control loop
}

void rtcSetup()
{
	RTCCTL01 |= RTCTEV_3 + RTCTEVIE;	// 32 bits overflow ACLK default, Interrupt Enable
}

void rtcStart(unsigned int rtcOffsetH, unsigned int rtcOffsetL)
{
	RTCTIM0 = 0xF000;//rtcOffsetL;				// if not reseted, reset
	RTCTIM1 = 0xFFFF;//rtcOffsetH; 				// changed to offset
	RTCCTL01 &= ~RTCHOLD; 				// Start RTC (Hold is writen to Zero)
	__bis_SR_register(LPM3_bits + GIE); // Sets the MCU into LPM3 => only (ACLK != 0)
}

void rtcStop()
{
	RTCCTL01 |= RTCHOLD;
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
	LPM3_EXIT;
	rtcStop(); 				// stop RTC

	if (loop1 > loopChange)
	{	// when lopp1 have count to loopChange then loop2 is enable
		loop2Mode = '1';	// Enable GSM loop
		startMode = '0';	// Disable Startup Mode

		loop2++;

		if (loop2 > loopChange2)
		{	// When loop2 reaches loopChange2 loop 2 will be
			loop1 = 0;
			loop2 = 0;
			loop2Mode = '0';
			timerAlarmFlag = '1';
		}
	}
	else
	{
		loop1++;
	}
	RTCCTL01 &= ~RTCTEVIFG;	// Clear RTC-Interrupt
}
