
#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"

//[[
//This function is responsible for manually putting every piece of the Low
//Voltage domain through a fake reset cycle.  This function is intended to
//mimick a Low Voltage domain reset, with the except being any piece of
//the chip that is enabled via cstartup (since cstartup will not be called
//after this function).
void halInternalMimickLvReset(void)
{
  //WBB350FIXME -- This function needs to differentiate between DS1 and DS0!!
  
  // Only three register blocks keep power across deep sleep:
  //  CM_HV, GPIO, SLOW_TIMERS
  //
  // All other register blocks lose their state across deep sleep and we
  // must force a reset of them to mimick a LV reset.  The blocks are listed
  // here in a loose order of importance.
  
  //// NVIC          ////
  //ST_CSR
  //ST_RVR
  //ST_CALVR
  //INT_CFGCLR
  //INT_PENDCLR
  //NVIC_IPR_3to0
  //NVIC_IPR_7to4
  //NVIC_IPR_11to8
  //NVIC_IPR_15to12
  //NVIC_IPR_19to16
  //SCS_VTOR
  //SCS_AIRCR
  
  //// EVENT         ////
  //// CM_LV         ////
  //// RAM_CTRL      ////
  //// FLASH_CONTROL ////
  //// TPIU          ////
  //// AUX_ADC       ////
  //// SERIAL        ////
  //// TMR1          ////
  //// TMR2          ////
  //// ITM           ////
  //// DWT           ////
  //// FPB           ////
  //// CAL_ADC       ////
  //// BASEBAND      ////
  //// MAC           ////
  //// SECURITY      ////
  
  //WBB350FIXME -- Fill out this function and complete SLEEP13 testcase to
  //               cover this
}


#ifdef SLEEP_TRACE //WBB350FIXME -- Find a less intrusive technique
  extern boolean sleepTraceOn;
  extern int8u sleepTraceBuffer[];
  extern int8u sleepTraceIndex;
  extern int8u sleepTraceDelayPosition;
  #define SLEEP_TRACE_ADD_MARKER(byte)                 \
    do {                                               \
      if(sleepTraceOn) {                               \
        if(sleepTraceIndex<50) {                       \
          sleepTraceBuffer[sleepTraceIndex] = byte;    \
        }                                              \
        sleepTraceIndex++;                             \
      }                                                \
    } while(0)
  #define SLEEP_TRACE_1SEC_DELAY(position)             \
    do {                                               \
      if(sleepTraceDelayPosition==position) {          \
        int8u delayCnt=(20*1);                         \
        while(delayCnt-->0) {                          \
          halCommonDelayMicroseconds(50000);           \
        }                                              \
      }                                                \
    } while(0)
#else //SLEEP_TRACE
  #define SLEEP_TRACE_ADD_MARKER(byte) do{}while(0)
  #define SLEEP_TRACE_1SEC_DELAY(position) do{}while(0)
#endif //SLEEP_TRACE
//]]


//We don't have a real register to hold this composite information.
//Pretend we do so halGetWakeInfo can operate like halGetResetInfo.
//This "register" is only ever set by halSleep.
// [31] = WakeInfoValid
// [30] = SleepSkipped
// [29] = CSYSPWRUPREQ
// [28] = CDBGPWRUPREQ
// [27] = WAKE_CORE
// [26] = TIMER_WAKE_WRAP
// [25] = TIMER_WAKE_COMPB
// [24] = TIMER_WAKE_COMPA
// [23:0] = corresponding GPIO activity
#define WAKEINFOVALID_INTERNAL_WAKE_EVENT_BIT 31
#define SLEEPSKIPPED_INTERNAL_WAKE_EVENT_BIT  30
#define CSYSPWRUPREQ_INTERNAL_WAKE_EVENT_BIT  29
#define CDBGPWRUPREQ_INTERNAL_WAKE_EVENT_BIT  28
#define WAKE_CORE_INTERNAL_WAKE_EVENT_BIT     27
#define WRAP_INTERNAL_WAKE_EVENT_BIT          26
#define CMPB_INTERNAL_WAKE_EVENT_BIT          25
#define CMPA_INTERNAL_WAKE_EVENT_BIT          24
//This define shifts events from the PWRUP_EVENT register into the proper
//place in the halInternalWakeEvent variable
#define INTERNAL_WAKE_EVENT_BIT_SHIFT         20

static int32u halInternalWakeEvent=0;

int32u halGetWakeInfo(void)
{
  return halInternalWakeEvent;
}
  
