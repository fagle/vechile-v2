#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "config.h"
#include "network.h"
#include "spi.h"

////////////////////////////////////////////////////////////////////////
//constants and globals


////////////////////////////////////////////////////////////////////////
//functions and routines

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : printNetInfo
//* 功能        : 打印加入网络信息
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printNetInfo ( EmberNetworkParameters * networkParameters );

// +++++++++++++++++++++++ Ember callback handlers+++++++++++++++++++++++++++++++++++++
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspRouterMessageSentHandler
//* 功能        : 当发送成功时，自动调用
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 ++ray:重复
//* 备注        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//*------------------------------------------------*/
void ezspRouterMessageSentHandler(EmberOutgoingMessageType type,
                      int16u indexOrDestination,
                      EmberApsFrame *apsFrame,
                      EmberMessageBuffer message,
                      EmberStatus status);



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspRouterIncomingMessageHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : ++ray:重复
//*------------------------------------------------*/
void ezspRouterIncomingMessageHandler(EmberIncomingMessageType type,
                                 EmberApsFrame *apsFrame,
                                 EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberRouterStackStatusHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 ++ray:重复
//* 备注        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberRouterStackStatusHandler(EmberStatus status);

//使用bootloader库
#ifdef USE_BOOTLOADER_LIB 
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilQueryResponseHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :// When a device sends out a bootloader query, the bootloader
//                  query response messages are parsed by the bootloader-util
//                  libray and and handled to this function. This application
//                  simply prints out the EUI of the device sending the query
//                  response.
//*------------------------------------------------*/
void bootloadUtilQueryResponseHandler(boolean bootloaderActive,
                                      int16u manufacturerId,
                                      int8u *hardwareTag,
                                      EmberEUI64 targetEui,
                                      int8u bootloaderCapabilities,
                                      int8u platform,
                                      int8u micro,
                                      int8u phy,
                                      int16u blVersion)


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilLaunchRequestHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : This function is called by the bootloader-util library
//                to ask the application if it is ok to start the bootloader.
//                This happens when the device is meant to be the target of
//                a bootload. The application could compare the manufacturerId
//                and/or hardwareTag arguments to known values to enure that
//                the correct image will be bootloaded to this device.
//*------------------------------------------------*/
boolean bootloadUtilLaunchRequestHandler(int16u manufacturerId,
                                         int8u *hardwareTag,
                                         EmberEUI64 sourceEui) ;

#endif // USE_BOOTLOADER_LIB



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberSwitchNetworkKeyHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberSwitchNetworkKeyHandler(int8u sequenceNumber);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : RouterTick
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :
//*------------------------------------------------*/
void RouterTick(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sensorInit
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : init common state for sensor nodes
//*------------------------------------------------*/
void sensorInit(void) ;


u8 sensorJoinNetwork(void);

void scanModule(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : routerExceptionHandler
//* 功能        : 处理异常情况，比如“sensor目前不在网络中”
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :
//*------------------------------------------------*/
void routerExceptionHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void  routerTimeEventHandler ( void )
//* 功能        : 路由定时开/关灯，开完后要修改自己的数据
//*             : 自动开关灯模式；
//*             : ++comment by ray:ONESECOND调用一次
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  routerTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sensorCommandHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void sensorCommandHandler();

//ray：以下是接口代码
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sensorRejoinNetwork ( void )
//* 功能        : sensor attemp to rejoin network
//* 输入参数    : 无
//* 输出参数    : rejoin status
//* 修改记录    : 无
//* 备注        : ++ray:重复
//*------------------------------------------------*/
EmberStatus sensorRejoinNetwork ( void );

void led_open ( void );
void led_close ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
#endif  //__SENSOR_H__