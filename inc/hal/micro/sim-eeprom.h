/** @file hal/micro/sim-eeprom.h
 * See @ref simeeprom for documentation.
 *
 * <!-- Copyright 2008 by Ember Corporation. All rights reserved.        *80*-->
 */
 
 /** @addtogroup simeeprom
 * @brief Simulated EEPROM access functions.
 *
 * The Simulated EEPROM system is designed to operate under the @ref tokens
 * API and provide a non-volatile storage system.  Since the flash write cycles
 * are finite, the Simulated EEPROM's primary purpose is to perform wear
 * leveling across several hardware flash pages, ultimately increasing the
 * number of times tokens may be written before a hardware failure.
 *
 * The parameters the Simulated EEPROM uses to carve out a section of
 * flash and operate within it include:
 * - ::REAL_PAGES_PER_VIRTUAL
 * - ::LEFT_BASE
 * - ::LEFT_TOP
 * - ::RIGHT_BASE
 * - ::RIGHT_TOP
 *
 * These parameters are derived from the location of the Simulated EEPROM
 * as defined in the platform specific sim-eeprom.h.
 *
 * ::ERASE_CRITICAL_THRESHOLD is the metric the freePtr is compared against.
 * The freePtr is a marker used internally by the Simulated EEPROM to track
 * where data ends and where available write space begins. If the freePtr
 * crosses this threhold, ::halSimEepromCallback() will be called with an
 * EmberStatus of ::EMBER_SIM_EEPROM_ERASE_PAGE_RED, indicating a critical
 * need for the application to call ::halSimEepromErasePage() which will erase a
 * hardware page and provide fresh storage for the Simulated EEPROM to write
 * token data.  If freePtr is less than the threshold, the callback will
 * have an EmberStatus of ::EMBER_SIM_EEPROM_ERASE_PAGE_GREEN indicating the
 * application should call ::halSimEepromErasePage() at its earliest convenience,
 * but doing so is not critically important at this time.
 *
 * Some functions in this file return an ::EmberStatus value. See 
 * error-def.h for definitions of all ::EmberStatus return values.
 *
 * See hal/micro/sim-eeprom.h for source code.
 *@{ 
 */


#ifndef __SIM_EEPROM_H__
#define __SIM_EEPROM_H__


//pull in the platform specific information here
#if defined(XAP2B)
  #include "xap2b/sim-eeprom.h"
#elif defined(CORTEXM3) || defined(EMBER_TEST)
  #include "cortexm3/sim-eeprom.h"
#else
  #error invalid sim-eeprom platform
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//Prototype for Startup which the Init and Repair macros point to
EmberStatus halInternalSimEeStartup(boolean forceRebuildAll);

//Properly round values when converting bytes to words
#define BYTES_TO_WORDS(x) (((x) + 1) / 2)

#endif


//application functions

/** @brief The Simulated EEPROM callback function, implemented by the
 * application.  
 *
 * @param status  An ::EmberStatus error code indicating one of the conditions
 * described below.
 *
 * The Simulated EEPROM needs to periodically perform a page erase 
 * operation to recover storage area for future token writes.  The page
 * erase operation requires an ATOMIC block of 21ms.  Since this is such a long
 * time to not be able to service any interrupts, the page erase operation is
 * under application control providing the application the opportunity to
 * decide when to perform the operation and complete any special handling
 * needed that might be needed.
 *
 * This callback will report an EmberStatus of
 * ::EMBER_SIM_EEPROM_ERASE_PAGE_GREEN whenever a token is set and a page needs
 * to be erased.  This <i>ERASE_PAGE_GREEN</i> means the amount of free storage
 * available has not passed the ::ERASE_CRITICAL_THRESHOLD and erasing is
 * desired but not critical at this time.
 *
 * This callback will report an EmberStatus of ::EMBER_SIM_EEPROM_ERASE_PAGE_RED
 * whenever a token is set and a page must be erased.  This <i>ERASE_PAGE_RED</i>
 * means the amount of free storage available has passed the
 * ::ERASE_CRITICAL_THRESHOLD and erasing is critical to prevent possible data
 * loss occuring on future writes.
 *
 * If the application does not call ::halSimEepromErasePage() often enough, it is
 * possible for the Simulated EEPROM to run out of available space to write new
 * data.  When this happens, this callback will report an EmberStatus of
 * ::EMBER_SIM_EEPROM_FULL indicating that the Simulated EEPROM has run out of
 * room, the data currently being set has been lost, and any future write
 * attempts will be lost as well.  If this Status is ever returned, the
 * application <b><i>must</i></b> call ::halSimEepromErasePage() several times 
 * to guarantee there is sufficient room to write new data.
 *
 * This callback will report an EmberStatus of ::EMBER_SIM_EEPROM_REPAIRING
 * when the Simulated EEPROM needs to repair itself.  While there's nothing
 * for an app to do when the SimEE is going to repair itself (SimEE has to
 * be fully functional for the rest of the system to work), alert the
 * application to the fact that repairing is occuring.  There are debugging
 * scenarios where an app might want to know that repairing is happening;
 * such as monitoring frequency.
 * @note  Common situations will trigger an expected repair, such as using
 *        a new chip or changing token definitions.
 *
 * If the callback ever reports the status ::EMBER_ERR_FLASH_WRITE_INHIBITED or
 * ::EMBER_ERR_FLASH_VERIFY_FAILED, this indicates a catastrophic failure in
 * flash writing, meaning either the address being written is not empty or the
 * write itself has failed.  If ::EMBER_ERR_FLASH_WRITE_INHIBITED is 
 * encountered, the function ::halInternalSimEeRepair(FALSE) should be called 
 * and the chip should then be reset to allow proper initialization to recover.
 * If ::EMBER_ERR_FLASH_VERIFY_FAILED is encountered the Simulated EEPROM (and
 * tokens) on the specific chip with this error should not be trusted anymore.
 *
 */
