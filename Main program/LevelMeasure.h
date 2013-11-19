/*
 * LevelMeasure.h
 *
 *  Created on: 3 okt 2013
 *      Author: Challe
 */

#ifndef LEVELMEASURE_H_
#define LEVELMEASURE_H_

#include <msp430.h>
#include "math.h"

#define trigPin P11OUT
#define trigPin_nr BIT2
#define trigPin_DIR P11DIR
#define BUTTON BIT6
#define ECHO BIT2
#define LED BIT0
#define DataLength 10
#define Oveflow 220
#define Underflow 35

unsigned int SonicEcho;
int EdgeCount;

unsigned int measure();

int mainFunctionSensor(int data[], int dataLength, int* position, char* dataEnable, int* overflowCount);

void sortData(unsigned int data[], int length);

void directionSetup();

void timerA0Setup();

void interuptPin();

void triggerPulse();

void echo();

void SensorCalc(unsigned int* dist);

unsigned int meanMeasurement(int length, unsigned int data[], int* pos, int number);

char evaluateData(int data, int normal, int upper, int lower, unsigned int* rtcTimeH, unsigned int* rtcTimeL);

__interrupt void CCR1_ISR(void);

#endif /* LEVELMEASURE_H_ */
