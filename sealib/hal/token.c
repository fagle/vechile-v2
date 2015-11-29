
#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"
#include "sim-eeprom.h"

//special handling for the manufacturing tokens
#define DEFINETOKENS
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...)
#define TOKEN_MFG(name,creator,iscnt,isidx,type,arraysize,...) \
  const int16u TOKEN_##name = TOKEN_##name##_ADDRESS;
  #include "token-stack.h"
#undef TOKEN_DEF
#undef TOKEN_MFG
#undef DEFINETOKENS


#ifndef SER232
  #define SER232 0
#endif

#define TOKENDBG(x)    //#define TOKENDBG(x) x   // prints debug data from the token access functions

boolean tokensActive = FALSE;
#ifdef EMBER_EMU_TEST  // Strip emulator only code from official build
  // On emulators, the CIB and FIB come up as 0x00 which can confuse code that expects 0xFF for a token's erased state.  
  // If we're on emulators and the CIB/FIB is 0x00, fake 0xFF.
  boolean emuFib0x00 = TRUE;
  boolean emuCib0x00 = TRUE;
#endif //EMBER_EMU_TEST


EmberStatus halStackInitTokens ( void )
{
#if !defined(BOOTLOADER)
    tokTypeMfgFibVersion tokMfg;
    EmberStatus status=EMBER_ERR_FATAL;
    tokTypeStackNvdataVersion tokStack;
#endif
    tokensActive = TRUE;
  
#ifdef EMBER_EMU_TEST   // Strip emulator only code from official build
    // If we're on an emu, scan the FIB and CIB MFG tokens to see if we're in the emu's default state of 0x00.  
    // This means MFG tokens have not been loaded and we'll have to fake an erased, 0xFF, state.
    {
        int32u i;
        int8u * ptr;
        
        ptr = (int8u *)(DATA_BIG_INFO_BASE + 0x077E);
        for (i = 0x00; i < (0x07F8-0x077E); i ++) 
        {
            if (ptr[i] != 0x00) 
            {
                emuFib0x00 = FALSE;
                break;
            }
        }
        ptr = (int8u *)(DATA_BIG_INFO_BASE + 0x0810);
        for (i = 0x00; i < (0x08F0 - 0x0810); i ++) 
        {
            if (ptr[i] != 0x00) 
            {
                emuCib0x00 = FALSE;
                break;
            }
        }
    }
#endif //EMBER_EMU_TEST
  
    if (halInternalSimEeInit() != EMBER_SUCCESS) 
    {
        TOKENDBG(sea_printf("halInternalSimEeInit Attempt 1 fail\r\n");)
        if (halInternalSimEeInit() != EMBER_SUCCESS) 
        {
            TOKENDBG(sea_printf("halInternalSimEeInit Attempt 2 fail\r\n");)
            tokensActive = FALSE;
            return EMBER_SIM_EEPROM_INIT_2_FAILED;
        }
        TOKENDBG(} else {sea_printf("halInternalSimEeInit Successful\r\n");)
    }
  
  
#if !defined(BOOTLOADER)
    halCommonGetToken(&tokMfg, TOKEN_MFG_FIB_VERSION);
    halCommonGetToken(&tokStack, TOKEN_STACK_NVDATA_VERSION);
    
    if (CURRENT_MFG_TOKEN_VERSION != tokMfg || CURRENT_STACK_TOKEN_VERSION != tokStack) 
    {
        if ((CURRENT_MFG_TOKEN_VERSION != tokMfg) && (CURRENT_STACK_TOKEN_VERSION != tokStack)) 
        {
            status = EMBER_EEPROM_MFG_STACK_VERSION_MISMATCH;
        } 
        else if (CURRENT_MFG_TOKEN_VERSION != tokMfg) 
        {
            status = EMBER_EEPROM_MFG_VERSION_MISMATCH;
        } 
        else if (CURRENT_STACK_TOKEN_VERSION != tokStack) 
        {
            status = EMBER_EEPROM_STACK_VERSION_MISMATCH;
        }
#if defined(DEBUG)
        if(CURRENT_MFG_TOKEN_VERSION != tokMfg) 
        {
            // Even is we're a debug image, a bad manufacturing token version is a fatal error, so return the error status.
            tokensActive = FALSE;
            return status;
        }
        if (CURRENT_STACK_TOKEN_VERSION != tokStack) 
        {
            // Debug images with a bad stack token version should attempt to fix the SimEE before continuing on.
            TOKENDBG(sea_printf("Stack Version mismatch, reloading\r\n");)
            halInternalSimEeRepair(TRUE);
            if (halInternalSimEeInit() != EMBER_SUCCESS) 
            {
                TOKENDBG(sea_printf("halInternalSimEeInit Attempt 3 fail\r\n");)
                tokensActive = FALSE;
                return EMBER_SIM_EEPROM_INIT_3_FAILED;
            }
        }
#else //  node release image
        TOKENDBG(sea_printf("EEPROM_x_VERSION_MISMATCH (%d)\r\n",status);)
        tokensActive = FALSE;
#if defined(PLATFORMTEST) || defined(LEVEL_ONE_TEST)
        tokensActive = TRUE;             // Keep tokens active for test code.
#endif //defined(PLATFORMTEST)
        return status;
#endif
    }
#endif //!defined(BOOTLOADER)
  
    TOKENDBG(sea_printf("TOKENS ACTIVE\r\n");)
    return EMBER_SUCCESS;
}

