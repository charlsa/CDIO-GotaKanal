/*
 * LevelMeasure.c
 *
 *  Created on: 3 okt 2013
 *      Author: hej
 */
#include "LevelMeasure.h"
int loopChange = 100;
int loopChange2 = 5;

void directionSetup()
{
	 trigPin_DIR |= trigPin_nr;					// Set output direction
	 trigPin &= ~trigPin_nr;					// Set pin low

	 P1DIR &= ~ECHO; 							// Echo pin
	 P1SEL = ECHO;								// set P1.2 to TA0
}

int mainFunctionSensor(int vector[], int dataLength, int* position, char* dataEnable, int* overflowCount)
{	// Temporary variabler to be able to use references
	unsigned int meanValue;
	int tmp_pos = *position;
	int tmp_overf = *overflowCount;
	char tmp_enable = *dataEnable;

	int value = measure(); 						// Make a measure ment with the ultra sonic sensor

	if(value != 0)
	{	// if not a overflow
		vector[tmp_pos] = value;				// store in data vector in main

		if(tmp_pos == 29)
		{	// When 30 values are stored, reset vector position.
			*position = 0;
			 tmp_pos++;
		}
		else if(tmp_pos == 5 && tmp_enable == 0)
		{	// Set dataEnable to insure that a mean value can be taken even if the position has been reseted
			tmp_enable = '1';
			*dataEnable = tmp_enable;
		}
		else
		{
			tmp_pos++;
			*position = tmp_pos;
		}

		if(tmp_pos > 5 || tmp_enable != 0)
		{	// If more than 5 values stored take mean value of the 5 last. (Maybe changed later)
			meanValue = meanMeasurement(dataLength, vector, &tmp_pos, 5);
		}
		else
		{}
		tmp_overf = 0;
		*overflowCount = tmp_overf;
	}
	else
	{
		tmp_overf++;
		*overflowCount = tmp_overf;
	}
	return meanValue;
}

unsigned int measure()
{	// Measures 10 values and store them in data, data is then sorted by size
	unsigned int data[DataLength];
	int i = 0, j=0;

	while(i < DataLength){
		triggerPulse();
		echo();
		SensorCalc(&data[i]);
		// Verifie that the data is correct
		if((data[i] < Oveflow && data[i] > Underflow) || j > 9){
			i++;
			j = 0;
		}
		j++;
		__delay_cycles(200);
	}
	sortData(data, DataLength);
	int value = 0;
	value = pickvalue(data, DataLength);
	__delay_cycles(20);
	return value;
}

int pickvalue(unsigned int data[], int length)
{	// Pick ...
	int countend = 0;
	int countstart = 0;
	int g = 0;
	int d = 0;

	for (d; d < length; d++)
	{
		if (data[d]< Underflow)
		{
			countstart++;
		}
		if (data[d] > Oveflow)
		{
			countend++;
		}
	}

	g = length - countend;
	g = g - countstart;
	g = g/2;
	g = g + countstart;

	if (data[g] <= Oveflow && data[g] >= Underflow)
	{
		return data[g];
	}
	return 0;
}


void sortData(unsigned int data[], int length)
{
    int i, j, tmp;
    for(j = 1; j < length; j++)    							// Start with 1 (not 0)
    {
    	tmp = data[j];
    	for(i = j - 1; (i >= 0) && (data[i] < tmp); i--)	// Smaller values move up
        {
    		data[i+1] = data[i];
        }
        data[i+1] = tmp;    								//Put key into its proper location
    }
}

void timerA0Setup()
{
	 TA0CTL = TASSEL_2 + MC_0 + ID_0 + TACLR; 				// SMCLK, stop, clearTAR, interrupt enable
	 TA0CCTL1 = CM_3 + CCIS_0 + CAP + SCCI;					// Capturer mode pos/neg flank, P1.2 input
}

void triggerPulse()
{
	/*
	 * Generate a 12.2 us pulse to trig the ultra sonic sensor
	 * The cycle delay is calculated by
	 * f_mclk*Pulse_time = (1 MHz)*(10 us) = 10 cyckes
	 */
	trigPin ^= trigPin_nr;
	__delay_cycles(30);
	trigPin ^= trigPin_nr;
}

