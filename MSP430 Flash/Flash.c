/*
 * Flash.c
 *
 *  Created on: 11 nov 2013
 *      Author: Peter
 */
#include "Flash.h"

void writePosition(char Value[])
{
	int Length = 0;
	int j = 0;
	Length = strlen(Value);
	char * Flash_ptr;                         // Initialize Flash pointer
	Flash_ptr = (char *) 0x1800;
	__disable_interrupt();                    // 5xx Workaround: Disable global
                                            // interrupt while erasing. Re-Enable
                                            // GIE if needed
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
  *(unsigned int *)Flash_ptr = 0;
  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

  for(j = 0; j < Length; j++)
  {
	  *Flash_ptr++ = Value[j];                 // Write value to flash increase adress
  }
  *Flash_ptr++ = '#';

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();

}

char readPosition(char* buffer)
{
int Length = 30;
int j = 0;
  //Length = strlen(buffer);
  char *Flash_ptrD;

  Flash_ptrD = (char *) 0x1800;             // Initialize Flash segment C ptr
  Flash_ptrD = Flash_ptrD;
  __disable_interrupt();                    // 5xx Workaround: Disable global
                                            // interrupt while erasing. Re-Enable
                                            // GIE if needed
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

  for(j = 0; j < Length; j++)
    {
	  buffer[j] = *Flash_ptrD++;
	  if(buffer[j] == '#')
	  {
		  break;
	  }
    }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
}

// Writes value of type Int to flash segment c.
// tolerans value is placed on adress 0x1880
void writeTolerance(int value)
{
	int adress = 1;
	  int * Flash_ptr;                         // Initialize Flash pointer
	  Flash_ptr = (int *) 0x1880;
	  Flash_ptr = Flash_ptr + adress;
	  __disable_interrupt();                    // 5xx Workaround: Disable global
	                                            // interrupt while erasing. Re-Enable
	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  //*(unsigned int *)Flash_ptr = 0;
	  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

	  *Flash_ptr = value;                 // Write value to flash
	  FCTL1 = FWKEY;                            // Clear WRT bit
	  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
}


// Reads tolerance of type Int from flash segment c.
// tolerance is saved on adress 2;
int readTolerance()
{
int adress = 1;
  int k;
  int *Flash_ptrC;

  Flash_ptrC = (int *) 0x1880;             // Initialize Flash segment C ptr
  Flash_ptrC = Flash_ptrC + adress;
  __disable_interrupt();                    // 5xx Workaround: Disable global
                                            // interrupt while erasing. Re-Enable
                                            // GIE if needed
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation
   k = *Flash_ptrC;
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
  return k;
}

int readSensorHight()
{
	  int k;
	  int *Flash_ptrC;

	  Flash_ptrC = (int *) 0x1884;             // Initialize Flash segment C ptr
	  __disable_interrupt();                    // 5xx Workaround: Disable global
	                                            // interrupt while erasing. Re-Enable
	                                            // GIE if needed
	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation
	   k = *Flash_ptrC;
	  FCTL1 = FWKEY;                            // Clear WRT bit
	  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
	  return k;
}
// Writes value of type Int to flash segment c.
// Sensor Hight value is placed on adress 0x1884
void writeSensorHight(int value)
{
	  int * Flash_ptr;                         // Initialize Flash pointer
	  Flash_ptr = (int *) 0x1884;
	  __disable_interrupt();                    // 5xx Workaround: Disable global
	                                            // interrupt while erasing. Re-Enable
	  FCTL3 = FWKEY;                            // Clear Lock bit
	  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
	  //*(unsigned int *)Flash_ptr = 0;
	  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

	  *Flash_ptr = value;                 // Write value to flash
	  FCTL1 = FWKEY;                            // Clear WRT bit
	  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
}



