
#ifndef __EZSP_CALLBACK_H__
#define __EZSP_CALLBACK_H__

#include "config.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberIncomingMessageHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : ++ray:重复
//*------------------------------------------------*/
void emberIncomingMessageHandler(EmberIncomingMessageType type,
                                 EmberApsFrame *apsFrame,
                                 EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberMessageSentHandler
//* 功能        : 当发送成功时，自动调用
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 ++ray:重复
//* 备注        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//*------------------------------------------------*/
void emberMessageSentHandler(EmberOutgoingMessageType type,
                      int16u indexOrDestination,
                      EmberApsFrame *apsFrame,
                      EmberMessageBuffer message,
                      EmberStatus status);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberFragmentMessageSentHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void emberFragmentMessageSentHandler(EmberStatus status);


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberStackStatusHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 ++ray:重复
//* 备注        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberStackStatusHandler(EmberStatus status);


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void emberScanErrorHandler
//* 功能        : callback
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void emberScanErrorHandler(EmberStatus status);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberUnusedPanIdFoundHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :
//*------------------------------------------------*/
void emberUnusedPanIdFoundHandler(EmberPanId panId, int8u channel);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberJoinableNetworkFoundHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :
//*------------------------------------------------*/
void emberJoinableNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                                      int8u lqi,
                                      int8s rssi);


void emberRawTransmitCompleteHandler(EmberMessageBuffer message,EmberStatus status);

#endif   // __EZSP_CALLBACK_H__

