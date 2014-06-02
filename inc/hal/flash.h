
#ifndef __FLASH_H__
#define __FLASH_H__

#include "memmap.h"

/** @brief Tells the calling code if a Flash Erase operation is active.
 *
 * This state is import to know because Flash Erasing is ATOMIC for 21ms
 * and could disrupt interrupt latency.  But if an ISR can know that it wasn't
 * serviced immediately due to Flash Erasing, then the ISR has the opportunity
 * to correct in whatever manner it needs to.
 * 
 * @return A boolean flag: TRUE if Flash Erase is active, FALSE otherwise.
 */
boolean halFlashEraseIsActive ( void );

#ifndef DOXYGEN_SHOULD_SKIP_THIS

//[[ The following eraseType definitions must match the FIB erase types! ]]
/**
 * @brief Assign numerical value to the type of erasure requested.
 */
#define MFB_MASS_ERASE 0x01
#define MFB_PAGE_ERASE 0x02
#define CIB_ERASE      0x03

/** @brief Erases a section of flash back to all 0xFFFF.
 * 
 * @param eraseType Choose one of the three types of erasures to perform.
 *  - MFB_MASS_ERASE (0x01) Erase the entire main flash block.
 *  - MFB_PAGE_ERASE (0x02) Erase one hardware page in the main flash block
 *  chosen by the \c address parameter.
 *  - CIB_ERASE      (0x03) Erase the entire customer information block.
 * 
 * @param address This parameter is only effectual when a MFB_PAGE_ERASE is
 * being performed.  The hardware page encapsulating the address given in this
 * parameter will be erased.  A hardware page size depends on the chip
 * 
 * @return An ::EmberStatus value indicating the success or failure of the
 * command:
 *  - EMBER_ERR_FATAL if the \c eraseType is not valid
 *  - EMBER_ERR_FLASH_ERASE_FAIL if erasing failed due to write protection
 *  - EMBER_ERR_FLASH_VERIFY_FAILED if erase verification failed
 *  - EMBER_SUCCESS if erasure completed and verified properly
 */
EmberStatus halInternalFlashErase ( int8u eraseType, int32u address );

/** @brief Writes a block of words to flash.  A page is erased
 * to 0xFFFF at every address.  Only two writes can be performed to the same
 * address between erasures and this is enforced by the flash interface
 * controller.  If the value already in the address being written to is 0xFFFF,
 * any value can be written.  If the value is not 0xFFFF and not 0x0000, only
 * 0x0000 can be written.  If the value is 0x0000, nothing can be written.
 *
 * \b NOTE: This function can NOT write the option bytes and will throw an
 * error if that is attempted.
 *
 * @param address The starting address of where the programming will occur.
 * This parameter MUST be half-word aligned since all programming operations
 * are half-words.  Also, the address parameter is NOT a pointer.  This
 * routines will cast the address to a pointer for the actual hardware access.
 *
 * @param data A pointer to a buffer containing the 16bit (half-words) to
 * be written.
 *
 * @param length The number of 16bit (half-words) contained in the data buffer
 * to be written to flash.
 *
 * @return An ::EmberStatus value indicating the success or failure of the
 * command:
 *  - EMBER_ERR_FLASH_PROG_FAIL if the address is not half-word aligned, the
 *    address is inside the option bytes, write protection is enabled, or the
 *    address is being written to more than twice between erasures.
 *  - EMBER_ERR_FLASH_VERIFY_FAILED if write verification failed
 *  - EMBER_SUCCESS if writing completed and verified properly
 */
EmberStatus halInternalFlashWrite ( int32u address, int16u * data, int32u length );

/** @brief Writes an option byte to the customer information block.  Only
 * two writes can be performed to the same address between erasures and this
 * is enforced by the flash interface controller.
 *
 * @param byte The option byte number, 0 though 7, to be programmed.
 *
 * @param data The 8 bit value to be programmed into the option byte.  The
 * hardware is responsible for calculating the compliment and programming
 * the full 16bit option byte space.
 *
 * @return An ::EmberStatus value indicating the success or failure of the
 * command:
 *  - EMBER_ERR_FLASH_PROG_FAIL if the byte chosen is greater than 7, write
 *    protection is enabled, or the byte is being written to more than twice
 *    between erasures.
 *  - EMBER_ERR_FLASH_VERIFY_FAILED if write verification failed
 *  - EMBER_SUCCESS if writing completed and verified properly
 */
EmberStatus halInternalCibOptionByteWrite ( int8u byte, int8u data );

/** @brief Configures the flash access controller for optimal current
 * consumption when FCLK is operating at 24MHz.  By providing this
 * function the calling code does not have to be aware of the
 * details of setting FLASH_ACCESS.
 */
void halInternalConfigXtal24MhzFlashAccess ( void );

///////////////////////////////////////////////////////////////////////////////////////////
//
//#define STUSERADDR      (0x0801D000)  // (0x0801F800)--MFB, the last page, save sys_info_t body
#define STUSERADDR      (0x0801C000)   // (0x0801D800), (0x0801F800)--MFB, the last page, save sys_info_t body
#define ENDUSERADDR     (0x08020000)   // (0x08000000-0x08020000)--MFB
#define PAGESIZE        (0x400)        //  MFB--(1024), FIB--(2048), CIB--(512)
#define TABLEOFFSET     (0x1000)       //  passed_t and device_t start address, offset of STUSERADDR (0x0801E000)
#define TABLESIZE       (0x1000)       //  passed_t and device_t table size 0x1000, 0x0801E000~0x0801F000
                                       //  (0x0801E000~0x08020000), for eeprom save data of zigbee, 2014/01/01
                                       //  (0x0801c000~0x080e0000), for user application save data, 2014/01/01

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwritelong ( u16 offset, u32 data )
//* ����        : write word to flash
//* �������    : u16 offset, u32 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwritelong ( u16 offset, u32 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwriteshort( u16 offset, u16 data )
//* ����        : write word to flash
//* �������    : u16 offset, u16 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwriteshort ( u16 offset, u16 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwrite ( u16 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u32 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwrite ( u16 offset, void * data, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_tableflashwrite ( u32 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u32 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_tableflashwrite ( u32 addr, void * data, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_flashread ( u16 offset, u16 size, void * ptr )
//* ����        : read data from flash
//* �������    : u16 offset, u16 size, void * ptr
//* �������    : void pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_flashread ( u16 offset, u16 size, void * ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_tableflashread ( u32 offset, u16 size, void * ptr )
//* ����        : read data from flash
//* �������    : u32 offset, u16 size, void * ptr
//* �������    : void pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_tableflashread ( u32 addr, u16 size, void * ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashreadshort ( u16 offset )
//* ����        : read short from flash
//* �������    : u16 offset
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_flashreadshort ( u16 offset );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_tableflasherase ( u32 addr )
//* ����        : erase data flash
//* �������    : u32 addre
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_tableflasherase ( u32 addr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashreadlong ( u16 offset )
//* ����        : read long from flash
//* �������    : u16 offset, u16 size
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u32 sea_flashreadlong ( u16 offset );

#endif //DOXYGEN_SHOULD_SKIP_THIS

///////////////////////////////////////////////////////////////////////////////////////////
#endif //__FLASH_H__


