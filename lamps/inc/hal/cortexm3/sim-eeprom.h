/** @file hal/micro/cortexm3/sim-eeprom.h
 * @brief Simulated EEPROM location definition.
 *
 * Author(s): Brooks Barrett
 *
 * Copyright 2007 by Ember Corporation. All rights reserved.                *80*
 */
 
/** @addtogroup simeeprom
 * By default, the EM35x Simulated EEPROM is designed to consume 8kB of
 * upper flash within which it will perform wear leveling.  It is possible
 * to reduce the reserved flash to only 4kB by defining EMBER_SIM_EEPROM_4KB
 * when building your application.
 *
 * While there is no specific EMBER_SIM_EEPROM_8KB, it is possible to use
 * the define EMBER_SIM_EEPROM_8KB for clarity in your application.
 *
 * @note Switching between an 8kB and 4kB Simulated EEPROM should be done
 * by rebuilding the entire application to ensure all sources recognize
 * the change.  The Simualted EEPROM is designed to seamlessly move
 * between 8kB and 4kB without erasing flash first.
 *
 * @note Switching between 8kB and 4kB Simulated EEPROM will <b>
 * automatically erase all </b> Simulated EEPROM tokens and reload
 * them from default values.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef __PLAT_SIM_EEPROM_H__
#define __PLAT_SIM_EEPROM_H__

#include "memmap.h"
#include "flash.h"

//The default choice is an 8kByte storage area, but defining
//EMBER_SIM_EEPROM_4KB in your build or configuration header
//will give you 4kB of storage.
#if defined(EMBER_SIM_EEPROM_4KB)
  #define SIMEE_SIZE_B        0x1000 //Use a 4k 8bit SimEE
#else //EMBER_SIM_EEPROM_8KB
  #define SIMEE_SIZE_B        0x2000 //Use a 8k 8bit SimEE
#endif

//These three define where the SimEE lives.  The reset of the defines/consts
//are derived from these three.
#define SIMEE_SIZE_HW       (SIMEE_SIZE_B/2)
#define FLASH_PAGE_SIZE_HW  (MFB_PAGE_SIZE_W*2) //derived from flash.h/regs.h
//This is confusing, so pay attention...  :-)
//The actual SimEE storage lives inside of simulatedEepromStorage and begins
//at the very bottom of simulatedEepromStorage and fills the entirety of
//this storage array.  The SimEE code, though, uses 16bit addresses for
//everything since it was originally written for the XAP2b.  On a 250,
//the base address was 0xF000 since this corresponded to the actual absolute
//address that was used in flash.  On a non-250, though, the base address
//is largely irrelevant since there is a translation shim layer that converts
//SimEE 16bit addresses into the real 32bit addresses needed to access flash.
//But, the base address *must* be greater than 0x00FF because the pointer
//cache lookup code uses anything less than 0xFF to indicate an indirect
//cache reference instead of a real address.  Therefore, SIMEE_BASE_ADDR_HW
//is set to 0x0100.  If you look at the translation shim layer in
//sim-eeprom-internal.c you'll see that the address used by the SimEE is
//subtracted by LEFT_BASE (which is the same as SIMEE_BASE_ADDR_HW) to
//return back to the bottom of the simulatedEepromStorage area.
#define SIMEE_BASE_ADDR_HW  0x0100

//Define a variable that holds the actual SimEE storage and then place
//this storage at the proper location in the linker.
extern int8u simulatedEepromStorage[SIMEE_SIZE_B];

//these parameters frame the sim-eeprom and are derived from the location
//of the sim-eeprom as defined in memmap.h
/**
 * @brief The number of Virtual Pages in the Simulated EEPROM (always 2).
 */
extern const int8u VIRTUAL_PAGES;
/**
 * @brief The bottom address of the Left Virtual Page.
 */
extern const int16u LEFT_BASE;
/**
 * @brief The top address of the Left Virtual Page.
 */
extern const int16u LEFT_TOP;
/**
 * @brief The bottom address of the Right Virtual Page.
 */
extern const int16u RIGHT_BASE;
/**
 * @brief The top address of the Right Virtual Page.
 */
extern const int16u RIGHT_TOP;
/**
 * @brief The memory address at which point erasure requests transition
 * from being "GREEN" to "RED" when the freePtr crosses this address.
 */
extern const int16u ERASE_CRITICAL_THRESHOLD;

#endif //__PLAT_SIM_EEPROM_H__

#endif //DOXYGEN_SHOULD_SKIP_THIS

