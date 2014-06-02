
#include "config.h"
#include "ember.h"
#include "error.h"

#include "hal.h"
#include "serial.h"
#include "diagnostic.h"
#include "memmap.h"
#include "flash.h"

#define GPIO_PxCLR_BASE (GPIO_PACLR_ADDR)
#define GPIO_PxSET_BASE (GPIO_PASET_ADDR)
#define GPIO_PxOUT_BASE (GPIO_PAOUT_ADDR)
// Each port is offset from the previous port by the same amount
#define GPIO_Px_OFFSET  (GPIO_PBCFGL_ADDR-GPIO_PACFGL_ADDR)

void halInternalEnableWatchDog ( void )
{
    WDOG_RESET = 0x01;        // Just to be on the safe side, restart the watchdog before enabling it
    WDOG_KEY   = 0xEABE;
    WDOG_CFG   = WDOG_ENABLE;
}

void halInternalResetWatchDog ( void )
{
    WDOG_RESET = 0x01;        // Writing any value will restart the watchdog
}

void halInternalDisableWatchDog ( int8u magicKey )
{
    if (magicKey == MICRO_DISABLE_WATCH_DOG_KEY) 
    {
        WDOG_KEY = 0xDEAD;
        WDOG_CFG = WDOG_DISABLE;
    }
}

boolean halInternalWatchDogEnabled ( void )
{
    return (WDOG_CFG & WDOG_ENABLE) ? (TRUE) : (FALSE);
}

void halGpioConfig ( int32u io, int32u config )
{
    static volatile int32u *const configRegs[] = 
    { (volatile int32u *)GPIO_PACFGL_ADDR,
      (volatile int32u *)GPIO_PACFGH_ADDR,
      (volatile int32u *)GPIO_PBCFGL_ADDR,
      (volatile int32u *)GPIO_PBCFGH_ADDR,
      (volatile int32u *)GPIO_PCCFGL_ADDR,
      (volatile int32u *)GPIO_PCCFGH_ADDR };
    int32u portcfg;
    
    portcfg = *configRegs[io / 0x04];                        // get current config                   
    portcfg = portcfg & ~((0x0F) << ((io & 0x03) * 0x04));   // mask out config of this pin
    *configRegs[io / 0x04] = portcfg | (config << ((io & 0x03) * 0x04));
}

// Calibrate the GPIO Pad Drive Strength.  If VDD_PADS is below 2.7V then set the PAD_STRENGTH bit high. If VDD_PADS is above 2.7V clear 
// the bit. To match EM250 functionality we need to calibrate pads while coming out of a reset and powerup. It is best to also periodically 
// calibrate pads (on the order of 0.5-2 seconds is perfectly fine).
void halInternalCalibratePads ( void )
{
    if (emberMeasureVddFast() < 2700) 
        GPIO_DBGCFG |= GPIO_DBGCFGRSVD;
    else 
        GPIO_DBGCFG &= ~GPIO_DBGCFGRSVD;
}

extern tokTypeMfgOsc24mBiasTrim biasTrim;

#define	GPIO_EXT_PA_HIGH_GAIN	PORTA_PIN(6)


void halSetGPIO ( u8 ucIO )
{
    if (ucIO / 0x08 < 0x03)
        *((volatile u32 *)(GPIO_PASET_ADDR + ((GPIO_PBCFGL_ADDR - GPIO_PACFGL_ADDR) * (ucIO / 0x08)))) = BIT(ucIO & 0x07);
}


void halClearGPIO ( u8 ucIO )
{
    if (ucIO / 0x08 < 0x03)
        *((volatile u32 *)(GPIO_PACLR_ADDR + ((GPIO_PBCFGL_ADDR - GPIO_PACFGL_ADDR) * (ucIO / 0x08)))) = BIT(ucIO & 0x07);
}

