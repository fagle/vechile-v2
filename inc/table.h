
#ifndef __TABLE_H__
#define __TABLE_H__

/////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x_type.h"
#include "ember-types.h"
#include "config.h"
#include "flash.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define ENDTABLE    (0xffff)   // end of table, flash not to be writen
#define TBDEVICE    (0x01)     // device_t body in flash
#define TBPASSED    (0x02)     // passed_t body in flash

/////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u16    index;
    u16    cnt;
    u16    size;
    void * (*first) ( void );
    void * (*next)  ( u8 type );
    void * (*last)  ( u8 type );
    u16    (*count) ( u8 type );
    EmberStatus  (*clear) ( void );
} table_t, *ptable_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void  init_tableinfo ( void )
//* 功能        : initialize table body
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  init_tableinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_isdeviceintable ( pdevice_t ptr )
//* 功能        : find device in flash table
//* 输入参数    : pdevice_t ptr
//* 输出参数    : true/flase
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_isdeviceintable ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pdevice_t sea_findphydevice ( EmberEUI64 * eui, pdevice_t ptr )
//* 功能        : find the device_t in flash table
//* 输入参数    : EmberEUI64 * eui, pdevice_t ptr
//* 输出参数    : pointer of device_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pdevice_t sea_findphydevice ( EmberEUI64 * eui, pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr )
//* 功能        : find the device_t in flash table
//* 输入参数    : u16 num, pdevice_t ptr
//* 输出参数    : pointer of device_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_savedevice ( pdevice_t ptr )
//* 功能        : save new device_t to flash table
//* 输入参数    : pdevice_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_savedevice ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_ispassedintable ( ppassed_t ptr )
//* 功能        : is passed_t in flash table
//* 输入参数    : ppassed_t ptr
//* 输出参数    : true/flash
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_ispassedintable ( ppassed_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ppassed_t sea_finpassed ( u8 card )
//* 功能        : find the last passed_t in flash table
//* 输入参数    : u8 card
//* 输出参数    : pointer of passed_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
ppassed_t sea_finpassed ( u8 card );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_writedefaultconfig ( void )
//* 功能        : write default system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_savepassed ( ppassed_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern table_t   table_info;

//////////////////////////////////////////////////////////////////////////////////////
#endif   // __TABLE_H__