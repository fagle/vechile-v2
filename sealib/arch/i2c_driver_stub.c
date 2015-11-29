/*
 * File: i2c_driver_stub.c
 * Description: I2C driver stub APIs interface x Serial EEPROM: to be customized by user
 * based on its I2C EEPROM.
 *
 */

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "hal/hal.h"
#include "eeprom.h"

//#define EEPROM_SIZE           (128ul * 1024ul)   // 128 kBytes

//#define EEPROM_CTL 0x50               // Control code + chip select bits

/* Initialize serial controller SC2 for 400kbps I2C operation.
 *
 * param: none
 * 
 * return none
 */
void halEepromInit(void)
{
  // To be implemented
}

//
// ~~~~~~~~~~~~~~~~~ Standard EEPROM Interface ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
