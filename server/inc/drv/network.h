
#ifndef __NETWORK_H__
#define __NETWORK_H__

//////////////////////////////////////////////////////////////////////////////////
// Description: 
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"

#include "config.h" 

#include "serial.h"
#include "console.h"
#include "smem.h"
#include "flash.h"
#include "iic.h"
#include "led.h"
#include "adc.h"
#include "key.h"
#include "systicker.h"
#include "pwm.h"
#include "lamp.h"
#include "system.h"
#include "wifi.h"
#include "gpio.h"
#include "cmd.h"
#include "spi.h"
#include "traffic.h"

//////////////////////////////////////////////////////////////////////////////////
// Description: The application profile ID we use.  This profile ID is assigned to
// ST and can only be used during development.
#define DEF_CHANNEL            (26)
#define DEF_PANID              (0x01ff)
#define DEF_POWER              (-1)    // u16 (-1)
#define PROFILE_ID             (0xC266)
#define ENDPOINT               (0x01)     // 0x01 
#define DEFALUTMASK            (0x0000ffff)    // total 0x10(MAXCHANNEL) channels for lamp system

#define TINTERVAL              (0x100000) // 0x1 ~ 0xffff ffff, and 0xfffff is about 3 second
#define EINTERVAL              (0x50000)  // 0x50000 is about 1 second 
#define DELAYTIME              (0x3ff)    // delay times

#define EMBER_NOERROR          (0x00)
#define ADDRESS_TABLE_INDEX    (0x00)
#define MULTICAST_TABLE_INDEX  (0x00)
#define MULTICAST_ID           (0x1111)

#define SYSMARK                (0x55a0)         // mark in flash information 0x55a0
#define NAMESIZE               (0x20)
#define DOSIZE                 (0x30)
#define IPSIZE                 (0x04)
#define PHSIZE                 (0x04)
#define MACSIZE                (0x06)

/////////////////////////////////////////////////////////////////////////////////////////////
// zigbee types of device, modified 2012/06/22, update 2013/11/18
#define COORDID                (0x00)     // coordinator's id, it must be different with other devices.  
#define ENDEVID                (0x01)     // please modify node id here, and use different node ids for different node.
#define ROUTERID               (0x02)

/////////////////////////////////////////////////////////////////////////////////////////////
// control device types, updated 2013/12/14
#define CENTERID               (0x01)     // control center must be coordinator
#define LAMPID                 (0x02)     // lamp id make as router
#define CALLAID                (0x03)     // for green, a call devices, added 3 devices for green's agv
#define CALLBID                (0x04)     // another green's call devices, commented by renyu 2013/11/03
#define MOBILEAID              (0x05)     // install on car loader, mobile zigbee devices
#define MOBILEBID              (0x06)     // install on car loader, mobile zigbee devices
#define MOBILECID              (0x07)     // install on car loader, mobile zigbee devices
#define MOBILEDID              (0x08)     // install on car loader, mobile zigbee devices
#define VEHICLEID              (0x09)     // install on car loader, vehicle zigbee devices
#define CONDITIONID            (0x0a)     // air condition control devices
#define GATEWAYID              (0x0b)     // gateway devices
#define OTHERID                (0x0c)

////////////////////////////////////////////////////////////////////////////////////
// lamp works control mode definition
// ++comment by ray: initialized as LAMP_AUTO | LAMP_TEST at init_singleinfo()
#define LAMP_AUTO              (0x01)
#define LAMP_TEST              (0x02)
#define LAMP_ODD               (0x04)
#define LAMP_EVEN              (0x08)
#define LAMP_POWER             (0x10)
#define LAMP_NETWORK           (0x20)
#define LAMP_FORCE             (0x40)
#define LAMP_OTHER             (0x80)

/////////////////////////////////////////////////////////////////////////////////////////////
// error types
#define NOERROR                (0x00)

