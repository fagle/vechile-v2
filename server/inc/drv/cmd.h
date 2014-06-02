
#ifndef __CMD_H__
#define __CMD_H__

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "config.h"
#include "lamp.h"
#include "frame.h"

#define MSGBODYSIZE       (0x20)
#define MSGBUFSIZE        (0x20)
#define RTSENDSIZE        (0x0a)
#define GARAGESIZE        (0x0a)

#define beepidx(n)        ((n - MOBILEAID) >> 0x01)

/////////////////////////////////////////////////////////////////////////////////////////////////
// message body definition
typedef struct
{
    u8     id;             // note: use different message ids for different node.
    u8     size;           // message body size.
    u16    dest;           // destination short id
    u8     body[MSGBODYSIZE];
} msg_t, *pmsg_t;

////////////////////////////////////////////////////////////////////////////////
// message information definition
typedef struct 
{
    u8     maxcard;
    path_t route[MOBILEDEVS];
    call_t call[CALLERDEVS];
    u8     state[(MAXCARDS >> 0x03) + 0x01];
    u8     garage[GARAGESIZE];
    u8     act[(MOBILEDEVS >> 0x03) + 0x01];
    
    ppath_t   (*find)  ( u8 num );
    u8        (*add)   ( u8 num, u8 index, u8 size, u8 * ptr );
    void      (*clear) ( u8 num );
    void      (*print) ( u8 num );
    u8        (*action)( ppath_t ptr, u8 card );
} msg_info_t, *pmsg_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////////
// report message body definition
typedef struct
{
    u8 carid;
    u8 cardid;
    u8 type;    
    u8 status;
    u8 obstacle;
    u8 step;
    u8 count;     // task length
    u8 fail;
    u8 target;
    u8 endpoint;
    u16 speed;
    u16 magic;
} report_t, preport_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
// functions of lamp and coordiantor
#define set_cardbitmap(map, index)     (map[(index) >> 0x03] |= (0x01 << ((index) % 0x08)))
#define clr_cardbitmap(map, index)     (map[(index) >> 0x03] &= ~(0x01 << ((index) % 0x08)))
#define get_cardbitmap(map, index)     (map[(index) >> 0x03] & (0x01 << ((index) % 0x08)))

/////////////////////////////////////////////////////////////////////////////////////////////
//
/*******************************************************************************
* Function Name  : void sea_initmsg ( void )
* Description    : initialize msg_info structure body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_initmsg ( void );

/*******************************************************************************
* Function Name  : void sea_sendmsg ( pmsg_info_t ptr, u8 id, u8 size, u16 dest, void * msg )
* Description    : Inserts msg_t body to list.
* Input          : pmsg_info_t ptr, u8 id, u8 size, u16 dset, void * msg 
* Output         : None
* Return         : None
*******************************************************************************/
void sea_sendmsg ( pmsg_info_t ptr, u8 id, u8 size, u16 dest, void * msg );

/*******************************************************************************
* Function Name  : pmsg_t sea_getmsg ( pmsg_info_t ptr )
* Description    : get msg_t body from ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
pmsg_t sea_getmsg ( pmsg_info_t ptr );

/*******************************************************************************
* Function Name  : void sea_deletemsg ( pmsg_info_t ptr, u8 id, u8 num )
* Description    : clear msg_t body from ring.
* Input          : pmsg_info_t ptr, u8 id, u8 num
* Output         : None
* Return         : None
*******************************************************************************/
void sea_deletemsg ( pmsg_info_t ptr, u8 id, u8 num );

/*******************************************************************************
* Function Name  : void sea_deleteallmsg ( pmsg_info_t ptr, u8 num )
* Description    : clear msg_t body from ring.
* Input          : pmsg_info_t ptr, u8 num
* Output         : None
* Return         : None
*******************************************************************************/
void sea_deleteallmsg ( pmsg_info_t ptr, u8 num );

/*******************************************************************************
* Function Name  : void sea_clearmsg ( pmsg_info_t ptr )
* Description    : clear msg_t body from ring.
* Input          : pmsg_info_t ptr
* Output         : None
* Return         : None
*******************************************************************************/
void sea_clearmsg ( pmsg_info_t ptr );

