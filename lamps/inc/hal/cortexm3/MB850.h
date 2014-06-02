/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : MB850.h
* Author             : MCD Application Team
* Version            : V1.0.1
* Date               : December 2009
* Description        : STM32W108 MB850 board header file 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/** @addtogroup board
 * The STM32W108 micro on a STM32W108 MB850 board  has the following example 
 *  GPIO configuration.
 * This board file and the default HAL (setup reflects this configuration.
 *
 * - PA0 - DUMMY GPIO/PIN  (TBR ???)
 * - PA1 - DUMMY GPIO/PIN 2 (TBR ???)
 * - PB1 - USART TX (SC1TXD)(x communication with STM32F103VET6 micro on Primer II)
 * - PB2 - USART RX (SC1RXD) (x communication with STM32F103VET6 micro on Primer II)
 * - Other ?? (JTAG/SWD) (TBR)
 *@{
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port A pins into a single number.
 */
#define PORTA_PIN(y) ((0<<3)|y)
/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port B pins into a single number.
 */
#define PORTB_PIN(y) ((1<<3)|y)
/**
 * @brief Some registers and variables require indentifying GPIO by
 * a single number instead of the port and pin.  This macro converts
 * Port C pins into a single number.
 */
#define PORTC_PIN(y) ((2<<3)|y)

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
 * 921600.  Refer to the STM32W108 datasheet's discussion on UART buad rates for
 * the equation used to derive this value.
 */
#define EMBER_SERIAL_BAUD_CUSTOM  13
//@} //END OF CUSTOM BAUD DEFINITIONS


/** @name Custom Dummy GPIOs definition
 *
 *  The following defins are used for mapping the original dev0680.h GPIO lines 
 *  (not used in the STM32W108 MB850 board) to dummy GPIO lines.
 *
 *  Note: DUMMY_GPIO_PORT_PIN, DUMMY_GPIO_PORT_PIN_2 are used for BUTTON0,
 *  BUTTON1 definition (required to be different due to the switch instructions on 
 *  halButtonState(), halButtonPinState() button functions.
 */
#define DUMMY_GPIO_PORT_PIN       PORTA_PIN(0)  /* GPIO A, PIN 0 (TBR ???) */ 
#define DUMMY_GPIO_PORT_PIN_2     PORTA_PIN(1)  /* GPIO A, PIN 1 (TBR ???) */ 
/* Dummy GPIO configuration and irqs settings: IRQC, IRQD are used for dummy gpio lines */
#define DUMMY_GPIO_IN             GPIO_PAIN
#define DUMMY_GPIO_ISR            halIrqCIsr
#define DUMMY_GPIO_ISR_2          halIrqDIsr     
#define DUMMY_GPIO_INTCFG         GPIO_INTCFGC
#define DUMMY_GPIO_INTCFG_2       GPIO_INTCFGD
#define DUMMY_GPIO_INT_EN_BIT     INT_IRQC
#define DUMMY_GPIO_INT_EN_BIT_2   INT_IRQD
#define DUMMY_GPIO_FLAG_BIT       INT_IRQCFLAG
#define DUMMY_GPIO_FLAG_BIT_2     INT_IRQDFLAG
#define DUMMY_GPIO_MISS_BIT       INT_MISSIRQC
#define DUMMY_GPIO_MISS_BIT_2     INT_MISSIRQD

//@{

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
 */
//@{

/**
 * @brief Assign each GPIO with an LED connected to a convenient name.
 * ::BOARD_ACTIVITY_LED and ::BOARD_HEARTBEAT_LED provide a further layer of
 * abstraction ontop of the 3 LEDs for verbose coding.
 * 
 * NOTE: No leds on the STM32W108 MB850 board. No activity and heartbeat leds.
 */