void halInternalSleep(SleepModes sleepMode)
{
  //This code assumes all wake source registers are properly configured.
  //As such, it should be called from halSleep() which configues the
  //wake sources.
  
  //The parameter gpioWakeSel is a bitfield composite of the GPIO wake
  //sources derived from the 3 ports, indicating which of the 24 GPIO
  //are configured as a wake source.
  int32u gpioWakeSel  = (GPIO_PAWAKE<<0);
         gpioWakeSel |= (GPIO_PBWAKE<<8);
         gpioWakeSel |= (GPIO_PCWAKE<<16);
  
  halInternalWakeEvent = 0; //clear old wake events
  
  switch ( sleepMode )
  {
    case SLEEPMODE_POWERSAVE:  // Deep Sleep, with maintaining system time
      //The only difference between POWERSAVE and POWERDOWN is the sleep timer.
      //POWERDOWN does not use the sleep timer, but POWERSAVE does.  Since
      //the rest of the sleep code is identical, POWERSAVE simply needs to
      //enable the sleep timer wake sources and then fall into
      //POWERDOWN's sleep code.
      //
      //NOTE: POWERSAVE will simply enable sleep timer wake events and trigger
      //the sleep timer ISR as appropriate.  This mode assumes the caller has
      //configured the *entire* sleep timer properly.
      //POWERSAVE uses the sleep timer interrupts to infer which wake sources
      //should be enabled.
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRWRAP) {
        WAKE_SEL |= WAKE_SLEEPTMRWRAP;
      }
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRCMPB) {
        WAKE_SEL |= WAKE_SLEEPTMRCMPB;
      }
      if(INT_SLEEPTMRCFG&INT_SLEEPTMRCMPA) {
        WAKE_SEL |= WAKE_SLEEPTMRCMPA;
      }
      //fall into POWERDOWN's sleep code:
      
    case SLEEPMODE_POWERDOWN:  // Deep Sleep, without maintaining system time
      // Interrupts *must* be/stay disabled for DEEP SLEEP operation
      // INTERRUPTS_OFF will use BASEPRI to disable all interrupts except
      // fault handlers and PendSV.
      INTERRUPTS_OFF();
      // This is the point of no return.  From here on out, only the interrupt
      // sources available in WAKE_SEL will be captured and propagated across
      // deep sleep.
      //stick all our saved info onto stack since it's only temporary
      {
        boolean restoreWatchdog = halInternalWatchDogEnabled();
        boolean skipSleep = FALSE;
        //If the 10kHz is to be disabled during deep sleep (DS2), save its
        //state so it can restored after sleep, and turn it off for sleep.
        #ifdef DISABLE_SLEEPTMR_DEEPSLEEP
          boolean sleepTmrEnabled = SLEEPTMR_CLKEN&SLEEPTMR_CLK10KEN;
          SLEEPTMR_CLKEN &= ~SLEEPTMR_CLK10KEN;
        #endif //DISABLE_SLEEPTMR_DEEPSLEEP
        
        // Only three register blocks keep power across deep sleep:
        //  CM_HV, GPIO, SLOW_TIMERS
        //
        // All other register blocks lose their state across deep sleep:
        //  BASEBAND, MAC, SECURITY, SERIAL, TMR1, TMR2, EVENT, CM_LV, RAM_CTRL,
        //  AUX_ADC, CAL_ADC, FLASH_CONTROL, ITM, DWT, FPB, NVIC, TPIU
        //
        // The sleep code will only save and restore registers where it is
        // meaningful and necessary to do so.  In most cases, there must still
        // be a powerup function to restore proper state.
        //
        // NOTE: halPowerUp() and halPowerDown() will always be called before
        // and after this function.  halPowerDown and halPowerUp should leave
        // the modules in a safe state and then restart the modules.
        // (For example, shutting down and restarting Timer1)
        //
        //----BASEBAND
        //        reinitialized by emberStackPowerUp()
        //----MAC
        //        reinitialized by emberStackPowerUp()
        //----SECURITY
        //        reinitialized by emberStackPowerUp()
        //----SERIAL
        //        reinitialized by halPowerUp() or similar
        //----TMR1
        //        reinitialized by halPowerUp() or similar
        //----TMR2
        //        reinitialized by halPowerUp() or similar
        //----EVENT
        //SRC or FLAG interrupts are not saved or restored
        //MISS interrupts are not saved or restored
        //MAC_RX_INT_MASK - reinitialized by emberStackPowerUp()
        //MAC_TX_INT_MASK - reinitialized by emberStackPowerUp()
        //MAC_TIMER_INT_MASK - reinitialized by emberStackPowerUp()
        //BB_INT_MASK - reinitialized by emberStackPowerUp()
        //SEC_INT_MASK - reinitialized by emberStackPowerUp()
        int32u INT_SLEEPTMRCFG_SAVED = INT_SLEEPTMRCFG_REG;
        int32u INT_MGMTCFG_SAVED = INT_MGMTCFG_REG;
        //INT_TIM1CFG - reinitialized by halPowerUp() or similar
        //INT_TIM2CFG - reinitialized by halPowerUp() or similar
        //INT_SC1CFG - reinitialized by halPowerUp() or similar
        //INT_SC2CFG - reinitialized by halPowerUp() or similar
        //INT_ADCCFG - reinitialized by halPowerUp() or similar
        int32u GPIO_INTCFGA_SAVED = GPIO_INTCFGA_REG;
        int32u GPIO_INTCFGB_SAVED = GPIO_INTCFGB_REG;
        int32u GPIO_INTCFGC_SAVED = GPIO_INTCFGC_REG;
        int32u GPIO_INTCFGD_SAVED = GPIO_INTCFGD_REG;
        //SC1_INTMODE - reinitialized by halPowerUp() or similar
        //SC2_INTMODE - reinitialized by halPowerUp() or similar
        //----CM_LV
        int32u OSC24M_BIASTRIM_SAVED = OSC24M_BIASTRIM_REG;
        int32u OSCHF_TUNE_SAVED = OSCHF_TUNE_REG;
        int32u DITHER_DIS_SAVED = DITHER_DIS_REG;
        //OSC24M_CTRL - reinitialized by halPowerUp() or similar
        //CPU_CLKSEL  - reinitialized by halPowerUp() or similar
        //TMR1_CLK_SEL - reinitialized by halPowerUp() or similar
        //TMR2_CLK_SEL - reinitialized by halPowerUp() or similar
        int32u PCTRACE_SEL_SAVED = PCTRACE_SEL_REG;
        //----RAM_CTRL
        int32u MEM_PROT_0_SAVED = MEM_PROT_0_REG;
        int32u MEM_PROT_1_SAVED = MEM_PROT_1_REG;
        int32u MEM_PROT_2_SAVED = MEM_PROT_2_REG;
        int32u MEM_PROT_3_SAVED = MEM_PROT_3_REG;
        int32u MEM_PROT_4_SAVED = MEM_PROT_4_REG;
        int32u MEM_PROT_5_SAVED = MEM_PROT_5_REG;
        int32u MEM_PROT_6_SAVED = MEM_PROT_6_REG;
        int32u MEM_PROT_7_SAVED = MEM_PROT_7_REG;
        int32u MEM_PROT_EN_SAVED = MEM_PROT_EN_REG;
        //----AUX_ADC
        //        reinitialized by halPowerUp() or similar
        //----CAL_ADC
        //        reinitialized by emberStackPowerUp()
        //----FLASH_CONTROL
        //        configured on the fly by the flash library
        //----ITM
        //        reinitialized by halPowerUp() or similar
        //----DWT
        //        not used by software on chip
        //----FPB
        //        not used by software on chip
        //----NVIC
        //ST_CSR - fixed, restored by cstartup when exiting deep sleep
        //ST_RVR - fixed, restored by cstartup when exiting deep sleep
        int32u INT_CFGSET_SAVED = INT_CFGSET_REG; //mask against wake sources
        //INT_PENDSET - used below when overlapping interrupts and wake sources
        //NVIC_IPR_3to0 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_7to4 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_11to8 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_15to12 - fixed, restored by cstartup when exiting deep sleep
        //NVIC_IPR_19to16 - fixed, restored by cstartup when exiting deep sleep
        int32u SCS_VTOR_SAVED = SCS_VTOR_REG;
        //SCS_CCR - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_7to4 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_11to8 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHPR_15to12 - fixed, restored by cstartup when exiting deep sleep
        //SCS_SHCSR - fixed, restored by cstartup when exiting deep sleep
        //----TPIU
        //        reinitialized by halPowerUp() or similar
        
        //emDebugPowerDown() should have shutdown the DWT/ITM/TPIU already.
        
        //freeze input to the GPIO from LV (alternate output functions freeze)
        EVENT_CTRL = LV_FREEZE;
        //record GPIO state for wake monitoring purposes
        //By having a snapshot of GPIO state, we can figure out after waking
        //up exactly which GPIO could have woken us up.
        //Reading the three IN registers is done separately to avoid warnings
        //about undefined order of volatile access.
        int32u GPIO_IN_SAVED =   GPIO_PAIN;
               GPIO_IN_SAVED |= (GPIO_PBIN<<8);
               GPIO_IN_SAVED |= (GPIO_PCIN<<16);
        //reset the power up events by writing 1 to all bits.
        PWRUP_EVENT = 0xFFFFFFFF;
        //[[
        SLEEP_TRACE_ADD_MARKER('A');
        //]]
        //By clearing the events, the wake up event capturing is activated.
        //At this point we can safely check our interrupt flags since event
        //capturing is now overlapped.  Up to now, interrupts indicate
        //activity, after this point, powerup events indicate activity.
        //If any of the interrupt flags are set, that means we saw a wake event
        //sometime while entering sleep, so we need to skip over sleeping
        //
        //--possible interrupt sources for waking:
        //  IRQA, IRQB, IRQC, IRQD
        //  SleepTMR CMPA, CMPB, Wrap
        //  WAKE_CORE (DebugIsr)
        //
        //check for IRQA interrupt and if IRQA (PB0) is wake source
        if((INT_PENDSET&INT_IRQA) &&
           (GPIO_PBWAKE&PB0) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQA as a wake event
          halInternalWakeEvent |= BIT(PORTB_PIN(0));
          //[[
          SLEEP_TRACE_ADD_MARKER('B');
          //]]
        }
        //check for IRQB interrupt and if IRQB (PB6) is wake source
        if((INT_PENDSET&INT_IRQB) &&
           (GPIO_PBWAKE&PB6) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQB as a wake event
          halInternalWakeEvent |= BIT(PORTB_PIN(6));
          //[[
          SLEEP_TRACE_ADD_MARKER('C');
          //]]
        }
        //check for IRQC interrupt and if IRQC (GPIO_IRQCSEL) is wake source
        if((INT_PENDSET&INT_IRQC) &&
           (gpioWakeSel&BIT(GPIO_IRQCSEL)) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQC as a wake event
          halInternalWakeEvent |= BIT(GPIO_IRQCSEL);
          //[[
          SLEEP_TRACE_ADD_MARKER('D');
          //]]
        }
        //check for IRQD interrupt and if IRQD (GPIO_IRQDSEL) is wake source
        if((INT_PENDSET&INT_IRQD) &&
           (gpioWakeSel&BIT(GPIO_IRQDSEL)) &&
           (WAKE_SEL&GPIO_WAKE)) {
          skipSleep = TRUE;
          //log IRQD as a wake event
          halInternalWakeEvent |= BIT(GPIO_IRQDSEL);
          //[[
          SLEEP_TRACE_ADD_MARKER('E');
          //]]
        }
        //check for SleepTMR CMPA interrupt and if SleepTMR CMPA is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRCMPA) && (WAKE_SEL&WAKE_SLEEPTMRCMPA)) {
          skipSleep = TRUE;
          //log SleepTMR CMPA as a wake event
          halInternalWakeEvent |= BIT32(CMPA_INTERNAL_WAKE_EVENT_BIT);
          //[[
          SLEEP_TRACE_ADD_MARKER('F');
          //]]
        }
        //check for SleepTMR CMPB interrupt and if SleepTMR CMPB is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRCMPB) && (WAKE_SEL&WAKE_SLEEPTMRCMPB)) {
          skipSleep = TRUE;
          //log SleepTMR CMPB as a wake event
          halInternalWakeEvent |= BIT32(CMPB_INTERNAL_WAKE_EVENT_BIT);
          //[[
          SLEEP_TRACE_ADD_MARKER('G');
          //]]
        }
        //check for SleepTMR WRAP interrupt and if SleepTMR WRAP is wake source
        if((INT_SLEEPTMR&INT_SLEEPTMRWRAP) && (WAKE_SEL&WAKE_SLEEPTMRWRAP)) {
          skipSleep = TRUE;
          //log SleepTMR WRAP as a wake event
          halInternalWakeEvent |= BIT32(WRAP_INTERNAL_WAKE_EVENT_BIT);
          //[[
          SLEEP_TRACE_ADD_MARKER('H');
          //]]
        }
        //check for Debug interrupt and if WAKE_CORE is wake source
        if((INT_PENDSET&INT_DEBUG) && (WAKE_SEL&WAKE_WAKE_CORE)) {
          skipSleep = TRUE;
          //log WAKE_CORE as a wake event
          halInternalWakeEvent |= BIT32(WAKE_CORE_INTERNAL_WAKE_EVENT_BIT);
          //[[
          SLEEP_TRACE_ADD_MARKER('I');
          //]]
        }

        //only propagate across deep sleep the interrupts that are both
        //enabled and possible wake sources
        {
          int32u wakeSourceInterruptMask = 0;
          
          if(GPIO_PBWAKE&PB0) {
            wakeSourceInterruptMask |= INT_IRQA;
            //[[
            SLEEP_TRACE_ADD_MARKER('J');
            //]]
          }
          if(GPIO_PBWAKE&PB6) {
            wakeSourceInterruptMask |= INT_IRQB;
            //[[
            SLEEP_TRACE_ADD_MARKER('K');
            //]]
          }
          if(gpioWakeSel&BIT(GPIO_IRQCSEL)) {
            wakeSourceInterruptMask |= INT_IRQC;
            //[[
            SLEEP_TRACE_ADD_MARKER('L');
            //]]
          }
          if(gpioWakeSel&BIT(GPIO_IRQDSEL)) {
            wakeSourceInterruptMask |= INT_IRQD;
            //[[
            SLEEP_TRACE_ADD_MARKER('M');
            //]]
          }
          if( (WAKE_SEL&WAKE_SLEEPTMRCMPA) ||
              (WAKE_SEL&WAKE_SLEEPTMRCMPB) ||
              (WAKE_SEL&WAKE_SLEEPTMRWRAP) ) {
            wakeSourceInterruptMask |= INT_SLEEPTMR;
            //[[
            SLEEP_TRACE_ADD_MARKER('N');
            //]]
          }
          if(WAKE_SEL&WAKE_WAKE_CORE) {
            wakeSourceInterruptMask |= INT_DEBUG;
            //[[
            SLEEP_TRACE_ADD_MARKER('O');
            //]]
          }
          
          INT_CFGSET_SAVED &= wakeSourceInterruptMask;
        }
        
        //[[
        //Since it is possible to perform a deep sleep cycle without actually
        //leaving the running state (a wake source was captured while
        //entering deep sleep or CSYSPWRUPREQ remained set), it is possible for
        //the application to perform a sleep cycle and not have the entire
        //low voltage domain reset.  In theory, the application is responsible
        //for cleanly shutting down the entire chip and then bringing
        //it back up around a sleep cycle (stack and hal sleep/powerup).  In
        //practice, though, the safest thing to do is shut down the entire
        //low voltage domain while entering deep sleep.  By doing this,
        //we ensure the application always has the same sleep cycle operation
        //and does not experience accidental peripheral operation that should
        //not have survived.
        halInternalMimickLvReset();
        //]]
        
        //disable watchdog while sleeping (since we can't reset it asleep)
        halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
        
        //The chip is not allowed to enter a deep sleep mode (which could
        //cause a core reset cycle) while CSYSPWRUPREQ is set.  CSYSPWRUPREQ
        //indicates that the debugger is trying to access sections of the
        //chip that would get reset during deep sleep.  Therefore, a reset
        //cycle could very easily cause the debugger to error and we don't
        //want that.  While the power management state machine will stall
        //if CSYSPWRUPREQ is set (to avoid the situation just described),
        //in this stalled state the chip will not be responsive to wake
        //events.  To be sensitive to wake events, we must handle them in
        //software instead.  To accomplish this, we request that the
        //CSYSPWRUPACK be inhibited (which will indicate the debugger is not
        //connected).  But, we cannot induce deep sleep until CSYSPWRUPREQ/ACK
        //go low and these are under the debuggers control, so we must stall
        //and wait here.  If there is a wake event during this time, break
        //out and wake like normal.  If the ACK eventually clears,
        //we can proceed into deep sleep.  The CSYSPWRUPACK_INHIBIT
        //functionality will hold off the debugger (by holding off the ACK)
        //until we are safely past and out of deep sleep.  The power management
        //state machine then becomes responsible for clearing
        //CSYSPWRUPACK_INHIBIT and responding to a CSYSPWRUPREQ with a
        //CSYSPWRUPACK at the right/safe time.
        CSYSPWRUPACK_INHIBIT = CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT;
        {
          //Use a local copy of WAKE_SEL to avoid warnings from the compiler
          //about order of volatile accesses
          int32u wakeSel = WAKE_SEL;
          //stall until a wake event or CSYSPWRUPREQ/ACK clears
          while( (CSYSPWRUPACK_STATUS) && (!(PWRUP_EVENT&wakeSel)) ) {}
          //if there was a wake event, allow CSYSPWRUPACK and skip sleep
          if(PWRUP_EVENT&wakeSel) {
            CSYSPWRUPACK_INHIBIT = CSYSPWRUPACK_INHIBIT_RESET;
            skipSleep = TRUE;
          }
        }
        
        
        //[[
        SLEEP_TRACE_ADD_MARKER('P');
        //]]
        if(!skipSleep) {
          //[[
          SLEEP_TRACE_ADD_MARKER('Q');
          //]]
          //FogBugz 7283 states that we must switch to the OSCHF when entering
          //deep sleep since using the 24MHz XTAL could result in RAM
          //corruption.  This switch must occur at least 2*24MHz cycles before
          //sleeping.
          //FogBugz 8858 states that we cannot go into deep-sleep when the
          //chip is clocked with the 24MHz XTAL with a duty cycle as low as
          //70/30 since this causes power_down generation timing to fail.
          OSC24M_CTRL &= ~OSC24M_CTRL_OSC24M_SEL;
          //If DS12 needs to be forced regardless of state, clear
          //REGEN_DSLEEP here.  This is hugely dangerous and
          //should only be done in very controlled chip tests.
          SCS_SCR |= SCS_SCR_SLEEPDEEP;      //enable deep sleep
          extern volatile boolean halPendSvSaveContext;
          halPendSvSaveContext = 1;          //1 means save context
          //The INTERRUPTS_OFF used at the beginning of this function set
          //BASEPRI such that the only interrupts that will fire are faults
          //and PendSV.  Trigger PendSV now to induce a context save.
          SCS_ICSR |= SCS_ICSR_PENDSVSET;    //pend the context save and Dsleep
          //Since the interrupt will not fire immediately it is possible to
          //execute a few lines of code.  To stay halted in this spot until the
          //WFI instruction, spin on the context flag (which will get cleared
          //during the startup sequence when restoring context).
          while(halPendSvSaveContext) {}
          //I AM ASLEEP.  WHEN EXECUTION RESUMES, CSTARTUP WILL RESTORE TO HERE
        } else {
          //Record the fact that we skipped sleep
          halInternalWakeEvent |= BIT32(SLEEPSKIPPED_INTERNAL_WAKE_EVENT_BIT);
        }
        //[[
        SLEEP_TRACE_1SEC_DELAY(1);
        //]]
        //If this was a true deep sleep, we would have executed cstartup and
        //PRIMASK would be set right now.  If we skipped sleep, PRIMASK is not
        //set so we explicitely set it to guarantee the powerup sequence
        //works cleanly and consistently with respect to interrupt
        //dispatching and enabling.
        _setPriMask();
        
        //[[
        SLEEP_TRACE_ADD_MARKER('a');
        //]]
        
        //immediately restore the registers we saved before sleeping
        //so IRQ and SleepTMR capture can be reenabled as quickly as possible
        //this is safe because our global interrupts are still disabled
        //other registers will be restored later
        INT_SLEEPTMRCFG_REG = INT_SLEEPTMRCFG_SAVED;
        INT_MGMTCFG_REG = INT_MGMTCFG_SAVED;
        GPIO_INTCFGA_REG = GPIO_INTCFGA_SAVED;
        GPIO_INTCFGB_REG = GPIO_INTCFGB_SAVED;
        GPIO_INTCFGC_REG = GPIO_INTCFGC_SAVED;
        GPIO_INTCFGD_REG = GPIO_INTCFGD_SAVED;
        OSC24M_BIASTRIM_REG = OSC24M_BIASTRIM_SAVED;
        OSCHF_TUNE_REG = OSCHF_TUNE_SAVED;
        DITHER_DIS_REG = DITHER_DIS_SAVED;
        PCTRACE_SEL_REG = PCTRACE_SEL_SAVED;
        MEM_PROT_0_REG = MEM_PROT_0_SAVED;
        MEM_PROT_1_REG = MEM_PROT_1_SAVED;
        MEM_PROT_2_REG = MEM_PROT_2_SAVED;
        MEM_PROT_3_REG = MEM_PROT_3_SAVED;
        MEM_PROT_4_REG = MEM_PROT_4_SAVED;
        MEM_PROT_5_REG = MEM_PROT_5_SAVED;
        MEM_PROT_6_REG = MEM_PROT_6_SAVED;
        MEM_PROT_7_REG = MEM_PROT_7_SAVED;
        MEM_PROT_EN_REG = MEM_PROT_EN_SAVED;
        INT_CFGSET_REG = INT_CFGSET_SAVED;
        SCS_VTOR_REG = SCS_VTOR_SAVED;
        
        //Clear the interrupt flags for all wake sources.  This
        //is necessary because if we don't execute an actual deep sleep cycle
        //the interrupt flags will never be cleared.  By clearing the flags,
        //we always mimick a real deep sleep as closely as possible and
        //guard against any accidental interrupt triggering coming out
        //of deep sleep.  (The interrupt dispatch code coming out of sleep
        //is responsible for translating wake events into interrupt events,
        //and if we don't clear interrupt flags here it's possible for an
        //interrupt to trigger even if it wasn't the true wake event.)
        INT_SLEEPTMRFLAG = INT_SLEEPTMRCMPA;
        INT_SLEEPTMRFLAG = INT_SLEEPTMRCMPB;
        INT_SLEEPTMRFLAG = INT_SLEEPTMRWRAP;
        INT_GPIOFLAG = INT_IRQAFLAG;
        INT_GPIOFLAG = INT_IRQBFLAG;
        INT_GPIOFLAG = INT_IRQCFLAG;
        INT_GPIOFLAG = INT_IRQDFLAG;
        //we have to delay a brief moment to let the SleepTMR and GPIO flag
        //writes to cross the LV/HV domain and get cleared before we clear
        //the NVIC top level
        halCommonDelayMicroseconds(1);
        //WAKE_CORE/INT_DEBUG and INT_IRQx is cleared by INT_PENDCLR below
        INT_PENDCLR = 0xFFFFFFFF;
        
        //Now that we're awake, normal interrupts are operational again
        //Take a snapshot of the new GPIO state and the EVENT register to
        //record our wake event
        int32u GPIO_IN_NEW =   GPIO_PAIN;
               GPIO_IN_NEW |= (GPIO_PBIN<<8);
               GPIO_IN_NEW |= (GPIO_PCIN<<16);
        //Only operate on power up events that are also wake events.  Power
        //up events will always trigger like an interrupt flag, so we have
        //to check them against events that are enabled for waking. (This is
        //a two step process because we're accessing two volatile values.)
        int32u powerUpEvents = PWRUP_EVENT;
               powerUpEvents &= WAKE_SEL;
        halInternalWakeEvent |= ((GPIO_IN_SAVED^GPIO_IN_NEW)&gpioWakeSel);
        halInternalWakeEvent |= ((powerUpEvents & 
                                  (PWRUP_CSYSPWRUPREQ_MASK  |
                                   PWRUP_CDBGPWRUPREQ_MASK  |
                                   PWRUP_WAKECORE_MASK      |
                                   PWRUP_SLEEPTMRWRAP_MASK  |
                                   PWRUP_SLEEPTMRCOMPB_MASK |
                                   PWRUP_SLEEPTMRCOMPA_MASK ))
                                          <<INTERNAL_WAKE_EVENT_BIT_SHIFT);
        //at this point wake events are fully captured and interrupts have
        //taken over handling all new events
        
        //[[
        SLEEP_TRACE_1SEC_DELAY(2);
        SLEEP_TRACE_ADD_MARKER('b');
        //]]
        
        //Bring limited interrupts back online.  INTERRUPTS_OFF will use
        //BASEPRI to disable all interrupts except fault handlers and PendSV.
        //PRIMASK is still set though (global interrupt disable) so we need
        //to clear that next.
        INTERRUPTS_OFF();
        
        //[[
        SLEEP_TRACE_ADD_MARKER('c');
        //]]
        
        //Now that BASEPRI has taken control of interrupt enable/disable,
        //we can clear PRIMASK to reenable global interrupt operation.
        _clearPriMask();
        
        //[[
        SLEEP_TRACE_ADD_MARKER('d');
        //]]
        
        //wake events are saved and interrupts are back on track,
        //disable gpio freeze
        EVENT_CTRL = EVENT_CTRL_RESET;
        
        //restart watchdog if it was running when we entered sleep
        //do this before dispatching interrupts while we still have tight
        //control of code execution
        if(restoreWatchdog) {
          halInternalEnableWatchDog();
        }
        
        //[[
        SLEEP_TRACE_ADD_MARKER('e');
        //]]
        
        //Restore the 10kHz if it was enabled when entering deep sleep
        #ifdef DISABLE_SLEEPTMR_DEEPSLEEP
          if(sleepTmrEnabled) {
            SLEEPTMR_CLKEN |= SLEEPTMR_CLK10KEN;
          }
        #endif //DISABLE_SLEEPTMR_DEEPSLEEP
        
        //[[
        SLEEP_TRACE_ADD_MARKER('f');
        //]]
        
        //Pend any interrupts associated with deep sleep wake sources.  The
        //restoration of INT_CFGSET above and the changing of BASEPRI below
        //is responsible for proper dispatching of interrupts at the end of
        //halSleep.
        //
        //
        //The WAKE_CORE wake source triggers a Debug Interrupt.  If INT_DEBUG
        //interrupt is enabled and WAKE_CORE is a wake event, then pend the
        //Debug interrupt (using the wake_core bit).
        if( (INT_CFGSET&INT_DEBUG) &&
            (halInternalWakeEvent&BIT(WAKE_CORE_INTERNAL_WAKE_EVENT_BIT)) ) {
          WAKE_CORE = WAKE_CORE_FIELD;
          //[[
          SLEEP_TRACE_ADD_MARKER('g');
          //]]
        }
        //
        //
        //The SleepTMR CMPA is linked to a real ISR.  If the SleepTMR CMPA
        //interrupt is enabled and CMPA is a wake event, then pend the CMPA
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRCMPA) &&
            (halInternalWakeEvent&BIT(CMPA_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRCMPA;
          //[[
          SLEEP_TRACE_ADD_MARKER('h');
          //]]
        }
        //
        //The SleepTMR CMPB is linked to a real ISR.  If the SleepTMR CMPB
        //interrupt is enabled and CMPB is a wake event, then pend the CMPB
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRCMPB) &&
            (halInternalWakeEvent&BIT(CMPB_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRCMPB;
          //[[
          SLEEP_TRACE_ADD_MARKER('i');
          //]]
        }
        //
        //The SleepTMR WRAP is linked to a real ISR.  If the SleepTMR WRAP
        //interrupt is enabled and WRAP is a wake event, then pend the WRAP
        //interrupt (force the second level interrupt).
        if( (INT_SLEEPTMRCFG&INT_SLEEPTMRWRAP) &&
            (halInternalWakeEvent&BIT(WRAP_INTERNAL_WAKE_EVENT_BIT)) ) {
          INT_SLEEPTMRFORCE = INT_SLEEPTMRWRAP;
          //[[
          SLEEP_TRACE_ADD_MARKER('j');
          //]]
        }
        //
        //
        //The four IRQs are linked to a real ISR.  If any of the four IRQs
        //triggered, then pend their ISR
        //
        //If the IRQA interrupt mode is enabled and IRQA (PB0) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGA&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(PORTB_PIN(0))) ) {
          INT_PENDSET = INT_IRQA;
          //[[
          SLEEP_TRACE_ADD_MARKER('k');
          //]]
        }
        //If the IRQB interrupt mode is enabled and IRQB (PB6) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGB&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(PORTB_PIN(6))) ) {
          INT_PENDSET = INT_IRQB;
          //[[
          SLEEP_TRACE_ADD_MARKER('l');
          //]]
        }
        //If the IRQC interrupt mode is enabled and IRQC (GPIO_IRQCSEL) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGC&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(GPIO_IRQCSEL)) ) {
          INT_PENDSET = INT_IRQC;
          //[[
          SLEEP_TRACE_ADD_MARKER('m');
          //]]
        }
        //If the IRQD interrupt mode is enabled and IRQD (GPIO_IRQDSEL) is wake
        //event, then pend the interrupt.
        if( ((GPIO_INTCFGD&GPIO_INTMOD)!=0) &&
            (halInternalWakeEvent&BIT(GPIO_IRQDSEL)) ) {
          INT_PENDSET = INT_IRQD;
          //[[
          SLEEP_TRACE_ADD_MARKER('n');
          //]]
        }
      }
      
      //[[
      SLEEP_TRACE_1SEC_DELAY(3);
      SLEEP_TRACE_ADD_MARKER('o');
      //]]
      
      //Mark the wake events valid just before exiting
      halInternalWakeEvent |= BIT32(WAKEINFOVALID_INTERNAL_WAKE_EVENT_BIT);
      
      //We are now reconfigured, appropriate ISRs are pended, and ready to go,
      //so enable interrupts!
      INTERRUPTS_ON();
      
      //[[
      SLEEP_TRACE_ADD_MARKER('p');
      //]]
      
      break; //and deep sleeping is done!
    
    case SLEEPMODE_RESERVED:   // treat like idling
    case SLEEPMODE_IDLE:       // Idle, wakes on any interrupt
      {
        boolean restoreWatchdog = halInternalWatchDogEnabled();
        //disable watchdog while sleeping (since we can't reset it asleep)
        halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
        //Normal ATOMIC/INTERRUPTS_OFF/INTERRUPTS_ON uses the BASEPRI mask
        //to juggle priority levels so that the fault handlers can always
        //be serviced.  But, the WFI instruction is only capable of
        //working with the PRIMASK bit.  Therefore, we have to switch from
        //using BASEPRI to PRIMASK to keep interrupts disabled so that the
        //WFI can return on an interrupt
        //Globally disable interrupts with PRIMASK
        _setPriMask();
        //Bring the BASEPRI up to 0 to allow interrupts (but still disabled
        //with PRIMASK)
        INTERRUPTS_ON();
        //an internal function call is made here instead of injecting the
        //"WFI" assembly instruction because injecting assembly code will
        //cause the compiler's optimizer to reduce efficiency.
        halInternalIdleSleep();
        //The WFI instruction does not actually clear the PRIMASK bit, it
        //only allows the PRIMASK bit to be bypassed.  Therefore, we must
        //manually clear PRIMASK to reenable all interrupts.
        _clearPriMask();
        //restart watchdog if it was running when we entered sleep
        if(restoreWatchdog)
          halInternalEnableWatchDog();
      }
      break;
      
  }
}

