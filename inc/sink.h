#ifndef __SINK_H__
#define __SINK_H__

#include "config.h"
#include "spi.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////
// functions

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspCoordinatorMessageSentHandler
//* 功能        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void ezspCoordinatorMessageSentHandler(EmberOutgoingMessageType type,
                      int16u indexOrDestination,
                      EmberApsFrame *apsFrame,
                      EmberMessageBuffer message,
                      EmberStatus status);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspCoordinatorIncomingMessageHandler
//* 功能        : callback
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : ezspCoordinatorMessageHandler
//*------------------------------------------------*/
void ezspCoordinatorIncomingMessageHandler(EmberIncomingMessageType type,
                                   EmberApsFrame *apsFrame,
                                   EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberStackStatusHandler(EmberStatus status)
//* 功能        : // this is called when the stack status changes
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void emberCoordinatorStackStatusHandler(EmberStatus status);

#ifdef USE_BOOTLOADER_LIB
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilQueryResponseHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : // When a device sends out a bootloader query, the bootloader
//                query response messages are parsed by the bootloader-util
//                libray and and handled to this function. This application
//                simply prints out the EUI of the device sending the query
//                response.
//*------------------------------------------------*/
void bootloadUtilQueryResponseHandler(boolean bootloaderActive,
                                      int16u manufacturerId,
                                      int8u *hardwareTag,
                                      EmberEUI64 targetEui,
                                      int8u bootloaderCapabilities,
                                      int8u platform,
                                      int8u micro,
                                      int8u phy,
                                      int16u blVersion);


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilLaunchRequestHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : // This function is called by the bootloader-util library 
//                to ask the application if it is ok to start the bootloader.
//                This happens when the device is meant to be the target of
//                a bootload. The application could compare the manufacturerId 
//                and/or hardwareTag arguments to known values to enure that 
//                the correct image will be bootloaded to this device.
//*------------------------------------------------*/
boolean bootloadUtilLaunchRequestHandler(int16u manufacturerId,
                                         int8u *hardwareTag,
                                         EmberEUI64 sourceEui);
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sinkFormNetwork
//* 功能        : 协调器组网
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void sinkFormNetwork ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void CoordinatorTick ( void )
//* 功能        : coordinator tick handler.
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void CoordinatorTick ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : coordinatorExceptionHandler
//* 功能        : 协调器对异常状况的处理
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void coordinatorExceptionHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void coordinatorTimeEventHandler ( void )
//* 功能        : sink作为路由实现开/关灯功能,开完后要修改自己的数据;
//*             : sink上报FULL信息；
//*             : sink上报KEY信息；
//*             : ++comment by ray:ONESECOND调用一次
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  coordinatorTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sinkInit ( void )
//* 功能        : 初始化sink节点信息
//*             : 
//*             : 检查有无节点掉出
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : // init common state for sensor and sink nodes
//*------------------------------------------------*/
void sinkInit(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sinkCommandHandler ( void )
//* 功能        : 处理串口信息
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sinkCommandHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 getLampNumberByRandom ( void )
//* 功能        : assign lamp number by random
//* 输入参数    : 无
//* 输出参数    : number of lamp
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_lampnumberandom ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_lampassign ( void )
//* 功能        : assign lamp number by search empty number 
//* 输入参数    : 无
//* 输出参数    : number of lamp
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_lampnumberassign ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_setlampstate ( u8 id, u16 number )
//* 功能        : set active lamp state
//* 输入参数    : u8 id, u16 number 
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_setlampstate ( u8 id, u16 number );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8  sea_reporthandler ( plamp_t ptr, u16 sender)
//* 功能        : lamp report handler
//* 输入参数    : plamp_t ptr, u16 sender
//* 输出参数    : true/flase
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  sea_reporthandler ( plamp_t ptr, u16 sender );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printlampstate ( void )
//* 功能        : print state of all lamps
//* 输入参数    : 无 
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printlampstate ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatelogicaddress ( u16 num, u16 sender )
//* 功能        : update logic address and active state
//* 输入参数    : u16 num, u16 sender
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatelogicaddress ( u16 num, u16 sender );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern int16u concentratorType;

////////////////////////////////////////////////////////////////////////
#endif // __SINK_H__ 

