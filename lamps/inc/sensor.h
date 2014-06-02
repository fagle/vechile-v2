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
//* ������      : printNetInfo
//* ����        : ��ӡ����������Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printNetInfo ( EmberNetworkParameters * networkParameters );

// +++++++++++++++++++++++ Ember callback handlers+++++++++++++++++++++++++++++++++++++
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ezspRouterMessageSentHandler
//* ����        : �����ͳɹ�ʱ���Զ�����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        : Called when a message has completed transmission --
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
//* ������      : ezspRouterIncomingMessageHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ++ray:�ظ�
//*------------------------------------------------*/
void ezspRouterIncomingMessageHandler(EmberIncomingMessageType type,
                                 EmberApsFrame *apsFrame,
                                 EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberRouterStackStatusHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberRouterStackStatusHandler(EmberStatus status);

//ʹ��bootloader��
#ifdef USE_BOOTLOADER_LIB 
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bootloadUtilQueryResponseHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :// When a device sends out a bootloader query, the bootloader
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
//* ������      : bootloadUtilLaunchRequestHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : This function is called by the bootloader-util library
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
//* ������      : emberSwitchNetworkKeyHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberSwitchNetworkKeyHandler(int8u sequenceNumber);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : RouterTick
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void RouterTick(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sensorInit
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : init common state for sensor nodes
//*------------------------------------------------*/
void sensorInit(void) ;


u8 sensorJoinNetwork(void);

void scanModule(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : routerExceptionHandler
//* ����        : �����쳣��������硰sensorĿǰ���������С�
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void routerExceptionHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void  routerTimeEventHandler ( void )
//* ����        : ·�ɶ�ʱ��/�صƣ������Ҫ�޸��Լ�������
//*             : �Զ����ص�ģʽ��
//*             : ++comment by ray:ONESECOND����һ��
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void  routerTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sensorCommandHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void sensorCommandHandler();

//ray�������ǽӿڴ���
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus sensorRejoinNetwork ( void )
//* ����        : sensor attemp to rejoin network
//* �������    : ��
//* �������    : rejoin status
//* �޸ļ�¼    : ��
//* ��ע        : ++ray:�ظ�
//*------------------------------------------------*/
EmberStatus sensorRejoinNetwork ( void );

void led_open ( void );
void led_close ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
#endif  //__SENSOR_H__