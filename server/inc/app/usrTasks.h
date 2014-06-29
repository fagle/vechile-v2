#ifndef __USRTASK_H__
#define __USRTASK_H__

#include "FreeRTOS.h"

/////////////////////////////////////////////////////////////////////////////////////
//
/*********************************************************
* ������      : void W108MsgHandler ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vMiscellaneaTask ( void *pvParameters );

/*********************************************************
* ������      : void W108MsgHandler ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vW108MsgTask ( void *pvParameters );

/*********************************************************
* ������      : void W108MsgHandler ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vWifiMsgTask ( void *pvParameters );

/*********************************************************
* ������      : void vCommandTask ( void )
* ����        : ��console���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vCommandTask ( void *pvParameters );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void vTCPRecvTask ( void *pvParameters )
//* ����        : tcp client task
//* �������    : void *pvParameters
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void vTCPRecvTask ( void *pvParameters );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void vTCPSendTask ( void *pvParameters )
//* ����        : tcp client task
//* �������    : void *pvParameters
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void vTCPSendTask ( void *pvParameters );

#endif  // __USRTASK_H__