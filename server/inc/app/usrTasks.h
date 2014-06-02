#ifndef __USRTASK_H__
#define __USRTASK_H__

#include "FreeRTOS.h"

/////////////////////////////////////////////////////////////////////////////////////
//
/*********************************************************
* 函数名      : void W108MsgHandler ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vMiscellaneaTask ( void *pvParameters );

/*********************************************************
* 函数名      : void W108MsgHandler ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vW108MsgTask ( void *pvParameters );

/*********************************************************
* 函数名      : void W108MsgHandler ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vWifiMsgTask ( void *pvParameters );

/*********************************************************
* 函数名      : void vCommandTask ( void )
* 功能        : 从console接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vCommandTask ( void *pvParameters );

void vArcTask ( void *pvParameters );

void vGprsMsgTask( void *pvParameters );

void vTestTask( void *pvParameters );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vTCPClientTask ( void *pvParameters )
//* 功能        : tcp client task
//* 输入参数    : void *pvParameters
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vTCPClientTask ( void *pvParameters );

#endif  // __USRTASK_H__