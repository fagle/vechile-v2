/** @file hal/micro/cortexm3/em35x/board/ref0657.h
 * See @ref board for detailed documentation.
 * 
 * <!-- Copyright 2008-2010 by Ember Corporation. All rights reserved.   *80*-->
 */

/** @addtogroup board
 *  @brief Functions and definitions specific to the breakout board.
 * 
 * @note The file ref0657.h is a special variant of dev0680.h
 * intended only for use with the SiGe SE2432L reference design.
 *
 * The file dev0680.h is the default BOARD_HEADER file used with the breakout 
 * board of the development kit.
 * 
 * The EM35x on a dev0680 BoB has the following example GPIO configuration.
 * This board file and the default HAL setup reflects this configuration.
 * - PA0 - SC2MOSI
 * - PA1 - SC2MISO
 * - PA2 - SC2SCLK
 * - PA3 - SC2nSSEL
 * - PA4 - PTI_EN
 * - PA5 - PTI_DATA
 * - PA6 - LED (on RCM)
 * - PA7 - LED (on RCM)
 * - PB0 - Power Amplifier shutdown control
 * - PB1 - SC1TXD
 * - PB2 - SC1RXD
 * - PB3 - SC1nCTS
 * - PB4 - SC1nRTS
 * - PB5 - TEMP_SENSE (also used for SiGe SE2432L CPS)
 * - PB6 - Button (IRQB fixed to PB6)
 * - PB7 - Buzzer (also used for DataFlash Enable)
 * - PC0 - JTAG (JRST)
 * - PC1 - Power Amplifier antenna select control
 * - PC2 - JTAG (JTDO) / SWO
 * - PC3 - JTAG (JTDI)
 * - PC4 - JTAG (JTMS)
 * - PC5 - LED (on BoB)
 * - PC6 - Button (IRQC pointed to PC6)
 * - PC7 - TEMP_EN
 *
 *@{
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/** @name Custom Baud Rate Definitions
 *
 * The following define is used with defining a custom baud rate for the UART.
 * This define provides a simple hook into the definition of
 * the baud rates used with the UART.  The baudSettings[] array in uart.c
 * links the BAUD_* defines with the actual register values needed for
 * operating the UART.  The array baudSettings[] can be edited directly for a
 * custom baud rate or another entry (the register settings) can be provided
 * here with this define.
 */
//@{
/**
 * @brief This define is the register setting for generating a baud of
 * 921600.  Refer to the EM35x datasheet's discussion on UART baud rates for
 * the equation used to derive this value.
 */
#define EMBER_SERIAL_BAUD_CUSTOM  13
//@} //END OF CUSTOM BAUD DEFINITIONS

/** @name LED Definitions
 *
 * The following are used to aid in the abstraction with the LED
 * connections.  The microcontroller-specific sources use these
 * definitions so they are able to work across a variety of boards
 * which could have different connections.  The names and ports/pins
 * used below are intended to match with a schematic of the system to
 * provide the abstraction.
 *
 * The ::HalBoardLedPins enum values should always be used when manipulating the 
 * state of LEDs, as they directly refer to the GPIOs to which the LEDs are
 * connected.
 *
 * \b Note: LEDs 0 and 1 are on the RCM.
 *
 * \b Note: LED 2 is on the breakout board (dev0680).
 *
 * \b Note: LED 3 simply redirects to LED 2.
 */
//@{

/**
 * @brief Assign each GPIO with an LED connected to a convenient name.
 * ::BOARD_ACTIVITY_LED and ::BOARD_HEARTBEAT_LED provide a further layer of
 * abstraction ontop of the 3 LEDs for verbose coding.
 */
enum HalBoardLedPins {
  BOARDLED0 = PORTA_PIN(6),
  BOARDLED1 = PORTA_PIN(7),
  BOARDLED2 = PORTC_PIN(5),
  BOARDLED3 = BOARDLED2,
  BOARD_ACTIVITY_LED  = BOARDLED0,
  BOARD_HEARTBEAT_LED = BOARDLED1
};

/** @} END OF LED DEFINITIONS  */

