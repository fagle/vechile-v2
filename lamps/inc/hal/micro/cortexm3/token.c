/** @file hal/micro/cortexm3/token.c
 * @brief Token implementation for the Cortex-M3/EM3XX chip.
 * See token.h for details.
 *
 * Author(s): Brooks Barrett
 *
 * Copyright 2007 by Ember Corporation. All rights reserved.                *80*
 */
#include PLATFORM_HEADER

#include "stack/include/ember.h"
#include "stack/include/error.h"
#include "hal/hal.h"
#include "app/util/serial/serial.h"
#include "hal/micro/sim-eeprom.h"

#ifndef SER232
  #define SER232 0
#endif

//prints debug data from the token access functions
#define TOKENDBG(x)
//#define TOKENDBG(x) x

boolean tokensActive = FALSE;











EmberStatus halStackInitTokens(void)
{
  #if !defined(BOOTLOADER) && !defined(EMBER_TEST)
    tokTypeMfgFibVersion tokMfg;
    EmberStatus status=EMBER_ERR_FATAL;
    tokTypeStackNvdataVersion tokStack;
  #endif
  tokensActive = TRUE;
  

























  
  if(halInternalSimEeInit()!=EMBER_SUCCESS) {
    TOKENDBG(emberSerialPrintf(SER232,"halInternalSimEeInit Attempt 1 fail\r\n");)
    if(halInternalSimEeInit()!=EMBER_SUCCESS) {
      TOKENDBG(emberSerialPrintf(SER232,"halInternalSimEeInit Attempt 2 fail\r\n");)
      tokensActive = FALSE;
      return EMBER_SIM_EEPROM_INIT_2_FAILED;
    }
    TOKENDBG(} else {emberSerialPrintf(SER232,"halInternalSimEeInit Successful\r\n");)
  }
  
  
  #if !defined(BOOTLOADER) && !defined(EMBER_TEST)
    halCommonGetToken(&tokMfg, TOKEN_MFG_FIB_VERSION);
    halCommonGetToken(&tokStack, TOKEN_STACK_NVDATA_VERSION);
    
    if (CURRENT_MFG_TOKEN_VERSION != tokMfg
        || CURRENT_STACK_TOKEN_VERSION != tokStack) {
      if((CURRENT_MFG_TOKEN_VERSION != tokMfg)
         && (CURRENT_STACK_TOKEN_VERSION != tokStack)) {
        status = EMBER_EEPROM_MFG_STACK_VERSION_MISMATCH;
      } else if(CURRENT_MFG_TOKEN_VERSION != tokMfg) {
        status = EMBER_EEPROM_MFG_VERSION_MISMATCH;
      } else if(CURRENT_STACK_TOKEN_VERSION != tokStack) {
        status = EMBER_EEPROM_STACK_VERSION_MISMATCH;
      }
      #if defined(DEBUG)
        if(CURRENT_MFG_TOKEN_VERSION != tokMfg) {
          //Even is we're a debug image, a bad manufacturing token version
          //is a fatal error, so return the error status.
          tokensActive = FALSE;
          return status;
        }
        if (CURRENT_STACK_TOKEN_VERSION != tokStack) {
          //Debug images with a bad stack token version should attempt to
          //fix the SimEE before continuing on.
          TOKENDBG(emberSerialPrintf(SER232,"Stack Version mismatch, reloading\r\n");)
          halInternalSimEeRepair(TRUE);
          if(halInternalSimEeInit()!=EMBER_SUCCESS) {
            TOKENDBG(emberSerialPrintf(SER232,"halInternalSimEeInit Attempt 3 fail\r\n");)
            tokensActive = FALSE;
            return EMBER_SIM_EEPROM_INIT_3_FAILED;
          }
        }
      #else //  node release image
        TOKENDBG(emberSerialPrintf(SER232,"EEPROM_x_VERSION_MISMATCH (%d)\r\n",status);)
        tokensActive = FALSE;
        #if defined(PLATFORMTEST) || defined(LEVEL_ONE_TEST)
          tokensActive = TRUE;  //Keep tokens active for test code.
        #endif //defined(PLATFORMTEST)
        return status;
      #endif
    }
  #endif //!defined(BOOTLOADER) && !defined(EMBER_TEST)
  
  TOKENDBG(emberSerialPrintf(SER232,"TOKENS ACTIVE\r\n");)
  return EMBER_SUCCESS;
}