///////////////////////////////////////////////////////////////////////////////////////////////////
// the command of the GPRS module received from the remote web server, modified 2012/06/22, update on 2013/11/09
#define ICHP_SV_REBOOT              0x51      //++reset all devices                               81
#define ICHP_SV_RPT                 0x52      //++return open/close status of lamps on road       83
#define ICHP_SV_DATE                0x53      //++update coordinator current date and time        84
#define ICHP_SV_OPEN                0x54      //++open all the light                              85
#define ICHP_SV_CLOSE               0x55      //++close all the light                             87
#define ICHP_SV_CHANNEL             0x56      //++get channel_t body                              89
#define ICHP_SV_CTRL                0x57      //++subcommand of server control mode               90
#define ICHP_SV_SECKEY              0x58      //++get key_t body                                  91
#define ICHP_SV_ADDRESS             0x59      //++get device_t body                               92
#define ICHP_SV_GPRSRST             0x5a      //++reset gprs                                      94
#define ICHP_SV_W108RST             0x5b      //++reset zigbee of w108                            95
#define ICHP_SV_END                 0x5c      //++ the vehicle arrives target point. 
#define ICHP_SV_GRAPH               0x5d      //++get device_t table                              96
#define ICHP_SV_ROUTE               0x5e      //++server load route table to sink
#define ICHP_SV_ACKNOWLEDGE         0x5f      //++ send key response to all lamps
#define ICHP_SV_ADJUST              0x60      //++ adjust radio default channel

#define ICHP_SV_RECV                0x61      //++ adjust radio default channel
#define ICHP_SV_ERROR               0x62      //++ adjust radio default channel
#define ICHP_SV_ACTSTOP             0x66
#define ICHP_SV_GETCARD             0x67
#define ICHP_SV_CMD                 0x68

///////////////////////////////////////////////////////////////////////////////////////////////////
// the command send and recieve between coordiantor and lamp router
#define ICHP_CO_JOIN                0x13      //++ zigbee new node jion, protocol command, added 2013/12/18
#define ICHP_CO_DATE                0x21      //++ update all router and lamps date and time
#define ICHP_CO_ACKNOWLEDGE         0x22      //++ send key response to all lamps
#define ICHP_CO_OPEN                0x23      //++ open lamps
#define ICHP_CO_CLOSE               0x24      //++ close lamps
#define ICHP_CO_CTRL                0x25      //++ send road index
#define ICHP_CO_RPT                 0x26      //++ request lamp send lamp_t information
#define ICHP_CO_SECKEY              0x27      //++ get key_t body information
#define ICHP_CO_CHANNEL             0x28      //++ change radio channel, added in 2011-10-06
#define ICHP_CO_DEFAULT             0x29      //++ restore default information, added in 2011-10-06
#define ICHP_CO_REBOOT              0x2a      //++ lamps reboot, added in 2011-10-06
#define ICHP_CO_ADDRESS             0x2b      //++ get lamps physical address and logic address, added in 2011-10-26
#define ICHP_CO_END                 0x2c      //++ the vehicle arrives target point. 
#define ICHP_CO_TABLE               0x2d      //++ get passed_t table                       
#define ICHP_CO_ROUTE               0x2e      //++ the sink send the router table to the car
#define ICHP_CO_ADJUST              0x2f      //++ adjust radio default channel

#define ICHP_MSG_HELLO              0x39      //++ multicast hello string for test
#define ICHP_LARGE_DATA             0x3b
#define ICHP_INTER_PAN              0x3c

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define ICHP_INTER_PAN_RANGE_TEST   0x3d
#define ICHP_INTER_PAN_RANGE_TEST_RESPONE 0x3e

#ifdef ENERGY_LAMP
#define ICHP_SWITCH1_PWON                  0x60
#define ICHP_SWITCH1_PWOFF                 0x61
#define ICHP_SWITCH2_PWON                  0x62
#define ICHP_SWITCH2_PWOFF                 0x63
#endif

#define ICHP_SV_GETROUTE            0x63
#define ICHP_SV_INDEX               0x64
#define ICHP_SV_BUF                 0x65
#define ICHP_CAR_ROUTE              0x40

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define ACK_OK                              0x00
#define ACK_FAIL                            0x01
#define ICHP_SV_ROUTE_ACK                   0xB1
#define ICHP_SV_OPEN_ACK                    0xB2
#define ICHP_SV_CLOSE_ACK                   0xB3
#define ICHP_SV_END_ACK                     0xB4
#define ICHP_SV_BEEPER_ACK                  0xB5