/** @name Button Definitions
 *
 * The following are used to aid in the abstraction with the Button
 * connections.  The microcontroller-specific sources use these
 * definitions so they are able to work across a variety of boards
 * which could have different connections.  The names and ports/pins
 * used below are intended to match with a schematic of the system to
 * provide the abstraction.
 *
 * The BUTTONn macros should always be used with manipulating the buttons
 * as they directly refer to the GPIOs to which the buttons are connected.
 *
 * @note The GPIO number must match the IRQ letter
 */
//@{
/**
 * @brief The actual GPIO BUTTON0 is connected to.  This define should
 * be used whenever referencing BUTTON0.
 */
#define BUTTON0             PORTB_PIN(6)
/**
 * @brief The GPIO input register for BUTTON0.
 */
#define BUTTON0_IN          GPIO_PBIN
/**
 * @brief Point the proper IRQ at the desired pin for BUTTON0.
 * @note IRQB is fixed and as such does not need any selection operation.
 */
#define BUTTON0_SEL()       do { } while(0)
/**
 * @brief The interrupt service routine for BUTTON0.
 */
#define BUTTON0_ISR         halIrqBIsr
/**
 * @brief The interrupt configuration register for BUTTON0.
 */
#define BUTTON0_INTCFG      GPIO_INTCFGB
/**
 * @brief The interrupt enable bit for BUTTON0.
 */
#define BUTTON0_INT_EN_BIT  INT_IRQB
/**
 * @brief The interrupt flag bit for BUTTON0.
 */
#define BUTTON0_FLAG_BIT    INT_IRQBFLAG
/**
 * @brief The missed interrupt bit for BUTTON0.
 */
#define BUTTON0_MISS_BIT    INT_MISSIRQB

/**
 * @brief The actual GPIO BUTTON1 is connected to.  This define should
 * be used whenever referencing BUTTON1.
 */
#define BUTTON1             PORTC_PIN(6)
/**
 * @brief The GPIO input register for BUTTON1.
 */
#define BUTTON1_IN          GPIO_PCIN
/**
 * @brief Point the proper IRQ at the desired pin for BUTTON1.
 * @note For this board, IRQC is pointed at PC6
 */
#define BUTTON1_SEL()       do { GPIO_IRQCSEL = PORTC_PIN(6); } while(0)
/**
 * @brief The interrupt service routine for BUTTON1.
 */
#define BUTTON1_ISR         halIrqCIsr
/**
 * @brief The interrupt configuration register for BUTTON1.
 */
#define BUTTON1_INTCFG      GPIO_INTCFGC
/**
 * @brief The interrupt enable bit for BUTTON1.
 */
#define BUTTON1_INT_EN_BIT  INT_IRQC
/**
 * @brief The interrupt flag bit for BUTTON1.
 */
#define BUTTON1_FLAG_BIT    INT_IRQCFLAG
/**
 * @brief The missed interrupt bit for BUTTON1.
 */
#define BUTTON1_MISS_BIT    INT_MISSIRQC
//@} //END OF BUTTON DEFINITIONS

/** @name Temperature sensor ADC channel
 *
 * Define the analog input channel connected to the LM-20 temperature sensor.
 * The scale factor compensates for different platform input ranges.
 * PB5/ADC0 must be an analog input.
 * PC7 must be an output and set to a high level to power the sensor.
 *
 *@{
 */
/**
 * @brief The analog input channel to use for the temperature sensor.
 */
#define TEMP_SENSOR_ADC_CHANNEL ADC_SOURCE_ADC0_VREF2
/**
 * @brief The scale factor to compensate for different input ranges.
 */
#define TEMP_SENSOR_SCALE_FACTOR 1
/** @} END OF TEMPERATURE SENSOR ADC CHANNEL DEFINITIONS */

/** @name Packet Trace
 *
 * When ::PACKET_TRACE is defined, ::GPIO_PACFGH will automatically be setup by
 * halInit() to enable Packet Trace support on PA4 and PA5,
 * in addition to the configuration specified below.
 * 
 * @note This define will override any settings for PA4 and PA5.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable Packet Trace support on the breakout board (dev0680).
 */
#define PACKET_TRACE  // We do have PACKET_TRACE support
//@} //END OF PACKET TRACE DEFINITIONS

/** @name ENABLE_OSC32K
 *
 * When ENABLE_OSC32K is defined, halInit() will configure system
 * timekeeping to utilize the external 32.768 kHz crystal oscillator
 * rather than the internal 1 kHz RC oscillator.
 *
 * @note ENABLE_OSC32K is mutually exclusive with
 * ENABLE_ALT_FUNCTION_NTX_ACTIVE since they define conflicting
 * usage of GPIO PC6.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable 32.768 kHz XTAL oscillator on the RCM plugged
 * into the Breakout board (dev0680).
 * Default is to disable 32.768 kHz XTAL and use 1 kHz RC oscillator instead.
 */
//#define ENABLE_OSC32K  // Enable 32.768 kHz osc instead of 1 kHz RC osc
//@} //END OF ENABLE OSC32K DEFINITIONS

/** @name DISABLE_SLEEPTMR_DEEPSLEEP
 *
 * When DISABLE_SLEEPTMR_DEEPSLEEP is defined, halSleep() will turn off the
 * internal 10kHz SlowRC used for the system-timer when entering deep sleep.
 * The result of doing this is called DeepSleep2 and will yield a lower current
 * consumption than with the 10kHz SlowRC enabled (which is called DeepSleep1).
 *
 * To still be able to do timed sleep operations and maintain the system-timer
 * across deep sleep while DISABLE_SLEEPTMR_DEEPSLEEP is defined, ENABLE_OSC32K
 * must also be defined causing the system-timer to switch to the external
 * 32kHz XTAL.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to disable the internal 10kHz SlowRC oscillator.  The default is to
 * keep the 10kHz SlowRC enabled.
 */
//#define DISABLE_SLEEPTMR_DEEPSLEEP  // Disable internal 10kHz SlowRC
//@} //END OF DISABLE SLOWRC DEFINITIONS

/** @name DISABLE_OSC24M_BIAS_TRIM
 *
 * When DISABLE_OSC24M_BIAS_TRIM is defined, halInit() will disable adjusting
 * the 24 MHz oscillator's bias trim based on its corresponding Manufacturing
 * token, and instead utilize full bias at power up and deep sleep wakeups.
 * This should be utilized only if the Manufacturing token value proves to be
 * unreliable.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to disable bias adjustment of the 24 MHz crystal oscillator on
 * the RCM plugged into the Breakout board (dev0680).
 * Default is to enable bias adjustment.
 */
//#define DISABLE_OSC24M_BIAS_TRIM  // Disable 24 MHz bias trim adjustment
//@} //END OF DISABLE OSC24M BIAS TRIM DEFINITIONS


/** @name ENABLE_ALT_FUNCTION_REG_EN
 *
 * When ENABLE_ALT_FUNCTION_REG_EN is defined, halInit() will enable the REG_EN
 * alternate functionality on PA7.  REG_EN is the special signal provided
 * by the EM35x's internal power controller which can be used to tell an
 * external power regulator when the EM35x is in deep sleep or not and as such
 * signal the external regulator to cut power.  This signal will override all
 * GPIO configuration and use of PA7.  When the alternate functionality is
 * not enabled, PA7 can be operated as a standard GPIO.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable the REG_EN alternate function on PA7.
 * Default is to not enable REG_EN functionality on PA7.
 */
//#define ENABLE_ALT_FUNCTION_REG_EN
//@} //END OF ENABLE_ALT_FUNCTION_REG_EN DEFINITIONS


/** @name ENABLE_ALT_FUNCTION_TX_ACTIVE
 *
 * When ENABLE_ALT_FUNCTION_TX_ACTIVE is defined, halInit() and halPowerUp()
 * will enable the TX_ACTIVE alternate functionality of PC5.  halPowerDown()
 * will configure PC5 to be a low output.  TX_ACTIVE can be used for
 * external PA power management and RF switching logic.  In transmit mode
 * the Tx baseband drives TX_ACTIVE high.  In receive mode the TX_ACTIVE
 * signal is low.  This define will override any settings for PC5.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable the TX_ACTIVE alternate function on PC5.
 * It is enabled by default in this board file because the SiGe SE2432L
 * requires the TX_ACTIVE alternate function to operate properly.
 */
#define ENABLE_ALT_FUNCTION_TX_ACTIVE
//@} //END OF ENABLE_ALT_FUNCTION_TX_ACTIVE DEFINITIONS


