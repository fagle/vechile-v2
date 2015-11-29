/*
 * File: spi_driver_stub.c
 * Description: SPI driver stub APIs interface x Serial EEPROM: to be customized 
 * by user based on its SPI EEPROM.
 */

#include PLATFORM_HEADER
#include "eeprom.h"
#include "bootloader-common.h"
#include "bootloader-serial.h"

//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Generic SPI Routines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#if 0 
static void halSpiWrite(int8u txData) 
{
  // To be implemented
}

static int8u halSpiReadWrite(int8u txData) 
{
  int8u rxData = 0;

  // To Be  implemented

  return rxData;
}

static int8u halSpiRead(void) 
{
  // To Be  implemented
  return 0;
}
#endif 

//
// ~~~~~~~~~~~~~~~~~~~~~~~~ Device Specific Interface ~~~~~~~~~~~~~~~~~~~~~~~~~~
//

//#define EEPROM_SIZE           (256ul * 1024ul)   // 256 kBytes

void halEepromInit(void) 
{
   // To Be  implemented
}


//
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Standard EEPROM Interface ~~~~~~~~~~~~~~~~~~~~~~~~~
//

//
//halEepromRead
//
// address: the address in EEPROM to start reading
// data: write the data here
// len: number of bytes to read
//
// return: result of API call
//
//
int8u halEepromRead (int32u address, int8u *data, int16u len)
{
  // To be implemented
  return 0;
}

//
// halEepromWrite
//
// address: the address in EEPROM to start writing
// data: pointer to the data to write
// len: number of bytes to write
//
int8u halEepromWrite ( int32u address, int8u *data, int16u len)
{
  // To be implemented
  return 0;
}