#define ICHP_BEEPER_CALL                    0xE0   // beeper call to sink
#define ICHP_CAR_RPT_ERROR                  0xE1   // send error ID to sink
#define ICHP_CAR_RPT_INFO                   0xE2   // car info report to sink
#define ICHP_SV_RPT_CAR_INFO                0xE3   // car info report to server
#define ICHP_SV_RPT_CAR_ERROR               0xE4   // send error ID to server
#define ICHP_SV_RPT_BEEPER_CALL             0xE5   // sink report beeper call to server
#define ICHP_SV_LOAD_ROUTE_TABLE            0xE6   // server load route table to sink
#define ICHP_SV_BEEPER_STATUS               0XE7   // request for car info
#define ICHP_CO_REBOOT_CAR                  0xE8   // reboot the car
#define ICHP_CO_LOAD_ROUTE_TABLE            0xE9   // the sink send the router table to the car
#define ICHP_CO_STOP_CAR                    0xEA
#define ICHP_CAR_ADDRESS                    0xEB   // physic address information of device_t body

#define ICHP_SV_SENDTABLE                   0xF0   // 
#define PLC_CAR_INFO                        0xED   // plc report car info to w108

#define DEFAULT_START_TIME                  0x13
#define DEFAULT_STOP_TIME                   0x04

//////////////////////////////////////////////////////////////////////////////////////////////////////
// function of endian
#define sea_ntos(n)                 (((n >> 0x08) & 0xff) | ((n & 0xff) << 0x08))
#define sea_ntol(n)                 (((n >> 0x18) & 0xff) | ((n & 0xff) << 0x18) | ((n >> 0x08) & 0xff00) | ((n & 0xff00) << 0x08))

///////////////////////////////////////////////////////////////////////////////////////////////////
// control mode functions
#define set_ctrlmode(type)     (sys_info.ctrl.config |= (type))
#define clr_ctrlmode(type)     (sys_info.ctrl.config &= ~(type))
#define get_ctrlmode(type)     (sys_info.ctrl.config & (type))

#define Hex2Bcd(ch)            (((ch / 10) * 16) + ch % 10)
#define Bcd2Hex(ch)            (((ch / 16) * 10) +  ch % 16)

//////////////////////////////////////////////////////////////////////////////////////
// note: each device has a phsical information of device id and device type(2bytes),
//       physical address eui(8bytes), location of longitude and latitude(8bytes).
//       those information has to be saved in special blocks.
//       totally, size = (500(lamps) * 18) + 18(coordinator).
#define STNAMESIZE                  (0x08)
#define STPHONESIZE                 (0x10)
#define SSIDLEN                     (0x20)
#define WKEYLEN                     (0x20)

/////////////////////////////////////////////////////////////////////////////////////////
// ready bitmap for w108 
#define ADDRBIT    0x01
#define CTRLBIT    0x02
#define CHNBIT     0x04
#define KEYBIT     0x08  
#define INITBIT    0x80
#define W108MASK   0x0f

enum { eLights = 0x01, eVehicle, eCondition };  // type of application, such as staightlights, vehicle, air condition etc. 2013/12/14

//////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8        link[EMBER_ENCRYPTION_KEY_SIZE];
    u8        network[EMBER_ENCRYPTION_KEY_SIZE];
} key_t, *pkey_t;

typedef struct
{
    u32       mask;            // mask of radio channel 
    u16       panid;           // pan id
    u16       profile;         // profile id
    u8        power;           // power
    u8        channel;         // radio channel
    u8        index;           // index of pan id
    u8        expanid[EXTENDED_PAN_ID_SIZE];         // extend id
} channel_t, *pchannel_t;

typedef struct
{
    EmberEUI64 eui;          // physic address
    u8         num;          // number of lamp only 1~255, modified by renyu 2013/11/18
    u8         type;         // device id, such as coordinator, control center etc.
} device_t, *pdevice_t;

typedef struct
{
    u32        longitude;    // longitude of lamps
    u32        latitude;     // latitude of lamps
} locate_t, *plocate_t;

