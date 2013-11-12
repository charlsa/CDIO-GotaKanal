/*
 * Flash.h
 *
 *  Created on: 11 nov 2013
 *      Author: Peter
 */
#include <msp430.h>
#ifndef FLASH_H_
#define FLASH_H_

// Writes Position to flash segment D max length = 30 chars
void writePosition(char Value[]);
// Reads position from flash segment D. buffer arrayen fylls med chars och avslutas med # 
char readPosition(char* buffer);

void writeTolerance(int value);

void writeSensorHight(int value);

int readTolerance();

int readSensorHight();





#endif /* FLASH_H_ */
