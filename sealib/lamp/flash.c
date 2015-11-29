#include "network.h"
#include "common.h"
#include "flash.h"
#include "mem.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwritelong ( u16 offset, u32 data )
//* ����        : write word to flash
//* �������    : u16 offset, u32 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwritelong ( u16 offset, u32 data )
{  
    return sea_flashwrite(offset, (u16 *)&data, sizeof(u32));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
//* ����        : write word to flash
//* �������    : u16 offset, u16 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
{  
    return sea_flashwrite(offset, &data, sizeof(u16));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwrite ( u16 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u16 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_flashwrite ( u16 offset, void * data, u16 size )
{
#if 0
    if (((offset + STUSERADDR) % PAGESIZE) == 0x00)
    {
        if (halInternalFlashErase(MFB_PAGE_ERASE, offset + STUSERADDR) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;  
    }
    if (halInternalFlashWrite(offset + STUSERADDR, data, size) != EMBER_SUCCESS)
        return EMBER_ERR_FLASH_PROG_FAIL;  
#else
    u16 * ptr = (u16 *)data;
    
    if (size & 0x01)  
        size ++;
    while (size)
    {
        if (((offset + STUSERADDR) % PAGESIZE) == 0x00)
        {
            if (halInternalFlashErase(MFB_PAGE_ERASE, offset + STUSERADDR) != EMBER_SUCCESS)
                return EMBER_ERR_FLASH_PROG_FAIL;  
        }
        if (halInternalFlashWrite(offset + STUSERADDR, ptr, 0x01) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;  
        offset += 0x02;
        size   -= 0x02;
        ptr ++;
    }
#endif
    return EMBER_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_tableflasherase ( u32 addr )
//* ����        : erase data flash
//* �������    : u32 addre
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_tableflasherase ( u32 addr )
{
    if ((addr % PAGESIZE) == 0x00)
    {
        if (halInternalFlashErase(MFB_PAGE_ERASE, addr) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;  
    }
    return EMBER_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_tableflashwrite ( u32 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u32 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
EmberStatus sea_tableflashwrite ( u32 addr, void * data, u16 size )
{
    u16 * ptr = (u16 *)data;
    
    if (size & 0x01)  
        size ++;
    while (size)
    {
        if (sea_tableflasherase(addr) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;  
        if (halInternalFlashWrite(addr, ptr, 0x01) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;  
        addr += 0x02;
        size -= 0x02;
        ptr ++;
    }
    return EMBER_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_tableflashread ( u32 offset, u16 size, void * ptr )
//* ����        : read data from flash
//* �������    : u32 offset, u16 size, void * ptr
//* �������    : void pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_tableflashread ( u32 addr, u16 size, void * ptr )
{
    sea_memcpy(ptr, (void *)addr, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_flashread ( u16 offset, u16 size, void * ptr )
//* ����        : read data from flash
//* �������    : u16 offset, u16 size, void * ptr
//* �������    : void pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_flashread ( u16 offset, u16 size, void * ptr )
{
    sea_memcpy(ptr, (void *)(offset + STUSERADDR), size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashreadshort ( u16 offset )
//* ����        : read short from flash
//* �������    : u16 offset
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_flashreadshort ( u16 offset )
{
    return *((u16 *)(offset + STUSERADDR));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashreadlong ( u16 offset )
//* ����        : read long from flash
//* �������    : u16 offset, u16 size
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u32 sea_flashreadlong ( u16 offset )
{
    return *((u32 *)(offset + STUSERADDR));
}

/////////////////////////////////////////////////////////////////////////////////////////////
