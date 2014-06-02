#ifndef __LAMP_H__
#define __LAMP_H__

#include "stm32f10x_type.h"
#include "config.h"
#include "ticker.h"
#include "sysinfo.h"
#include "vehicle.h"

////////////////////////////////////////////////////////////////////////////////////
// lamp status 4bits definition
#define LAMP_CLOSE         (0x00)
#define LAMP_OPEN          (0x01)

////////////////////////////////////////////////////////////////////////////////////
// lamp works control mode definition
// ++comment by ray: initialized as LAMP_AUTO | LAMP_TEST at init_singleinfo()
#define LAMP_AUTO          (0x01)
#define LAMP_NETWORK       (0x02)
#define LAMP_ODD           (0x04)
#define LAMP_EVEN          (0x08)
#define LAMP_CHANGE        (0x10)
#define LAMP_UPDATE        (0x20)
#define LAMP_FORCE         (0x40)
#define LAMP_OTHER         (0x80)

#define LAMPINTERVAL       (0x300000)
#define ONEDAYMIN          (1440)     // 24x60(minute)
#define HOURMIN            (60)

////////////////////////////////////////////////////////////////////////////////////
// lamp fail 7 bits definition
#define LAMP_ER_NO         (0x00)  // 正常=0x00
#define LAMP_ER_OV         (0x01)  // 电压过高=0x01, over voltage
#define LAMP_ER_OC         (0x02)  // 电流过高=0x02, over current
#define LAMP_ER_PT         (0x04)  // 灯具老化=0x04，time out
#define LAMP_ER_SC         (0x08)  // 线路短路=0x08，short circuit
#define LAMP_ER_CB         (0x10)  // 线路断路=0x10, circuit breaker 
#define LAMP_ER_TM         (0x20)  // 时间故障-0x20, error of syn. time
#define LAMP_ER_NP         (0x40)  // 编号故障=0x40, number problems

//the configuration infomation
#define MAXLAMPS           (EMBER_MAXDEVS)   // 道路的总路灯数, max lamps on a road
#define MAXPERIOD          (0x50)  // max report period is 48 min
#define MINPERIOD          (0x01)  // min report period is 2 min
#define RDMAXLIST          (0x0a)  // max counter to save lamp_t inforation
#define MAXPERCENT         (100)   // max. percent of pwm
#define MINPERCENT         (30)    // min. percent of pwm
#define MAXACKSIZE         (32)    // 

///////////////////////////////////////////////////////////////////////////////////////////////////
//move them to token(NandFlash)
//#define ROADID             (29)    // 长久路
//#define MAXSTAGE           (0x05)  // max stage of open/close lamps for a day
//#define DEFALUTPERIOD      (0x05)  // report single lamp information time 5 min.

//the value of State_Of_App
#define ICHP_CONN_PREPA    (0x01)
#define ICHP_CONN_REQUE    (0x02)  // 请求服务器连接的阶段
#define ICHP_WAIT_ACK      (0x03)
#define ICHP_CONN_ESTAB    (0x04)
#define ICHP_CONN_CLOSE    (0x05)  // 整个TCP连接终止的标志 

///////////////////////////////////////////////////////////////////////////////////////////////////
// state of lamps on the road
#define OPEN_LAMPS         (0x01)
#define CLOSE_LAMPS        (0x02)

#define CTRL_NET           (0x01)
#define CTRL_AUTO          (0x02)
#define CTRL_AUTO_CHANGE   (0x04)

#define SETFAILBIT         (0x01)
#define CLRFAILBIT         (0x02)
#define GETFAILBIT         (0x03)

///////////////////////////////////////////////////////////////////////////////////////////////////
// functions of lamp
#define set_lampmode(type)        (single_info.ctrl |= (type))
#define clr_lampmode(type)        (single_info.ctrl &= ~(type))
#define get_lampmode(type)        (single_info.ctrl & (type))

///////////////////////////////////////////////////////////////////////////////////////////////////
// functions of lamp and coordiantor
#define set_lampbitmap(map, index)     (map[(index) >> 0x03] |= (0x01 << ((index) % 0x08)))
#define clr_lampbitmap(map, index)     (map[(index) >> 0x03] &= ~(0x01 << ((index) % 0x08)))
#define get_lampbitmap(map, index)     (map[(index) >> 0x03] & (0x01 << ((index) % 0x08)))

//++comment by ray: not used currently
#define set_lampstate(state, type)     (state |= (type))
#define clr_lampstate(state, type)     (state &= ~(type))
#define get_lampstate(state, type)     (state & (type))

///////////////////////////////////////////////////////////////////////////////////////////////////
// temperature of lamp
typedef struct
{
    u8   light;       // 光源温度
    u8   trans:4;     // 变压器温度, max 15
    u8   env:4;       // 环境温度, max 15
    u8   lux;         // 光照度
} temp_t, *ptemp_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Definition : lamp road map information
typedef struct
{
    u16     max;           // max. lamps on a road
    u16 *   addr;          // logic address list of all lamps
    u8      size;          // bit map size
    u8  *   state;         // state bit map of lamps, to all lamps
} map_t, *pmap_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Definition : road bit map information
typedef struct
{
    u8        state;                                  // open/close state
    map_t     map;                                    // bit map of lamps on a road
    u16       (*assign) ( void );                     // 分配路灯号的方法
    void      (*error)  ( plamp_t ptr, u16 sender );  // report error handler
    void      (*update) ( u16 num, u16 sender );      // update logic address
    u16       (*number) ( u16 address );
    u16       (*address)( u16 number );
} road_t, *proad_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  definition:   路由器的功能结构
typedef struct 
{
    u8        avail;            // indicate how available the networkinflash is
    u8        scan;             // scan radio channel
    systime_t time;             // system tick time
    u8        count;            // counters of open/close command in a minute
    u8        retry;            // report send fail counter
    u8        min;              // the same time, (min) 
    u8        sec;              // the same time, (sec)
    u16       ctrl;             // control method
    u16       adc;              // ADC值
    u16       address;          // 16 bits address in PAN(短地址)
    lamp_t    lamp;             // 单个路灯信息
    void      (*on)     ( void * ptr );
    void      (*off)    ( void * ptr );
    u16       (*num)    ( void );
    u8        (*fail)   ( u8 op, u8 fail );
    u8        (*type)   ( void );
    u8        (*status) ( void );
    void      (*toggle) ( void * ptr );
} single_t, *psingle_t;          

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void init_singleinfo ( void )
//* 功能        : 初始化本地的状态信息
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void init_singleinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void  sea_singletimeevent ( void )
//* 功能        : 路由实现开/关灯功能;开完后要修改自己的数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  sea_singletimevent ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatepercent ( u8 percent )
//* 功能        : update output percent of lamp
//* 输入参数    : u8 percent
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatepercent ( u8 percent );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatetime ( psystime_t tm )
//* 功能        : update time of lamp
//* 输入参数    : psystime_t ptm
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatetime ( psystime_t ptm );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern single_t single_info;         // 单结点路灯
#ifdef SINK_APP
extern road_t road_info;             // sum information of each road 
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#endif   // ___LAMP_H__

