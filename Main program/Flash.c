/*
 * Flash.c
 *
 *  Created on: 11 nov 2013
 *      Author: Peter
 */
#include "Flash.h"
#include <string.h>
// segment D 0x1800 -> 0x1879
// Segment C 0x1880 -> 128byte


// Reads Position to flash segment D
void writeFlashPosition(char Value[])
{
	int Length = 0;
	int j = 0;
	Length = strlen(Value);
	char * Flash_ptrD;                         // Initialize Flash pointer
	Flash_ptrD = (char *) 0x1800;

	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit

	char buffer[105];
	  for(j = 0; j < 105; j++)					// copy flash value to buffer
	  {
		  buffer[j] = *Flash_ptrD++;
	  }

	  for(j = 0; j < Length ; j++)				// copy flash value to buffer
	  {
		  buffer[j] = Value[j];				// Startsaving postion on index 0
	  }
	  buffer[j] = '#';						// End with #;


	Flash_ptrD = (char *) 0x1800;
	__disable_interrupt();                   	// 5xx Workaround: Disable global
                                            	// dissable interrupt while erasing. Re-Enable

  *(unsigned int *)Flash_ptrD = 0;				// Erase Flash
  FCTL1 = FWKEY+WRT;                        	// Set WRT bit for write operation

  for(j = 0; j < 105; j++)
  {
	  *Flash_ptrD++ = buffer[j];                 	// Write value to flash increase adress
  }


  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
}

// Reads Position from flashsegment D
void readFlashPosition(char* buffer)
{
int Length = 30;
int j = 0;
  //Length = strlen(buffer);
  char *Flash_ptrD;
  Flash_ptrD = (char *) 0x1800;             // Initialize Flash segment C ptr
  __disable_interrupt();                    // 5xx Workaround: Disable global

  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

  for(j = 0; j < Length; j++)
    {
	  buffer[j] = *Flash_ptrD++;
	  if(buffer[j] == '#')
	  {
		  buffer[j] = '\0';
		  break;
	  }
    }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
}


void writeFlashTolerance(int lowTolerance, int highTolerance)
{
	int j = 0;
	int * Flash_ptrD;           	            // Initialize Flash pointer
	Flash_ptrD = (int *) 0x1800;
	__disable_interrupt();                    	// 5xx Workaround: Disable global
	int buffer[128];
		  for(j = 0; j < 128; j++)				// copy flash value to buffer
		  {
			  buffer[j] = *Flash_ptrD++;
		  }
		  buffer[40] = lowTolerance; 			// Write value to buff
		  buffer[42] = highTolerance; 			// Write value to buff

		  Flash_ptrD = (int *) 0x1800;

	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  *(unsigned int *)Flash_ptrD = 0;			// Erase Flash
	  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

	  for(j = 0; j < 128 ; j++)					// Write to Flash
	  {
		  *Flash_ptrD++ = buffer[j];
	  }

	  FCTL1 = FWKEY;                            // Clear WRT bit
	  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
}


// Reads tolerance of type Int from flash segment D.
// tolerance is saved on adress 0X1850;
int readFlashLowTolerance()
{
	int j = 0;
	int k = 0;
	int * Flash_ptrD;                         // Initialize Flash pointer
	Flash_ptrD = (int *) 0x1800;
	__disable_interrupt();                    // 5xx Workaround: Disable global
	int buffer[128];
		  for(j = 0; j < 128; j++)		// copy flash value to buffer
		  {
			  buffer[j] = *Flash_ptrD++;
		  }
		  k = buffer[40]; 					// Treshold saved on position 40
  return k;
}
int readFlashHighTolerance()
{
	int j = 0;
	int k = 0;
	int * Flash_ptrD;                         // Initialize Flash pointer
	Flash_ptrD = (int *) 0x1800;
	__disable_interrupt();                    // 5xx Workaround: Disable global
	int buffer[128];
		  for(j = 0; j < 128; j++)		// copy flash value to buffer
		  {
			  buffer[j] = *Flash_ptrD++;
		  }
		  k = buffer[42]; 					// Treshold saved on position 40
  return k;
}

