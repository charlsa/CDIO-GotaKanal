//#include <msp430.h>

#include "CLK_RTC_setup.h"
#include "powerControl.h"
#include "LevelMeasure.h"
#include "UART.h"
#include "GSM_module.h"
#include "Flash.h"

#include "msp430.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LengthOfSensordata 30

// used for change operation mode

char loop2Mode = '0';
char startMode = '1';
char timerAlarmFlag = '1';		// Enable = 1 ready to send

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Setup and pin configurations
    clkSetup();
    directionSetup();
    timerA0Setup();
    boardSetup();
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
	char dataEnable = 0; 				// != 0 if the vector is filled ones
	int dataPosition = 0;
	int overflowCount = 0;
	char alarm = '0';

	// Parameters (From and to the flash)
	int lowerThresholds = readFlashLowTolerance();
	int upperThresholds = readFlashHighTolerance();
	int normalLvl = readFlashSensorOffset();			// Default higth over the water lvl

	// GSM decision variable
	char execution = '0';
	char disableAlarmFlag = '0';	// Disable = 1

	// RTC variable time offset1 = 1 min
	unsigned int rtcOffsetH = 0xFC6C;
	unsigned int rtcOffsetL = 0x78FF;

	__enable_interrupt();

	_delay_cycles(100000);

	while(1)
	{
		V5Start();
		V4Start();



		if(loop2Mode == '1' || startMode == '1')
		{	// Power on the GSM regulator
//			int statusGSM = P8IN;
//			statusGSM &= BIT4;
		//	if (!(P8IN &~ BIT4))	/*GSM out of power*/
		//	{
				V4Start(); 	// Enable power to GSM
				pwrOnOff(); 	// if GSM off
		//	}
		}

		sensorValue = mainFunctionSensor(sensorData, LengthOfSensordata, &dataPosition, &dataEnable, &overflowCount);

		if (loop2Mode == '1' || startMode == '1')
		{	// wait for connection and check if SMS
			unsigned int i = 0;
			int statusGSM = P8IN;
			statusGSM &= BIT4;

			while ( (i <= 6500)) // Debug test test change ! at the first statment later... !(P8IN &= BIT4) ||
			{	// Wait until GSM status goes high or in <3 seconds. (change to timer...)
				__delay_cycles(185);
				i++;// test
			}
			initGSM();
			execution = readSMS();

			if (execution == '0')
			{	// Nothing

			}
			else if (execution == 'S')
			{	// Status report
				responseStatus("Status\n", (normalLvl-sensorValue));
				deleteSMS();
			}
			else if (execution == 'N')
			{	// Confirm Nr change
				responseNrChange("Nummerlista uppdaterad:\n");
				deleteSMS();
			}
			else if (execution == 'L')
			{	// Confirm changed normal level
				normalLvl = readFlashSensorOffset();
				responseLvlChange("Normalniva:\n", normalLvl);
				deleteSMS();
			}
			else if (execution == 'T')
			{	// Confirm changed thresholds
				lowerThresholds = readFlashLowTolerance();
				upperThresholds = readFlashHighTolerance();
				responseThChange("Toleransniva:\n", lowerThresholds, upperThresholds);
				deleteSMS();
			}
			else if (execution == 'E')
			{	// Enable SMS
				sendSMS("Modulen har blivit aktiverad");
				deleteSMS();
			}
			else if (execution == 'D')
			{	// Disable SMS
				sendSMS("Modulen har blivit inaktiverad");
			}
			else if (execution == 'A')
			{	// Disable SMS with when alarm
				sendSMS("Larmet stoppat");
				deleteSMS();
				disableAlarmFlag = '1';		// Reseted when the lvl goes back to normal.
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
			startGSMmodule();		// Change name to power blablabal
			unsigned int i = 0;
	//		int statusGSM = P8IN;
	//		statusGSM &= BIT4;

//			while (P8IN &~ BIT4 || i++ < 99) // Debug  Variable for att inte sicka vid i owf
			//			{	// Wait until GSM status goes high or in 3 seconds. (change to timer...)
				__delay_cycles(3000);
	//			statusGSM = P8IN;
	//			statusGSM &= BIT4;
				//	}

			if (alarm == '+')
			{	// Alarm for high water lvl
				if (disableAlarmFlag != '1' && timerAlarmFlag == '1')
				{
				//	sendAlarm("Hog vattneniva: ", (normalLvl-sensorValue));
					timerAlarmFlag = '0';
				}
			}
			else if (alarm == '-')
			{	// Alarm for low water lvl
				if (disableAlarmFlag != '1' && timerAlarmFlag == '1')
				{
				//	sendAlarm("Lag vattneniva: ", (sensorValue-normalLvl));
					timerAlarmFlag = '0';
				}
			}
			else if (alarm == 'O')
			{	// Alarm for overflow
				if (timerAlarmFlag == '1')
				{
					sendSMS("Sensor kan vara ur funktion");
					timerAlarmFlag = '0';
				}
			}

		}
		else if (alarm == '0' && timerAlarmFlag == '0')
		{	// return to normal mode
			disableAlarmFlag = '0';
			sendSMS("Vattennivan har atergott");
			timerAlarmFlag = '1';
			disableAlarmFlag = '0';
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