/*******************************************************************************
* Function Name  : bool sea_isempty ( pmsg_info_t ptr  )
* Description    : is empty of list.
* Input          : pmsg_info_t ptr 
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_msgisempty ( pmsg_info_t ptr  );

/*******************************************************************************
* Function Name  : void sea_msgfinish ( pmsg_info_t ptr )
* Description    : get msg_t body from ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
void sea_msgfinish ( pmsg_info_t ptr );

/*******************************************************************************
* Function Name  : void sea_resetw108 ( void )
* Description    : w108 reboot
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_resetw108 ( void );

/*******************************************************************************
* Function Name  : ppath_t sea_findsend ( void )
* Description    : find send route table
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ppath_t sea_findsend ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_parsereport ( plamp_t ptr, u16 road )
//* ����        : print lamp_t information
//* �������    : plamp_t ptr, u16 road
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_parsereport ( plamp_t ptr, u16 road );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sea_printreport( plamp_t ptr )
//* ����        : print report information
//* �������    : plamp_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printreport ( plamp_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_sendroutetable ( ppath_t ptr )
//* ����        : send route table to w108
//* �������    : ppath_t ptr
//* �������    : true/flase
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8  sea_sendroutetable ( ppath_t ptr );   

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printaddress ( void )
//* ����        : print address information
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printaddress ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void printEUI64 ( EmberEUI64 * eui )
//* ����        : utility for printing EUI64 addresses 
//* �������    : EmberEUI64 * eui
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printEUI64 ( EmberEUI64 * eui );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u16 sea_lookuplogic ( u16 logic )
//* ����        : look up number from logic address
//* �������    : EmberEUI64 * eui
//* �������    : number
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_lookuplogic ( u16 logic );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8  sea_sendsinglecommand ( u8 id, u8 num )
//* ����        : send single command to coordinator
//* �������    : u8 id, u8 num
//* �������    : ture/false
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8  sea_sendsinglecommand ( u8 id, u8 num );

/*******************************************************************************
* Function Name  : u8 sea_ismsginlist ( pmsg_info_t ptr, u8 id, u8 num )
* Description    : is msg in list.
* Input          : pmsg_info_t ptr, u8 id, u8 num
* Output         : ture/false
* Return         : None
*******************************************************************************/
u8 sea_ismsginlist ( pmsg_info_t ptr, u8 id, u8 num );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u16 sea_lookuphysical ( EmberEUI64 * eui )
//* ����        : look up number from physical address
//* �������    : EmberEUI64 * eui
//* �������    : number
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_lookuphysical ( EmberEUI64 * eui );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8  isEUI64 ( EmberEUI64 * eui ) 
//* ����        : is EUI64 addresses 
//* �������    : EmberEUI64 * eui
//* �������    : true/false
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8  isEUI64 ( EmberEUI64 * eui );

/************************************************
* ������      : void ServerFrameCmdHandler ( frame_t fr )
* ����        : frame command handler from server
* �������    : frame_t fr
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/ 
void ServerFrameCmdHandler ( frame_t fr );

/************************************************
* ������      : void CoordFrameCmdHandler ( frame_t fr )
* ����        : frame command handler from coordinator
* �������    : frame_t fr
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/ 
void CoordFrameCmdHandler ( frame_t fr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bool isCallDevice ( u8 type, u8 num )
//* ����        : is caller device
//* �������    : u8 type, u8 num
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
bool isCallDevice ( u8 type, u8 num );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bool isCarDevice ( u8 type, u8 num )
//* ����        : is car device
//* �������    : u8 type, u8 num
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
bool isCarDevice ( u8 type, u8 num );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bool isCarActive ( u8 num )
//* ����        : is car active
//* �������    : u8 num
//* �������    : ture/false
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
bool isCarActive ( u8 num );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_getcartype ( u8 num )
//* ����        : get car type
//* �������    : u8 num
//* �������    : type
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 sea_getcartype ( u8 num );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_getcurrentcard ( u8 num )
//* ����        : get car current card
//* �������    : u8 num
//* �������    : card
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 sea_getcurrentcard ( u8 num );

/////////////////////////////////////////////////////////////////////////////////////
//

static u8 sea_findcard(ppath_t ptr, u8 card);

extern msg_info_t msg_info;

/////////////////////////////////////////////////////////////////////////////////////////
//
#endif  
