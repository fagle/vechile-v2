
#include "config.h"
#include "ember.h"
#include "hal.h"
#include "serial.h"
#include "diagnostic.h"

//------------------------------------------------------------------------------
// Preprocessor definitions

// Reserved instruction executed after a failed assert to cause a usage fault
#define ASSERT_USAGE_OPCODE 0xDE42

// Codes for DMA channels in DMA_PROT_CH
#define DMA_PROT_CH_SC1_RX    1
#define DMA_PROT_CH_GP_ADC    3
#define DMA_PROT_CH_MAC       4
#define DMA_PROT_CH_SC2_RX    5


//------------------------------------------------------------------------------
// Local Variables

static int8u* cfsrBits[] =
{

  // Memory management (MPU) faults 
  "IACCVIOL: attempted instruction fetch from a no-execute address",  // B0 
  "DACCVIOL: attempted load or store at an illegal address",          // B1
  "",                                                                 // B2
  "MUNSTKERR: unstack from exception return caused access violation", // B3
  "MSTKERR: stacking from exception caused access violation",         // B4
  "",                                                                 // B5
  "",                                                                 // B6
  "MMARVALID: MMAR contains valid fault address",                     // B7

  // Bus faults
  "IBUSERR: instruction prefetch caused bus fault",                   // B8
  "PRECISERR: precise data bus fault",                                // B9
  "IMPRECISERR: imprecise data bus fault",                            // B10
  "UNSTKERR: unstacking on exception return caused data bus fault",   // B11
  "STKERR: stacking on exception entry caused data bus fault",        // B12
  "",                                                                 // B13
  "",                                                                 // B14
  "BFARVALID: BFAR contains valid fault address",                     // B15

  // Usage faults
  "UNDEFINSTR: tried to execute an undefined instruction",            // B16
  "INVSTATE: invalid EPSR - e.g., tried to switch to ARM mode",       // B17
  "INVPC: attempted to illegally load EXC_RETURN into PC",            // B18
  "NOCP: attempted to execute a coprocessor instruction",             // B19
  "",                                                                 // B20
  "",                                                                 // B21
  "",                                                                 // B22
  "",                                                                 // B23
  "UNALIGNED: attempted an unaligned memory access",                  // B24
  "DIVBYZERO: attempted to execute SDIV or UDIV with divisor of 0"    // B25
};

static int8u* afsrBits[] =
{
  "",                                                                 // B0
  "RESERVED: attempted access past last peripheral register address", // B1
  "PROTECTED: attempted user mode write to privileged peripheral",    // B2
  "WRONGSIZE: attempted 8/16-bit access to peripheral register"       // B3
};

static int8u* intActiveBits[] =
{
  "Timer1",       // B0
  "Timer2",       // B1
  "Management",   // B2
  "Baseband",     // B3
  "Sleep_Timer",  // B4
  "SC1",          // B5
  "SC2",          // B6
  "Security",     // B7
  "MAC_Timer",    // B8
  "MAC_TX",       // B9
  "MAC_RX",       // B10
  "ADC",          // B11
  "IRQ_A",        // B12
  "IRQ_B",        // B13
  "IRQ_C",        // B14
  "IRQ_D",        // B15
  "Debug"         // B16
};

// Names of raw crash data items - each name is null terminated, and the
// end of the array is flagged by two null bytes in a row.
// NOTE: the order of these names must match HalCrashInfoType members.
static const char nameStrings[] = "R0\0R1\0R2\0R3\0"
                                  "R4\0R5\0R6\0R7\0"
                                  "R8\0R9\00R10\0R11\0"
                                  "R12\0R13(LR)\0MSP\0PSP\0"
                                  "PC\0xPSR\0MSP used\0PSP used\0"
                                  "ICSR\0SHCSR\0INT_ACTIVE\0CFSR\0"
                                  "HFSR\0DFSR\0MMAR/BFAR\0AFSR\0"
                                  "Ret0\0Ret1\0Ret2\0Ret3\0"
                                  "Ret4\0Ret5\0Dat0\0Dat1\0";

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Functions