void halInternalGetTokenData(void *data, int16u ID, int8u index, int8u len)
{  
  if(ID < 256) {
    //the ID is within the SimEEPROM's range, route to the SimEEPROM
    if(tokensActive) {
      halInternalSimEeGetData(data, ID, index, len);
    } else {
      TOKENDBG(emberSerialPrintf(SER232, "getIdxToken supressed.\r\n");)
    }
  } else {
    #ifdef EMBER_TEST
      assert(FALSE);
    #else //EMBER_TEST
      halInternalGetMfgTokenData(data,ID,index,len);
    #endif //EMBER_TEST
  }
}


void halInternalGetIdxTokenPtr(void *ptr, int16u ID, int8u index, int8u len)
{  
  if(ID < 256) {
    //the ID is within the SimEEPROM's range, route to the SimEEPROM
    if(tokensActive) {
      halInternalSimEeGetPtr(ptr, ID, index, len);
    } else {
      TOKENDBG(emberSerialPrintf(SER232, "getIdxToken supressed.\r\n");)
    }
  } else {
    #ifdef EMBER_TEST
      assert(FALSE);
    #else //EMBER_TEST
      int32u *ptrOut = (int32u *)ptr;
      int32u realAddress;
      
      //0x7F is a non-indexed token.  Remap to 0 for the address calculation
      index = (index==0x7F) ? 0 : index;
      
      realAddress = (DATA_BIG_INFO_BASE|ID) + (len*index);
      
      *ptrOut = realAddress;
    #endif //EMBER_TEST
  }
}


boolean simEeSetDataActiveSemaphore = FALSE;
void halInternalSetTokenData(int16u ID, int8u index, void *data, int8u len)
{
  if(ID < 256) {
    //the ID is within the SimEEPROM's range, route to the SimEEPROM
    if(tokensActive) {
      //You cannot interrupt SetData with another SetData!
      assert(!simEeSetDataActiveSemaphore);
      simEeSetDataActiveSemaphore = TRUE;
      halInternalSimEeSetData(ID, data, index, len);
      simEeSetDataActiveSemaphore = FALSE;
    } else {
      TOKENDBG(emberSerialPrintf(SER232, "setIdxToken supressed.\r\n");)
    }
  } else {
    //The Information Blocks can only be written by an external tool!
    //Something is making a set token call on a manufacturing token, and that
    //is not allowed!
    assert(0);
  }
}


void halInternalIncrementCounterToken(int8u ID)
{
  if(tokensActive) {
    halInternalSimEeIncrementCounter(ID);
  } else {
    TOKENDBG(emberSerialPrintf(SER232, "IncrementCounter supressed.\r\n");)
  }
}


#ifndef EMBER_TEST

// The following interfaces are admittedly code space hogs but serve
// as glue interfaces to link creator codes to tokens for test code.

int16u getTokenAddress(int16u creator)
{
  #define DEFINETOKENS
  switch (creator) {
    #define TOKEN_MFG TOKEN_DEF
    #define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
      case creator: return TOKEN_##name;
    #include "hal/micro/cortexm3/token-manufacturing.h"
    #include "stack/config/token-stack.h"
    #undef TOKEN_MFG
    #undef TOKEN_DEF
  };
  #undef DEFINETOKENS
  return INVALID_EE_ADDRESS;
}

int8u getTokenSize(int16u creator)
{
  #define DEFINETOKENS
  switch (creator) {
    #define TOKEN_MFG TOKEN_DEF
    #define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
      case creator: return sizeof(type);
    #include "hal/micro/cortexm3/token-manufacturing.h"
    #include "stack/config/token-stack.h"
    #undef TOKEN_MFG
    #undef TOKEN_DEF
  };
  #undef DEFINETOKENS
  return 0;  
}

int8u getTokenArraySize(int16u creator)
{
  #define DEFINETOKENS
  switch (creator) {
    #define TOKEN_MFG TOKEN_DEF
    #define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
      case creator: return arraysize;
    #include "hal/micro/cortexm3/token-manufacturing.h"
    #include "stack/config/token-stack.h"
    #undef TOKEN_MFG
    #undef TOKEN_DEF
  };
  #undef DEFINETOKENS
  return 0;  
}

#endif //EMBER_TEST

