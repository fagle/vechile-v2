#ifndef __NETWORK_H__
#define __NETWORK_H__

//////////////////////////////////////////////////////////////////////////////////
// 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "sysinfo.h"
#include "common.h"
#include "flash.h"
#include "table.h"
#include "command.h"
#include "sink.h"
#include "lamp.h"
#include "sensor.h"
#include "frame.h"
#include "micro-m3.h"
#include "serial.h"
#include "console.h"
#include "ticker.h"
#include "mobile.h"
#include "pwm.h"
#include "mem.h"
#include "spi.h"
#include "msg.h"
#ifdef ENABLE_VEHICLE
#include "vehicle.h"
#endif
#include "flash.h"
#ifdef ENABLE_GATEWAY
#include "gateway.h"
#endif
#include "callback.h"
#include "security.h"
#include "form-and-join-adapter.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// zigbee types of device, modified 2012/06/22, update 2013/11/18
#define COORDID                (0x00)     // coordinator's id, it must be different with other devices.  
#define ENDEVID                (0x01)     // please modify node id here, and use different node ids for different node.
#define ROUTERID               (0x02)

/////////////////////////////////////////////////////////////////////////////////////////////
// control device types, updated 2013/12/14
//#define CENTERID               (0x01)     // control center must be coordinator
//#define LAMPID                 (0x02)     // lamp id make as router
//#define CALLAID                (0x03)     // for green, a call devices, added 3 devices for green's agv
//#define CALLBID                (0x04)     // another green's call devices, commented by renyu 2013/11/03
//#define MOBILEAID              (0x05)     // install on car loader, mobile zigbee devices
//#define MOBILEBID              (0x06)     // install on car loader, mobile zigbee devices
//#define MOBILECID              (0x07)     // install on car loader, mobile zigbee devices
//#define MOBILEDID              (0x08)     // install on car loader, mobile zigbee devices
//#define VEHICLEID              (0x09)     // install on car loader, vehicle zigbee devices
//#define CONDITIONID            (0x0a)     // air condition control devices
//#define GATEWAYID              (0x0b)     // gateway devices
//#define OTHERID                (0x0c)

/////////////////////////////////////////////////////////////////////////////////////////////
// control device types, modified by renyu 2014/06/02
#define CENTERIDST             (0x01)     // control center must be coordinator
#define CENTERIDCNT            (0x0f)     

#define CALLIDST               (0x10)
#define CALLIDCNT              (0x10)
#define CARIDST                (0x20)
#define CARIDCNT               (0x20)
#define MOVEIDST               (0x50)
#define MOVEIDCNT              (0x10)

#define LAMPIDST               (0x10)
#define LAMPIDCNT              (0x10)
#define GWIDST                 (0x20)
#define GWIDCNT                (0x10)

#define AIRIDST                (0x10)
#define AIRIDCNT               (0x20)

/////////////////////////////////////////////////////////////////////////////////////////////
// application timers are based on quarter second intervals, each
// quarter second is measured in millisecond ticks. This value defines
// the approximate number of millisecond ticks in a quarter second.
// Account for variations in system timers.
#ifdef AVR_ATMEGA_32
#define TICKS_PER_QUARTER_SECOND 225
#else
#define TICKS_PER_QUARTER_SECOND 250
#define TICKS_PER_SECOND         1000
#endif
#define JOINTIMEOUT              240             // 120s = 2 minute ，该值小于u8      

// num of pkts to miss before deciding that other node is gone
#define NWPK_TOLERANCE           (0x02)        // 0x03 too small, the sensor network will be rebooted. 
#define strHello                 "hello\0"
#define strSensor                "sensor-test\0"
#define strSink                  "sink-test\0"

#define TINTERVAL                (0x100000) // 0x1 ~ 0xffff ffff, and 0xfffff is about 3 second
#define EINTERVAL                (0x50000)  // 0x50000 is about 1 second 
#define DELAYTIME                (0x3ff)    // delay times

#define EMBER_NOERROR            (0x00)
#define ADDRESS_TABLE_INDEX      (0x00)
#define SENDRETRYS               (0x03)
#define SENDDEFCHAR              (0x07)
#define MAXGWSIZE                (0x10)
#define ACTSIZE                  (0x10)

#define SENDOK                   (0x00)
#define SENDERROR                (0xff)

