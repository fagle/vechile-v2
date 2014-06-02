/** @file hal/micro/cortexm3/bootloader/bootloader-serial.h
 * See @ref cbhc_serial for detailed documentation.
 * 
 * <!-- Copyright 2009 by Ember Corporation. All rights reserved.       *80* -->
 */
 
//[[ Author(s): David Iacobone, diacobone@ember.com
//              Lee Taylor, lee@ember.com
//]]

/** @addtogroup cbhc_serial
 * @brief EM35x common bootloader serial definitions.
 *
 * See bootloader-serial.h for source code.
 *@{
 */

#ifndef __BOOTLOADER_SERIAL_H__
#define __BOOTLOADER_SERIAL_H__

/** @brief Initialize serial port.
 */
void serInit(void);

/** @brief Flush the transmiter.
 */
void serPutFlush(void);

/** @brief Transmit a character.
 * @param ch A character.
 */
void serPutChar(int8u ch);

/** @brief  Transmit a string.
 * @param str A string.
 */
void serPutStr(const char *str);

/** @brief Transmit a buffer.
 * @param buf A buffer.
 * @param size Length of buffer.
 */
void serPutBuf(const int8u buf[], int8u size);

/** @brief Transmit a byte as hex.
 * @param byte A byte.
 */
void serPutHex(int8u byte);

/** @brief  Transmit a 16bit integer as hex.
 * @param word A 16bit integer.
 */
void serPutHexInt(int16u word);

/** @brief Determine if a character is available.
 * @return ::TRUE if a character is available, ::FALSE otherwise.
 */
boolean serCharAvailable(void);

/** @brief Get a character if available, otherwise return an error.
 * @param ch Pointer to a location where the received byte will be placed.
 * @return ::BL_SUCCESS if a character was obtained, ::BL_ERR otherwise.
 */
int8u   serGetChar(int8u* ch);

/** @brief Flush the receiver.
 */
void    serGetFlush(void);

/** @brief  Wait until a character is available or timeout exceeded.
 * @param ch Pointer to a location where the received byte will be placed.
 * @param timeout Time to wait for a character until timed out.  One of the
 * TIMEOUT_* macros.
 * @return ::BL_SUCCESS if a character was obtained, ::BL_ERR otherwise.
 */
int8u   serWaitChar(int8u* ch, int16u timeout);

// Timer 1 set to 12Mhz / 2^10 prescale == 11.72khz == 85.33uSec ticks
/** @brief 5 second timeout define for serWaitChar.
 */
#define TIMEOUT_5SEC            65534  // 5.5sec
/** @brief 3 second timeout define for serWaitChar.
 */
#define TIMEOUT_3SEC            35156  // 3sec / 85.33uSec
/** @brief 1 second timeout define for serWaitChar.
 */
#define TIMEOUT_1SEC            11719  // 1sec / 85.33uSec
/** @brief Infinite timeout define for serWaitChar.
 */
#define TIMEOUT_FOREVER         0
/** @brief 1 byte time with a baud rate of 19200 timeout define for serWaitChar.
 */
#define TIMEOUT_19200BYTE       2400   // 8 bits = 416.6uSec

#endif //__BOOTLOADER_SERIAL_H__

/**@} end of group*/