/** @name ENABLE_ALT_FUNCTION_NTX_ACTIVE
 *
 * When ENABLE_ALT_FUNCTION_NTX_ACTIVE is defined, halInit() and halPowerUp()
 * will enable the nTX_ACTIVE alternate functionality of PC6.  halPowerDown()
 * will configure PC6 to be a low output.  nTX_ACTIVE can be used for
 * external PA power management and RF switching logic.  In transmit mode
 * the Tx baseband drives nTX_ACTIVE low.  In receive mode the nTX_ACTIVE
 * signal is high.  This define will override any settings for PC6.
 *
 * @note ENABLE_ALT_FUNCTION_NTX_ACTIVE is mutually exclusive with
 * ENABLE_OSC32K since they define conflicting usage of GPIO PC6.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable the nTX_ACTIVE alternate function on PC6.
 * Default is to not enable nTX_ACTIVE functionality on PC6.
 */
//#define ENABLE_ALT_FUNCTION_NTX_ACTIVE
//@} //END OF ENABLE_ALT_FUNCTION_NTX_ACTIVE DEFINITIONS

/** @name EEPROM_USES_SHUTDOWN_CONTROL
 *
 * When EEPROM_USES_SHUTDOWN_CONTROL is defined, logic is enabled in the
 * EEPROM driver which drives PB7 high upon EEPROM initialization.  In 
 * Ember reference designs, PB7 acts as an EEPROM enable pin and therefore
 * must be driven high in order to use the EEPROM.  This option is intended
 * to be enabled when running app-bootloader on designs based on current
 * Ember reference designs.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable the logic that drives PB7 high in the EEPROM driver.
 */
#define EEPROM_USES_SHUTDOWN_CONTROL
//@} //END OF EEPROM_USES_SHUTDOWN_CONTROL DEFINITIONS


/** @name GPIO Configuration Definitions
 *
 * The following are used to specify the GPIO configuration to establish
 * when Powered (POWERUP_), and when Deep Sleeping (POWERDOWN_).  The reason
 * for separate Deep Sleep settings is to allow for a slightly different
 * configuration that minimizes power consumption during Deep Sleep.  For
 * example, inputs that might float could be pulled up or down, and output
 * states chosen with care, e.g. to turn off LEDs or other devices that might
 * consume power or be unnecessary when Deep Sleeping.
 */
//@{


/** @name Packet Trace Configuration Defines
 *
 * Provide the proper set of pin configuration for when the Packet
 * Trace is enabled (look above for the define which enables it).  When Packet
 * Trace is not enabled, leave the two PTI pins in their default configuration.
 * If Packet Trace is not being used, feel free to set the pin configurations
 * as desired.  The config shown here is simply the Power On Reset defaults.
 *@{
 */
/**
 * @brief Give the packet trace configuration a friendly name.
 */
#ifdef  PACKET_TRACE
  #define PWRUP_CFG_PTI_EN    GPIOCFG_OUT_ALT
  #define PWRUP_OUT_PTI_EN    0
  #define PWRDN_CFG_PTI_EN    GPIOCFG_IN_PUD
  #define PWRDN_OUT_PTI_EN    GPIOOUT_PULLDOWN
  #define PWRUP_CFG_PTI_DATA  GPIOCFG_OUT_ALT
  #define PWRUP_OUT_PTI_DATA  1
  #define PWRDN_CFG_PTI_DATA  GPIOCFG_IN_PUD
  #define PWRDN_OUT_PTI_DATA  GPIOOUT_PULLUP
#else//!PACKET_TRACE
  #define PWRUP_CFG_PTI_EN    GPIOCFG_IN
  #define PWRUP_OUT_PTI_EN    0
  #define PWRDN_CFG_PTI_EN    GPIOCFG_IN
  #define PWRDN_OUT_PTI_EN    0
  #define PWRUP_CFG_PTI_DATA  GPIOCFG_IN
  #define PWRUP_OUT_PTI_DATA  0
  #define PWRDN_CFG_PTI_DATA  GPIOCFG_IN
  #define PWRDN_OUT_PTI_DATA  0
#endif//PACKET_TRACE
//@} END OF Packet Trace Configuration Defines


