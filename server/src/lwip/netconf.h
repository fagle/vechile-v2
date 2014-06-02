/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H__
#define __NETCONF_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "config.h"
  
///////////////////////////////////////////////////////////////////////////////////////
//
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LWIP_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vDHCPClientTask ( void *pvParameters )
//* 功能        : dhcp client task
//* 输入参数    : void *pvParameters
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vDHCPClientTask ( void *pvParameters );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatenetaddr ( void )
//* 功能        : update client network address
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatenetaddr ( void );

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */