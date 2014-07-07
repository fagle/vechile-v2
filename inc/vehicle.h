/*
 * car.h
 * define the car function
 *
 */

#ifndef __VEHICLE_H__
#define __VEHICLE_H__

/////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x_type.h"
#include "ember-types.h"
#include "config.h"
#include "sysinfo.h"
#include "frame.h"

#define MAXCARNUM                 0x02
#define DEFMSTICK                 0x24           // 10  // 32 ms send command to plc
#define MAGICSIZE                 0x10
#define HOMESIZE                  0x05

/*the STM32W on the car send command to PLC*/
#define CAR_CMD_PREFIX            0x7e
#define CAR_LEFT                  0x50
#define CAR_RIGHT                 0x51
#define CAR_STRAIGHT              0x52
#define CAR_STOP                  0x53
#define CAR_RUN                   0x54
#define CAR_ASSIGN                0x55
#define CAR_OBSTACLE              0x58
#define CAR_REQ_MSG               0x60

/*the command len*/
#define CARPLCCMDLEN              (0x09)
#define MAXROUTERTABLEN           (20)
#define RS485CMDSIZE              (0x20)
#define MAXPATH                   (MAXCARDS >> 0x01)     // (0x80 / 2)
#define KEYTIMES                  (0x100)
#define RECVTIMES                 (0x80)    // (0x100)    // (0x200)
#define PLCPERIOD                 (50)

#define RS485SIZE                 (32)
//#define SINK_ADDRESS_TABLE_INDEX  (0x00)

////////////////////////////////////////////////////////////////////////////////////
// error status of vehicle, 
#define	GPIO_485_TX	          PORTA_PIN(2)
#define	GPIO_LED	          PORTA_PIN(1)
#define	GPIO_BUTTONA	          PORTA_PIN(0)
#define	GPIO_BUTTONB	          PORTA_PIN(4)

////////////////////////////////////////////////////////////////////////////////////
// error status of vehicle, 
#define NO_ERROR                  (0x00)
#define WRONG_WAY                 (0x01)
#define MISS_CARD                 (0x02)
#define END_LINE                  (0x04)
#define MEET_OBSTACLE             (0x08)
#define EMPTY_TABLE               (0x10)
//#define LAMP_ER_TM                (0x20)  // the same as light, 时间故障-0x20, error of syn. time
//#define LAMP_ER_NP                (0x40)  // the same as light, 编号故障=0x40, number problems
#define MEET_CLASH                (0x80)

/////////////////////////////////////////////////////////////////////////////////////
//  define the car speed
#define LEFTSPEED                 (3500)
#define RIGHTSPEED                (3750)
#define LEFTMINSPEED              (3600)
#define RIGHTMINSPEED             (3650)

/////////////////////////////////////////////////////////////////////////////////////
//  define the car status
#define WAITINGCAR                (0x01)
#define ASSIGNCAR                 (0x02)
#define ONLINECAR                 (0x03)

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
// the car trun_t information
typedef struct
{
    u8   cmd;
    u16  left;
    u16  right;
} turn_t, *pturn_t; 

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
    u8 buf[CARPLCCMDLEN];        // message to plc
} rs485_t, *prs485_t;

typedef struct
{
    s8   dir;
    u8   cnt;
} magic_t, *pmagic_t;

////////////////////////////////////////////////////////////////////////////////////
// the car command ring
typedef struct
{
    u8    in, out;
    rs485_t list[RS485CMDSIZE];
} rs485_cmd_t, *prs485_cmd_t;

////////////////////////////////////////////////////////////////////////////////////
// the car route table
typedef struct
{
    u8          count;          // count of route cards
    u8          index;
    action_t    line[MAXPATH];
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
    u8      index;              // index of card
    u8      count;              // count of route
    u16     speed;              // right speed
    u16     magic;              // magic state  
} vehicle_t, *pvehicle_t;

////////////////////////////////////////////////////////////////////////////////////
// the beeper key report
typedef struct                  // 12bytes body, like as light_t in lamp.h
{
    u8      number;             // beep 物理编号，灯号(1 bytes(1~255))
    u8      type;               // device type, beep, mobile etc
    u8      fail;               // fail state
    u8      status;             // beep report 状态
    u8      last;               // last led state
    u8      car;                // car number
    u8      call;               // caller type
    u8      assign;             // assign vehicle
    u16     times;              // time of flash
    u16     tick;               // tick count  
} beep_t, *pbeep_t;