/** @name 32kHz Oscillator and nTX_ACTIVE Configuration Defines
 *
 * Since the 32kHz Oscillator and nTX_ACTIVE both share PC6, their
 * configuration defines are linked and instantiated together.  Look above
 * for the defines that enable the 32kHz Oscillator and nTX_ACTIVE.
 *
 * @note ENABLE_OSC32K is mutually exclusive with
 * ENABLE_ALT_FUNCTION_NTX_ACTIVE since they define conflicting
 * usage of GPIO PC6.
 * 
 * When using the 32kHz, configure PC6 and PC7 for analog for the XTAL.
 * 
 * When using nTX_ACTIVE, configure PC6 for alternate output while awake
 * and a low output when deepsleeping.  Also, configure PC7 for TEMP_EN.
 * 
 * When not using the 32kHz or nTX_ACTIVE, configure PC6 and PC7 for
 * Button1 and TEMP_EN.
 *@{
 */
/**
 * @brief Give GPIO PC6 configuration a friendly name.
 */
#if defined(ENABLE_OSC32K) && defined(ENABLE_ALT_FUNCTION_NTX_ACTIVE)
  //Oops!  Only one of these can be used at a time!
  #error ENABLE_OSC32K and ENABLE_ALT_FUNCTION_NTX_ACTIVE are mutually\
 exclusive.  They define conflicting usage for GPIO PC6.

#elif defined(ENABLE_OSC32K) && !defined(ENABLE_ALT_FUNCTION_NTX_ACTIVE)
  //Use OCS32K configuration
  #define PWRUP_CFG_BUTTON1  GPIOCFG_ANALOG
  #define PWRUP_OUT_BUTTON1  0
  #define PWRDN_CFG_BUTTON1  GPIOCFG_ANALOG
  #define PWRDN_OUT_BUTTON1  0

#elif !defined(ENABLE_OSC32K) && defined(ENABLE_ALT_FUNCTION_NTX_ACTIVE)
  //Use nTX_ACTIVE configuration
  #define PWRUP_CFG_BUTTON1  GPIOCFG_OUT_ALT
  #define PWRUP_OUT_BUTTON1  0
  #define PWRDN_CFG_BUTTON1  GPIOCFG_OUT
  #define PWRDN_OUT_BUTTON1  0

#else
  //Use Button1 configuration
  #define PWRUP_CFG_BUTTON1  GPIOCFG_IN_PUD
  #define PWRUP_OUT_BUTTON1  GPIOOUT_PULLUP /* Button needs a pullup */
  #define PWRDN_CFG_BUTTON1  GPIOCFG_IN_PUD
  #define PWRDN_OUT_BUTTON1  GPIOOUT_PULLUP /* Button needs a pullup */
  
#endif

/**
 * @brief Give GPIO PC7 configuration a friendly name.
 */
#ifdef  ENABLE_OSC32K
  #define CFG_TEMPEN         GPIOCFG_ANALOG
#else//!ENABLE_OSC32K
  #define CFG_TEMPEN         GPIOCFG_OUT
#endif//ENABLE_OSC32K
//@} END OF 32kHz Oscillator and nTX_ACTIVE Configuration Defines


/** @name TX_ACTIVE Configuration Defines
 *
 * Provide the proper set of pin (PC5) configurations for when TX_ACTIVE is
 * enabled (look above for the define which enables it).  When TX_ACTIVE is
 * not enabled, configure the pin for LED2.
 *@{
 */
/**
 * @brief Give the TX_ACTIVE configuration a friendly name.
 */
#ifdef  ENABLE_ALT_FUNCTION_TX_ACTIVE
  #define PWRUP_CFG_LED2  GPIOCFG_OUT_ALT
  #define PWRUP_OUT_LED2  0
  #define PWRDN_CFG_LED2  GPIOCFG_OUT
  #define PWRDN_OUT_LED2  0
#else //!ENABLE_ALT_FUNCTION_TX_ACTIVE
  #define PWRUP_CFG_LED2  GPIOCFG_OUT
  #define PWRUP_OUT_LED2  1  /* LED default off */
  #define PWRDN_CFG_LED2  GPIOCFG_OUT
  #define PWRDN_OUT_LED2  1  /* LED default off */
#endif//ENABLE_ALT_FUNCTION_TX_ACTIVE
//@} END OF TX_ACTIVE Configuration Defines


/** @name GPIO Configuration Macros
 *
 * These macros define the GPIO configuration and initial state of the output
 * registers for all the GPIO in the powerup and powerdown modes.
 *@{
 */