static void halInternalAssertFault(PGM_P filename, int linenumber)
{
  // Cause a usage fault by executing a special UNDEFINED instruction.
  // The high byte (0xDE) is reserved to be undefined - the low byte (0x42)
  // is arbitrary and distiguishes a failed assert from other usage faults.
  // the fault handler with then decode this, grab the filename and linenumber
  // parameters from R0 and R1 and save the information for display after
  // a reset 
  asm("DC16 0DE42h");
}

void halInternalAssertFailed(PGM_P filename, int linenumber)
{
  halResetWatchdog();              // In case we're close to running out.
  INTERRUPTS_OFF();

  #if !defined(EMBER_ASSERT_OUTPUT_DISABLED)
   sea_printf( "\r\n[ASSERT:%p:%d]\r\n",filename,linenumber);
  #endif
  
  #if defined (__ICCARM__)
    // With IAR, we can use the special fault mechanism to preserve more assert
    //  information for display after a crash
    halInternalAssertFault(filename, linenumber);
  #else
    // Other toolchains don't handle the inline assembly correctly, so
    // we just call the internal reset
    halResetInfo.crash.data.assert.file = (const char *)halResetInfo.crash.R0;
    halResetInfo.crash.data.assert.line = halResetInfo.crash.R1;
    halInternalSysReset(RESET_CRASH_ASSERT);
  #endif
}

