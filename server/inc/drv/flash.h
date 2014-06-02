#ifndef __FLASH_H__
#define __FLASH_H__

/////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h" 
#include "config.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define FLASH_SYSINFO_ADDRESS      0x08040000   //our program is growing fat, sysinfo should move backward
#define FLASH_PAGE_SIZE            0x0800
#define FLASH_LOCATE_OFFSET        0x0800
#define FLASH_SHRIMP_TIMESTAGES_OFFSET    (2*FLASH_PAGE_SIZE)//2k for time stages starting from this flash offset

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwritelong ( u16 offset, u32 data )
//* ����        : write word to flash
//* �������    : u16 offset, u32 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
FLASH_Status sea_flashwritelong ( u16 offset, u32 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
//* ����        : write word to flash
//* �������    : u16 offset, u16 data
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
FLASH_Status sea_flashwriteshort ( u16 offset, u16 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : FLASH_Status sea_flashwrite ( u16 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u16 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
FLASH_Status sea_flashwrite ( u16 offset, void * data, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashread ( u16 offset, u16 size )
//* ����        : read data from flash
//* �������    : u16 offset, u16 size
//* �������    : void pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void * sea_flashread ( u16 offset, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashlong ( u16 offset )
//* ����        : read long from flash
//* �������    : u16 offset, u16 size
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u32 sea_flashlong ( u16 offset );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void * sea_flashshort ( u16 offset )
//* ����        : read short from flash
//* �������    : u16 offset
//* �������    : value
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_flashshort ( u16 offset );

/////////////////////////////////////////////////////////////////////////////////////////////
//
#endif

