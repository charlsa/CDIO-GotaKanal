//#include <msp430.h>
#include "CLK_RTC_setup.h"
#include "powerControl.h"
#include "LevelMeasure.h"
#include "UART.h"
#include "GSM_module.h"

#include "msp430.h"
#include <stdio.h>
#include <string.h>

#define LengthOfSensordata 30

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    // Setup and pin configurations
    clkSetup();
//    clockSetup1();
    directionSetup();
    timerA0Setup();
    powerPinSetup();
    rtcSetup();
	initUART();
	pinGSM();
//	clkDebug();
    // Start-Up
    // Set in Low-Power Mode 1

	// Sensor variables
	V4Stop();
	__delay_cycles(800000);

	int sensorValue = 0;
	int sensorData[LengthOfSensordata] = 0;
	char dataEnable = 0; 		// != 0 if the vector is filled ones
	int dataPosition = 0;
	int overflowCount = 0;
	char alarm = '0'; 			// 0 = Alarm false, 1 = true
	char DIP = readDIP();

	V5Start(); // enable power for sensor
	V4Start(); // Enable power to GSM
	__delay_cycles(800000);
	pwrOn();

	__enable_interrupt();

	while(1)
	{
    	// if RTC enable hold
		if (dataEnable != 0)
		initGSM();

    	// if time for GSM operation
		sensorValue = mainFunctionSensor(sensorData, LengthOfSensordata, &dataPosition, &alarm, &dataEnable, &overflowCount);
		__no_operation();
    	// Take measurement

    	// Cheak Data if alarme

    	// Send SMS

    	// Enable Low-Power Mode 3

		// if statment for
	//	V4Stop();

		if(sensorValue > 40)
			sendSMS("ALARM!");

		rtcStart();
	}
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	 switch(UCA1IV){
	    case 0:break;             // Vector 0 - no interrupt
	    case 2:                   // Vector 2 - RXIF
	    	  uartRead(UCA1RXBUF);
	    	  UCA1IFG &= ~UCRXIFG;
	        break;
	    case 4:                 // Vector 4 - TXIFG
	        break;
	    default: break;
    }
}