// After the low-level fault handler (in faults.s79) has saved the processor
// registers (R0-R12, LR and both MSP an PSP), it calls halInternalCrashHandler
// to finish saving additional crash data. This function returns the reason for
// the crash to the low-level fault handler that then calls 
// halInternalSystsemReset() to reset the processor.
//
// NOTE:
// This function should not use more than 16 words on the stack to avoid
// overwriting halResetInfo at the bottom of the stack segment.
// The 16 words include this function's return address, plus any stack
// used by functions called by this one. The stack size allowed is defined
// by the symbol CRASH_STACK_SIZE in faults.s79.
// As compiled by IAR V5.30, it now uses 8 words (1 for its return address,
// 6 for registers pushed onto the stack and 1 for the return address of
// halGetMainStackBytesUsed().
//
int16u halInternalCrashHandler(void)
{
  int32u activeException;
  int16u reason = RESET_FAULT_UNKNOWN;
  HalCrashInfoType *c = &halResetInfo.crash;
  int8u i, j;
  int32u *sp, *s; 
  int32u data;

  c->icsr.word = SCS_ICSR;
  c->shcsr.word = SCS_SHCSR;
  c->intActive.word = INT_ACTIVE;
  c->cfsr.word = SCS_CFSR;
  c->hfsr.word = SCS_HFSR;
  c->dfsr.word = SCS_DFSR;
  c->faultAddress = SCS_MMAR;
  c->afsr.word = SCS_AFSR;

  // Examine B2 of the saved LR to know the stack in use when the fault occurred 
  sp = (int32u *)((c->LR & 4) ? c->processSP : c->mainSP);

  // If the stack pointer is valid, read and save the stacked PC and xPSR
  if ( ((void *)sp >= STACK_SEGMENT_BEGIN) 
       && ((void *)sp <= STACK_SEGMENT_END)) { 
    sp += 6;
    c->PC = *sp++;
    c->xPSR.word = *sp++;
   
    // See if fault was due to a failed assert. This is indicated by 
    // a usage fault caused by executing a reserved instruction.
   if ( c->icsr.bits.VECTACTIVE == USAGE_FAULT_VECTOR_INDEX &&
        *(int16u *)(c->PC) == ASSERT_USAGE_OPCODE ) {
      // Copy halInternalAssertFailed() arguments into data member specific
      // to asserts.
      c->data.assert.file = (const char *)c->R0;
      c->data.assert.line = c->R1;
#ifdef PUSH_REGS_BEFORE_ASSERT
      // Just before calling halInternalAssertFailed(), R0, R1, R2 and LR were
      // pushed onto the stack - copy these values into the crash data struct.
      c->R0 = *sp++;
      c->R1 = *sp++;
      c->R2 = *sp++;
      c->LR = *sp++;
#endif
      reason = RESET_CRASH_ASSERT;
    }
  // If a bad stack pointer, PC and xPSR to 0 to indicate they are not known.
  } else {
    c->PC = 0;
    c->xPSR.word = 0;
  }

  c->mainSPUsed = halGetMainStackBytesUsed();
  c->processSPUsed = 0;   // process stack is not currently used

  for (i = 0; i < NUM_RETURNS; i++) {
    c->returns[i] = 0;
  }

  // If there was a valid stack pointer,  search the stack downward for
  // probable return addresses. A probable return address is a value in the
  // CODE segment that also has bit 0 set (since we're in Thumb mode).
  if (c->PC){
    for (i = 0, s = (int32u *)STACK_SEGMENT_END; s >= sp; ) {
      data = *(--s);
      if ( ((void *)data >= CODE_SEGMENT_BEGIN) && 
           ((void *)data < CODE_SEGMENT_END) &&
           (data & 1) ) {
        // Only record the first occurrence of a return - other copies could
        // have been in registers that then were pushed.
        for (j = 0; j < NUM_RETURNS; j++) {
          if (c->returns[j] == data) {
              break;
          }
        }
        if (j != NUM_RETURNS) {
          continue;
        }
        // Save the return in the returns array managed as a circular buffer.
        // This keeps only the last NUM_RETURNS in the event that there are more.
        i = i ? i-1 : NUM_RETURNS - 1;          
        c->returns[i] = data;
      }
    }
    // Shuffle the returns array so returns[0] has last probable return found.
    // If there were fewer than NUM_RETURNS, unused entries will contain zero.
    while (i--) {
      data = c->returns[0];
      for (j = 0; j < NUM_RETURNS - 1; j++ ) {
        c->returns[j] = c->returns[j+1];
      }
      c->returns[NUM_RETURNS - 1] = data;
    }
  }

  // Read the highest priority active exception to get reason for fault
  activeException = c->icsr.bits.VECTACTIVE;
  switch (activeException) {
  case NMI_VECTOR_INDEX:
    if (INT_NMIFLAG_REG & INT_NMICLK24M_MASK) {
      reason = RESET_FATAL_CRYSTAL;
    } else if (INT_NMIFLAG_REG & INT_NMIWDOG_MASK) {
      reason = RESET_WATCHDOG_CAUGHT;
    }
    break;
  case HARD_FAULT_VECTOR_INDEX:
    reason = RESET_FAULT_HARD;
    break;
  case MEMORY_FAULT_VECTOR_INDEX:
    reason = RESET_FAULT_MEM;
    break;
  case BUS_FAULT_VECTOR_INDEX:
    reason = RESET_FAULT_BUS;
    break;
  case USAGE_FAULT_VECTOR_INDEX:
    // make sure we didn't already identify the usage fault as an assert
    if (reason == RESET_FAULT_UNKNOWN) {
      reason = RESET_FAULT_USAGE;
    }
    break;
  case DEBUG_MONITOR_VECTOR_INDEX:
    reason = RESET_FAULT_DBGMON;
    break;
  default:
    if (activeException && (activeException < VECTOR_TABLE_LENGTH) ) {
      reason = RESET_FAULT_BADVECTOR;
    }
    break;
  }
  return reason;
}

