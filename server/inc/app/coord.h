
#ifndef __COORDINATOR_H__
#define __COORDINATOR_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "stm32f10x.h"
#include "network.h"

#define TIMEOUT             0x100000

#define SIZE_SINGLE_CMD     0x08//单灯开关命令长度
#define SIZE_ALL_PWM_CMD    0x07//调整PWM命令的长度
#define SIZE_SINGLE_PWM_CMD 0x09//调整单灯PWM命令的长度
#define SIZE_REQ_FULL_CMD   0x06//请求上报全部数据命令的长度
    
#define SIZE_TIME_CMD       0x0B//len字段值
#define SIZE_SYSTIME_CMD    0x0D//整条命令长度
#define SIZE_SYSTIME        0x07//数据字段长度
    
/*********************************************************
* 函数名      : void W108MsgHandlerTick ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void W108MsgHandler ( void );

/*********************************************************
* 函数名      : void GprsMsgHandlerTick ( void )
* 功能        : 解析从服务器收到的命令
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void GprsMsgHandler ( void );

/*********************************************************
* 函数名      : TIME_SyschTimeServer
* 功能        : 检测时间
* 输入参数    : 时间
* 输出参数    : 0x01:时间到  0x00:时间未到
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void TIME_SyschTime( u8 * server_time );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

////////////////////////////////////////////////////////////////////////
#endif // __COORDINATOR_H__ 