void halSleep(SleepModes sleepMode)
{
  //use the defines found in the board file to choose our wakeup source(s)
  WAKE_SEL = 0;  //start with no wake sources
  //configure all GPIO wake sources
  GPIO_PAWAKE = ( (WAKE_ON_PA0 << PA0_BIT) |
                  (WAKE_ON_PA1 << PA1_BIT) |
                  (WAKE_ON_PA2 << PA2_BIT) |
                  (WAKE_ON_PA3 << PA3_BIT) |
                  (WAKE_ON_PA4 << PA4_BIT) |
                  (WAKE_ON_PA5 << PA5_BIT) |
                  (WAKE_ON_PA6 << PA6_BIT) |
                  (WAKE_ON_PA7 << PA7_BIT) );
  GPIO_PBWAKE = ( (WAKE_ON_PB0 << PB0_BIT) |
                  (WAKE_ON_PB1 << PB1_BIT) |
                  (WAKE_ON_PB2 << PB2_BIT) |
                  (WAKE_ON_PB3 << PB3_BIT) |
                  (WAKE_ON_PB4 << PB4_BIT) |
                  (WAKE_ON_PB5 << PB5_BIT) |
                  (WAKE_ON_PB6 << PB6_BIT) |
                  (WAKE_ON_PB7 << PB7_BIT) );
  GPIO_PCWAKE = ( (WAKE_ON_PC0 << PC0_BIT) |
                  (WAKE_ON_PC1 << PC1_BIT) |
                  (WAKE_ON_PC2 << PC2_BIT) |
                  (WAKE_ON_PC3 << PC3_BIT) |
                  (WAKE_ON_PC4 << PC4_BIT) |
                  (WAKE_ON_PC5 << PC5_BIT) |
                  (WAKE_ON_PC6 << PC6_BIT) |
                  (WAKE_ON_PC7 << PC7_BIT) );
  
  //if any of the GPIO wakeup monitor bits are set, enable the top level
  //GPIO wakeup monitor
  if((GPIO_PAWAKE)||(GPIO_PBWAKE)||(GPIO_PCWAKE)) {
    WAKE_SEL |= GPIO_WAKE;
  }
  //always wakeup when the debugger is connected
  WAKE_SEL |= WAKE_CDBGPWRUPREQ;
  //always wakeup when the debugger attempts to access the chip
  WAKE_SEL |= WAKE_CSYSPWRUPREQ;
  //always wakeup when the debug channel attempts to access the chip
  WAKE_SEL |= WAKE_WAKE_CORE;
  //the timer wakeup sources are enabled below in POWERSAVE, if needed
  
  //wake sources are configured so do the actual sleeping
  halInternalSleep(sleepMode);
}