void halSimEepromCallback(EmberStatus status);

void halSimEeprom2Callback(EmberStatus status);

/** @brief Erases a hardware flash page, if needed.  
 * 
 * This function can be 
 * called at anytime from anywhere in the application and will only take effect
 * if needed (otherwise it will return immediately).  Since this function takes
 * 21ms to erase a hardware page during which interrupts cannot be serviced,
 * it is preferable to call this function while in a state that can withstand
 * being unresponsive for so long.  The Simulated EEPROM will periodically
 * request through the ::halSimEepromCallback() that a page be erased.  The
 * Simulated EEPROM will never erase a page (which could result in data loss)
 * and relies entirely on the application to call this function to approve
 * a page erase (only one erase per call to this function).
 * 
 * The Simulated EEPROM depends on the ability to move between two Virtual
 * Pages, which are comprised of multiple hardware pages.  Before moving to the
 * unused Virtual Page, all hardware pages comprising the unused Virtual Page
 * must be erased first.  The erase time of a hardware flash page is 21ms.
 * During this time the chip will be unresponsive and unable to service an
 * interrupt or execute any code (due to the flash being unavailable during
 * the erase procedure).  This function is used to trigger a page erase.
 */
void halSimEepromErasePage(void);

/** @brief Erases a hardware flash page, if needed.
 * 
 * The difference between ::halSimEeprom2ErasePage and ::halSimEepromErasePage
 * is that this function is designed for use with SimEE2 and returns a byte.
 *
 * @return  A count of how many virtual pages are left to be erased.  This
 * return value allows for calling code to easily loop over this function
 * until the function returns 0.
 */
int8u halSimEeprom2ErasePage(void);

/** @brief Provides two basic statistics.
 *  - The number of unused words in the current page
 *  - The total page use count
 * 
 * There is a lot
 * of management and state processing involved with the Simulated EEPROM,
 * and most of it has no practical purpose in the application.  These two
 * parameters provide a simple metric for knowing how soon the Simulated
 * EEPROM will need to switch to the other Virtual Page (::freeWordsInCurrPage)
 * and how many times (approximatly) the switch has occured (::totalPageUseCount).
 *
 * @param freeWordsInCurrPage  A 16 bit number equal to the difference
 * between the top of the Virtual Page and the freePtr; the unused words.
 *
 * @param totalPageUseCount  The value of the highest page counter between
 * the two Virtual Pages indicating how many times the Simulated EEPROM
 * has switched between the pages (and approximate write cycles).
 */
void halSimEepromStatus(int16u *freeWordsInCurrPage, int16u *totalPageUseCount);


//internal functions
/** @brief  Initializes the Simulated EEPROM system.  This function must
 * be called before any token or SimEE access.  This function scans the current
 * active Virtual Page, verifies the Page Management, compares the stored token
 * definitions versus the compile time definitions for differences, and walks
 * the Virtual Page (starting at the Base Tokens) building up the RAM Cache.
 * If any discrepancies or errors are found in the management information, this
 * function will automatically trigger halInternalSimEeRepair() which will
 * fix the errors while recovering as much data as possible.
 *
 * Since the SimEE is dependant on the RAM Cache for operation, this function
 * must be called first.  Since this function builds up the RAM Cache and the
 * RAM Cache is always kept up to date, it is not harmful to call this
 * function multiple times.
 *
 * @return   An ::EMBER_SUCCESS if the initialization succeeds, and
 * ::EMBER_ERR_FATAL if a write to flash is both required and fails.
 * halSimEepromCallback() is automatically called if a write fails.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberStatus halInternalSimEeInit(void);
#else
#define halInternalSimEeInit() (halInternalSimEeStartup(FALSE))
#endif

/** @brief Gets the token data.  
 *
 * The Simulated EEPROM uses a RAM Cache
 * to hold the current state of the Simulated EEPROM, including the location
 * of the freshest token data.  The GetData function simply uses the ID and
 * index parameters to access the pointer RAM Cache.  The flash absolute
 * address stored in the pointer RAM Cache is extracted and passed
 * along with the data and length parameter to the flash read function.
 *
 * @param data  A pointer to where the data being read should be placed.
 *
 * @param ID    The ID of the token to get data from.  Since the token system
 * is designed to enumerate the token names down to a simple 8 bit ID,
 * generally the TOKEN_name is the parameter used when invoking this function.
 *
 * @param index If token being accessed is indexed, this parameter is
 * combined with the ID to formulate both the RAM Cache lookup as well
 * as the desired InfoWord to look for.  If the token is not an indexed token,
 * this parameter is ignored.
 *
 * @param len   The number of bytes being worked on.  This should always be
 * the size of the token (available from both the sizeof() intrinsic as well
 * as the tokenSize[] array).
 *
 */
