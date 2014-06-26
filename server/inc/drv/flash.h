#ifndef __FLASH_H__
#define __FLASH_H__

/////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h" 
#include "config.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define FLASH_SYSINFO_ADDRESS      0x08040000   // our program is growing fat, sysinfo should move backward
#define FLASH_PAGE_SIZE            0x0800
#define FLASH_LOCATE_OFFSET        0x0800
#define FLASH_ROUTE_ADDRESS        0x08042000   // save beeper's route table, max. router size = 0x20 * 2 = 0x40
#define FLASH_ROUTE_SIZE           0x1000       // total size = 0x040 * 60(beepers) ~= 4KB

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define BEEPROUTESIZE              0x40
#define EMPTYROUTE                 0xffff

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_flashwritelong ( u16 offset, u32 data )
//* 功能        : write word to flash
//* 输入参数    : u16 offset, u32 data
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
FLASH_Status sea_flashwritelong ( u16 offset, u32 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_flashwriteshort ( u16 offset, u16 data )
//* 功能        : write word to flash
//* 输入参数    : u16 offset, u16 data
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
FLASH_Status sea_flashwriteshort ( u16 offset, u16 data );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : FLASH_Status sea_flashwrite ( u16 offset, void * data, u16 size )
//* 功能        : write data flash
//* 输入参数    : u16 offset, u16 * data, u16 size
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
FLASH_Status sea_flashwrite ( u16 offset, void * data, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashread ( u16 offset, u16 size )
//* 功能        : read data from flash
//* 输入参数    : u16 offset, u16 size
//* 输出参数    : void pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void * sea_flashread ( u16 offset, u16 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashlong ( u16 offset )
//* 功能        : read long from flash
//* 输入参数    : u16 offset, u16 size
//* 输出参数    : value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u32 sea_flashlong ( u16 offset );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashshort ( u16 offset )
//* 功能        : read short from flash
//* 输入参数    : u16 offset
//* 输出参数    : value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_flashshort ( u16 offset );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void * sea_flashreadroute ( u8 beep )
//* 功能        : read beeper's route table
//* 输入参数    : u8 beep
//* 输出参数    : pointer route table
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void * sea_flashreadroute ( u8 beep );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : FLASH_Status sea_flashwriteroute ( u8 beep, const void * table, u8 size )
//* 功能        : write beeper's route table
//* 输入参数    : u8 beep, const void * table, u8 size
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
FLASH_Status sea_flashwriteroute ( u8 beep, const void * table, u8 size );

/////////////////////////////////////////////////////////////////////////////////////////////
//
#endif

