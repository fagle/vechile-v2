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

#include "hal/micro/cortexm3/memmap.h"
#include "hal/micro/cortexm3/flash.h"

//The default choice is an 8kByte SimEE.  Defining
//EMBER_SIM_EEPROM_4KB in your build or configuration header
//will consume 4kB of flash for the SimEE.  Both choices of 8kByte and
//4kByte of SimEE size allow for a maximum of 2kByte of data.

#if defined(EMBER_SIM_EEPROM_4KB)
  #define SIMEE_SIZE_B        4096  //Use a 4k 8bit SimEE
  #define SIMEE_BTS_SIZE_B    2048
#elif defined(EMBER_SIM_EEPROM_32KB)
  //WARNING
  //WARNING
  //  THE USE OF EMBER_SIM_EEPROM_32KB IS NOT OFFICIALLY SUPPORTED!
  //WARNING
  //WARNING
  #warning EMBER_SIM_EEPROM_32KB is not officially supported!  Do not use!
  #ifndef CORTEXM3_EM357
    #error Using EMBER_SIM_EEPROM_32KB is only valid on EM357 chips!
  #endif
  //Defining EMBER_SIM_EEPROM_32KB in your build or configuration header
  //will consume 32kB of flash for the SimEE and allows for a maximum
  //of 10kByte of data.
  #define SIMEE_SIZE_B        32768 //Use a 32k 8bit SimEE
  #define SIMEE_BTS_SIZE_B    10240
#elif (defined(EMBER_SIM_EEPROM_36KB) || defined(EMBER_SIM_EEPROM_V2))
  //NOTE: Using SimEE2 implicitly means using a 36kB SimEE.
  //WARNING
  //WARNING
  //  THE USE OF EMBER_SIM_EEPROM_36KB IS NOT OFFICIALLY SUPPORTED!
  //WARNING
  //WARNING
  #if !defined(CORTEXM3_EM357) && !defined(SIM_EEPROM_TEST)
    #error Using EMBER_SIM_EEPROM_36KB or EMBER_SIM_EEPROM_V2 is only valid on EM357 chips!
  #endif
  //Defining EMBER_SIM_EEPROM_36KB in your build or configuration header
  //will consume 36kB of flash for the SimEE and allows for a maximum
  //of 6kByte of data.
  #define SIMEE_SIZE_B        36864 //Use a 36k 8bit SimEE
  #define SIMEE_BTS_SIZE_B    6144
#else //EMBER_SIM_EEPROM_8KB
  #define SIMEE_SIZE_B        8192  //Use a 8k 8bit SimEE
  #define SIMEE_BTS_SIZE_B    2048
#endif
//NOTE: SIMEE_SIZE_B size must be a precise multiple of 4096.
#if ((SIMEE_SIZE_B%4096) != 0)
  #error Illegal SimEE storage size.  SIMEE_SIZE_B must be a multiple of 4096.
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
//subtracted by VPA_BASE (which is the same as SIMEE_BASE_ADDR_HW) to
//return back to the bottom of the simulatedEepromStorage area.
#define SIMEE_BASE_ADDR_HW  0x0100

//Define a variable that holds the actual SimEE storage and then place
//this storage at the proper location in the linker.
extern int8u simulatedEepromStorage[SIMEE_SIZE_B];

//these parameters frame the sim-eeprom and are derived from the location
//of the sim-eeprom as defined in memmap.h
/**
 * @brief The size of a physical flash page, in SimEE addressing units.
 */
extern const int16u REAL_PAGE_SIZE;
/**
 * @brief The size of a Virtual Page, in SimEE addressing units.
 */
extern const int16u VIRTUAL_PAGE_SIZE;
/**
 * @brief The number of physical pages in a Virtual Page.
 */
extern const int8u REAL_PAGES_PER_VIRTUAL;
/**
 * @brief The bottom address of the Left Virtual Page.
 * Only used in Simulated EEPROM 1.
 */
extern const int16u LEFT_BASE;
/**
 * @brief The top address of the Left Virtual Page.
 * Only used in Simulated EEPROM 1.
 */
extern const int16u LEFT_TOP;
/**
 * @brief The bottom address of the Right Virtual Page.
 * Only used in Simulated EEPROM 1.
 */
extern const int16u RIGHT_BASE;
/**
 * @brief The top address of the Right Virtual Page.
 * Only used in Simulated EEPROM 1.
 */
extern const int16u RIGHT_TOP;
/**
 * @brief The bottom address of Virtual Page A.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPA_BASE;
/**
 * @brief The top address of Virtual Page A.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPA_TOP;
/**
 * @brief The bottom address of Virtual Page B.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPB_BASE;
/**
 * @brief The top address of Virtual Page B.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPB_TOP;
/**
 * @brief The bottom address of Virtual Page C.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPC_BASE;
/**
 * @brief The top address of Virtual Page C.
 * Only used in Simulated EEPROM 2.
 */
extern const int16u VPC_TOP;
/**
 * @brief The memory address at which point erasure requests transition
 * from being "GREEN" to "RED" when the freePtr crosses this address.
 */
extern const int16u ERASE_CRITICAL_THRESHOLD;


/** @brief Gets a pointer to the token data.  
 *
 * NOTE: This function only works on Cortex-M3 chips since it requires
 * being able to return a pointer to flash memory.
 *
 * The Simulated EEPROM uses a RAM Cache
 * to hold the current state of the Simulated EEPROM, including the location
 * of the freshest token data.  This function simply uses the ID and
 * index parameters to access the pointer RAM Cache.  The absolute flash
 * address stored in the pointer RAM Cache is extracted and ptr is
 * set to this address. 
 *
 * Here is an example of calling this function:
 * @code
 * tokTypeStackCalData calData = {0,};
 * tokTypeStackCalData *calDataToken = &calData;
 * halStackGetIdxTokenPtrOrData(&calDataToken,
 *                              TOKEN_STACK_CAL_DATA,
 *                              index);
 * @endcode
 *
 * @param ptr  A pointer to where the absolute address of the data should 
 * be placed.
 *
 * @param ID    The ID of the token to get data from.  Since the token system
 * is designed to enumerate the token names down to a simple 8 bit ID,
 * generally the TOKEN_name is the parameter used when invoking this function.
 *
 * @param index If token being accessed is indexed, this parameter is
 * combined with the ID to formulate both the RAM Cache lookup as well
 * as the desired InfoWord to look for.  If the token is not an indexed token,
 * this parameter is ignored.
 *
 * @param len   The number of bytes being worked on.  This should always be
 * the size of the token (available from both the sizeof() intrinsic as well
 * as the tokenSize[] array).
 *
 */
void halInternalSimEeGetPtr(void *ptr, int8u compileId, int8u index, int8u len);

#endif //__PLAT_SIM_EEPROM_H__

#endif //DOXYGEN_SHOULD_SKIP_THIS