void halPrintCrashSummary(int8u port)
{
  HalCrashInfoType *c = &halResetInfo.crash;
  int32u sp, stackBegin, stackEnd, size, used;
  int16u pct;
  int8u *mode, *stack;
  int8u bit;

  if (c->LR & 4) {
    stack = "process";
    sp = c->processSP;
    used = c->processSPUsed;
    stackBegin = 0;
    stackEnd = 0;
  } else {
    stack = "main";
    sp = c->mainSP;
    used = c->mainSPUsed;
    stackBegin = (int32u)STACK_SEGMENT_BEGIN;
    stackEnd = (int32u)STACK_SEGMENT_END;
  }

  mode = (int8u *)((c->LR & 8) ? "Thread" : "Handler");
  size = stackEnd - stackBegin;
  pct = size ? (int16u)( ((100 * used) + (size / 2)) / size) : 0;
  sea_printf( "%p mode using %p stack (%4x to %4x), SP = %4x",
                        mode, stack, stackBegin, stackEnd, sp);
  sea_printf( "%u bytes used (%u%%) in %p stack"
                        " (out of %u bytes total)", 
                        (int16u)used, pct, stack, (int16u)size);
  if ( !(c->LR & 4) && (used == size - 4*RESETINFO_WORDS) ) {
    sea_printf( "Stack _may_ have used up to 100%% of total.");
  }
  if ((sp >= stackEnd) || (sp < stackBegin)) {
    sea_printf( "SP is outside %p stack range!", stack);
  } 
  if (c->intActive.word) {
    sea_printf( "Interrupts active (or pre-empted and stacked):");
    for (bit = INT_TIM1_BIT; bit <= INT_DEBUG_BIT; bit++) {
      if ( (c->intActive.word & (1 << bit)) && *intActiveBits[bit] ) {
        sea_printf( " %p", intActiveBits[bit]);
      }
    }
    //emberSerialPrintCarriageReturn(port);
  } else {
    sea_printf( "No interrupts active");
  }
}

