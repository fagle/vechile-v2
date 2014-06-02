#ifndef __LAMP_H__
#define __LAMP_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stm32f10x.h"
#include "systicker.h"

#ifndef EUI64_SIZE
#define EUI64_SIZE                  (0x08)     // @brief Size of EUI64 (an IEEE address) in bytes (8).
#define EXTENDED_PAN_ID_SIZE        (0x08)     // @brief Size of an extended PAN identifier in bytes (8).
#define EMBER_ENCRYPTION_KEY_SIZE   (0x10)     // @brief Size of an encryption key in bytes (16).
#define EMBER_CERTIFICATE_SIZE      (0x30)     // @brief Size of Implicit Certificates used for Certificate Based Key Exchange.
#define EMBER_PUBLIC_KEY_SIZE       (0x16)     // @brief Size of Public Keys used in Eliptical Cryptography ECMQV algorithms.
#define EMBER_PRIVATE_KEY_SIZE      (0x15)     // @brief Size of Private Keys used in Eliptical Cryptography ECMQV alogrithms.
#define EMBER_SMAC_SIZE             (0x10)     // @brief Size of the SMAC used in Eliptical Cryptography ECMQV alogrithms.
#define EMBER_SIGNATURE_SIZE        (0x2a)     // @brief Size of the DSA signature used in Eliptical Cryptography Digital Signature Algorithms. 
#define SERIAL_SIZE                 (0x08)     // @brief serial number size
#endif  // EUI64_SIZE
#ifndef EmberEUI64
typedef u8    EmberEUI64[EUI64_SIZE];          // @brief EUI 64-bit ID (an IEEE address).
#endif  // EmberEUI64

///////////////////////////////////////////////////////////////////////////////////////////////////
//the configuration infomation
//#define ROADID             (0x1b)   // (27) (0x19)   // 金沙南路
#define ROADID             (0x24)   // (36) (0x24)   // 邓实验
//#define ROADID             (0x21)   // (33) 杭电三教
#define MAXLAMPS           (250)    // 道路的总路灯数, max lamps on a road
//#define LINESIZE                  (0x80)
#define MAXCARDS           (0x80)

#define DEFALUTPERIOD      (0x01)   // report single lamp information time 1 min.
#define MAXPERIOD          (0x80)   // max report period is 128 min
#define MINPERIOD          (0x01)   // min report period is 1 min
#define DEFMANUALTIME      (15)     // idel time, manual -> automatic
#define DEFPOWERTIME       (60)     // power on/off time(second), before or delay
#define DEFVERSION         (0x102)  // software version 
#define SWVERSION          (0x0102) // software version
#define DEFROADNAME        "邓实验\0"
#define DEFSTAFFNAME       "邓团飞\0"
#define DEFPHONE           "18767106481\0"     // "18368145148\0"
#define DEFDOMAIN          "lamp.dnsd.info\0"     //"lamp.dnsd.info" 
#define DEFSSID            "ecs_610\0"            // ssid wifi route name
#define DEFPWAPSK          "hello610\0"           // security key
#define DEFPORT            (6186)             // (6181)
#define MAXPERCENT         (100)    // max. percent of pwm
#define MINPERCENT         (30)     // min. percent of pwm
#define MAXSTAGE           (0x04)   // max stage of open/close lamps for a day
#define MAXMONTH           (0x0c)   // 12 month per year
#define REPORTSIZE         (0x30)

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

#define FULL_LIST_SIZE     (0x10)   // full report list size
#define LAMP_DATA_SIZE     sizeof(lamp_t)
#define FULL_DATA_SIZE     ((MAXLAMPS >> 0x03) + 0x01)
#define ALL_LAMP_KEY_DATA_SIZE    (MAXLAMPS * sizeof(lamp_t))

#define LAMP_ERROR_VOL_HIGH 0x01
#define LAMP_ERROR_VOL_LOW  0x02
#define LAMP_ERROR_CUR_HIGH 0x03

#define LAMP_ERROR_CUR_LOW  0x04
#define LAMP_ERROR_TEM_HIGH 0x05
#define LAMP_ERROR_POW_HIGH 0x06
#define LAMP_ERROR_POW_LOW  0x07
#define LAMP_ERROR_BROKE    0x08
#define LAMP_ERROR_REPAIR   0x09

#define LAMP_VOL_HIGH_LIMIT 0xFF
#define LAMP_VOL_LOW_LIMIT  0x00
#define LAMP_CUR_HIGH_LIMIT 0xFF

