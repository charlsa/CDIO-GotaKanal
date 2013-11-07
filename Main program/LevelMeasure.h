/*
 * LevelMeasure.h
 *
 *  Created on: 3 okt 2013
 *      Author: Challe
 */

#ifndef LEVELMEASURE_H_
#define LEVELMEASURE_H_

#include <msp430.h>

#define trigPin P1OUT
#define trigPin_nr BIT7
#define trigPin_DIR P1DIR
#define BUTTON BIT6
#define ECHO BIT2
#define LED BIT0
#define DataLength 10
#define Oveflow 220
#define Underflow 35

unsigned int SonicEcho;
int EdgeCount;
//#include <msp430.h>


int measure();

void sortData(unsigned int data[], int length);

void directionSetup();

void timerA0Setup();

void triggerPulse();

void echo();

void SensorCalc(int* dist);

__interrupt void CCR1_ISR(void);

#endif /* LEVELMEASURE_H_ */