void halInternalSimEeGetData(void *data, int8u ID, int8u index, int8u len);

/** @brief Sets token data.  
 * 
 * Like GetData, the passed parameters are
 * used to access into the RAM Cache as well as build the InfoWord.  Once the
 * basic token and SimEE parameters involved are stored locally, SetData
 * triggers the halSimEepromCallback indicating it would like to erase a page
 * (if it needs one erase) and if the erasure if critical (RED) or
 * routine (GREEN).
 *
 * If there is enough room in the current Virtual Page, SetData will simply
 * place down the new token data (and proper InfoWord) at the freePtr and move
 * the freePtr.
 *
 * If there is not enough room in the current Virtual Page and the other page
 * is not empty, a fatal callback will be triggered and the data will be lost
 * (halSimEepromCallback will be sent the status ::EMBER_SIM_EEPROM_FULL).
 *
 * If there is not enough room in the current Virtual Page and the other page
 * is empty, GetData will copy the freshest token data over to the Base Token
 * Storage, implanting the new token data as it goes.  It will then place the
 * Stored Token Definitions in the new page.  And finally it will write the
 * Page Management with a higher counter value activating this new page.
 *
 * @param ID    The ID of the token to set data for.  Since the token system
 * is designed to enumerate the token names down to a simple 8 bit ID,
 * generally the TOKEN_name is the parameter used when invoking this function.
 *
 * @param data  A pointer to the data that should be written.
 *
 * @param index If token being accessed is indexed, this parameter is
 * combined with the ID to formulate both the RAM Cache lookup as well
 * as the desired InfoWord to look for.  If the token is not an indexed token,
 * this parameter is ignored.
 *
 * @param len   The number of bytes being worked on.  This should always be
 * the size of the token (available from both the sizeof() intrinsic as well
 * as the tokenSize[] array).
 *
 */
void halInternalSimEeSetData(int8u ID, void *data, int8u index, int8u len);

/** @brief Increments the value of a token that is a counter.
 *
 * This is more efficient than just setting the token because the
 * SimEE stores just a +1 mark which takes up less space and improves
 * write cycles.
 *
 * See platform's token.h for the padding size which is the number of +1 marks
 * that can be stored for counter tokens.  When this padding is full of +1's,
 * the next increment call will trigger a full SetData that moves the total
 * value to a new location and begins again.  Because the full SetData
 * function is triggered, this will handle switching pages as well.
 *
 * When the SimEE switches to a new page, it will automatically truncate the
 * +1 marks into a new number and write that number into the new page.
 *
 * @note This function will only work on tokens that have the IsCnt flag
 * in their TOKEN_DEF set and only on tokens that are simple scalars (such as
 * int8u, int16u, or int32u).
 *
 * @param ID  The ID of the token to increment.  Since the token system
 * is designed to enumerate the token names down to a simple 8 bit ID,
 * generally the TOKEN_name is the parameter used when invoking this function.
 *
 */
void halInternalSimEeIncrementCounter(int8u ID);

/** @brief Repairs the Simulated EEPROM.  
 * 
 * This function is automatically
 * triggered if there are any problems found.  This function can also be
 * triggered manually by test functions to forcefully rebuild the Simulated
 * EEPROM.
 *
 * The page counters are examined to find the oldest and the newest Virtual
 * Page and the oldest page is erased.  For every token found in the compiled
 * definitions, this function walks through the newest Virtual Page looking for
 * that token and the newest data associated with that token.  If the token is
 * found intact, it is copied over to the new page.  If the token is not found,
 * or does not match the compiled definition of the token, the compiled token
 * is written, with its default values, to the new page.  Any tokens that
 * exist in the old Virtual Page and do not exist in the compiled definitions
 * are simply ignored.  Once all tokens have had their management information,
 * stored definitions, and data writen to the new page, the Page Management
 * information is written.  The next time halInternalSimEeInit is called, the
 * new Virtual Page will be used.
 *
 * @param forceRebuildAll  If TRUE, this repair function will erase the entire
 * Simulated EEPROM and reload it all from defaults.  If FALSE, the repair
 * function will operate normally.
 *
 * @return   An ::EMBER_SUCCESS if the repair succeeds, and
 * ::EMBER_ERR_FATAL if a write to flash fails.  halSimEepromCallback() is
 * automatically called if a write fails.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberStatus halInternalSimEeRepair(boolean forceRebuildAll);
#else
#define halInternalSimEeRepair(rebuild) (halInternalSimEeStartup(rebuild))
#endif

#endif //__SIM_EEPROM_H__

/**@} // END simeeprom group
 */
  