////////////////////////////////////////////////////////////////////////
// these are errors or states of build network and send message
#define ESENDCMD                 (0x0001)
#define EALLCLOSE                (0x0002)
#define ESINGLEOPEN              (0x0004)
#define ESINGLECLOSE             (0x0008)
#define EPOWERON                 (0x0010)
#define EMULTIHEL                (0x0020)
#define ERESET                   (0x0040)
#define ELAMPREPORT              (0x0080)
#define ELAMPREPORTOK            (0x0100)
#define EWAITLAMPREPORT          (0x0200)

////////////////////////////////////////////////////////////////////////
// these are defines used for the variable networkFormMethod. This variable affects the serial output when a network is formed. 
#define SINK_FORM_NEW_NETWORK    (0x01)
#define SINK_USE_NETWORK_INIT    (0x02)
#define SINK_USE_SCAN_UTILS      (0x03)

///////////////////////////////////////////////////////////////////////////////////////////////////
// the command of the GPRS module received from the remote web server, modified 2012/06/22, update on 2013/11/09
#define ICHP_SV_JOIN                0x13      //++ zigbee new node jion, protocol command, added 2013/12/18
//#define ICHP_SV_REVER              0x50      //++reset all devices                               80
#define ICHP_SV_REBOOT              0x51      //++reset all devices                               81
#define ICHP_SV_RPT                 0x52      //++return open/close status of lamps on road       83
#define ICHP_SV_CLOSE               0x53      //++close all the light                             87
#define ICHP_SV_OPEN                0x54      //++open all the light                              85
#define ICHP_SV_DATE                0x55      //++update coordinator current date and time        84
#define ICHP_SV_CHANNEL             0x56      //++get channel_t body                              89
#define ICHP_SV_CTRL                0x57      //++subcommand of server control mode               90
#define ICHP_SV_SECKEY              0x58      //++get key_t body                                  91
#define ICHP_SV_ADDRESS             0x59      //++get device_t body                               92
#define ICHP_SV_END                 0x5a      //++ the vehicle arrives target point. 
#define ICHP_SV_DEFAULT             0x5b      //++get device_t table                              96
#define ICHP_SV_ROUTE               0x5c      //++server load route table to sink
#define ICHP_SV_ACKNOWLEDGE         0x5d      //++ send key response to all lamps
#define ICHP_SV_ADJUST              0x5e      //++ adjust radio default channel
#define ICHP_SV_RESPONSE            0x5f      //++ response of devices

///////////////////////////////////////////////////////////////////////////////////////////////////
// the command send and recieve between coordiantor and lamp router
//#define ICHP_CO_DATE                0x21      //++ update all router and lamps date and time
//#define ICHP_CO_ACKNOWLEDGE         0x22      //++ send key response to all lamps
//#define ICHP_CO_OPEN                0x23      //++ open lamps
//#define ICHP_CO_CLOSE               0x24      //++ close lamps
//#define ICHP_CO_CTRL                0x25      //++ send road index
//#define ICHP_CO_RPT                 0x26      //++ request lamp send lamp_t information
//#define ICHP_CO_SECKEY              0x27      //++ get key_t body information
//#define ICHP_CO_CHANNEL             0x28      //++ change radio channel, added in 2011-10-06
//#define ICHP_CO_DEFAULT             0x29      //++ restore default information, added in 2011-10-06
//#define ICHP_CO_REBOOT              0x2a      //++ lamps reboot, added in 2011-10-06
//#define ICHP_CO_ADDRESS             0x2b      //++ get lamps physical address and logic address, added in 2011-10-26
//#define ICHP_CO_END                 0x2c      //++ the vehicle arrives target point. 
//#define ICHP_CO_TABLE               0x2d      //++ get passed_t table                       
//#define ICHP_CO_ROUTE               0x2e      //++ the sink send the router table to the car
//#define ICHP_CO_ADJUST              0x2f      //++ adjust radio default channel

#define ICHP_MSG_HELLO              0x39      //++ multicast hello string for test
#define ICHP_LARGE_DATA             0x3b
#define ICHP_INTER_PAN              0x3c

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define ICHP_BEEPER_CALL                    0xE0   // beeper call to sink
#define ICHP_CAR_RPT_ERROR                  0xE1   // send error ID to sink
#define ICHP_CAR_RPT_INFO                   0xE2   // car info report to sink
#define ICHP_SV_RPT_CAR_INFO                0xE3   // car info report to server
#define ICHP_SV_RPT_CAR_ERROR               0xE4   // send error ID to server
#define ICHP_SV_RPT_BEEPER_CALL             0xE5   // sink report beeper call to server
#define ICHP_SV_BEEPER_STATUS               0XE7   // request for car info
//#define ICHP_CO_REBOOT_CAR                  0xE8   // reboot the car
//#define ICHP_CO_STOP_CAR                    0xEA
//#define ICHP_CAR_ADDRESS                    0xEB   // physic address information of device_t body