// Reads sensorhight of type Int from flash segment D.
// Attention!!  Tolerance and sensorhight is saved on same segment!
int readFlashSensorOffset()
{
		int j = 0;
		int k = 0;
		int * Flash_ptrD;                         // Initialize Flash pointer
		Flash_ptrD = (int *) 0x1800;
		__disable_interrupt();                    // 5xx Workaround: Disable global
		int buffer[128];
			  for(j = 0; j < 128; j++)		// copy flash value to buffer
			  {
				  buffer[j] = *Flash_ptrD++;
			  }
			  k = buffer[50]; 					// sensor hight saved on position 0
	  return k;
}

// Writes sensorhight of type Int from flash segment D.
// Attention!!  Tolerance and sensorhight is saved on same segment!
void writeFlashSensorOffset(int value)
{
	int j = 0;
	int * Flash_ptrD;                         // Initialize Flash pointer
	Flash_ptrD = (int *) 0x1800;
	__disable_interrupt();                    // 5xx Workaround: Disable global
	int buffer[128];
		  for(j = 0; j < 128; j++)		// copy flash value to buffer
		  {
			  buffer[j] = *Flash_ptrD++;
		  }
		  buffer[50] = value; // Write value to buff

		  Flash_ptrD = (int *) 0x1800;

	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  *(unsigned int *)Flash_ptrD = 0;
	  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

	  for(j = 0; j < 128 ; j++)					// Write to Flash
	  {
		  *Flash_ptrD++ = buffer[j];
	  }
	  FCTL1 = FWKEY;                            // Clear WRT bit
	  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
}

// Writes tele number to segment C, Starting adress is 0X1800
// other data is stored on the same segment so adress has to bee eaqual or greater then 0X1810
// index is the index position of the telephone nr (0-9)
void writeFlashTele(char Value[],int index)
{
	int Length = 0;
	int j = 0;
	Length = strlen(Value);

	char * Flash_ptrC;                         // Initialize Flash pointer
	Flash_ptrC = (char *) 0x1880;

	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit

	char buffer[105];
	  for(j = 0; j < 105; j++)					// copy flash value to buffer
	  {
		  buffer[j] = *Flash_ptrC++;
	  }

	  for(j = 0; j < Length ; j++)				// copy flash value to buffer
	  {
		  buffer[j+(index*13)] = Value[j];
	  }
	  buffer[j+(index*13)] = '\n';


	Flash_ptrC = (char *) 0x1880;
	__disable_interrupt();                   	// 5xx Workaround: Disable global
                                            	// dissable interrupt while erasing. Re-Enable

  *(unsigned int *)Flash_ptrC = 0;
  FCTL1 = FWKEY+WRT;                        	// Set WRT bit for write operation

  for(j = 0; j < 104; j++)
  {
	  *Flash_ptrC++ = buffer[j]; // Write value to flash increase adress
  }
  *Flash_ptrC = '\0';

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
}



void readFlashTele(char* buffer)
{
	int j = 0;
	char *Flash_ptrC;
	Flash_ptrC = (char *) 0x1880;             // Initialize Flash segment C ptr

	__disable_interrupt();                    // 5xx Workaround: Disable global
                                            // interrupt while erasing. Re-Enable
                                            // GIE if needed
	FCTL3 = FWKEY;                            // Clear Lock bit
	FCTL1 = FWKEY+ERASE;                      // Set Erase bit
 //FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

	for(j = 0; j < 104; j++)
    {
		buffer[j] = *Flash_ptrC++;
    }

	buffer[j] = '\0';

/*	for(j = 0; j < 104; j++)
    {
		buffer[j] = buffer[j];
    }*/

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	__enable_interrupt();
}

