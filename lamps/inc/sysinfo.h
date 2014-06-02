
#ifndef __SYSINFO_H__
#define __SYSINFO_H__

//////////////////////////////////////////////////////////////////////////////////
// 
#include "config.h"
#include "ember-types.h"
#include "lamp.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#define SYSMARK                 (0x55aa)         // mark in flash information
#define SWVERSION               (0x0102)         // software version
#define MAXCHANNEL              (0x10)

//////////////////////////////////////////////////////////////////////////////////
// All nodes running the sensor or sink application will belong to the
// SENSOR MULTICAST group and will use the same location in the multicast table.
//
#define MULTICAST_ID            (0x1111)
#define MULTICAST_TABLE_INDEX   (0x00)

#define PROFILE_ID              (0xC266)         // The application profile ID we use.  This profile ID is assigned to STMicroelectronics and can only be used during development.
#define ENDPOINT                (0x01)           // Numeric index for the first endpoint. Applications can use any endpoints between 1 and 238.
                                                 // 0 = ZDO, 239 = SPDO, 240 = EDO, 241-255 reserved This application uses only one endpoint. This constant makes the code easier to read.
/////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef EMZ3118
  #define APP_POWER             (0x03)          // 3-5
#else 
  #define APP_POWER             (0x05)
#endif

////////////////////////////////////////////////////////////////////////////////
// default network information(store in flash)
#define DEF_CHANNEL             (17)
#define DEF_PANID               (0x7777)
#define DEF_PROFILE             (PROFILE_ID)
#define DEF_POWER               (APP_POWER)
#define DEF_EXPAN                "searen\0"
#define DEF_LINKKEY              "Searen Network\0"
#define DEF_NETWORKKEY           "Mirco Servo\0"
#define DEF_SERIAL               "310018\0"

////////////////////////////////////////////////////////////////////////////////////
// lamp works config mode definition
#define LAMP_CF_AUTO            (0x01)
#define LAMP_CF_OPEN            (0x02)
#define LAMP_CF_ODD             (0x04)
#define LAMP_CF_EVEN            (0x08)
#define LAMP_CF_TEST            (0x10)
#define LAMP_CF_NETWORK         (0x20)
#define LAMP_CF_FORCE           (0x40)

////////////////////////////////////////////////////////////////////////////////
// 应用层信息(写入flash)
#define DEFALUTROADID           (25)
#define DEFALUTLAMPNUM          (0x05)
#define DEFALUTPERIOD           (0x01)          // report single lamp information time 1 min.
#define DEFALUTOPEN             (0x04380258)    // 18:00 last 10 hour
#define DEFALUTMASK             (0x0000ffff)    // total 0x10(MAXCHANNEL) channels for lamp system
#define DEFALUTGWCH             (0x00008000)    // gateway default channel
#define DEFALUTCOORDCH          (0x0000000f)    // router default channel
#define DEFALUTLOTI             (0x00)
#define DEFALUTLATI             (0x00)

enum { eLights = 0x01, eVehicle, eCondition };  // type of application, such as staightlights, vehicle, air condition etc. 2013/12/14

////////////////////////////////////////////////////////////////////////////////
// network information structure definition here
//
typedef struct
{
    u8   channel;
    u16  panid;
} panid_t;

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
} sys_info_t, *psys_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask )
//* 功能        : write default system information configuration
//* 输入参数    : u8 app, u8 type, u8 index, u32 mask
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
//* 功能        : update channel information configuration
//* 输入参数    : u8 num, u8 type, u8 index, u32 mask
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask );

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
//* 函数名      : void sea_defaultconfig ( void )
//* 功能        : default system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_defaultconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printsysconfig ( void )
//* 功能        : print system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printsysconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printchannel ( pchannel_t ptr )
//* 功能        : print channel_t information 
//* 输入参数    : pchannel_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printchannel ( pchannel_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printcontrol ( pctrl_t ptr )
//* 功能        : print ctrl_t information 
//* 输入参数    : pctrl_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printcontrol ( pctrl_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printlamp ( plamp_t ptr )
//* 功能        : print lamp_t information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printlamp ( plamp_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printsingle ( void )
//* 功能        : print single_info_t information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printsingle ( void );

////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printdevice ( pdevice_t ptr )
//* 功能        : print device_t information 
//* 输入参数    : pdevice_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printdevice ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printdevicetable ( void )
//* 功能        : print device_t information in table
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printdevicetable ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatesysconfig ( void )
//* 功能        : update system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatesysconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_adjustchannel ( u8 index )
//* 功能        : change radio channel and panid
//* 输入参数    :  u8 index
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_adjustchannel ( u8 index );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_findchannelindex ( u8 channel, u16 pandid )
//* 功能        : find index of radio channel and panid
//* 输入参数    : u8 channel, u16 pandid 
//* 输出参数    : index
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_findchannelindex ( u8 channel, u16 pandid );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_configprint ( void )
//* 功能        : config sea_printf and DBG 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_configprint ( void );

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
extern sys_info_t sys_info;
extern const panid_t panid_infox[16];

/////////////////////////////////////////////////////////////////////////////////////////////////////
#endif  // __SYSINFO_H__


