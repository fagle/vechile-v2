/** @file bootloader-interface-app.h
 * See @ref app_bootload for documentation.
 * 
 * <!-- Copyright 2007-2009 by Ember Corporation. All rights reserved.  *80* -->
 *
 */
 
/** @addtogroup app_bootload
 * @brief Defiunition of the application bootloader interface.
 * 
 * Some functions in this file return an ::EmberStatus value. See 
 * error-def.h for definitions of all ::EmberStatus return values.
 *
 * See bootloader-interface-app.h for source code.
 *@{
 */

#ifndef __BOOTLOADER_INTERFACE_APP_H__
#define __BOOTLOADER_INTERFACE_APP_H__


/** @brief This is the working unit of data for the app bootloader.  We want
 * it as big as possible, but it must be a factor of the NVM page size and
 * fit into a single Zigbee packet.  We choose  2^6 = 64 bytes.
 */
#define BOOTLOADER_SEGMENT_SIZE_LOG2  6 
/** @brief This is the working unit of data for the app bootloader.  We want
 * it as big as possible, but it must be a factor of the NVM page size and
 * fit into a single Zigbee packet.  We choose  2^6 = 64 bytes.
 */
#define BOOTLOADER_SEGMENT_SIZE       (1 << BOOTLOADER_SEGMENT_SIZE_LOG2)


/** @brief Call this function as part of your application initialization 
 *          to ensure the storage mechanism is ready to use
 */
void halAppBootloaderInit(void);

/** @brief Call this function when you are done accessing the storage mechanism
 *          to ensure that it is returned to its lowest power state.
 */
void halAppBootloaderShutdown(void);

/** @brief Call this function once before checking for a valid image to
 *  reset the call flag. 
 */
void halAppBootloaderImageIsValidReset(void);

/** @brief Define a numerical value for checking image validity when calling
 * the image interface functions.
 */
#define BL_IMAGE_IS_VALID_CONTINUE  ((int16u)0xFFFF)

/** @brief Reads the app image out of storage, calculates the total file
 *  CRC to verify the image is intact. 
 *
 *  Caller should loop calling this function
 *  while it returns ::BL_IMAGE_IS_VALID_CONTINUE to get final result. This
 *  allows caller to service system needs during validation.
 *  
 *  Call ::halAppBootloaderImageIsValidReset() before calling
 *  ::halAppBootloaderImageIsValid() to reset the call flag.
 *  
 *  Here is an example application call:
 *
 *  @code
 *  halAppBootloaderImageIsValidReset();
 *  while ( (pages = halAppBootloaderImageIsValid() ) == BL_IMAGE_IS_VALID_CONTINUE) {
 *    // make app specific calls here, if any
 *    emberTick();
 *  }
 *  @endcode
 * 
 *  @return One of the following:
 *     - Number of pages in a valid image
 *     - 0 for an invalid image
 *     - ::BL_IMAGE_IS_VALID_CONTINUE (-1) to continue to iterate for the final result.
 */ 
int16u halAppBootloaderImageIsValid(void);


/** @brief Invokes the bootloader to install the application in storage.
 *  This function resets the device to start the bootloader code and
 *  does not return!
 */
EmberStatus halAppBootloaderInstallNewImage(void);



/** @brief Writes data to the specified raw storage address and length without
 *         being restricted to any page size
 *         Note: not all storage implementations support accesses that are
 *               not page aligned, and ::EEPROM_ERR may be returned
 * 
 *  @param address  Address to start writing data 
 *
 *  @param data     A pointer to the buffer of data to write.
 *
 *  @param len      Length of the data to write
 *
 *  @return ::EEPROM_SUCCESS or ::EEPROM_ERR. 
 */
int8u halAppBootloaderWriteRawStorage(int32u address, 
                                      const int8u *data, 
                                      int16u len);

/** @brief Reads data from the specified raw storage address and length without
 *         being restricted to any page size
 *         Note: not all storage implementations support accesses that are
 *               not page aligned, and ::EEPROM_ERR may be returned
 * 
 *  @param address  Address from which to start reading data 
 *
 *  @param data     A pointer to a buffer where data should be read into
 *
 *  @param len      Length of the data to read
 *
 *  @return ::EEPROM_SUCCESS or ::EEPROM_ERR. 
 */
int8u halAppBootloaderReadRawStorage(int32u address, int8u *data, int16u len);


/** @brief Converts pageToBeRead to an address and the calls storage read function.
 * 
 *  @param pageToBeRead   pass in the page to be read. This will be converted to the 
 *  appropriate address. Pages are ::EEPROM_PAGE_SIZE long. 
 *
 *  @param destRamBuffer  a pointer to the buffer to write to.
 *
 *  @return ::EEPROM_SUCCESS or ::EEPROM_ERR. 
 */
int8u halAppBootloaderReadDownloadSpace(int16u pageToBeRead, 
                                        int8u* destRamBuffer);


/** @brief Converts pageToBeWritten to an address and calls the storage write
 *  function.
 *
 * @param pageToBeWritten  pass in the page to be written. This will be
 * converted to the appropriate address. Pages are ::EEPROM_PAGE_SIZE long.
 *
 * @param RamPtr           a pointer to the data to be written.
 * 
 * @return ::EEPROM_SUCCESS or ::EEPROM_ERR
 */
int8u halAppBootloaderWriteDownloadSpace(int16u pageToBeWritten,
                                         int8u* RamPtr);

/** @brief Read the application image data from storage
 * 
 *  @param timestamp  write the image timestamp to this data pointer.
 *
 *  @param imageInfo  write the user data field to this buffer.
 * 
 *  @return ::EEPROM_SUCCESS or ::EEPROM_ERR
 */
int8u halAppBootloaderGetImageData(int32u *timestamp, int8u *imageInfo);


/** @brief Returns the application bootloader version.
 */
int16u halAppBootloaderGetVersion(void);


/** @brief Returns the recovery image version.
 */
int16u halAppBootloaderGetRecoveryVersion(void);


#endif //__BOOTLOADER_INTERFACE_APP_H__

/** @}  END addtogroup */