/* new function */
void halGpioSet(int32u gpio, boolean value)
{
  if(gpio/8 < 3) {
    if (value) {
      *((volatile int32u *)(GPIO_PxSET_BASE+(GPIO_Px_OFFSET*(gpio/8)))) = BIT(gpio&7);
    } else {
      *((volatile int32u *)(GPIO_PxCLR_BASE+(GPIO_Px_OFFSET*(gpio/8)))) = BIT(gpio&7);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : PAM_Configuration()
//* 功能        : PA配置
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void PAM_Configuration ( void )
{  
#ifdef EMZ3118
    halGpioConfig(GPIO_EXT_PA_ON_OFF, GPIOCFG_OUT);	// for external PA control
    halClearGPIO(GPIO_EXT_PA_ON_OFF);			// turn on PA

    halGpioConfig(GPIO_EXT_PA_HIGH_GAIN, GPIOCFG_OUT);	//
    halClearGPIO(GPIO_EXT_PA_HIGH_GAIN);			// SKY65336 - set low to high gain

    halGpioConfig( GPIO_TX_RX_SWITCH, GPIOCFG_OUT_ALT);	// STM32W TX/RX switch

    emberSetTxPowerMode(EMBER_TX_POWER_MODE_ALTERNATE); // (0x0002);
#endif
#ifdef EMZ3118B
    halGpioConfig(GPIO_EXT_PA_ON_OFF, GPIOCFG_OUT);	// for external PA control
    halSetGPIO(GPIO_EXT_PA_ON_OFF);			//set PA3 to turn on PA power

    halGpioConfig(GPIO_EXT_PA_LNA_ENABLE, GPIOCFG_OUT);	//
    halSetGPIO(GPIO_EXT_PA_LNA_ENABLE);			// set PA6 high to enable LNA

    halGpioConfig( GPIO_TX_RX_SWITCH, GPIOCFG_OUT_ALT);	// STM32W TX/RX switch
    halGpioConfig( GPIO_ANT_SEL, GPIOCFG_OUT);
    halSetGPIO( GPIO_ANT_SEL ); //select ANT1, the external antenna 

    emberSetTxPowerMode(EMBER_TX_POWER_MODE_ALTERNATE); // (0x0002);
#endif
}

// ST_RadioSetPowerMode()这条语句是至关重要的。
// 注意：带PA模块：ST_RadioSetPowerMode(0x0002);
// 不带PA模块：ST_RadioSetPowerMode(0x0000);
void halBoardPowerUp ( void )
{
#ifdef EMZ3118
    POWERUP_SET_GPIO_OUTPUT_DATA_REGISTERS();    
    POWERUP_SET_GPIO_CFG_REGISTERS();
  
// 修改寄存器的值  
//  GPIO_PACFGL |= (GPIOCFG_OUT<<PA3_CFG_BIT);        // 功放控制IO  PA3
//  GPIO_PACLR_REG = (1<<PA3_BIT);                    // 置低，打开功放  PA3
//  GPIO_PACFGL |= (GPIOCFG_OUT<<PA6_CFG_BIT);       // 功放高增益模式控制IO  PA6
//  GPIO_PACLR_REG = (1<<PA6_BIT);                   // 置低，使功放处于高增益模式  PA6
//  GPIO_PCCFGH |= (GPIOCFG_OUT_ALT<<PC5_CFG_BIT); // 功放控制,特殊功能引脚    PC5
  
//  GPIO_PACFGH &= (~0x0f) << PA6_CFG_BIT ;  
//  GPIO_PACFGH |= GPIOCFG_OUT << PA6_CFG_BIT; 
//  
//  GPIO_PAOUT &= 0 << PA6_BIT; 
//  GPIO_PAOUT |= 0 << PA6_BIT;
  
    GPIO_PACFGH &= ~0xffff;
    GPIO_PACFGH  = (GPIOCFG_OUT           <<PA4_CFG_BIT)|   \
                   (PWRUP_CFG_PTI_DATA    <<PA5_CFG_BIT)|   \
                   (GPIOCFG_OUT           <<PA6_CFG_BIT)|   \
                   (CFG_BUTTON1           <<PA7_CFG_BIT);   
  
    GPIO_PACFGL &= ~0xffff;             
    GPIO_PACFGL  = (DUMMY_GPIO_STATE_UP   <<PA0_CFG_BIT)|                      \
                   (GPIOCFG_OUT_ALT_OD    <<PA1_CFG_BIT)|  /* Mems SDA */      \
                   (GPIOCFG_OUT_ALT_OD    <<PA2_CFG_BIT)|  /* Mems SCL */      \
                   (GPIOCFG_OUT           <<PA3_CFG_BIT);
  
  
    GPIO_PACLR_REG = (1<<PA3_BIT)| (1 << PA6_BIT);    //GPIO_PAOUT    
  
    GPIO_PCCFGH &= ~0xffff;
    GPIO_PCCFGH  = (GPIOCFG_IN            <<PC4_CFG_BIT)|                      \
                   (GPIOCFG_OUT_ALT       <<PC5_CFG_BIT)|                      \
                   (DUMMY_GPIO_STATE_UP   <<PC6_CFG_BIT)|                      \
                   (DUMMY_GPIO_STATE_UP   <<PC7_CFG_BIT);
  
    emberSetTxPowerMode(EMBER_TX_POWER_MODE_ALTERNATE); // 设置发送功率模式  PC5
    // EMBER_TX_POWER_MODE_BOOST_AND_ALTERNATE
#endif
}

////////////////////////////////////////////////////////////////////////////////

void HAL_Configuration ( void )   // halInit ( void )
{
#ifdef EMBER_EMU_TEST
    // Strip emulator only code from official build. On the emulator, give our fake XTAL reasonable thresholds so the cal algorithm ends up at 4.
    EMU_OSC24M_CTRL = ((0x08 << EMU_OSC24M_CTRL_OSC24M_THRESH_H_BIT) |
                       (0x02 << EMU_OSC24M_CTRL_OSC24M_THRESH_L_BIT) |
                       (0x00 << EMU_OSC24M_CTRL_OSC24M_THRESH_STOP_BIT));
#endif
  
    // Preload our biasTrim shadow variable from the token. If the token is not set, then run a search to find an initial value. 
    // The bias trim algorithm/clock switch logic will always use the biasTrim shadow variable as the starting point for finding 
    // the bias, and then save that new bias to the shadow variable.
    halCommonGetToken(&biasTrim, TOKEN_MFG_OSC24M_BIAS_TRIM);
    if (biasTrim == 0xFFFF) 
    {
        halInternalSearchForBiasTrim();
    }
  
    halInternalSetRegTrim(FALSE);
  
    GPIO_DBGCFG |= GPIO_DBGCFGRSVD;
  
    halInternalSwitchToXtal();
  
#ifndef DISABLE_RC_CALIBRATION
    halInternalCalibrateFastRc();
#endif  //DISABLE_RC_CALIBRATION
  
#ifndef DISABLE_WATCHDOG
    halInternalEnableWatchDog();
#endif

    halInternalStartSystemTimer();
    halInternalCalibratePads();
    halInternalInitBoard();  
  
#ifdef INTERRUPT_DEBUGGING
    // When debugging interrupts/ATOMIC, ensure that our debug pin is properly enabled and idle low.
    I_OUT(I_PORT, I_PIN, I_CFG_HL);
    I_CLR(I_PORT, I_PIN);
#endif //INTERRUPT_DEBUGGING
    
    INTERRUPTS_ON();          // allow interrupts
}

void halReboot ( void )
{
    halInternalSysReset(RESET_SOFTWARE_REBOOT);
}

void halPowerDown ( void )
{
    halInternalPowerDownUart();
    halInternalPowerDownBoard();
}

void halPowerUp ( void )
{
    halInternalCalibratePads();
    halInternalPowerUpBoard(); 
    halInternalSwitchToXtal();
    halInternalPowerUpUart();
}

void halStackProcessBootCount ( void )
{
    // Note: Becuase this always counts up at every boot (called from emberInit), and non-volatile storage has a finite number of write cycles,
    // this will eventually stop working.  Disable this token call if non-volatile write cycles need to be used sparingly.
    halCommonIncrementCounterToken(TOKEN_STACK_BOOT_COUNTER);
}

PGM_P halGetResetString ( void )
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
int8u halGetEm2xxResetInfo ( void )
{
    int8u reset = halGetResetInfo();

    // Any reset with an extended value field of zero is considered an unknown
    // reset, except for FIB resets.
    if ((RESET_EXTENDED_FIELD(halGetExtendedResetInfo()) == 0x00) && (reset != RESET_FIB)) 
    {
        return EM2XX_RESET_UNKNOWN;
    }

    switch (reset) 
    {
        case RESET_UNKNOWN:
            return EM2XX_RESET_UNKNOWN;
        case RESET_BOOTLOADER:
            return EM2XX_RESET_BOOTLOADER;
        case RESET_EXTERNAL:
            return EM2XX_RESET_EXTERNAL;  
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

//Burning cycles for milliseconds is generally a bad idea, but it is
//necessary in some situations.  If you have to burn more than 65ms of time,
//the halCommonDelayMicroseconds function becomes cumbersome, so this
//function gives you millisecond granularity.
void halCommonDelayMilliseconds ( int16u ms )
{
    if (ms == 0x00) 
    {
        return;
    }
  
    while (ms-- > 0x00) 
    {
        halCommonDelayMicroseconds(1000);
    }
}

//[[ Most of the system-timer functionality is part of the hal-library
//  This functionality is kept public because it depends on configuration 
//  options defined in the BOARD_HEADER
//]]
int16u halInternalStartSystemTimer ( void )
{
    // Since the SleepTMR is the only timer maintained during deep sleep, it is
    // used as the System Timer (RTC).  We maintain a 32 bit hardware timer
    // configured for a tick value time of 1024 ticks/second (0.9765625 ms/tick)
    // using either the 10 kHz internal SlowRC clock divided and calibrated to
    // 1024 Hz or the external 32.768 kHz crystal divided to 1024 Hz.
    // With a tick time of ~1ms, this 32bit timer will wrap after ~48.5 days.
  
    // disable top-level interrupt while configuring
    INT_CFGCLR = INT_SLEEPTMR;
  
#ifdef ENABLE_OSC32K
  #ifdef DIGITAL_OSC32_EXT
    // Disable both OSC32K and SLOWRC if using external digital clock input
    SLEEPTMR_CLKEN = 0x00;
  #else //!DIGITAL_OSC32_EXT
    // Enable the 32kHz XTAL (and disable SlowRC since it is not needed)
    SLEEPTMR_CLKEN = SLEEPTMR_CLK32KEN;
  #endif
    // Sleep timer configuration is the same for crystal and external clock
    SLEEPTMR_CFG = (SLEEPTMR_ENABLE            | //enable TMR
                   (0 << SLEEPTMR_DBGPAUSE_BIT)| //TMR paused when halted
                   (5 << SLEEPTMR_CLKDIV_BIT)  | //divide down to 1024Hz
                   (1 << SLEEPTMR_CLKSEL_BIT)) ; //select XTAL
#else //!ENABLE_OSC32K
    // Enable the SlowRC (and disable 32kHz XTAL since it is not needed)
    SLEEPTMR_CLKEN = SLEEPTMR_CLK10KEN;
    SLEEPTMR_CFG = (SLEEPTMR_ENABLE            | //enable TMR
                   (0 << SLEEPTMR_DBGPAUSE_BIT)| //TMR paused when halted
                   (0 << SLEEPTMR_CLKDIV_BIT)  | //already 1024Hz
                   (0 << SLEEPTMR_CLKSEL_BIT)) ; //select SlowRC
  #ifndef DISABLE_RC_CALIBRATION
      halInternalCalibrateSlowRc(); //calibrate SlowRC to 1024Hz
  #endif//DISABLE_RC_CALIBRATION
#endif//ENABLE_OSC32K
    
    INT_SLEEPTMRFLAG = (INT_SLEEPTMRWRAP | INT_SLEEPTMRCMPA | INT_SLEEPTMRCMPB);  // clear out any stale interrupts
    INT_SLEEPTMRCFG  = INT_SLEEPTMRCFG_RESET;                                     // turn off second level interrupts.  they will be enabled elsewhere as needed
    INT_CFGSET       = INT_SLEEPTMR;                                              // enable top-level interrupt
  
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
