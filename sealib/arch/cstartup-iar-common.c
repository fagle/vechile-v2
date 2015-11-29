
#include "config.h"
#include "diagnostic.h"
#include "mpu.h"
#include "micro.h"
#include "memmap.h"
#include "cstartup-iar-common.h"

#include "ember-types.h"
#include "bootloader-interface.h"

// Exit with an error if building for a bootload type that is not yet supported
// Currently this means an error will be triggered with the default (standalone)
// bootloader type, which is indicated by the abscense of a build option
#if !defined(APP_BTL) && !defined(SERIAL_UART_BTL) && !defined(NULL_BTL)
  #error Standalone bootloader not yet supported.  Must define NULL_BTL, APP_BTL, or SERIAL_UART_BTL
#endif


//=============================================================================
// Define the size of the call stack and define a block of memory for it.
// 
// Place the cstack area in a segment named CSTACK.  This segment is
// defined soley for the purpose of placing the stack.  Since this area
// is manually placed, it cannot be part of the normal data initialization
// and therefore must be declared '__no_init'.  Refer to reset handler for the
//initialization code and iar-cfg-common.icf for segment placement in memory.
// 
// halResetInfo, used to store crash information and bootloader parameters, is
// overlayed on top of the base of this segment so it can be overwritten by the 
// call stack.
// This assumes that the stack will not go that deep between reset and
// use of the crash or the bootloader data.
//=============================================================================
#ifndef CSTACK_SIZE
  // Right now we define the stack size to be for the worst case scenario,
  // ECC.  The ECC library uses stack for its calculations.  Empirically I have
  // seen it use as much as 1504 bytes for the 'key bit generate' operation.
  // So we add a 25% buffer: 1504 * 1.25 = 1880
  // Later we may want to conditionally change the stack based on whether
  // or not the customer is building a Smart Energy Application.
  #define CSTACK_SIZE  (470)  // *4 = 1880 bytes
#endif
__root __no_init int32u cstackMemory[CSTACK_SIZE] @ __CSTACK__;

// Reset cause and crash info live in a special RAM segment that is
// not modified during startup.  This segment is overlayed on top of the
// bottom of the cstack.
__root __no_init HalResetInfoType halResetInfo @ __RESETINFO__;

//=============================================================================
// Declare the address tables which will always live at well known addresses
//=============================================================================
__root __no_init const HalFixedAddressTableType halFixedAddressTable @ __FAT__; 

#ifdef NULL_BTL
// In the case of a NULL_BTL application, we define a dummy BAT
__root const HalBootloaderAddressTableType halBootloaderAddressTable @ __BAT__ = {
  { __section_end("CSTACK"),
    halEntryPoint,
    halNmiIsr,
    halHardFaultIsr,
    BOOTLOADER_ADDRESS_TABLE_TYPE,
    BAT_NULL_VERSION,
    NULL                    // No other vector table.
  },
  BL_EXT_TYPE_NULL,           //int16u bootloaderType;
  BOOTLOADER_INVALID_VERSION, //int16u bootloaderVersion;  
  &halAppAddressTable,
  PLAT,   //int8u platInfo;   // type of platform, defined in micro.h
  MICRO,  //int8u microInfo;  // type of micro, defined in micro.h
  PHY,    //int8u phyInfo;    // type of phy, defined in micro.h
  0,      //int8u reserved;   // reserved for future use  
  NULL,   // eblProcessInit
  NULL,   // eblProcess
  NULL    // eblDataFuncs
};
#else
// otherwise we just define a variable that maps to the real bootloader BAT
__root __no_init const HalBootloaderAddressTableType halBootloaderAddressTable @ __BAT__;
#endif

