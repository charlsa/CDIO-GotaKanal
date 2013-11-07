/*
 * CLK_RTC_setup.h
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */

#ifndef CLK_RTC_SETUP_H_
#define CLK_RTC_SETUP_H_

#include <msp430.h>

void clkDebug();

void clkSetup();

void rtcSetup();

void rtcStart();

void rtcStop();

__interrupt void RTC_ISR(void);

#endif /* CLK_RTC_SETUP_H_ */
