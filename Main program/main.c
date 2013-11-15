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

// Global flags...
int loop1 = 0;
int loopChange = 20;
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

    // Start-Up
	char startMode = '1';
	char execution = '0';

	// Sensor variables
	int sensorValue = 0;
	int sensorData[LengthOfSensordata] = 0;
	char dataEnable = 0; 		// != 0 if the vector is filled ones
	int dataPosition = 0;
	int overflowCount = 0;

	// Parameters
	int lowerThresholds = 0;
	int upperThresholds = 0;
	int normalLvl = 0;

	// GSM decision variable

	V5Start(); // enable power for sensor
	V4Start(); // Enable power to GSM
	__delay_cycles(800000);
	pwrOn();

	__enable_interrupt();

	while(1)
	{
		V5Start();
		if(loop2Mode == '1' || startMode == '1')
		{	// Power on the GSM regulator
			// pwrOn(); // if GSM off
		}

		sensorValue = mainFunctionSensor(sensorData, LengthOfSensordata, &dataPosition, &dataEnable, &overflowCount);

		if (loop2Mode == '1' || startMode == '1')
		{	// wait for connection and check if SMS
			execution = readSMS();
			if (execution == '0')
			{	// Nothing
				break;
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
			//evaluateData();
			sensorValue = normalLvl - sensorValue ;
			int absValue = fabs(sensorValue);
			if (sensorValue > 0)
			{	// Check if over normal the normal lvl
				if (absValue > upperThresholds)
				{	// Send alarm for high water lvl

				}
				else if (absValue > (upperThresholds)/2)
				{	// Change RTC mode parameter

				}
				else if (absValue > (upperThresholds)/3)
				{	// Change RTC mode parameter

				}
				else ;
			}
			else if (sensorValue < 0)
			{	// Check if under the normal lvl
				if (absValue > lowerThresholds)
				{ // send alarm for low water lvl

				}
				else if (absValue > (lowerThresholds)/2)
				{	// Change RTC mode parameter

				}
				else if (absValue > (lowerThresholds)/3)
				{	// Change RTC mode parameter

				}
				else ;
			}
			else if (overflowCount > 5)
			{	// Alarm overflow (Problem om man minskar RTC och något ligger ivägen!!!!)

			}
			else {}
		}

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