void echo()
{
	/* 
	 * Starts timer in capture mode and waits on two interrupts.
	 * The timer start on the positive flank and ends on the negative.
	 * The time between the two flanks is stored in SonicEcho
	 * EdgeCount is used to insure 2 flanks
	 */
	EdgeCount = 0;
	SonicEcho = 0;
	TA0CCTL1 &= ~CCIFG;
	TA0CTL |= MC_2; 			// Start continius mode
	TA0CTL &= ~TAIFG;			// Reset TA1 interrupt/overflow flag
	TA0CCTL1 |= CCIE;			// Enable interrupt
	TA0R = 0x0000; 				// Reset timer

	while((TA0R < 0x9C40)); 	// Wait for timer to count to 9C40 = 40ms = max time
	TA0R = 0x0000;
	TA0CCTL1 &= ~CCIE;			// Disable catch interruptTA0CTL
	TA0CTL |= TACLR;
}

void SensorCalc(unsigned int* dist)
{	//	Converts the pulse width of the measuring echo to distance
	*dist = (SonicEcho/57);
}

unsigned int meanMeasurement(int length, int data[], int* pos, int number)
{
	unsigned int sum = 0;
	int tmp = *pos;

	if(tmp < (number-1))
	{ 	//	If position smaller than 5, ex 4-2 = 2 nr of numbers from the top of the vector // Debugga

		int tmp2 = 0;
		while(tmp != 0)
		{
			tmp2++;
			sum += data[tmp--];
		}

		length -= 1;
		int step = length;
		while((length-(number-tmp2)) != step)
		{ // pos = 0 and loop to tmp
			sum += data[step];
			step--;
		}
	}
	else
	{
		int endPos = (tmp-(number)); // +1
		tmp -= 1;
		while (tmp != (endPos-1)) 	// Get the last value
		{

			sum += data[tmp];
			tmp--;
		//	if (tmp == 0) break;
		}
	}
	return sum/number;
}

char evaluateData(int data, int normal, int upper, int lower, unsigned int* rtcTimerH, unsigned int* rtcTimerL, char* timerAlarmFlag)
{
	data = normal - data;
	int absValue = fabs(data);
	_no_operation(); 	// test
	if (data > 0)
	{	// Check if over normal the normal lvl
		if (absValue > upper)
		{	// Send alarm for high water lvl
			*rtcTimerL = 0x78FF;
			*rtcTimerH = 0xFC6C;
			loopChange = 100;
			loopChange2 = 5;
			return '+';
		}
		else if (absValue > 3*lower/4)
		{	// Used to return from alarm
			if(*timerAlarmFlag == '0')
			sendSMS("Nivan har atergatt!");
			return '0';
		}
		else if (absValue > (upper)/2)
		{	// Change RTC mode parameter
			*rtcTimerL = 0x6AFF;
			*rtcTimerH = 0xF545;
			loopChange = 38;
			loopChange2 = 2;
			return '0';
		}
		else if (absValue > (upper)/3)
		{	// Change RTC mode parameter
			*rtcTimerL = 0xC7FF;
			*rtcTimerH = 0xE363;
			loopChange = 15;
			loopChange2 = 1;
			return '0';
		}
		else
		{
			return '0';
		}
	}
	else if (data < 0)
	{	// Check if under the normal lvl
		if (absValue > lower)
		{ // send alarm for low water lvl
			_no_operation();// test
			*rtcTimerL = 0x78FF;
			*rtcTimerH = 0xFC6C;
			loopChange = 100;
			loopChange2 = 5;
			return '-';
		}
		else if (absValue > 3*lower/4)
		{	// Used to return form alarm
			_no_operation(); // test
			return '0';
		}
		else if (absValue > (lower)/2)
		{	// Change RTC mode parameter time offset2 = 3 min
			*rtcTimerL = 0x6AFF;
			*rtcTimerH = 0xF545;
			loopChange = 38;
			loopChange2 = 2;
			return '0';
		}
		else if (absValue > (lower)/3)
		{	// Change RTC mode parameter time offset3 = 8 min
			*rtcTimerL = 0xC7FF;
			*rtcTimerH = 0xE363;
			loopChange = 15;
			loopChange2 = 1;
			return '0';
		}
		else
		{
			return '0';
		}
	}
	else
	{
		return '0';
	}

}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void CCR1_ISR(void)
{
	if (EdgeCount == 0)
	{
		SonicEcho = TA0CCR1;
		EdgeCount++;
	}
	else if (EdgeCount == 1)
	{
		SonicEcho = TA0CCR1 - SonicEcho;
	}
	TA0CCTL1 &= ~CCIFG;			// Clear catch interruptflag
}
