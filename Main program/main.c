//#include <msp430.h>

#include "CLK_RTC_setup.h"
#include "powerControl.h"
#include "LevelMeasure.h"
#include "UART.h"
#include "GSM_module.h"

#include "msp430.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LengthOfSensordata 30

// used for change operation mode
int loopChange = 20;
int loopChange2 = 10;
char loop2Mode = '0';
char startMode = '1';

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Setup and pin configurations
    clkSetup();
    directionSetup();
    timerA0Setup();
    powerPinSetup();
    rtcSetup();
	initUART();
	pinGSM();
//	clkDebug();

	// test
	V4Stop();
	V5Stop();

	// Sensor variables
	int sensorValue = 0;
	int sensorData[LengthOfSensordata] = 0;
	char dataEnable = 0; 		// != 0 if the vector is filled ones
	int dataPosition = 0;
	int overflowCount = 0;
	char alarm = '0';

	// Parameters (From and to the flash)
	int lowerThresholds = 0;
	int upperThresholds = 5;
	int normalLvl = 40;			// Default higth over the water lvl

	// GSM decision variable
	char execution = '0';

	// RTC variable
	unsigned int rtcOffsetH;
	unsigned int rtcOffsetL;

	__enable_interrupt();

	while(1)
	{
		V5Start();

		if(loop2Mode == '1' || startMode == '1')
		{	// Power on the GSM regulator
//			if (/*GSM out of power*/)
//			{
				V4Start(); 	// Enable power to GSM
				pwrOn(); 	// if GSM off
//			}
//			else if (/*GSM not activated */)
//			{
//				pwrOn();
//			}
		}

		__delay_cycles(800000);
		sensorValue = mainFunctionSensor(sensorData, LengthOfSensordata, &dataPosition, &dataEnable, &overflowCount);

		if (loop2Mode == '1' || startMode == '1')
		{	// wait for connection and check if SMS
		//	execution = readSMS();
			if (execution == '0')
			{	// Nothing
				_no_operation();// test
			}
			else if (execution == 'S')
			{	// Status report

			}
			else if (execution == 'N')
			{	// Confirm Nr change

			}
			else if (execution == 'L')
			{	// Confirm changed normal level

			}
			else if (execution == 'T')
			{	// Confirm changed thresholds

			}
			else if (execution == 'E')
			{	// Enable SMS

			}
			else if (execution == 'D')
			{	// Disable SMS

			}
			else
			{	/* Nothing */	}
		}

		// if the GSM mode disable turn of the power
		if (loop2Mode != '1' && startMode != '1') V5Stop();

		if (dataEnable != 0 && overflowCount == 0)
		{	// Process the sensor value
			alarm = evaluateData(sensorValue, normalLvl, upperThresholds, lowerThresholds, &rtcOffsetH, &rtcOffsetL);
		}
		else if (overflowCount > 10)
		{	// Alarm overflow (Problem om man minskar RTC och något ligger ivägen!!!!)
			alarm = 'O';
		}
		else
		{}

		if (alarm != '0')
		{
			if (alarm == '+')
			{	// Alarm for high water lvl

			}
			else if (alarm == '-')
			{	// Alarm for low water lvl

			}
			else if (alarm == 'O')
			{	// Alarm for overflow

			}
		}
		rtcStart(rtcOffsetH, rtcOffsetL);
	}
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	 switch(UCA1IV){
	    case 0:break;             	// Vector 0 - no interrupt
	    case 2:                   	// Vector 2 - RXIF
	    	  uartRead(UCA1RXBUF);
	    	  UCA1IFG &= ~UCRXIFG;
	        break;
	    case 4:                 	// Vector 4 - TXIFG
	        break;
	    default: break;
    }
}
