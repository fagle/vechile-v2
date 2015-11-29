/*
 * File: micro.c
 * Description: EM3XX micro specific full HAL functions
 *
 *
 * Copyright 2008, 2009 by Ember Corporation. All rights reserved.          *80*
 */



#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "include/error.h"

#include "hal/hal.h"
#include "app/util/serial/serial.h"
#include "hal/micro/cortexm3/diagnostic.h"
#include "hal/micro/cortexm3/memmap.h"
#include "hal/micro/cortexm3/flash.h"


// halInit is called on first initial boot, not on wakeup from sleep.
void halInit(void)
{









  
  halCommonStartXtal();
  
  halInternalSetRegTrim(FALSE);
  
  GPIO_DBGCFG |= GPIO_DBGCFGRSVD;
  
  #ifndef DISABLE_WATCHDOG
    halInternalEnableWatchDog();
  #endif
  
  halCommonCalibratePads();
  
  halInternalInitBoard();
  
  halCommonSwitchToXtal();
  
  #ifndef DISABLE_RC_CALIBRATION
    halInternalCalibrateFastRc();
  #endif//DISABLE_RC_CALIBRATION
  
  halInternalStartSystemTimer();
  
  #ifdef INTERRUPT_DEBUGGING
    //When debugging interrupts/ATOMIC, ensure that our
    //debug pin is properly enabled and idle low.
    I_OUT(I_PORT, I_PIN, I_CFG_HL);
    I_CLR(I_PORT, I_PIN);
  #endif //INTERRUPT_DEBUGGING
}


void halReboot(void)
{
  halInternalSysReset(RESET_SOFTWARE_REBOOT);
}

void halPowerDown(void)
{
  halInternalPowerDownUart();
  
  halInternalPowerDownBoard();
}

// halPowerUp is called from sleep state, not from first initial boot.
void halPowerUp(void)
{
  halInternalPowerUpKickXtal();

  halCommonCalibratePads();

  halInternalPowerUpBoard();

  halInternalBlockUntilXtal();

  halInternalPowerUpUart();
}

void halStackProcessBootCount(void)
{
  //Note: Becuase this always counts up at every boot (called from emberInit),
  //and non-volatile storage has a finite number of write cycles, this will
  //eventually stop working.  Disable this token call if non-volatile write
  //cycles need to be used sparingly.
  halCommonIncrementCounterToken(TOKEN_STACK_BOOT_COUNTER);
}


PGM_P halGetResetString(void)
{
  // Table used to convert from reset types to reset strings.
  #define RESET_BASE_DEF(basename, value, string)  string,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)  /*nothing*/
  static PGM char resetStringTable[][4] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  return resetStringTable[halGetResetInfo()];
}

// Note that this API should be used in conjunction with halGetResetString
//  to get the full information, as this API does not provide a string for
//  the base reset type
PGM_P halGetExtendedResetString(void)
{
  // Create a table of reset strings for each extended reset type
  typedef PGM char ResetStringTableType[][4];
  #define RESET_BASE_DEF(basename, value, string)   \
                         }; static ResetStringTableType basename##ResetStringTable = {
  #define RESET_EXT_DEF(basename, extname, extvalue, string)  string,
  {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF
  
  // Create a table of pointers to each of the above tables
  #define RESET_BASE_DEF(basename, value, string)  (ResetStringTableType *)basename##ResetStringTable,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)  /*nothing*/
  static ResetStringTableType * PGM extendedResetStringTablePtrs[] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  int16u extResetInfo = halGetExtendedResetInfo();
  // access the particular table of extended strings we are interested in
  ResetStringTableType *extendedResetStringTable = 
                    extendedResetStringTablePtrs[RESET_BASE_TYPE(extResetInfo)];

  // return the string from within the proper table
  return (*extendedResetStringTable)[((extResetInfo)&0xFF)];
  
}

// Translate EM3xx reset codes to the codes previously used by the EM2xx.
// If there is no corresponding code, return the EM3xx base code with bit 7 set.
int8u halGetEm2xxResetInfo(void)
{
  int8u reset = halGetResetInfo();

  // Any reset with an extended value field of zero is considered an unknown
  // reset, except for FIB resets.
  if ( (RESET_EXTENDED_FIELD(halGetExtendedResetInfo()) == 0) && 
       (reset != RESET_FIB) ) {
     return EM2XX_RESET_UNKNOWN;
  }

 switch (reset) {
  case RESET_UNKNOWN:
    return EM2XX_RESET_UNKNOWN;
  case RESET_BOOTLOADER:
    return EM2XX_RESET_BOOTLOADER;
  case RESET_EXTERNAL:      // map pin resets to poweron for EM2xx compatibility
//    return EM2XX_RESET_EXTERNAL;  
  case RESET_POWERON:
    return EM2XX_RESET_POWERON;
  case RESET_WATCHDOG:
    return EM2XX_RESET_WATCHDOG;
  case RESET_SOFTWARE:
    return EM2XX_RESET_SOFTWARE;
  case RESET_CRASH:
    return EM2XX_RESET_ASSERT;
  default:
    return (reset | 0x80);      // set B7 for all other reset codes
  }
}