#define LAMP_CUR_LOW_LIMIT  0x00
#define LAMP_TEM_HIGH_LIMIT 0xFF
#define LAMP_POW_HIGH_LIMIT 0xFF
#define LAMP_POW_LOW_LIMIT  0x00

#define LAMP_DATA_IDEL       0x00
#define LAMP_DATA_COLLECTING 0x01

#define MAJOR_STATE_ON       0x01
#define MAJOR_STATE_OFF      0x00

#define LAMP_OPEN            0x01
#define LAMP_CLOSE           0x00

#define WAIT_STATE_ON        0x00
#define WAIT_STATE_OFF       0x01
#define WAIT_STATE_REALTIME  0x02

#define TIME_RECORD_LEN      0x0A
#define SWITCH_LAMP_NUM      18

#define MAXCARNUM                 0x02
#define MOBILEDEVS                (50)     // max. devices of mobile vehicle
#define MOBILEBASE                (MAXLAMPS - MOBILEDEVS)
#define CALLERDEVS                (60)     // max. callers

///////////////////////////////////////////////////////////////////////////////////////
// the STM32W on the car send command to PLC
//
#define CAR_CMD_PREFIX            0x7e
#define CAR_LEFT                  0x50
#define CAR_RIGHT                 0x51
#define CAR_STRAIGHT              0x52
#define CAR_STOP                  0x53
#define CAR_RUN                   0x54
#define CAR_OBSTACLE              0x58
#define CAR_REQ_MSG               0x60

///////////////////////////////////////////////////////////////////////////////////////
// car status
#define CAR_NONE                  (0x00)
#define CAR_START                 (0x01)
#define CAR_PAUSE                 (0x02)

///////////////////////////////////////////////////////////////////////////////////////
// the command len
#define CARPLCCMDLEN              (0x06)
#define MAXROUTERTABLEN           (20)
#define RS485CMDSIZE              (0x08)
#define MAXPATH                   (0x30)   // (MAXCARDS >> 0x01)

#define	GPIO_485_TX	          PORTA_PIN(2)
#define RS485SIZE                 (32)
#define SINK_ADDRESS_TABLE_INDEX  (0x00)

////////////////////////////////////////////////////////////////////////////////////
// error status of vehicle, 
#define NO_ERROR                  (0x00)
#define WRONG_WAY                 (0x01)
#define MISS_CARD                 (0x02)
#define END_LINE                  (0x04)
#define MEET_OBSTACLE             (0x08)
#define EMPTY_TABLE               (0x10)
#define LAMP_ER_TM                (0x20)  // the same as light, 时间故障-0x20, error of syn. time
#define LAMP_ER_NP                (0x40)  // the same as light, 编号故障=0x40, number problems
#define MEET_CLASH                (0x80)

#define MINSPEED                  (3500)
#define MAXSPEED                  (3750)

////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8    status;
    u8    obstacle;
    u8    card;
    u16   magic;
    u16   leftWheelSpeed;      // car left wheel rate
    u16   rightWheelSpeed;     // car right wheel rate
} plc_t, *pplc_t;

////////////////////////////////////////////////////////////////////////////////////
// the car route elements
typedef struct
{
    u8    id;         // the card ID or lane ID 
    u8    action;     // action on the lane or card
} action_t, *paction_t;

////////////////////////////////////////////////////////////////////////////////////
// the car passed route node to record in flash
typedef struct
{
    u8       min;
    u8       sec;
    action_t act;
} passed_t, *ppassed_t;

////////////////////////////////////////////////////////////////////////////////////
// the car command buffer
typedef struct
{
    u8 prefix;                   // protocol prefix
    u8 cmd;                      // command to plc
    u8 buf[CARPLCCMDLEN - 0x02]; // message to plc
} rs485_t, *prs485_t;

////////////////////////////////////////////////////////////////////////////////////
// the car command ring
typedef struct
{
    u8    in, out;
    u8    cnt;
    rs485_t list[RS485CMDSIZE];
} rs485_cmd_t, *prs485_cmd_t;

////////////////////////////////////////////////////////////////////////////////////
// the car route table
typedef struct
{
    u8        index;
    u8        count;
    action_t  line[MAXCARDS];
} route_t, *proute_t;

