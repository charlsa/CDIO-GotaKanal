//#include <msp430.h>
#include "CLK_RTC_setup.h"
#include "powerControl.h"
#include "LevelMeasure.h"

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    // Setup and pin configurations
    clkSetup();
    directionSetup();


    // Start-Up
    // write EEPROM
    // Set in Low-Power Mode 1

    while(1){
    	// if RTC enable hold

    		// if time for GSM operation

    		// Take measurement

    		// Cheak Data if alarme

    		// Send SMS

    	// Enable Low-Power Mode 3
    }
	return 0;
}
