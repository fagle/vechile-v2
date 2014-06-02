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
// 网关通信帧结构
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
//* 函数名      : void GWY_Configuration(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void GWY_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gatewayTickHandler ( void )
//* 功能        : coordinatior and router tick handler in gateway
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void gatewayTickHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sea_gwsendtestframe ( void )
//* 功能        : send a test frame with spi
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwsendtestframe ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwmsendframe ( u8 id, u8 len, u8 * value )
//* 功能        : send a frame with spi
//* 输入参数    : u8 id, u8 len, u8 * value
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwmsendframe ( u8 id, u8 len, u8 * value );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwroutehandler ( pgw_frame_t ptr )
//* 功能        : gateway route massage handler
//* 输入参数    : 无
//* 输出参数    : number of lamp
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwroutehandler ( pgw_frame_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwmcoordhandler ( pgw_frame_t ptr )
//* 功能        : gateway coordinate massage handler
//* 输入参数    : 无
//* 输出参数    : number of lamp
//* 修改记录    : 无
//* 备注        : 无
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