////////////////////////////////////////////////////////////////////////////////////
// the car key report
typedef struct                  // 12bytes body, like as light_t in lamp.h
{
    u8      number;             // car 物理编号，灯号(1 bytes(1~255))
    u8      type;               // device type, beep, mobile etc
    u8      fail;               // fail state
    u8      status;             // car report 状态
    u8      obstacle;           // car 损坏信息, car state of meet obstacle
    u8      card;               // car card index
    u8      index;              // left speed
    u8      count;              // 
    u16     speed;              // right speed
    u16     magic;              // magic state  
} vehicle_t, *pvehicle_t;

////////////////////////////////////////////////////////////////////////////////////
// the car information body
typedef struct
{
    u16         ms;             // ms tick
    u16         delay;          // rs485 send delay
    u8          rs485;          // rs485 enable/disable
    u8          status;         // car action status
    u8          key;            // for caller and standby devices
    u8          card;           // current card read from plc
    vehicle_t   plc;            // plc data information body
    route_t     route;          // vehicle on line's action 
    void        (*put)    ( u8 cmd, u16 left, u16 right );
    prs485_t    (*get)    ( void );
    void        (*on)     ( void );
    void        (*off)    ( void );
    void        (*toggle) ( void );
    s8          (*dir)    ( u16 magic );
    u8          (*center) ( u16 magic );
} car_info_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Definition :  all the format of the information to send to the server
//////////////////////////////////////////////////////////////////////////////////////
// temperature of lamp
typedef struct
{
    u8   light;       // 光源温度
    u8   trans:4;     // 变压器温度, max 15
    u8   env:4;       // 环境温度, max 15
    u8   lux;         // 光照度
} temp_t, *ptemp_t;

//////////////////////////////////////////////////////////////////////////////////////
// current and voltage of lamp
typedef struct
{
    u8 in:4;
    u8 out:4;
} cur_t, *pcur_t;

typedef struct
{
    u8 in;
    u8 out;
} vol_t, *pvol_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Definition :  all the format of the information to send to the server
//  Length:       12 Bytes for one light in all, changed to 12 bytes by renyu 11/03/28
//  此处定义每盏路灯要上报的信息，全部路灯汇总后要加上道路ID的信息
typedef struct
{
    u16     number;              // 路灯物理编号，灯号(2 bytes(1~65535))
    u8      status:1;            // 路灯开关状态
    u8      fail:7;              // 路灯损坏信息
    u8      percent;             // 当前电流输出百分比
    u8      manual:1;            // 路灯自动控制状态
    u8      factor:7;            // 功率因数
    u8      cost;                // 总耗电量
    vol_t   voltage;             // 输入/出电压, max 255
    cur_t   current;             // 输入/出电流, max 15
    temp_t  temperature;         // 当前路灯温度
} light_t, *plight_t;

typedef union 
{
   light_t    light;
   vehicle_t  vehicle;
} lamp_t, *plamp_t;

////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct 
{
    u8    cmd;
    u8    size;
    u8 *  body;
} rep_t, *prep_t;

////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8       num;
    u8       cnt;
    u8       status:6;
    u8       send:1;
    u8       reboot:1;
    action_t line[MAXPATH];
} path_t, *ppath_t;

typedef struct
{
    u8       num;
    u8       state:5;
    u8       ack:1;
    u8       cnt:2;
    u8       type;
    u8       vehicle;
    u16      logic[0x02];
    u8       body[0x04];
} call_t, *pcall_t;

typedef union
{
    ppath_t  path;
    pcall_t  call;
} goal_t, *pgoal_t;

////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
#ifdef VEHICLE_MODE    
    lamp_t   key[MAXLAMPS];
    void   * goal[MAXLAMPS];
    time_t   time[MAXLAMPS];
#else
    lamp_t   key[MAXLAMPS];
    u8       full[FULL_DATA_SIZE];
    u8       full_poweroff[FULL_DATA_SIZE];
    u8       active[FULL_DATA_SIZE];
    u8       cnt, in, out;
    u8       count;
    u8       state;
    time_t   time;
    rep_t    buf[REPORTSIZE];
#endif
} rep_info_t, *prep_info_t;

////////////////////////////////////////////////////////////////////////////////////////
//
#define WAITTIME 0x1000

typedef struct
{
    u8  received[FULL_DATA_SIZE];
    u8  broken[FULL_DATA_SIZE];
    u8  lampnum;
    u8  state;
    u8  count;
} stat_t,*pstat_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
// functions of lamp and coordiantor
#define set_statrece(num)     (stat.received[num/0x08] |=  0x01<<(num))
#define clr_statrece(num)     (stat.received[num/0x08] &= ~0x01<<(num))
#define get_statrece(num)     (stat.received[num/0x08] &   0x01<<(num))