#define PLC_CAR_INFO                        0xED   // plc report car info to w108

////////////////////////////////////////////////////////////////////////////////
// network information structure definition here
//
typedef struct
{
    u8    action;
    u32   tick;            // ticker of idle
    u16   error;           // network error state
    u16   result;          // result of command handler
    u8    endpoint;        // end point
    u8    stackstate;      // network status in stack
    u8    appstate;        // network status for app
    u8    intpstate;       // network status for inter pan
    u8    substate;        // substate of network
    u8    status;          // command return status
    u8    type;            // define in ember_types.h
    u8    devid;           // device's id
    u8    method;          // network mode
    device_t   dev;        // change user device information
#ifdef ENABLE_GATEWAY
    u16   gwaddr[MAXGWSIZE];  // max. gwateway size     
#endif    
    void  (*ember)       ( void );     // ember net tick handler
    void  (*formAndJoin) ( void );
    void  (*app)         ( void );     // application tick handler
    void  (*cmd)         ( void );
    void  (*init)        ( void );
    void  (*gateway)     ( void );
    void  (*exception)   ( void );
    void  (*incoming)    ( u8, EmberApsFrame *, u8 );
    void  (*msgSent)     ( u8, u16, EmberApsFrame *, u8, u8 );
    void  (*stackStatus) ( u8 );
    void  (*timevent)    ( void );
    void  (*msevent)     ( void );
    EmberStatus  (*unicast)   ( u8 clusterId, EmberNodeId dest, u8 len, void * msg );
    EmberStatus  (*multicast) ( u8 clusterId, u8 len, void * msg );
    EmberNetworkParameters params;     // network parameter information.
} network_info_t, *pnetwork_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void NET_Configuration ( void )
//* 功能        : network information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void NET_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : UART_Configuration
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void UART_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus EZSP_Configuration ( void )
//* 功能        : ember network configuration
//* 输入参数    : 无
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void EZSP_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void ExceptionHandler ( void )
//* 功能        : exception handler of network
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ExceptionHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void secTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void secTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void msTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void msTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void coordinatorMsEventHandler ( void )
//* 功能        : called to check application microsecend
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void coordinatorMsEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void hourTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void hourTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendmultimsg ( u8 clusterId, u8 len, void * msg )
//* 功能        : send multicast message with normal format
//* 输入参数    : 无
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendmultimsg ( u8 clusterId, u8 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_qsendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_qsendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendunimsg_m ( u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendunimsg_m ( u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_qsendmultimsg ( u8 clusterId, u8 len, void * msg )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_qsendmultimsg ( u8 clusterId, u8 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_addmultigroup ( void )
//* 功能        : 调用者加入某group
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void sea_addmultigroup ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_fragmentunicast ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
//* 功能        : 传输大数据
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_fragmentunicast ( u8 clusterId, EmberNodeId dest, u16 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_SendInterpanUnicast ( u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg )
//* 功能        : send interpan unicast
//* 输入参数    : u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg
//* 输出参数    : EmberStatus
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_SendInterpanUnicast ( u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendInterPanMsg(u8 type,u16 paid,u16 dest,u16 cmd, EmberEUI64* eui,u8 len,u8* msg)
//* 功能        : 子网之间传递信息
//* 输入参数    : u8 type,u16 paid, u16 dest, u16 cmd,  EmberEUI64* eui, u8 len,  u8* msg
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*-------------------------------------------*/
EmberStatus sea_SendInterPanMsg ( u8 type, u16 paid, u16 dest, u16 cmd, EmberEUI64 eui, u8 len, void * msg );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_feedwatchdog ( void )
//* 功能        : feed watch dog
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_feedwatchdog ( void );

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
extern network_info_t    network_info;
extern EmberEventData    lampEvents[];
extern EmberEventControl millTickEvent, secTickEvent;

/////////////////////////////////////////////////////////////////////////////////////////////////////
#endif  // __NETWORK_H__