enum HalBoardLedPins { 
  BOARDLED0 = DUMMY_GPIO_PORT_PIN, /* No leds */
  BOARDLED1 = DUMMY_GPIO_PORT_PIN_2, /* No leds */
  BOARDLED2 = BOARDLED0,    /* Redefined BOARDLED2 as BOARDLED0 */
  BOARDLED3 = BOARDLED1,    /* Redefined BOARDLED3 as BOARDLED1 */
  BOARD_ACTIVITY_LED  = DUMMY_GPIO_PORT_PIN, /* No activity led */
  BOARD_HEARTBEAT_LED = DUMMY_GPIO_PORT_PIN  /* No heartbeat led */
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
 * Note: No buttons on the STM32W108 MB850 board 
 * Map the original button 1 gpio, irqs settings to the dummy configuration/settings
 */
#define BUTTON0             DUMMY_GPIO_PORT_PIN /* No Buttons */
/**
 * @brief The GPIO input register for BUTTON0.
 */
#define BUTTON0_IN          DUMMY_GPIO_IN  /* No Buttons */
/**
 * @brief Point the proper IRQ at the desired pin for BUTTON0.

 * @note For this board no Buttons 
 */
#define BUTTON0_SEL()       do { } while(0)
/**
 * @brief The interrupt service routine for BUTTON0.
 */
#define BUTTON0_ISR         DUMMY_GPIO_ISR /* No Buttons */
/**
 * @brief The interrupt configuration register for BUTTON0.
 */
#define BUTTON0_INTCFG      DUMMY_GPIO_INTCFG /* No Buttons */
/**
 * @brief The interrupt bit for BUTTON0.
 */
#define BUTTON0_INT_EN_BIT  DUMMY_GPIO_INT_EN_BIT /* No Buttons */
/**
 * @brief The interrupt bit for BUTTON0.
 */
#define BUTTON0_FLAG_BIT    DUMMY_GPIO_FLAG_BIT /* No Buttons */
/**
 * @brief The missed interrupt bit for BUTTON0.
 */
#define BUTTON0_MISS_BIT    DUMMY_GPIO_MISS_BIT /* No Buttons */

/**
 * @brief The actual GPIO BUTTON1 is connected to.  This define should
 * be used whenever referencing BUTTON1.
 *
 * Note: No buttons on the STM32W108 MB850 board 
 * Map the original button 1 gpio, irqs settings to the dummy configuration/settings
 */
#define BUTTON1             DUMMY_GPIO_PORT_PIN_2 /* No Buttons */
/**
 * @brief The GPIO input register for BUTTON1.
 */
#define BUTTON1_IN          DUMMY_GPIO_IN /* No Buttons */
/**
 * @brief Point the proper IRQ at the desired pin for BUTTON1.
 * @note For this board, BUTTON1 is the DUMMY GPIO (no selection operation)
 */
#define BUTTON1_SEL()       do { } while(0) /* No Buttons */
/**
 * @brief The interrupt service routine for BUTTON1.
 */
#define BUTTON1_ISR         DUMMY_GPIO_ISR_2 /* No Buttons */
/**
 * @brief The interrupt configuration register for BUTTON1.
 */
#define BUTTON1_INTCFG      DUMMY_GPIO_INTCFG_2 /* No Buttons */
/**
 * @brief The interrupt bit for BUTTON1.
 */
#define BUTTON1_INT_EN_BIT  DUMMY_GPIO_INT_EN_BIT_2 /* No Buttons */
/**
 * @brief The interrupt bit for BUTTON1. 
 */
#define BUTTON1_FLAG_BIT    DUMMY_GPIO_FLAG_BIT_2 /* No Buttons */
/**
 * @brief The missed interrupt bit for BUTTON1.
 */
#define BUTTON1_MISS_BIT    DUMMY_GPIO_MISS_BIT_2 /* No Buttons */
//@} //END OF BUTTON DEFINITIONS

/** @name Temperature sensor ADC channel
 *
 * Note: No temperature sensor on the STM32W108 MB850 board:
 * Application code (sensor) requires TEMP_SENSOR_ADC_CHANNEL defined 
 * (dataMode = DATA_MODE_BCD_TEMP as default)
 *
 *@{
 */
#define TEMP_SENSOR_ADC_CHANNEL ADC_SOURCE_ADC2_VREF2 /* (TBR ???) */

#define TEMP_SENSOR_SCALE_FACTOR 1 /* (TBR ???) */

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
 * trigger to enable Packet Trace support on the board (TBR).
 * 
 * Note: No Packet Trace through PTI when using STM32W108 MB850 board (TBR)
 */
//#define PACKET_TRACE  // We do have PACKET_TRACE support (TBR)
//@} //END OF PACKET TRACE DEFINITIONS

/** @name ENABLE_OSC32K
 *
 * When ENABLE_OSC32K is defined, halInit() will configure system
 * timekeeping to utilize the external 32.768 kHz crystal oscillator
 * rather than the internal 1 kHz RC oscillator.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable 32.768 kHz XTAL oscillator on the RCM plugged
 * into the board: No RCM into STM32W108 MB850 board.
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
 * the RCM plugged into the board: No RCM into STM32W108 MB850 board.
 * Default is to enable bias adjustment.
 */
//#define DISABLE_OSC24M_BIAS_TRIM  // Disable 24 MHz bias trim adjustment
//@} //END OF DISABLE OSC24M BIAS TRIM DEFINITIONS


/** @name ENABLE_ALT_FUNCTION_REG_EN
 *
 * When ENABLE_ALT_FUNCTION_REG_EN is defined, halInit() will enable the REG_EN
 * alternate functionality on PA7.  REG_EN is the special signal provided
 * by the STM32RF's internal power controller which can be used to tell an
 * external power regulator when the EM350 is in deep sleep or not and as such
 * signal the external regulator to cut power.  This signal will override all
 * GPIO configuration and use of PA7.  When the alternate functionality is
 * not enabled, PA7 can be operated as a standard GPIO: Button S1 on STM32RF
 * application board.
 */
//@{
/**
 * @brief This define does not equate to anything.  It is used as a
 * trigger to enable the REG_EN alternate function on PA7.
 * Default is to not enable REG_EN functionality on PA7.
 */
//#define ENABLE_ALT_FUNCTION_REG_EN
//@} //END OF ENABLE_ALT_FUNCTION_REG_EN DEFINITIONS


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


/** @name GPIO Configuration Defines
 *
 * The following defines give the values used in the GPIO configuration
 * registers a friendly name.
 *@{
 */

/* define the dummy gpio state for Power Up */
#define DUMMY_GPIO_STATE_UP GPIOCFG_OUT /*(TBR) */
/* define the dummy gpio output value for Power Up */
#define DUMMY_GPIO_VALUE_UP  1 /* 0 (TBR) */
/* define the dummy gpio state for Power Down */
#define DUMMY_GPIO_STATE_DOWN GPIOCFG_IN_PUD /* (TBR) */
/* define the dummy gpio output value for Power Down */
#define DUMMY_GPIO_VALUE_DOWN  1 /* 0 (TBR) */

//@} //END OF GPIO Configuration Defines


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


/** @name 32kHz Oscillator Configuration Defines
 *
 * Provide the proper set of pin configuration for when the 32kHz oscillator is
 * enabled (look above for the define which enables it).  When the 32kHz is
 * not enabled, configure the two pins for Button1 and TEMP_EN.
 *
 * When using the 32kHz, configure the two GPIO for analog for the XTAL.  When
 * not using the 32kHz, configure the two GPIO for Button1 and TEMP_EN.
 *@{
 */
/**
 * @brief Give the 32kHz oscillator configuration a friendly name.
 */
#ifdef  ENABLE_OSC32K                     
  #define CFG_BUTTON1  GPIOCFG_ANALOG
  #define CFG_TEMPEN   GPIOCFG_ANALOG
#else//!ENABLE_OSC32K
  #define CFG_BUTTON1  GPIOCFG_IN_PUD            (TBR)
  #define CFG_TEMPEN   GPIOCFG_OUT
#endif//ENABLE_OSC32K
//@} END OF 32kHz Oscillator Configuration Defines


/** @name GPIO Configuration Macros
 *
 * These macros define the GPIO configuration and initial state of the output
 * registers for all the GPIO in the powerup and powerdown modes.
 *
 * Note: The STM32W108 MB850 board  GPIOs not used are configured as 
 * DUMMY_GPIO_STATE
 *@{
 */
/**
 * @brief Powerup GPIO configuration.
 */
#define POWERUP_SET_GPIO_CFG_REGISTERS()                               \
  GPIO_PACFGL = (DUMMY_GPIO_STATE_UP   <<PA0_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA1_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA2_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA3_CFG_BIT);                 \
  GPIO_PACFGH = (DUMMY_GPIO_STATE_UP   <<PA4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PA7_CFG_BIT);                 \
  GPIO_PBCFGL = (DUMMY_GPIO_STATE_UP   <<PB0_CFG_BIT)|                 \
                (GPIOCFG_OUT_ALT       <<PB1_CFG_BIT)|  /* Uart TX */  \
                (GPIOCFG_IN            <<PB2_CFG_BIT)|  /* Uart RX */  \
                (DUMMY_GPIO_STATE_UP   <<PB3_CFG_BIT);                 \
  GPIO_PBCFGH = (DUMMY_GPIO_STATE_UP   <<PB4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PB5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PB6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PB7_CFG_BIT);                 \
  GPIO_PCCFGL = (DUMMY_GPIO_STATE_UP   <<PC0_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC1_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC2_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC3_CFG_BIT);                 \
  GPIO_PCCFGH = (DUMMY_GPIO_STATE_UP   <<PC4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_UP   <<PC7_CFG_BIT);


/**
 * @brief Powerup GPIO output register.
 */
#define POWERUP_SET_GPIO_OUTPUT_DATA_REGISTERS()                 \
  GPIO_PAOUT = (DUMMY_GPIO_VALUE_UP   <<PA0_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA1_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA2_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA3_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA4_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA5_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA6_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PA7_BIT);                \
  GPIO_PBOUT = (DUMMY_GPIO_VALUE_UP   <<PB0_BIT)|                \
               (0                     <<PB1_BIT)|  /* Uart TX */ \
               (0                     <<PB2_BIT)|  /* Uart RX */ \
               (DUMMY_GPIO_VALUE_UP   <<PB3_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PB4_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PB5_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PB6_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PB7_BIT);                \
  GPIO_PCOUT = (DUMMY_GPIO_VALUE_UP   <<PC0_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC1_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC2_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC3_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC4_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC5_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC6_BIT)|                \
               (DUMMY_GPIO_VALUE_UP   <<PC7_BIT);


/**
 * @brief Powerdown GPIO configuration.
 */
#define POWERDOWN_SET_GPIO_CFG_REGISTERS()                               \
  GPIO_PACFGL = (DUMMY_GPIO_STATE_DOWN   <<PA0_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA1_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA2_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA3_CFG_BIT);                 \
  GPIO_PACFGH = (DUMMY_GPIO_STATE_DOWN   <<PA4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PA7_CFG_BIT);                 \
  GPIO_PBCFGL = (DUMMY_GPIO_STATE_DOWN   <<PB0_CFG_BIT)|                 \
                (GPIOCFG_OUT_ALT         <<PB1_CFG_BIT)|  /* Uart TX */  \
                (GPIOCFG_IN              <<PB2_CFG_BIT)|  /* Uart RX */  \
                (DUMMY_GPIO_STATE_DOWN   <<PB3_CFG_BIT);                 \
  GPIO_PBCFGH = (DUMMY_GPIO_STATE_DOWN   <<PB4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PB5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PB6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PB7_CFG_BIT);                 \
  GPIO_PCCFGL = (DUMMY_GPIO_STATE_DOWN   <<PC0_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC1_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC2_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC3_CFG_BIT);                 \
  GPIO_PCCFGH = (DUMMY_GPIO_STATE_DOWN   <<PC4_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC5_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC6_CFG_BIT)|                 \
                (DUMMY_GPIO_STATE_DOWN   <<PC7_CFG_BIT);



/**
 * @brief Powerdown GPIO output register.
 */
#define POWERDOWN_SET_GPIO_OUTPUT_DATA_REGISTERS()                 \
  GPIO_PAOUT = (DUMMY_GPIO_VALUE_DOWN   <<PA0_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA1_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA2_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA3_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA4_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA5_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA6_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PA7_BIT);                \
  GPIO_PBOUT = (DUMMY_GPIO_VALUE_DOWN   <<PB0_BIT)|                \
               (0                       <<PB1_BIT)| /* Uart TX */  \
               (0                       <<PB2_BIT)| /* Uart RX */  \
               (DUMMY_GPIO_VALUE_DOWN   <<PB3_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PB4_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PB5_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PB6_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PB7_BIT);                \
  GPIO_PCOUT = (DUMMY_GPIO_VALUE_DOWN   <<PC0_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC1_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC2_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC3_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC4_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC5_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC6_BIT)|                \
               (DUMMY_GPIO_VALUE_DOWN   <<PC7_BIT);

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
#define WAKE_ON_PB2   FALSE
#define WAKE_ON_PB3   FALSE
#define WAKE_ON_PB4   FALSE
#define WAKE_ON_PB5   FALSE
#define WAKE_ON_PB6   FALSE
#define WAKE_ON_PB7   FALSE
#define WAKE_ON_PC0   FALSE
#define WAKE_ON_PC1   FALSE
#define WAKE_ON_PC2   FALSE
#define WAKE_ON_PC3   FALSE
#define WAKE_ON_PC4   FALSE
#define WAKE_ON_PC5   FALSE
#define WAKE_ON_PC6   FALSE  
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
#define halInternalInitBoard()                        \
        do {                                          \
          halInternalPowerUpBoard();                  \
          halInternalInitButton();                    \
       } while(0)

/**
 * @brief Power down the board.  This function is called from
 * ::halPowerDown().
 */
#define halInternalPowerDownBoard()                   \
        do {                                          \
          /* Board peripheral deactivation */         \
          /*halInternalSleepAdc();*/                  \
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

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
