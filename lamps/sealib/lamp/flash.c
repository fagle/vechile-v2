#include "network.h"
#include "common.h"
#include "flash.h"
#include "mem.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_flashwritelong ( u16 offset, u32 data )
//* 功能        : write word to flash
//* 输入参数    : u16 offset, u32 data
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_flashwritelong ( u16 offset, u32 data )
{  
    return sea_flashwrite(offset, (u16 *)&data, sizeof(u32));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
//* 功能        : write word to flash
//* 输入参数    : u16 offset, u16 data
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
{  
    return sea_flashwrite(offset, &data, sizeof(u16));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_flashwrite ( u16 offset, void * data, u16 size )
//* 功能        : write data flash
//* 输入参数    : u16 offset, u16 * data, u16 size
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
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
//* 函数名      : EmberStatus sea_tableflasherase ( u32 addr )
//* 功能        : erase data flash
//* 输入参数    : u32 addre
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
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
//* 函数名      : EmberStatus sea_tableflashwrite ( u32 offset, void * data, u16 size )
//* 功能        : write data flash
//* 输入参数    : u32 offset, u16 * data, u16 size
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
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
//* 函数名      : void sea_tableflashread ( u32 offset, u16 size, void * ptr )
//* 功能        : read data from flash
//* 输入参数    : u32 offset, u16 size, void * ptr
//* 输出参数    : void pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_tableflashread ( u32 addr, u16 size, void * ptr )
{
    sea_memcpy(ptr, (void *)addr, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_flashread ( u16 offset, u16 size, void * ptr )
//* 功能        : read data from flash
//* 输入参数    : u16 offset, u16 size, void * ptr
//* 输出参数    : void pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_flashread ( u16 offset, u16 size, void * ptr )
{
    sea_memcpy(ptr, (void *)(offset + STUSERADDR), size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashreadshort ( u16 offset )
//* 功能        : read short from flash
//* 输入参数    : u16 offset
//* 输出参数    : value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_flashreadshort ( u16 offset )
{
    return *((u16 *)(offset + STUSERADDR));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashreadlong ( u16 offset )
//* 功能        : read long from flash
//* 输入参数    : u16 offset, u16 size
//* 输出参数    : value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u32 sea_flashreadlong ( u16 offset )
{
    return *((u32 *)(offset + STUSERADDR));
}

/////////////////////////////////////////////////////////////////////////////////////////////