/**
 * @brief Powerup GPIO configuration.
 */
#define POWERUP_SET_GPIO_CFG_REGISTERS()                           \
  GPIO_PACFGL = (GPIOCFG_OUT_ALT    <<PA0_CFG_BIT)|                \
                (GPIOCFG_IN         <<PA1_CFG_BIT)|                \
                (GPIOCFG_OUT_ALT    <<PA2_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA3_CFG_BIT);                \
  GPIO_PACFGH = (PWRUP_CFG_PTI_EN   <<PA4_CFG_BIT)|                \
                (PWRUP_CFG_PTI_DATA <<PA5_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA6_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA7_CFG_BIT);                \
  GPIO_PBCFGL = (GPIOCFG_OUT        <<PB0_CFG_BIT)|                \
                (GPIOCFG_OUT_ALT    <<PB1_CFG_BIT)|  /* SC1TXD  */ \
                (GPIOCFG_IN_PUD     <<PB2_CFG_BIT)|  /* SC1RXD  */ \
                (GPIOCFG_IN_PUD     <<PB3_CFG_BIT);  /* SC1nCTS */ \
  GPIO_PBCFGH = (GPIOCFG_OUT_ALT    <<PB4_CFG_BIT)|  /* SC1nRTS */ \
                (GPIOCFG_OUT        <<PB5_CFG_BIT)|                \
                (GPIOCFG_IN_PUD     <<PB6_CFG_BIT)|                \
                (GPIOCFG_OUT_ALT    <<PB7_CFG_BIT);                \
  GPIO_PCCFGL = (GPIOCFG_OUT_ALT    <<PC0_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PC1_CFG_BIT)|                \
                (GPIOCFG_OUT_ALT    <<PC2_CFG_BIT)|                \
                (GPIOCFG_IN         <<PC3_CFG_BIT);                \
  GPIO_PCCFGH = (GPIOCFG_IN         <<PC4_CFG_BIT)|                \
                (PWRUP_CFG_LED2     <<PC5_CFG_BIT)|                \
                (PWRUP_CFG_BUTTON1  <<PC6_CFG_BIT)|                \
                (CFG_TEMPEN         <<PC7_CFG_BIT);


/**
 * @brief Powerup GPIO output register.
 */
#define POWERUP_SET_GPIO_OUTPUT_DATA_REGISTERS()              \
  GPIO_PAOUT = (0                  <<PA0_BIT)|                \
               (0                  <<PA1_BIT)|                \
               (0                  <<PA2_BIT)|                \
               /* nSSEL is default idle high */               \
               (1                  <<PA3_BIT)|                \
               (PWRUP_OUT_PTI_EN   <<PA4_BIT)|                \
               (PWRUP_OUT_PTI_DATA <<PA5_BIT)|                \
               /* LED default off */                          \
               (1                  <<PA6_BIT)|                \
               /* LED default off */                          \
               (1                  <<PA7_BIT);                \
  GPIO_PBOUT = (1                  <<PB0_BIT)|                \
               (1                  <<PB1_BIT)|  /* SC1TXD  */ \
               (1                  <<PB2_BIT)|  /* SC1RXD  */ \
               (1                  <<PB3_BIT)|  /* SC1nCTS */ \
               (0                  <<PB4_BIT)|  /* SC1nRTS */ \
               /* SiGe SE2432L CPS pin defualt high */        \
               (1                  <<PB5_BIT)|                \
               /* PB6 has button needing a pullup */          \
               (GPIOOUT_PULLUP     <<PB6_BIT)|                \
               (0                  <<PB7_BIT);                \
  GPIO_PCOUT = (1                  <<PC0_BIT)|                \
               (0                  <<PC1_BIT)|                \
               (1                  <<PC2_BIT)|                \
               (0                  <<PC3_BIT)|                \
               (0                  <<PC4_BIT)|                \
               (PWRUP_OUT_LED2     <<PC5_BIT)|                \
               (PWRUP_OUT_BUTTON1  <<PC6_BIT)|                \
               /* Temp Sensor default on */                   \
               (1                  <<PC7_BIT);



/**
 * @brief Powerdown GPIO configuration.
 */