////////////////////////////////////////////////////////////////////////////////////
// the car information body
typedef struct
{
    u16         ms;              // ms tick
    u16         delay;           // rs485 send delay
    u16         recv;            // recieve frame count from plc
    u16         time;           // time (second) found new rfid card 
    u8          status;          // car action status
    u8          key;             // for caller and standby devices
    u8          card;            // current card read from plc
    u8          action;          // current line action
    u8          clash;           // car meet clash state
    u8          waiting;
    u8          homeway;         // count of home way
    pbeep_t     beep;
    vehicle_t   plc;             // plc data information body
    action_t    home[HOMESIZE];  // goto home
    route_t     route;           // vehicle on line's action 
    turn_t   *  turn;            // action of card
    u8       *  state;           // card state of vehicle on the line, added 2014/01/12
    magic_t     map[MAGICSIZE];  // magic sample
    void        (*put)    ( u8 cmd, u16 left, u16 right );
    prs485_t    (*get)    ( void );
    void        (*on)     ( void );
    void        (*off)    ( void );
    void        (*toggle) ( void );
    pmagic_t    (*magic)  ( u16 magic, pmagic_t ptr );
} car_info_t;

typedef struct{
    u8 type;
    u8 station;
} call_info_t;

/////////////////////////////////////////////////////////////////////////////////////
//
#define carSetError(st)   (carInfo.plc.fail |= (st))
#define carClrError(st)   (carInfo.plc.fail &= ~(st))
#define carGetError(st)   (carInfo.plc.fail & (st))

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleInit ( void )
//* 功能        : initialize car information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleInit ( void );

//
// configure the car table
//
//
void updateCarID(void);


u8 findEmptyCarTab( void );


//
// check the node id in the car table
// accroding to EUI64 to find to car information
// in the carTable so that it can update the car
// information
// 
u8 checkNodeID( u8 *eui64);


//
// carSendStateEventHandler
// the car send the state to the PC
// the show the car's state 
// @param void
// @return void 
//
void carSendStateEventHandler(void);

//
// carRouterEventHandler
// according to the route table, 
// the car run 
// @param void
// @return void 
//
void carRouteEventHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCallDevice ( void )
//* 功能        : is caller device
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCallDevice ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCarDevice ( void )
//* 功能        : is car device
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCarDevice ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCallerDevice ( void )
//* 功能        : is caller device
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCallerDevice ( void );

//
// carSetState
// accrding to the car state to choose the direction of the car
// @param carState: the car state 
// @param leftWheelRate: the left wheel rate
// @param rightWheelRate: the right wheel rate
// @return void
//
void carSetState ( u8 carStat, u16 leftWheelRate, u16 rightWheelRate );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 vehicleRouteTable ( u8 index, u8 length, action_t * ptr )
//* 功能        : the message max length is 31,the first is the car start or
//                if the router table length is more than 31,the first is the 0xFF,
//                meaning that there is more data
//* 输入参数    : @param length: the received message length
//                @param index: the table index
//                @param ptr: the message
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  vehicleRouteTable ( u8 index, u8 length, action_t * ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void carClearRouteTable ( void ) 
//* 功能        : clear the vehicle route table
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void carClearRouteTable ( void );

//
// car485SendStateEventHander
// handler the car send state vir 485
// @param void
// @return void
//
void car485SendStateEventHander( void );


//
// carSendTo485EventHander
// handler stm32W send message to car vir 485
// @param : void
// @return void
//
void carSendTo485EventHander( void );

// checkCarStateTick
// check the state
// @param void
void checkCarStateTick( void );
void reverseShowEnable(void);

void recvPlcStateFrm ( pframe_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleAppHandler ( void )
//* 功能        : real time application handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleAppHandler     ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleTimeEventHandler ( void ) 
//* 功能        : second tciker handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleMsEventHandler ( void ) 
//* 功能        : micro_second tciker handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleMsEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////
//
extern car_info_t carInfo;
extern call_info_t call;
///////////////////////////////////////////////////////////////////////////////////////
//
#endif //  __VEHICLE_H__
