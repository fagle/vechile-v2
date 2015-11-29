/** @file hal/micro/bootloader-eeprom.h
 * See @ref cbh_app for detailed documentation.
 * 
 * <!-- Copyright 2009 by Ember Corporation. All rights reserved.       *80* -->
 */

/** @addtogroup cbh_app
 * The file bootloader-eeprom.h defines generic EEPROM parameters. 
 *
 * Changing EEPROM size will change the size of the application image 
 * space without changing the size or relative location of the recovery and 
 * reserved sections. See eeprom.c for more information on modifying EEPROM
 * functionality.
 *
 * See bootloader-eeprom.h for source code.
 *@{
 */


/** @brief Definition of an EEPROM page size, in bytes.  The current
 * interface assumes all eeproms have 128 byte pages.  If a device has
 * a different physical page size, the driver needs to abstract it to
 * a 128 byte page.
 */
#define EEPROM_PAGE_SIZE      (128ul)

/** @brief Define the location of the first page in EEPROM.
 */
#define EEPROM_FIRST_PAGE   (0)

/** @brief Define the location of the image start in EEPROM as a function of
 * the ::EEPROM_FIRST_PAGE and ::EEPROM_PAGE_SIZE.
 */
#define EEPROM_IMAGE_START  (EEPROM_FIRST_PAGE*EEPROM_PAGE_SIZE)

/** @brief Define EEPROM success status.
 */
#define EEPROM_SUCCESS 0

/** @brief Define EEPROM error status.
 */
#define EEPROM_ERR 1

/** @brief Define EEPROM error mask.
 */
#define EEPROM_ERR_MASK 0x80

/** @brief Define EEPROM page boundary error.
 */
#define EEPROM_ERR_PG_BOUNDARY 0x81

/** @brief Define EEPROM page size error.
 */
#define EEPROM_ERR_PG_SZ 0x82

/** @brief Define EEPROM write data error.
 */
#define EEPROM_ERR_WRT_DATA 0x83

/** @brief Define EEPROM image too large error.
 */
#define EEPROM_ERR_IMG_SZ 0x84

/** @brief Define EEPROM invalid address error.
 */
#define EEPROM_ERR_ADDR 0x85


/** @name EEPROM interaction functions.
 *@{
 */

/** @brief Initialize EEPROM.
 */
void halEepromInit(void);

/** @brief Shutdown the EEPROM to conserve power
 */
void halEepromShutdown(void);

/** @brief Read from the external EEPROM. 
 *
 * This is the standard external EEPROM read function. The format of this
 * call must not be altered. However, the content can be changed to work with a
 * different device. 
 * To more easily work with a larger variety of external EEPROM/flash parts,
 * the app bootloader will always call this function with addresses that are
 * a multiple of 128 bytes.
 *
 * @param address  The address to start reading from.
 * @param data     A pointer to where read data is stored.
 * @param len      The length of data to read.
 * 
 * @return ::EEPROM_SUCCESS
 */
int8u halEepromRead(int32u address, int8u *data, int16u len);

/** @brief Write to the external EEPROM. 
 *
 * This is the standard external EEPROM write function. The format of this
 * call must not be altered. However, the content can be changed to work with a
 * different device. 
 * To more easily work with a larger variety of external EEPROM/flash parts,
 * the app bootloader will always call this function with addresses that are
 * a multiple of 128 bytes.
 *
 * @param address  The address to start writing to.
 * @param data     A pointer to the data to write.
 * @param len      The length of data to write.
 * 
 * @return EEPROM_SUCCESS
 */
int8u halEepromWrite(int32u address, const int8u *data, int16u len);
/**@} */

/** @} END addtogroup */
