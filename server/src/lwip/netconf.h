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
//* ������      : void vDHCPClientTask ( void *pvParameters )
//* ����        : dhcp client task
//* �������    : void *pvParameters
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void vDHCPClientTask ( void *pvParameters );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatenetaddr ( void )
//* ����        : update client network address
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatenetaddr ( void );

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */