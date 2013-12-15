//#include <msp430.h>

#include "CLK_RTC_setup.h"
#include "powerControl.h"
#include "LevelMeasure.h"
#include "UART.h"
#include "GSM_module.h"
#include "Flash.h"
#include "SMS.h"

#include "msp430.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LengthOfSensordata 30

// used for change operation mode

char loop2Mode = '0';
char startMode = '1';
char timerAlarmFlag = '1';

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
	readDip();
	// test
	V4Stop();
	V5Stop();

	// Sensor variables
	int sensorValue = 0;
	int sensorData[LengthOfSensordata] = {0};
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

	while(1)
	{
		V5Start();
		_no_operation();
		char c = '0';
		if(loop2Mode == '1' || startMode == '1')
		{	// Power on the GSM regulator
			V4Start(); 	// Enable power to GSMJa
			if(P8IN &= BIT4)
			{
				c = checkAT();
				if(c =='0') pwrOnOff();
			}
			else
			{
				pwrOnOff();
				Delay();
				c = '0';
			}
		}

		sensorValue = mainFunctionSensor(sensorData, LengthOfSensordata, &dataPosition, &dataEnable, &overflowCount);

		if (loop2Mode == '1' || startMode == '1')
		{	// wait for connection and check if SMS
			unsigned int i = 0;

			while(!(P8IN &= BIT4) || i <= 15)
			{
				i++;
			}
			if(c == '0') c = checkAT();

			if(c == '0')
			{
				V4Stop();		//? Varför?
				Delay();
				V5Start();
				V4Start();
				if(P8IN &= BIT4)
				{
					c = checkAT();
					if(c =='0') pwrOnOff();
				}
				else
				{
					pwrOnOff();
					Delay();
				}
			}
			initGSM();

			execution = readSMS();

			if (execution == '0')
			{	// Nothing
				_no_operation();// test
			}
			else if (execution == 'S')
			{	// Status report
				responseStatus("STATUS i ", (normalLvl-sensorValue));
				deleteSMS();
			}
			else if (execution == 'N')
			{	// Confirm Nr change
				responseNrChange("Nummerlista uppdaterad i ");
				deleteSMS();
			}
			else if (execution == 'L')
			{	// Confirm changed normal level
				normalLvl = readFlashSensorOffset();
				responseLvlChange("Offset i ", normalLvl);
				deleteSMS();
			}
			else if (execution == 'T')
			{	// Confirm changed thresholds
				lowerThresholds = readFlashLowTolerance();
				upperThresholds = readFlashHighTolerance();
				responseThChange("Toleranser i ", lowerThresholds, upperThresholds);
				deleteSMS();
			}
			else if (execution == 'E')
			{	// Enable SMS
				sendSMS("Modulen har blivit aktiverad i ");
				deleteSMS();
			}
			else if (execution == 'D')
			{	// Disable SMS
				sendSMS("Modulen har blivit inaktiverad i");

				deleteSMS();
			}
			else if (execution == 'A')
			{	// Disable SMS with when alarm
				sendSMS("Larmet stoppat");
				deleteSMS();
				disableAlarmFlag = '1';		// Reseted when the lvl goes back to normal.
			}
			else
			{	/* Nothing */  }
		}
		_no_operation();// test

		// if the GSM mode disable turn off the power
		if (loop2Mode != '1' && startMode != '1')
			{
				V5Stop();
			}

		if (dataEnable != 0 && overflowCount == 0)
		{	// Process the sensor value
			alarm = evaluateData(sensorValue, normalLvl, upperThresholds, lowerThresholds, &rtcOffsetH, &rtcOffsetL, &timerAlarmFlag);
		}
		else if (overflowCount > 10)
		{	// Alarm overflow (Problem om man minskar RTC och något ligger ivägen!!!!)
			alarm = 'O';
		}
		else
		{}
		_no_operation();
		if (alarm != '0')
		{
			if (loop2Mode != '1' && startMode != '1' && disableAlarmFlag != '1' && timerAlarmFlag == '1')
			{
				V5Start();
				V4Start();
				Delay();
				pwrOnOff();			// är vi säkra på att GSM är av när vi kommer hit?
				unsigned int count = 0;
				while(!(P8IN &= BIT4) || count < 1000)
				{
					count++;
					__delay_cycles(100);
				}
				c = checkAT();
				if(c == '0')
				{
					pwrOnOff();
				}
			}

			if (alarm == '+')
			{	// Alarm for high water lvl
				if (disableAlarmFlag != '1' && timerAlarmFlag == '1')
				{
					sendAlarm("Hog vattneniva i ", (normalLvl-sensorValue));
					timerAlarmFlag = '0';
				}
			}
			else if (alarm == '-')
			{	// Alarm for low water lvl
				if (disableAlarmFlag != '1' && timerAlarmFlag == '1')
				{
					sendAlarm("Lag vattneniva i ", (sensorValue-normalLvl));
					timerAlarmFlag = '0';
				}
			}
			else if (alarm == 'O' && timerAlarmFlag == '1')
			{	// Alarm for overflow
				sendSMS("Sensor kan vara ur funktion i");
				timerAlarmFlag = '0';
			}


		}
		else if (alarm == '0' && timerAlarmFlag == '1')
		{	// return to normal mode
			disableAlarmFlag = '0';
			// RTC for Repeat alarm
			// Send sms
		}
		rtcStart(rtcOffsetH, rtcOffsetL);
	}
}