void halPrintCrashDetails(int8u port)
{
  HalCrashInfoType *c = &halResetInfo.crash;
  int16u reason = halGetExtendedResetInfo();
  int8u bit;
  int8u *chan;

  switch (reason) {

  case RESET_WATCHDOG_EXPIRED:
    sea_printf( "Reset cause: Watchdog expired, no reliable extra information");
    break;
  case RESET_WATCHDOG_CAUGHT:
    sea_printf( "Reset cause: Watchdog caught with enhanced info");
    sea_printf( "Instruction address: %4x", c->PC);
    break;

  case RESET_FAULT_PROTDMA:
    switch (c->data.dmaProt.channel){
    case DMA_PROT_CH_SC1_RX:
      chan = "SC1 Rx";
      break;
    case DMA_PROT_CH_GP_ADC:
      chan = "ADC";
      break;
    case DMA_PROT_CH_MAC:
      chan = "MAC Rx";
      break;
    case DMA_PROT_CH_SC2_RX:
      chan = "SC2 Rx";
      break;
    default:
      chan = "??";
      break;
    }
    sea_printf( "Reset cause: DMA protection violation");
    sea_printf( "DMA: %p, address: %4x", 
                          chan, c->data.dmaProt.address);
    break;

  case RESET_CRASH_ASSERT:
    sea_printf( "Reset cause: Assert %p:%d",
          c->data.assert.file, c->data.assert.line);
    break;

  case RESET_FAULT_HARD:
    sea_printf( "Reset cause: Hard Fault");
    if (c->hfsr.bits.VECTTBL) {
      sea_printf( 
              "HFSR.VECTTBL: error reading vector table for an exception");
    }
    if (c->hfsr.bits.FORCED) {
      sea_printf( 
              "HFSR.FORCED: configurable fault could not activate");
    }
    if (c->hfsr.bits.DEBUGEVT) {
      sea_printf( 
              "HFSR.DEBUGEVT: fault related to debug - e.g., executed BKPT");
    }
    break;

  case RESET_FAULT_MEM:
    sea_printf( "Reset cause: Memory Management Fault");
    if (c->cfsr.word & (SCS_CFSR_DACCVIOL_MASK | SCS_CFSR_IACCVIOL) ) {
      sea_printf( "Instruction address: %4x", c->PC);
    }
    if (c->cfsr.bits.MMARVALID) {
      sea_printf( "Illegal access address: %4x", c->faultAddress);
    }
    for (bit = SCS_CFSR_IACCVIOL_BIT; bit <= SCS_CFSR_MMARVALID_BIT; bit++) {
      if ( (c->cfsr.word & (1 << bit)) && *cfsrBits[bit] ) {
        sea_printf( "CFSR.%p", cfsrBits[bit]);
      }
    }
    break;

  case RESET_FAULT_BUS:
    sea_printf( "Reset cause: Bus Fault");
    sea_printf( "Instruction address: %4x", c->PC);
    if (c->cfsr.bits.IMPRECISERR) {
      sea_printf( 
        "Address is of an instruction after bus fault occurred, not the cause.");
    }
    if (c->cfsr.bits.BFARVALID) {
      sea_printf( "Illegal access address: %4x", 
                            c->faultAddress);
    }
    for (bit = SCS_CFSR_IBUSERR_BIT; bit <= SCS_CFSR_BFARVALID_BIT; bit++) {
      if ( (c->cfsr.word & (1 << bit)) && *cfsrBits[bit] ) {
        sea_printf( "CFSR.%p", cfsrBits[bit]);
      }
    }
    if ( (c->cfsr.word & 0xFF) == 0) {
      sea_printf( "CFSR.(none) load or store at an illegal address");      
    }
    for (bit = SCS_AFSR_RESERVED_BIT; bit <= SCS_AFSR_WRONGSIZE_BIT; bit++) {
      if ( (c->afsr.word & (1 << bit)) && *afsrBits[bit] ) {
        sea_printf( "AFSR.%p", afsrBits[bit]);
      }
    }
    break;

  case RESET_FAULT_USAGE:
    sea_printf( "Reset cause: Usage Fault");
    sea_printf( "Instruction address: %4x", c->PC);
    for (bit = SCS_CFSR_UNDEFINSTR_BIT; bit <= SCS_CFSR_DIVBYZERO_BIT; bit++) {
      if ( (c->cfsr.word & (1 << bit)) && *cfsrBits[bit] ) {
        sea_printf( "CFSR.%p", cfsrBits[bit]);
      }
    }
    break;

  case RESET_FAULT_DBGMON:
    sea_printf( "Reset cause: Debug Monitor Fault");
    sea_printf( "Instruction address: %4x", c->PC);
    break;

  default:
    break;
  }
}

// Output an array of 32 bit values, 4 per line, each preceded by its name.
void halPrintCrashData(int8u port)
{
  int32u *data = (int32u *)&halResetInfo.crash.R0;
  char const *name = nameStrings;
  char const *separator;
  int8u i;

  for (i = 0; *name; i++) {
    sea_printf( "%p = %4x", name, *data++);
    while (*name++) {}  // intentionally empty while loop body
    separator = (*name && ((i & 3) != 3) ) ? ", " : "\r\n";
    sea_printf( separator);
  }
}

// Returns the bytes used in the main stack area.
int32u halGetMainStackBytesUsed(void)
{
  int32u *p;

  if((RESET_EVENT&RESET_SW_MASK) == RESET_SW_MASK) {
    // if it was a software reset, make sure we are scanning
    //  _after_ the overlayed reset info
    p = __segment_end(__RESETINFO__);
  } else {
    // for any other reset type, the entire cstack can be scanned
    p = __segment_begin(__CSTACK__);
  }

  for ( ; p < STACK_SEGMENT_END; p++) {
    if (*p != STACK_FILL_VALUE) {
      break;
    }
  }
  return (int32u)((int8u *)STACK_SEGMENT_END - (int8u *)p);
}

int16u halGetPCDiagnostics( void )
{
  return 0;
}

void halStartPCDiagnostics( void )
{
}

void halStopPCDiagnostics( void )
{
}


///////////////////////////////////////////////////////////////////////////////
// hal/micro/xap2b/diagnostic.h supplemental API:
