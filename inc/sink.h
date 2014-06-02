#ifndef __SINK_H__
#define __SINK_H__

#include "config.h"
#include "spi.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////
// functions

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ezspCoordinatorMessageSentHandler
//* ����        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void ezspCoordinatorMessageSentHandler(EmberOutgoingMessageType type,
                      int16u indexOrDestination,
                      EmberApsFrame *apsFrame,
                      EmberMessageBuffer message,
                      EmberStatus status);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ezspCoordinatorIncomingMessageHandler
//* ����        : callback
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ezspCoordinatorMessageHandler
//*------------------------------------------------*/
void ezspCoordinatorIncomingMessageHandler(EmberIncomingMessageType type,
                                   EmberApsFrame *apsFrame,
                                   EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberStackStatusHandler(EmberStatus status)
//* ����        : // this is called when the stack status changes
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberCoordinatorStackStatusHandler(EmberStatus status);

#ifdef USE_BOOTLOADER_LIB
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bootloadUtilQueryResponseHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : // When a device sends out a bootloader query, the bootloader
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
//* ������      : bootloadUtilLaunchRequestHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : // This function is called by the bootloader-util library 
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
//* ������      : sinkFormNetwork
//* ����        : Э��������
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void sinkFormNetwork ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void CoordinatorTick ( void )
//* ����        : coordinator tick handler.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void CoordinatorTick ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : coordinatorExceptionHandler
//* ����        : Э�������쳣״���Ĵ���
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void coordinatorExceptionHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void coordinatorTimeEventHandler ( void )
//* ����        : sink��Ϊ·��ʵ�ֿ�/�صƹ���,�����Ҫ�޸��Լ�������;
//*             : sink�ϱ�FULL��Ϣ��
//*             : sink�ϱ�KEY��Ϣ��
//*             : ++comment by ray:ONESECOND����һ��
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void  coordinatorTimeEventHandler ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sinkInit ( void )
//* ����        : ��ʼ��sink�ڵ���Ϣ
//*             : 
//*             : ������޽ڵ����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : // init common state for sensor and sink nodes
//*------------------------------------------------*/
void sinkInit(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sinkCommandHandler ( void )
//* ����        : ��������Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sinkCommandHandler(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u16 getLampNumberByRandom ( void )
//* ����        : assign lamp number by random
//* �������    : ��
//* �������    : number of lamp
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_lampnumberandom ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u16 sea_lampassign ( void )
//* ����        : assign lamp number by search empty number 
//* �������    : ��
//* �������    : number of lamp
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_lampnumberassign ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_setlampstate ( u8 id, u16 number )
//* ����        : set active lamp state
//* �������    : u8 id, u16 number 
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_setlampstate ( u8 id, u16 number );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8  sea_reporthandler ( plamp_t ptr, u16 sender)
//* ����        : lamp report handler
//* �������    : plamp_t ptr, u16 sender
//* �������    : true/flase
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8  sea_reporthandler ( plamp_t ptr, u16 sender );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printlampstate ( void )
//* ����        : print state of all lamps
//* �������    : �� 
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printlampstate ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatelogicaddress ( u16 num, u16 sender )
//* ����        : update logic address and active state
//* �������    : u16 num, u16 sender
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatelogicaddress ( u16 num, u16 sender );

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern int16u concentratorType;

////////////////////////////////////////////////////////////////////////
#endif // __SINK_H__ 

