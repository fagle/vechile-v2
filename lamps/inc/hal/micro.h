/** @file hal/micro/micro.h
 * 
 * @brief Functions common across all microcontroller-specific files.
 * See @ref micro for documentation.
 *
 * Some functions in this file return an ::EmberStatus value. 
 * See error-def.h for definitions of all ::EmberStatus return values.
 *
 * <!-- Copyright 2004 by Ember Corporation. All rights reserved.-->   
 */
 
/** @addtogroup micro
 * Many of the supplied example applications use these microcontroller functions.
 * See hal/micro/micro.h for source code.
 *
 * @note The term SFD refers to the Start Frame Delimiter.
 *@{
 */

#ifndef __MICRO_H__
#define __MICRO_H__

#include "em2xx-reset-defs.h"
#include "config.h"

#define	GPIO_EXT_PA_ON_OFF	PORTA_PIN(3)
#define	GPIO_EXT_PA_LNA_ENABLE	PORTA_PIN(6)
#define	GPIO_TX_RX_SWITCH	PORTC_PIN(5)
#define GPIO_ANT_SEL            PORTA_PIN(7)

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef EZSP_HOST
// If we are not a Host processor, Define distinct literal values for
// each platform and micro.  The Host processor is not tied to a
// specific platform or micro since it uses EZSP.  These values are
// used in ebl headers and the bootloader query response message.
// Some values may reserved for future use.

// PLAT 1 was the AVR ATMega, no longer supported
// for PLAT 1, MICRO 1 is the AVR ATMega 64
// for PLAT 1, MICRO 2 is the AVR ATMega 128
// for PLAT 1, MICRO 3 is the AVR ATMega 32

// PLAT 2 is the XAP2b
// for PLAT 2, MICRO 1 is the em250
// for PLAT 2, MICRO 2 is the em260

// PLAT 4 is the Cortex-M3
// for PLAT 4, MICRO 1 is the em350
// for PLAT 4, MICRO 2 is the em360
// for PLAT 4, MICRO 3 is the em357
// for PLAT 4, MICRO 4 is the em367
// for PLAT 4, MICRO 5 is the em351
// for PLAT 4, MICRO 6 is the em35x
// for PLAT 4, MICRO 7 is the stm32w108

#if defined( EMBER_TEST )
  #define PLAT 1
  #define MICRO 2
#elif defined (AVR_ATMEGA)
  #define PLAT 1
  #if defined (AVR_ATMEGA_64)
    #define MICRO 1
  #elif defined (AVR_ATMEGA_128)
    #define MICRO 2
  #elif defined (AVR_ATMEGA_32)
    #define MICRO 3
  #endif
#elif defined (XAP2B)
  #define PLAT 2
  #if defined (XAP2B_EM250)
    #define MICRO 1
  #elif defined (XAP2B_EM260)
    #define MICRO 2
  #endif
#elif defined (MSP430)
  #define PLAT 3
  #if defined (MSP430_1612)
    #define MICRO 1
  #elif defined (MSP430_1471)
    #define MICRO 2
  #endif
#elif defined (CORTEXM3)
  #define PLAT 4
  #if defined (CORTEXM3_EM350)
    #define MICRO 1
  #elif defined (CORTEXM3_EM360)
    #define MICRO 2
  #elif defined (CORTEXM3_EM357)
    #define MICRO 3
  #elif defined (CORTEXM3_EM367)
    #define MICRO 4
  #elif defined (CORTEXM3_EM351)
    #define MICRO 5
  #elif defined (CORTEXM3_EM35X)
    #define MICRO 6
  #elif defined (CORTEXM3_STM32W108)
    #define MICRO 7
  #endif
#else
  #error no platform defined, or unsupported
#endif

#ifndef MICRO
  #error no micro defined, or unsupported
#endif

#endif //EZSP_HOST

// Define distinct literal values for each phy.
// These values are used in the bootloader query response message.
// PHY 0 is a null phy
// PHY 1 is the em2420 (no longer supported)
// PHY 2 is the em250
// PHY 3 is the em3xx

#if defined (PHY_EM2420) || defined (PHY_EM2420B)
  #error EM2420 is no longer supported.
#elif defined (PHY_EM250) || defined (PHY_EM250B)
  #define PHY 2
#elif defined (PHY_EM3XX)
  #define PHY 3
#elif defined (PHY_NULL)
  #define PHY 0
#else
  #error no phy defined, or unsupported
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS

/** @brief Initializes microcontroller-specific peripherals.
*/
void HAL_Configuration ( void );   // halInit ( void )
void PAM_Configuration ( void );

/** @brief Restarts the microcontroller and therefore everything else.
*/
void halReboot ( void );

/** @brief Powers up microcontroller peripherals and board peripherals.
*/
void halPowerUp ( void );

