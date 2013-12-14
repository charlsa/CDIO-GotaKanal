/*
 * powerControl.h
 *
 *  Created on: 7 nov 2013
 *      Author: Challe
 */

#ifndef POWERCONTROL_H_
#define POWERCONTROL_H_

#include <msp430.h>

void boardSetup();

void chargerStart();

void stopCharger();

void V4Start();

void V4Stop();

void V5Start();

void V5Stop();

void readDip();

void startGSMmodule();


#endif /* POWERCONTROL_H_ */
