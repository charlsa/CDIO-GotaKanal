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
void writeFlashPosition(char Value[]);
// Reads position from flash segment D
void readFlashPosition(char* buffer);

void writeFlashTolerance(int lowTolerance,int highTolerance);

int readFlashLowTolerance();

int readFlashHighTolerance();

void writeFlashSensorOffset(int value);

int readFlashSensorOffset();

void writeFlashTele(char Value[],int index);

char readFlashTele(char* buffer);

#endif /* FLASH_H_ */