typedef struct
{
    u16        maxdev;               // max. devices in ember network
    u16        road;                 // 该组路灯所属的道路ID
    u16        ver;                  // software version
    u8         period;               // report time period (minute)
    u8         app;                  // type of application, such as staightlights, vehicle, air condition etc. 2013/12/14
    u8         config;               // lamp config, power on lamp open/close, etc.
    u8         base;                 // base number of vehicle
    u8         car;                  // max. vehicles
    u8         call;                 // max. callers
    u8         type;                 // car types
    u8         release;              // release version
} ctrl_t, *pctrl_t;

typedef struct
{
    u16        mark;                 // system information mark
    u16        size;                 // size of structure body
    ctrl_t     ctrl;                 // control information, such as max. devs, road, period, config, application
    key_t      key;                  // link and netwrok key
    device_t   dev;                  // device information
    locate_t   loc;                  // locate information
    channel_t  channel;              // network channel information
} w108_t, *pw108_t;

typedef struct 
{
    u16       logic;
    device_t  dev;
} addr_t, *paddr_t;

//////////////////////////////////////////////////////////////////////////////////////
//
typedef struct                  // 每天路灯开关时间和控制方式
{
    time_t    open;
    time_t    close;
    u8        percent;
} sect_t, *psect_t;

typedef struct
{
    sect_t    sect[MAXSTAGE];
} stage_t, *pstage_t;

typedef struct
{
    u8   name[STNAMESIZE];  
    u8   phone[STPHONESIZE];
    u8   group;
    u8   id;
} phone_t, *pphone_t;

typedef struct     // air condition controller information body, commented by renyu, 2013/10/26
{
    u16       id;
    u16       redctrl;
    u8        mode;
    u8        temper;
    u16       closetime;
    u16       opentime;
    u16       lasttime;
    u8        tm_tempdata[MAXMONTH];
    u8        tm_modedata[MAXMONTH];
} atc_t, *patc_t;

//////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8        gw[IPSIZE];      // gateway ip address 
    u8        ip[IPSIZE];      // host ip address 
    u8        mk[IPSIZE];      // ip mask address 
    u8        mac[MACSIZE];    // mac address
} host_t, *phost_t;

typedef struct
{
    u8        ssid[SSIDLEN];   // ssid name of wireless router
    u8        mode;            // security mode, 0-wep, 1-wpa/wpa2/psk, 2-none, 4-auto
    u8        size;            // key length, 0-none, 5-wep key is 10hex number(5chars), 13-wep key 26hex number(13chars)
    u8        key[WKEYLEN];    // security key
} gatew_t, *pgatew_t;

typedef struct
{
    u8        cnt;             // count of cards
    u8        list[MAXCARDS];  // map of cards
} card_t, *pcard_t;

//////////////////////////////////////////////////////////////////////////////////////
//
typedef struct                 // some information want to save in flash
{
    u16       mark;            // system information mark
    u16       size;            // size of sys_info_t
    u8        name[NAMESIZE];  // the road name
    u8        ip[IPSIZE];      // ip address 
    u16       port;            // tcp layer sevice port
    u8        domain[DOSIZE];  // domain name
    host_t    local;           // local host ip, mask and gateway
    phone_t   staff[PHSIZE];   // phone number
    ctrl_t    ctrl;            // control information, such as max. devs, road, period, config, application
    device_t  dev;             // device information
#ifdef VEHICLE_MODE
    card_t    card;            // map of cards        
#endif
#ifdef LANDSCAPE_MODE
    stage_t   stage[MAXMONTH]; // open/close lamps stages, each month 
#endif
#ifdef WIFI_MODE
    gatew_t   route;           // wifi router or gateway
#endif
} sys_info_t, *psys_info_t;