#define POWERDOWN_SET_GPIO_CFG_REGISTERS()                         \
  GPIO_PACFGL = (GPIOCFG_IN_PUD     <<PA0_CFG_BIT)|                \
                (GPIOCFG_IN_PUD     <<PA1_CFG_BIT)|                \
                (GPIOCFG_IN_PUD     <<PA2_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA3_CFG_BIT);                \
  GPIO_PACFGH = (PWRDN_CFG_PTI_EN   <<PA4_CFG_BIT)|                \
                (PWRDN_CFG_PTI_DATA <<PA5_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA6_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PA7_CFG_BIT);                \
  GPIO_PBCFGL = (GPIOCFG_OUT        <<PB0_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PB1_CFG_BIT)|  /* SC1TXD  */ \
                (GPIOCFG_IN_PUD     <<PB2_CFG_BIT)|  /* SC1RXD  */ \
                (GPIOCFG_IN_PUD     <<PB3_CFG_BIT);  /* SC1nCTS */ \
  GPIO_PBCFGH = (GPIOCFG_OUT        <<PB4_CFG_BIT)|  /* SC1nRTS */ \
                (GPIOCFG_OUT        <<PB5_CFG_BIT)|                \
                (GPIOCFG_IN_PUD     <<PB6_CFG_BIT)|                \
                /* need to use pulldown for sleep */               \
                (GPIOCFG_IN_PUD     <<PB7_CFG_BIT);                \
  GPIO_PCCFGL = (GPIOCFG_IN_PUD     <<PC0_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PC1_CFG_BIT)|                \
                (GPIOCFG_OUT        <<PC2_CFG_BIT)|                \
                (GPIOCFG_IN_PUD     <<PC3_CFG_BIT);                \
  GPIO_PCCFGH = (GPIOCFG_IN_PUD     <<PC4_CFG_BIT)|                \
                (PWRDN_CFG_LED2     <<PC5_CFG_BIT)|                \
                (PWRDN_CFG_BUTTON1  <<PC6_CFG_BIT)|                \
                (CFG_TEMPEN         <<PC7_CFG_BIT);


/**
 * @brief Powerdown GPIO output register.
 */
#define POWERDOWN_SET_GPIO_OUTPUT_DATA_REGISTERS()               \
  GPIO_PAOUT = (GPIOOUT_PULLUP     <<PA0_BIT)|                   \
               (GPIOOUT_PULLUP     <<PA1_BIT)|                   \
               (GPIOOUT_PULLUP     <<PA2_BIT)|                   \
               /* nSSEL is idle high */                          \
               (1                  <<PA3_BIT)|                   \
               /* enable is idle low */                          \
               (PWRDN_OUT_PTI_EN   <<PA4_BIT)|                   \
               /* data is idle high */                           \
               (PWRDN_OUT_PTI_DATA <<PA5_BIT)|                   \
               /* LED off */                                     \
               (1                  <<PA6_BIT)|                   \
               /* LED off */                                     \
               (1                  <<PA7_BIT);                   \
  GPIO_PBOUT = (0                  <<PB0_BIT)|                   \
               (GPIOOUT_PULLUP     <<PB1_BIT)|  /* SC1TXD  */    \
               (GPIOOUT_PULLUP     <<PB2_BIT)|  /* SC1RXD  */    \
               (GPIOOUT_PULLDOWN   <<PB3_BIT)|  /* SC1nCTS */    \
               (GPIOOUT_PULLDOWN   <<PB4_BIT)|  /* SC1nRTS */    \
               /* SiGe SE2432L CPS pin needs to be driven low */ \
               (0                  <<PB5_BIT)|                   \
               /* PB6 has button needing a pullup */             \
               (GPIOOUT_PULLUP     <<PB6_BIT)|                   \
               /* buzzer needs pulldown for sleep */             \
               (GPIOOUT_PULLDOWN   <<PB7_BIT);                   \
  GPIO_PCOUT = (GPIOOUT_PULLUP     <<PC0_BIT)|                   \
               (0                  <<PC1_BIT)|                   \
               (1                  <<PC2_BIT)|                   \
               (GPIOOUT_PULLDOWN   <<PC3_BIT)|                   \
               (GPIOOUT_PULLDOWN   <<PC4_BIT)|                   \
               (PWRDN_OUT_LED2     <<PC5_BIT)|                   \
               (PWRDN_OUT_BUTTON1  <<PC6_BIT)|                   \
               /* Temp Sensor off */                             \
               (0                  <<PC7_BIT);


