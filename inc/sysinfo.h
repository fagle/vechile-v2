
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
// Ӧ�ò���Ϣ(д��flash)
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
    u16        road;                 // ����·�������ĵ�·ID
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
//* ������      : void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask )
//* ����        : write default system information configuration
//* �������    : u8 app, u8 type, u8 index, u32 mask
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
//* ����        : update channel information configuration
//* �������    : u8 num, u8 type, u8 index, u32 mask
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void SYS_Configuration ( void )
//* ����        : system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void SYS_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_defaultconfig ( void )
//* ����        : default system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_defaultconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printsysconfig ( void )
//* ����        : print system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printsysconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printchannel ( pchannel_t ptr )
//* ����        : print channel_t information 
//* �������    : pchannel_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printchannel ( pchannel_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printcontrol ( pctrl_t ptr )
//* ����        : print ctrl_t information 
//* �������    : pctrl_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printcontrol ( pctrl_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printlamp ( plamp_t ptr )
//* ����        : print lamp_t information
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printlamp ( plamp_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printsingle ( void )
//* ����        : print single_info_t information
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printsingle ( void );

////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printdevice ( pdevice_t ptr )
//* ����        : print device_t information 
//* �������    : pdevice_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printdevice ( pdevice_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printdevicetable ( void )
//* ����        : print device_t information in table
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printdevicetable ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatesysconfig ( void )
//* ����        : update system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatesysconfig ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_adjustchannel ( u8 index )
//* ����        : change radio channel and panid
//* �������    :  u8 index
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_adjustchannel ( u8 index );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_findchannelindex ( u8 channel, u16 pandid )
//* ����        : find index of radio channel and panid
//* �������    : u8 channel, u16 pandid 
//* �������    : index
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 sea_findchannelindex ( u8 channel, u16 pandid );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_configprint ( void )
//* ����        : config sea_printf and DBG 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_configprint ( void );

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
extern sys_info_t sys_info;
extern const panid_t panid_infox[16];

/////////////////////////////////////////////////////////////////////////////////////////////////////
#endif  // __SYSINFO_H__


