
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
//* ������      : void  init_tableinfo ( void )
//* ����        : initialize table body
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void  init_tableinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_isdeviceintable ( pdevice_t ptr )
//* ����        : find device in flash table
//* �������    : pdevice_t ptr
//* �������    : true/flase
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 sea_isdeviceintable ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : pdevice_t sea_findphydevice ( EmberEUI64 * eui, pdevice_t ptr )
//* ����        : find the device_t in flash table
//* �������    : EmberEUI64 * eui, pdevice_t ptr
//* �������    : pointer of device_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
pdevice_t sea_findphydevice ( EmberEUI64 * eui, pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr )
//* ����        : find the device_t in flash table
//* �������    : u16 num, pdevice_t ptr
//* �������    : pointer of device_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_savedevice ( pdevice_t ptr )
//* ����        : save new device_t to flash table
//* �������    : pdevice_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_savedevice ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_ispassedintable ( ppassed_t ptr )
//* ����        : is passed_t in flash table
//* �������    : ppassed_t ptr
//* �������    : true/flash
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 sea_ispassedintable ( ppassed_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ppassed_t sea_finpassed ( u8 card )
//* ����        : find the last passed_t in flash table
//* �������    : u8 card
//* �������    : pointer of passed_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
ppassed_t sea_finpassed ( u8 card );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_writedefaultconfig ( void )
//* ����        : write default system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_savepassed ( ppassed_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern table_t   table_info;

//////////////////////////////////////////////////////////////////////////////////////
#endif   // __TABLE_H__