#define set_statbrok(num)     (stat.broken[num/0x08] |=  0x01<<(num))
#define clr_statbrok(num)     (stat.broken[num/0x08] &= ~0x01<<(num))
#define get_statbrok(num)     (stat.broken[num/0x08] &   0x01<<(num))

///////////////////////////////////////////////////////////////////////////////////////////////////
// functions of lamp and coordiantor
#define set_bitmap(map, index)     (map[(index) >> 0x03] |= (0x01 << ((index) % 0x08)))
#define clr_bitmap(map, index)     (map[(index) >> 0x03] &= ~(0x01 << ((index) % 0x08)))
#define get_bitmap(map, index)     (map[(index) >> 0x03] & (0x01 << ((index) % 0x08)))

/*******************************************************************************
* Function Name  : void sea_initreport ( void )
* Description    : initialize lamp report.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_initreport ( void );

/*******************************************************************************
* Function Name  : u8 sea_isreportempty ( void )
* Description    : get empty state of report.
* Input          : None
* Output         : ture/false
* Return         : None
*******************************************************************************/
u8 sea_isreportempty ( void );

/*******************************************************************************
* Function Name  : prep_t sea_getreport ( void )
* Description    : get report.
* Input          : None
* Output         : prep_t pointer of report
* Return         : None
*******************************************************************************/
prep_t sea_getreport ( void );

/*******************************************************************************
* Function Name  : void sea_putreport ( u8 cmd, u8 size, void * rep )
* Description    : Inserts report.
* Input          : u8 cmd, u8 size, void * rep.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_putreport ( u8 cmd, u8 size, void * rep );

/*******************************************************************************
* Function Name  : u8 get_lampstate ( u8 * state, u16 index )
* Description    : get full report lamp state.
* Input          : u8 * state, u16 index
* Output         : lamp state 
* Return         : None
*******************************************************************************/
u8 get_lampstate ( u8 * state, u16 number );

/*******************************************************************************
* Function Name  : u16 get_activelamps ( u8 * state )
* Description    : get active lamp count.
* Input          : u8 * state
* Output         : sum of active lamps 
* Return         : None
*******************************************************************************/
u16 get_activelamps ( u8 * state );

/*******************************************************************************
* Function Name  : void set_lampstate ( u8 * state, u16 number )
* Description    : get full report lamp state.
* Input          : u8 * state, u16 index
* Output         : lamp state 
* Return         : None
*******************************************************************************/
void set_lampstate ( u8 * state, u16 number );

/*******************************************************************************
* Function Name  : void clr_lampstate ( u8 * state, u16 number )
* Description    : get full report lamp state.
* Input          : u8 * state, u16 index
* Output         : lamp state 
* Return         : None
*******************************************************************************/
void clr_lampstate ( u8 * state, u16 number );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ptime_t  sea_getcurtime ( ptime_t tm )
//* 功能        : get current time
//* 输入参数    : ptime_t tm
//* 输出参数    : time_t pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
ptime_t  sea_getcurtime ( ptime_t tm );

/*******************************************************************************
* Function Name  : u8 sea_getfullistindex ( void )
* Description    : get full list index.
* Input          : None
* Output         : full list index
* Return         : None
*******************************************************************************/
u8 sea_getfullistlastindex ( void );

/*******************************************************************************
* Function Name  : u8 get_lamplaststate ( u16 number )
* Description    : get full report lamp last state.
* Input          : u16 index
* Output         : lamp state 
* Return         : None
*******************************************************************************/
u8 get_lamplaststate ( u16 number );

/*******************************************************************************
* Function Name  : void sea_parsefullreport ( u8 * ptr )
* Description    : show open lamps.
* Input          : u8 * ptr
* Output         : sum of open lamps
* Return         : None
*******************************************************************************/
u16 sea_parsefullreport ( u8 * ptr );

/*******************************************************************************
* Function Name  : u16 sea_getdifflampnumber ( u16 start )
* Description    : get state difference lamp number.
* Input          : u16 start
* Output         : next lamp number
* Return         : None
*******************************************************************************/
u16 sea_getdifflampnumber ( u16 start );

/*******************************************************************************
* Function Name  : u8 sea_reportnum ( void )
* Description    : Inserts report.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u8 sea_reportnum ( void );

/************************************************
* 函数名      : LAMP_ReportAllLampData
* 功能        : 记录开灯时间
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void LAMP_CkeckKeyData ( void );

////////////////////////////////////////////////////////////////////////////////////////
//
extern rep_info_t  rep_info;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#endif