/** @brief Powers down microcontroller peripherals and board peripherals.
*/
void halPowerDown ( void );

/** @brief Called from emberInit and provides a means for the HAL
 * to increment a boot counter, most commonly in non-volatile memory.
 *
 * This is useful while debugging to determine the number of resets that might
 * be seen over a period of time.  Exposing this functionality allows the
 * application to disable or alter processing of the boot counter if, for
 * example, the application is expecting a lot of resets that could wear
 * out non-volatile storage or some
 *
 * @stackusage Called from emberInit only as helpful debugging information.
 * This should be left enabled by default, but this function can also be
 * reduced to a simple return statement if boot counting is not desired.
 */
void halStackProcessBootCount ( void );

/** @brief Gets information about what caused the microcontroller to reset. 
 *
 * @return A code identifying the cause of the reset.
 */
int8u halGetResetInfo ( void );

/** @brief Calls ::halGetResetInfo() and supplies a string describing it.
 *
 * @appusage Useful for diagnostic printing of text just after program 
 * initialization.
 *
 * @return A pointer to a program space string.
 */
PGM_P halGetResetString ( void );

/** @brief Calls ::halGetExtendedResetInfo() and translates the EM35x reset 
 *  code to the corresponding value used by the EM2XX HAL. EM35x reset codes 
 * not present in the EM2XX are returned after being OR'ed with 0x80.
 *
 * @appusage Used by the EZSP host as a platform-independent NCP reset code.
 *
 * @return The EM2XX reset code, or a new EM3xx code if B7 is set.
 */
#ifdef CORTEXM3
  int8u halGetEm2xxResetInfo(void);
#else
  #define halGetEm2xxResetInfo() halGetResetInfo()
#endif

/** @brief The value that must be passed as the single parameter to 
 *  ::halInternalDisableWatchDog() in order to sucessfully disable the watchdog 
 *  timer.
 */ 
#define MICRO_DISABLE_WATCH_DOG_KEY 0xA5

/** @brief Enables the watchdog timer.
 */
void halInternalEnableWatchDog(void);

/** @brief Disables the watchdog timer.
 *
 * @note To prevent the watchdog from being disabled accidentally, 
 * a magic key must be provided.
 * 
 * @param magicKey  A value (::MICRO_DISABLE_WATCH_DOG_KEY) that enables the function.
 */
void halInternalDisableWatchDog(int8u magicKey);

/** @brief Determines whether the watchdog has been enabled or disabled.
 *
 * @return A boolean value indicating if the watchdog is current enabled.
 */
boolean halInternalWatchDogEnabled( void );


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Enumerations for the possible microcontroller sleep modes. 
 */
enum SleepModes
#else
typedef int8u SleepModes;
enum
#endif
{
   SLEEPMODE_IDLE,
   SLEEPMODE_RESERVED,
   SLEEPMODE_POWERDOWN,
   SLEEPMODE_POWERSAVE
};

/** @brief Puts the microcontroller to sleep in a specified mode.
 *
 * @note This routine always enables interrupts.
 *
 * @param sleepMode  A microcontroller sleep mode 
 * 
 * @sa ::SleepModes
 */
void halSleep(SleepModes sleepMode);

/** @brief Blocks the current thread of execution for the specified
 * amount of time, in microseconds.
 *
 * The function is implemented with cycle-counted busy loops
 * and is intended to create the short delays required when interfacing with
 * hardware peripherals.
 *
 * The accuracy of the timing provided by this function is not specified,
 * but a general rule is that when running off of a crystal oscillator it will
 * be within 10us.  If the micro is running off of another type of oscillator
 * (e.g. RC) the timing accuracy will potentially be much worse.
 *
 * @param us  The specified time, in microseconds. 
              Values should be between 1 and 65535 microseconds.
 */
void halCommonDelayMicroseconds(int16u us);

/** @brief Called implicitly through the standard C language assert() macro. 
 * An implementation where notification is, for instance, sent over the serial port
 * can provide meaningful and useful debugging information. 
 * @note Liberal usage of assert() consumes flash space.
 * 
 * @param filename   Name of the file throwing the assert.
 * 
 * @param linenumber Line number that threw the assert.
 */
void halInternalAssertFailed(PGM_P filename, int linenumber);


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "micro-m3.h"

// the number of ticks (as returned from halCommonGetInt32uMillisecondTick)
// that represent an actual second. This can vary on different platforms.
// It must be defined by the host system.
#ifndef MILLISECOND_TICKS_PER_SECOND
  #define MILLISECOND_TICKS_PER_SECOND 1024
// See bug 10232
//  #error "MILLISECOND_TICKS_PER_SECOND is not defined in micro.h!"
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS


#endif //__MICRO_H__

  