//////////////////////////////////////////////////////////////////////////////////////
//
typedef struct                 // some information want to save in flash
{
    u16       count;           // sum of active lamps on this road
    u16       openlamps;       // sum of open lamps
    u16       lightness;       // environment lightness around coordinator
    u16       error;           // errors of lamp control
    u8        state;           // lamp state of open/close
    u8        percent;         // open lamp percent
    u8        ready;           // system ready, gprs, w108 work.
    u8        config;          // configuration of device
    u8        serial;          // enable/disable serial, sea_printf
    u8        report;          // enable/disable report, put
    u8        landstate;       // landscape state
    w108_t    w108;            // w108 system information body, the same with current zigbee module
    addr_t    addr[MAXLAMPS];  // device address include logic and physical address 
    xSemaphoreHandle semserial;    
    xSemaphoreHandle semgprs;  // semaphore of gprs send
    xSemaphoreHandle semw108;  // semaphore of w108 send
    u8        buffer[0x80];    // buffer of command send
} dyn_info_t, *pdyn_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define BUF_SIZE    (0x400)
typedef struct
{
    u8    buf[BUF_SIZE];
    int   len;
} buf_t, *pbuf_t;

typedef struct 
{
    int   sock;		  // socket 标识符 -1无效，>= 0 有效
    int   conn;		  // socket 是否连接成功，0 未连接，>= 1 连接
    buf_t recv;
    buf_t send;
} client_info_t, *pclient_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : psys_info_t getsysinfo ( void )
//* 功能        : get system information structure body
//* 输入参数    : 无
//* 输出参数    : psys_info_t pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
psys_info_t getsysinfo ( void );

/************************************************
* 函数名      : sea_updatesysinfo ( void )
* 功能        : update system information to flash
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_updatesysinfo ( void );

/************************************************
* 函数名      : void sea_writedefaultsysinfo ( void )
* 功能        : write defalut system information to flash
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_writedefaultsysinfo ( void );

/************************************************
* 函数名      : phone_t * sea_getstaff ( u8 index )
* 功能        : get staff name and phone
* 输入参数    : 无
* 输出参数    : pointer of phone_t 
* 修改记录    : 无
* 备注        : 无
*************************************************/  
phone_t * sea_getstaff ( u8 index );

/************************************************
* 函数名      : void sea_releasemode ( void )
* 功能        : set debug/release mode
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_releasemode ( void );

/************************************************
* 函数名      : void sea_setstaff ( u8 index, phone_t *staff )
* 功能        : set staff name and phone
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_setstaff ( u8 index, phone_t *staff );

/************************************************
* 函数名      : void sea_printsysinfo ( void )
* 功能        : print system information
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_printsysinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printw108sysinfo ( void )
//* 功能        : print w108 system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printw108sysinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void SYS_Configuration ( void )
//* 功能        : system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void SYS_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void ApplicationHandler ( void )
//* 功能        : handler of user Application
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ApplicationHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void MiscellaneaHandler ( void )
//* 功能        : handler of user miscellanea events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void MiscellaneaHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void ShowCopyrights ( void )
//* 功能        : show copyrights
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ShowCopyrights ( void );

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
//* 函数名      : void CommandHandler ( void )
//* 功能        : command handler of network
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void CommandHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageinsert ( time_t open, time_t close, u8 percent, u8 mon )
//* 功能        : set lamp stage of open/close
//* 输入参数    : time_t open, time_t close, u8 percent, u8 mon
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageinsert ( time_t open, time_t close, u8 percent, u8 mon );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageupdate ( time_t open, time_t close, u8 percent, u8 mon, u8 index )
//* 功能        : update lamp stage of open/close
//* 输入参数    : time_t open, time_t close, u8 percent, u8 mon, u8 index
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageupdate ( time_t open, time_t close, u8 percent, u8 mon, u8 index );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageclear ( void )
//* 功能        : clear lamp stages
//* 输入参数    : None
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageclear ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_isopentime ( const systime_t * tm )
//* 功能        : is lamp lamp stage of open/close
//* 输入参数    : const systime_t * tm
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
u8 sea_isopentime ( const systime_t * tm );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_timeventhandler ( void )
//* 功能        : time event handler
//* 输入参数    : None
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_timeventhandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern sys_info_t sys_info;
extern dyn_info_t dyn_info;
#ifdef LWIP_ENABLE
extern client_info_t  client;
#endif

/////////////////////////////////////////////////////////////////////////////////
#endif  // ending __NETWORK_H__