__root const HalAppAddressTableType halAppAddressTable @ __AAT__ = {
  { __section_end("CSTACK"),
    halEntryPoint,
    halNmiIsr,
    halHardFaultIsr,
    APP_ADDRESS_TABLE_TYPE,
    AAT_VERSION,
    __vector_table
  },
  PLAT,  //int8u platInfo;   // type of platform, defined in micro.h
  MICRO, //int8u microInfo;  // type of micro, defined in micro.h
  PHY,   //int8u phyInfo;    // type of phy, defined in micro.h
  sizeof(HalAppAddressTableType),  // size of aat itself
  SOFTWARE_VERSION,   // int16u softwareVersion
  0,  // int16u reserved
  0,  //int32u timestamp; // Unix epoch time of .ebl file, filled in by ebl gen
  "", //int8u imageInfo[IMAGE_INFO_SZ];  // string, filled in by ebl generation
  0,  //int32u imageCrc;  // CRC over following pageRanges, filled in by ebl gen
  { {0xFF, 0xFF},   //pageRange_t pageRanges[6];  // Flash pages used by app, 
    {0xFF, 0xFF},                                 // filled in by ebl gen
    {0xFF, 0xFF}, 
    {0xFF, 0xFF}, 
    {0xFF, 0xFF}, 
    {0xFF, 0xFF} 
  }, 
  __section_begin(__SIMEE__),                         //void *simeeBottom;
  {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    // bootloaderReserved
   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
  __section_begin(__DEBUG_CHANNEL__),   //void *debugChannelBottom;
  __section_begin(__NO_INIT__),         //void *noInitBottom;
  __section_end(__APP_RAM__),           //void *appRamTop;
  __section_end(__BSS__),               //void *globalTop;
  __section_end(__CSTACK__),            //void *cstackTop;  
  __section_end(__DATA_INIT__),         //void *initcTop;
  __section_end(__TEXT__),              //void *codeTop;
};



//=============================================================================
// Define the vector table as a HalVectorTableType.  __root ensures the compiler
// will not strip the table.  const ensures the table is placed into flash.
// @ "INTVEC" tells the compiler/linker to place the vector table in the INTVEC
// segment which holds the reset/interrupt vectors at address 0x00000000.
// 
// All Handlers point to a corresponding ISR.  The ISRs are prototyped above.
// The file isr-stubs.s79 provides a weak definition for all ISRs.  To
// "register" its own ISR, an application simply has to define the function
// and the weak stub will be overridden.
//
// The list of handlers are extracted from the NVIC configuration file.  The
// order of the handlers in the NVIC configuration file is critical since it
// translates to the order they are placed into the vector table here.
//=============================================================================
__root const HalVectorTableType __vector_table[] @ __INTVEC__ =
{
  { .topOfStack = __section_end(__CSTACK__) },
  #ifndef INTERRUPT_DEBUGGING
    #define EXCEPTION(vectorNumber, functionName, priorityLevel) \
      functionName,
  #else //INTERRUPT_DEBUGGING  
    // The interrupt debug behavior inserts a special shim handler before
    // the actual interrupt.  The shim handler then redirects to the 
    // actual table, defined below
    #define EXCEPTION(vectorNumber, functionName, priorityLevel) \
      halInternalIntDebuggingIsr,
    // PERM_EXCEPTION is used for any vectors that cannot be redirected
    // throught the shim handler.  (such as the reset vector)
    #define PERM_EXCEPTION(vectorNumber, functionName, priorityLevel) \
      functionName,
  #endif //INTERRUPT_DEBUGGING
  #include NVIC_CONFIG
  #undef  EXCEPTION
  #undef PERM_EXCEPTION
};

// halInternalClassifyReset() records the cause of the last reset here
static int16u savedResetCause;

void halInternalClassifyReset(void) 
{
  // Table used to convert from RESET_EVENT register bits to reset types
  static const int16u resetEventTable[] = {
    RESET_POWERON_HV,         // bit 0: RESET_PWRHV_BIT
    RESET_POWERON_LV,         // bit 1: RESET_PWRLV_BIT
    RESET_EXTERNAL_PIN,       // bit 2: RESET_NRESET_BIT
    RESET_WATCHDOG_EXPIRED,   // bit 3: RESET_WDOG_BIT
    RESET_SOFTWARE,           // bit 4: RESET_SW_BIT
    RESET_UNKNOWN,            // bit 5: RESET_DSLEEP_BIT
    RESET_FATAL_OPTIONBYTE,   // bit 6: RESET_OPTBYTEFAIL_BIT
    RESET_FATAL_LOCKUP,       // bit 7: RESET_CPULOCKUP_BIT
  };
  
  // It is possible for RSTB and POWER_HV to be set at the same time, which
  // happens when RSTB releases between HV power good and LV power good. (All
  // other reset events are mutually exclusive.) When both RSTB and POWER_HV
  // are set, POWER_HV should be returned as the cause. The bit test order --
  // from LSB to MSB -- ensures that it will.
  int16u resetEvent = RESET_EVENT & 
                        ( RESET_CPULOCKUP_MASK   |
                          RESET_OPTBYTEFAIL_MASK |
  // handled by cstartup  RESET_DSLEEP_MASK      |
                          RESET_SW_MASK          |
                          RESET_WDOG_MASK        |
                          RESET_NRESET_MASK      |
                          RESET_PWRLV_MASK       |
                          RESET_PWRHV_MASK );

  int16u cause = RESET_UNKNOWN;
  int i;

  for (i = 0; i < sizeof(resetEventTable)/sizeof(resetEventTable[0]); i++) {
    if (resetEvent & (1 << i)) {
      cause = resetEventTable[i];
      break;
    }
  }

  if (cause == RESET_SOFTWARE) {
    if((halResetInfo.crash.resetSignature == RESET_VALID_SIGNATURE) &&
       (RESET_BASE_TYPE(halResetInfo.crash.resetReason) < NUM_RESET_BASE_TYPES)) {
      // The extended reset cause is recovered from RAM
      // This can be trusted because the hardware reset event was software
      //  and additionally because the signature is valid
      savedResetCause = halResetInfo.crash.resetReason;
    } else {
      savedResetCause = RESET_SOFTWARE_UNKNOWN;
    }
    // mark the signature as invalid 
    halResetInfo.crash.resetSignature = RESET_INVALID_SIGNATURE;
  } else {
    savedResetCause = cause;
  }
}  

int8u halGetResetInfo(void)
{
  return RESET_BASE_TYPE(savedResetCause);
}

int16u halGetExtendedResetInfo(void)
{
  return savedResetCause;
}



#ifdef INTERRUPT_DEBUGGING
//=============================================================================
// If interrupt debugging is enabled, the actual ISRs are listed in this
// secondary interrupt table.  The halInternalIntDebuggingIsr will use this
// table to jump to the appropriate handler
//=============================================================================
__root const HalVectorTableType __real_vector_table[] = 
{
  { .topOfStack = __section_end("CSTACK") },
  #define EXCEPTION(vectorNumber, functionName, priorityLevel) \
    functionName,
    #include NVIC_CONFIG
  #undef EXCEPTION
};

//[[  Macro used with the interrupt latency debugging hooks
  #define ISR_TX_BIT(bit)  GPIO_PAOUT = (GPIO_PAOUT&(~PA7_MASK))|((bit)<<PA7_BIT)
//]]

//=============================================================================
// The halInternalDebuggingIsr will intercept all exceptions in order to 
// set interrupt debugging IO flags so that interrupt latency and other timings
// may be measured with a logic analyzer
//=============================================================================
void halInternalIntDebuggingIsr(void)
{
  boolean prevState = I_STATE(I_PORT,I_PIN);
  int32u exception;
  
  //[[ Additional debug output for printing, via printf, the name of the
  //   ISR that has been invoked and the time it took (in us).  The ISR must
  //   exceed a time threshold to print the name.
  #ifdef PRINT_ISR_NAME
  int32u beginTime = MAC_TIMER;
  int32u endTime;
  #endif //PRINT_ISR_NAME
  //]]
 
  I_SET(I_PORT, I_PIN);

  // call the actual exception we were supposed to go to.  The exception
  // number is conveniently available in the SCS_ICSR register
  exception = (SCS_ICSR & SCS_ICSR_VECACTIVE_MASK) >> SCS_ICSR_VECACTIVE_BIT;
  
  //[[  This is a little bit of additional debug output that serially shows
  //     which exception was triggered, so that long ISRs can be determined
  //     from a logic analyzer trace
  #if 0
  {
    int32u byte = (exception<<3) | 0x2;
    int32u i;
    for(i=0;i<11;i++) {
     ISR_TX_BIT(byte&0x1); //data bit
      byte = (byte>>1);
    }
    ISR_TX_BIT(1); //stop bit
  }
  #endif
  //]]

  __real_vector_table[exception].ptrToHandler();

  // In order to deal with the possibility of nesting, only clear the status
  // output if it was clear when we entered
  if(!prevState)
    I_CLR(I_PORT, I_PIN);
  //[[ Some additional debug output to show the ISR exited
  #if 0
  else {
    ISR_TX_BIT(0);
    ISR_TX_BIT(1);
  }
  #endif
  //]]
  
  //[[ Additional debug output for printing, via printf, the name of the
  //   ISR that has been invoked and the time it took (in us).  The ISR must
  //   exceed a time threshold to print the name.
  #ifdef PRINT_ISR_NAME
  endTime = MAC_TIMER;
  if((endTime-beginTime)>150) {
    void Debug ( const char * format, ... );
    Debug( "[%d:", (endTime-beginTime));
    switch(INT_ACTIVE) {
      case INT_DEBUG:
        Debug( "DEBUG");
      break;
      case INT_IRQD:
        Debug( "IRQD");
      break;
      case INT_IRQC:
        Debug( "IRQC");
      break;
      case INT_IRQB:
        Debug( "IRQB");
      break;
      case INT_IRQA:
        Debug( "IRQA");
      break;
      case INT_ADC:
        Debug( "ADC");
      break;
      case INT_MACRX:
        Debug( "MACRX");
      break;
      case INT_MACTX:
        Debug( "MACTX");
      break;
      case INT_MACTMR:
        Debug( "MACTMR");
      break;
      case INT_SEC:
        Debug( "SEC");
      break;
      case INT_SC2:
        Debug( "SC1");
      break;
      case INT_SC1:
        Debug( "SC1");
      break;
      case INT_SLEEPTMR:
        Debug( "SLEEPTMR");
      break;
      case INT_BB:
        Debug( "BB");
      break;
      case INT_MGMT:
        Debug( "MGMT");
      break;
      case INT_TIM2:
        Debug( "TIM2");
      break;
      case INT_TIM1:
        Debug( "TIM1");
      break;
    }
    Debug( "]");
  }
  #endif //PRINT_ISR_NAME
  //]]
}
#endif //INTERRUPT_DEBUGGING