void halInternalGetTokenData ( void *data, int16u ID, int8u index, int8u len )
{
    int8u i;
    int8u *ram = (int8u*)data;
  
    if (ID < 256)     // the ID is within the SimEEPROM's range, route to the SimEEPROM
    {
        if (tokensActive) 
        {
            halInternalSimEeGetData(data, ID, index, len);
        } 
        else 
        {
            TOKENDBG(sea_printf( "getIdxToken supressed.\r\n");)
        }
    } 
    else 
    {
        if (ID == MFG_EUI_64_LOCATION)     // MFG_EUI_64_LOCATION is a virtual token
        {
            // There are two EUI64's stored in the Info Blocks, Ember and Custom. 0x0A00 is the address used by the generic EUI64 token, 
            // and it is token.c's responbility to pick the returned EUI64 from either Ember or Custom.  Return the Custom EUI64 
            // if it is not all FF's, otherwise return the Ember EUI64.
            tokTypeMfgEui64 eui64;
            halCommonGetToken(&eui64, TOKEN_MFG_CUSTOM_EUI_64);
            if ((eui64[0] == 0xFF) && (eui64[1] == 0xFF) && (eui64[2] == 0xFF) && (eui64[3] == 0xFF) &&
                (eui64[4] == 0xFF) && (eui64[5] == 0xFF) && (eui64[6] == 0xFF) && (eui64[7]==0xFF)) 
            {
                halCommonGetToken(&eui64, TOKEN_MFG_EMBER_EUI_64);
            }
            for (i = 0x00; i < EUI64_SIZE; i ++)
                ram[i] = eui64[i];
        } 
        else 
        {
            // read from the Information Blocks.  The token ID is only the bottom 16bits of the token's actual address. Since the info 
            // blocks exist in the range DATA_BIG_INFO_BASE-DATA_BIG_INFO_END, we need to OR the ID with DATA_BIG_INFO_BASE to get the real address.
            int32u realAddress = (DATA_BIG_INFO_BASE | ID);
            int8u *flash = (int8u *)realAddress;
#ifdef EMBER_EMU_TEST      // Strip emulator only code from official build
            // If we're on an emulator and the MFG area is 0x00, fake the erased state of 0xFF.
            if ((emuFib0x00) && (ID < 0x0800)) 
            {
                for (i = 0x00; i < len; i ++) 
                {
                    ram[i] = 0xFF;
                }
                return;
            }
            if ((emuCib0x00) && (ID >= 0x0800)) 
            {
                for (i = 0x00; i < len; i ++) 
                {
                    ram[i] = 0xFF;
                }
                return;
            }
#endif // EMBER_EMU_TEST
            for (i = 0x00; i < len; i ++) 
            {
                ram[i] = flash[i];
            }
        }
    }
}

boolean simEeSetDataActiveSemaphore = FALSE;
void halInternalSetTokenData ( int16u ID, int8u index, void *data, int8u len )
{
    if (ID < 256)              // the ID is within the SimEEPROM's range, route to the SimEEPROM
    {
        if (tokensActive)      // You cannot interrupt SetData with another SetData!
        {
            assert(!simEeSetDataActiveSemaphore);
            simEeSetDataActiveSemaphore = TRUE;
            halInternalSimEeSetData(ID, data, index, len);
            simEeSetDataActiveSemaphore = FALSE;
        } 
        else 
        {
            TOKENDBG(sea_printf( "setIdxToken supressed.\r\n");)
        }
    } 
    else 
    {
        // The Information Blocks can only be written by an external tool! Something is making a set token call on a manufacturing token, 
        // and that is not allowed!
        assert(0);
    }
}

void halInternalIncrementCounterToken ( int8u ID )
{
    if (tokensActive) 
    {
        halInternalSimEeIncrementCounter(ID);
    } 
    else 
    {
        TOKENDBG(sea_printf( "IncrementCounter supressed.\r\n");)
    }
}

// The following interfaces are admittedly code space hogs but serve as glue interfaces to link creator codes to tokens for test code.
int16u getTokenAddress ( int16u creator )
{
#define DEFINETOKENS
    switch (creator) 
    {
#define TOKEN_MFG TOKEN_DEF
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
        case creator: return TOKEN_##name;
    #include "token-stack.h"
#undef TOKEN_MFG
#undef TOKEN_DEF
    };
#undef DEFINETOKENS
    return INVALID_EE_ADDRESS;
}

int8u getTokenSize ( int16u creator )
{
#define DEFINETOKENS
    switch (creator) 
    {
#define TOKEN_MFG TOKEN_DEF
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
        case creator: return sizeof(type);
    #include "token-stack.h"
#undef TOKEN_MFG
#undef TOKEN_DEF
    };
#undef DEFINETOKENS
    return 0x00;  
}

int8u getTokenArraySize ( int16u creator )
{
#define DEFINETOKENS
    switch (creator) 
    {
#define TOKEN_MFG TOKEN_DEF
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
        case creator: return arraysize;
    #include "token-stack.h"
#undef TOKEN_MFG
#undef TOKEN_DEF
    };
#undef DEFINETOKENS
    return 0x00;  
}

///////////////////////////////////////////////////////////////////////////////////////////
