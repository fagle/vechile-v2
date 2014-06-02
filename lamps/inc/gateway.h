#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include "config.h" 
#include "spi.h"
#include "lamp.h"
#include "sysinfo.h"

////////////////////////////////////////////////////////////////////////
//constants and globals
#define GWDATASIZE       ((MAXLAMPS >> 0x02) + 0x01)     // MAXACKSIZE * 0x02)   // (0x30)
#define GWTXLINKSIZE     ((MAXLAMPS >> 0x02) + 0x05)     // prefix cmd len rev.
#define GWRXLINKSIZE         10
#define GWMAXCNT         (0xff)
#define GWMAXLIST        (0x02)
#define SPECIALCHAR      (0xfe)
#ifndef RREFIX
#define PREFIX           (0x68)   // (0x68) '\r' = 0x0a, '@' = 0x40, '#' = 0x23, '$' = 0x24, '%' = 0x25
#endif
#define SENDLIST         (0x00)
#define RECVLIST         (0x01)





///////////////////////////////////////////////////////////////////////////////////////////////////
// the commands of the gateway
#define ICHP_GW_NETWORK             0x81      //++used for the sink to ask for server date        81
#define ICHP_GW_CHANNEL             0x82      //++return all the information to the server        82
#define ICHP_GW_ACKNOWLEDGE         0x83      //++return open/close status of lamps on road       83
#define ICHP_GW_TEST                0x84      //++update coordinator current date and time        84
#define ICHP_GW_OPEN_ALL            0x85      //++open all the light                              85
#define ICHP_GW_OPEN_SINGLE         0x86      //++                                                86
#define ICHP_GW_CLOSE_ALL           0x87      //++close all the light                             87
#define ICHP_GW_CLOSE_SINGLE        0x88      //++                                                88
#define ICHP_GW_OPENTIME            0x89      //++lamp open and hold time in automatic mode       89
#define ICHP_GW_CTRLMODE            0x8a      //++subcommand of server control mode               90

/////////////////////////////////////////////////////////////////////////
// ����ͨ��֡�ṹ
typedef struct 
{
    u8    count;
    u8    in, out;
    u8    body[GWTXLINKSIZE];
} gw_txlink_t, *pgw_txlink_t;

typedef struct 
{
    u8    in, out;
    u8    body[GWRXLINKSIZE];
} gw_rxlink_t, *pgw_rxlink_t;


////////////////////////////////////////////////////////////////////////////////
// gateway information list definition
typedef struct
{
  u8   state;
  u8   rate;
  channel_t channel;
} gw_info_t, *pgw_info_t;

////////////////////////////////////////////////////////////////////////////////
// gateway information list definition
typedef struct
{
  u8   cmd;
  u8   len;  
  u8   cnt;
  u8   rxenable;
  u8   body[GWDATASIZE];
} gw_frame_t, *pgw_frame_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void GWY_Configuration(void)
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void GWY_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void gatewayTickHandler ( void )
//* ����        : coordinatior and router tick handler in gateway
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void gatewayTickHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sea_gwsendtestframe ( void )
//* ����        : send a test frame with spi
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_gwsendtestframe ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_gwmsendframe ( u8 id, u8 len, u8 * value )
//* ����        : send a frame with spi
//* �������    : u8 id, u8 len, u8 * value
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_gwmsendframe ( u8 id, u8 len, u8 * value );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_gwroutehandler ( pgw_frame_t ptr )
//* ����        : gateway route massage handler
//* �������    : ��
//* �������    : number of lamp
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_gwroutehandler ( pgw_frame_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_gwmcoordhandler ( pgw_frame_t ptr )
//* ����        : gateway coordinate massage handler
//* �������    : ��
//* �������    : number of lamp
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_gwcoordhandler ( pgw_frame_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern gw_info_t     gw_info;
extern gw_frame_t    gw_frame;
extern u8            spiperiod;
extern u8            gw_print;
extern gw_txlink_t   gw_txlink;
extern gw_rxlink_t   gw_rxlink;

////////////////////////////////////////////////////////////////////////
#endif  //__GATEWAY_H__