/**
 * @brief External regulator enable/disable macro.
 */
#ifdef ENABLE_ALT_FUNCTION_REG_EN
  #define CONFIGURE_EXTERNAL_REGULATOR_ENABLE()  GPIO_DBGCFG |= GPIO_EXTREGEN;
#else
  #define CONFIGURE_EXTERNAL_REGULATOR_ENABLE()  GPIO_DBGCFG &= ~GPIO_EXTREGEN;
#endif
//@} END OF GPIO Configuration Macros


/** @name GPIO Wake Source Definitions
 *
 * A convenient define that chooses if this external signal can
 * be used as source to wake from deep sleep.  Any change in the state of the
 * signal will wake up the CPU.
 */
 //@{
/**
 * @brief TRUE if this GPIO can wake the chip from deep sleep, FALSE if not.
 */
#define WAKE_ON_PA0   FALSE
#define WAKE_ON_PA1   FALSE
#define WAKE_ON_PA2   FALSE
#define WAKE_ON_PA3   FALSE
#define WAKE_ON_PA4   FALSE
#define WAKE_ON_PA5   FALSE
#define WAKE_ON_PA6   FALSE
#define WAKE_ON_PA7   FALSE
#define WAKE_ON_PB0   FALSE
#define WAKE_ON_PB1   FALSE
#ifdef SLEEPY_EZSP_UART  // SC1RXD
  #define WAKE_ON_PB2   TRUE
#else
  #define WAKE_ON_PB2   FALSE
#endif
#define WAKE_ON_PB3   FALSE
#define WAKE_ON_PB4   FALSE
#define WAKE_ON_PB5   FALSE
#define WAKE_ON_PB6   TRUE   //BUTTON0
#define WAKE_ON_PB7   FALSE
#define WAKE_ON_PC0   FALSE
#define WAKE_ON_PC1   FALSE
#define WAKE_ON_PC2   FALSE
#define WAKE_ON_PC3   FALSE
#define WAKE_ON_PC4   FALSE
#define WAKE_ON_PC5   FALSE
#define WAKE_ON_PC6   TRUE   //BUTTON1
#define WAKE_ON_PC7   FALSE
//@} //END OF GPIO Wake Source Definitions


//@} //END OF GPIO Configuration Definitions


/** @name Board Specific Functions
 *
 * The following macros exist to aid in the initialization, power up from sleep,
 * and power down to sleep operations.  These macros are responsible for
 * either initializing directly, or calling initialization functions for any
 * peripherals that are specific to this board implementation.  These
 * macros are called from halInit, halPowerDown, and halPowerUp respectively.
 */
 //@{
/**
 * @brief Initialize the board.  This function is called from ::halInit().
 */
#ifndef EZSP_UART
  #define halInternalInitBoard()                                  \
          do {                                                    \
            halInternalPowerUpBoard();                            \
            halInternalRestartUart();                             \
            halInternalInitButton();                              \
         } while(0)
#else
  #define halInternalInitBoard()                                  \
          do {                                                    \
            halInternalPowerUpBoard();                            \
            halInternalRestartUart();                             \
         } while(0)
#endif

/**
 * @brief Power down the board.  This function is called from
 * ::halPowerDown().
 */
#define halInternalPowerDownBoard()                   \
        do {                                          \
          /* Board peripheral deactivation */         \
          /* halInternalSleepAdc(); */                \
          POWERDOWN_SET_GPIO_OUTPUT_DATA_REGISTERS()  \
          POWERDOWN_SET_GPIO_CFG_REGISTERS()          \
        } while(0)

/**
 * @brief Power up the board.  This function is called from
 * ::halPowerUp().
 */
#define halInternalPowerUpBoard()                     \
        do {                                          \
          POWERUP_SET_GPIO_OUTPUT_DATA_REGISTERS()    \
          POWERUP_SET_GPIO_CFG_REGISTERS()            \
          CONFIGURE_EXTERNAL_REGULATOR_ENABLE()       \
          /* Board peripheral reactivation */         \
          halInternalInitAdc();                       \
        } while(0)
//@} //END OF BOARD SPECIFIC FUNCTIONS

#endif //__BOARD_H__

/** @} END Board Specific Functions */
 
/** @} END addtogroup */
