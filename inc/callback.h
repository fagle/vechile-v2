
#ifndef __EZSP_CALLBACK_H__
#define __EZSP_CALLBACK_H__

#include "config.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberIncomingMessageHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ++ray:�ظ�
//*------------------------------------------------*/
void emberIncomingMessageHandler(EmberIncomingMessageType type,
                                 EmberApsFrame *apsFrame,
                                 EmberMessageBuffer message);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberMessageSentHandler
//* ����        : �����ͳɹ�ʱ���Զ�����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        : Called when a message has completed transmission --
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
//* ������      : emberFragmentMessageSentHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberFragmentMessageSentHandler(EmberStatus status);


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberStackStatusHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberStackStatusHandler(EmberStatus status);


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void emberScanErrorHandler
//* ����        : callback
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberScanErrorHandler(EmberStatus status);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberUnusedPanIdFoundHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void emberUnusedPanIdFoundHandler(EmberPanId panId, int8u channel);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberJoinableNetworkFoundHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void emberJoinableNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                                      int8u lqi,
                                      int8s rssi);


void emberRawTransmitCompleteHandler(EmberMessageBuffer message,EmberStatus status);

#endif   // __EZSP_CALLBACK_H__

