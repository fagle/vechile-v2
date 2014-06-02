//////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef __MOBILE_H__
#define __MOBILE_H__

#include "config.h"
#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//command define
#define INTP_SEARCH                0x10      //++ search the channel and pan
#define INTP_SEARCH_RES            0x11      //++ back the lamp info about channel and panid
#define INTP_REJION_NET            0x12      //++ ask rejion net
#define INTP_LEAVE_NET             0x13      //++ ask leave net
#define INTP_UPDATE_LAMP_ID        0x14      //++ update the lampid
#define INTP_UPDATE_LAMP_STATUS    0x15      //++ close / open or adjust lamp
#define INTP_UPDATE_LATITUDE       0x16      //++ update the latitude and longitude
#define INTP_REPORT_TO_COR         0x17      //++ ask lamp report date to cordinate
#define INTP_FLICKER               0x18      //++ ask lamp flicker
#define INTP_STOP_FLICKER          0x19      //++ stop flicker and reback orignal
#define INTP_GET_INFO              0x1a      //++ get lamp info
#define INTP_GET_INFO_RES          0x1b      //++ back the lamp info
#define INTP_REBOOT_LAMP           0x1c      //++ reboot lamp


#define INTP_ACK                   0x00      //++ ACK command
#define INTP_ERR                   0xFF      //++ Err command

//////////////////////////////////////////////////////////////////////////////////////////////
//The Frame app profile and cluster id define

#define FRAME_HEAD 0x68
#define FRAME_END  0x43
//////////////////////////////////////////////////////////////////////////////////////////////
//Some size define

#define DEVICE_INFO_LIST_SIZE 5
#define FRAME_BUFFER_SZIE 0xFF

#define INTERPANMAXSIZE 73
#define MAXDATASIZE 68
#define BASEFRAMELEN 3             //Code(1)+Length(1)+Checksum(1)= 3 
#define DEVICEINFOSIZE 16

#define DEFAULT_MOBILE_PANID 0xAAAA

//typedef int8u    EmberEUI80[10];  // @brief EUI 64-bit ID (an IEEE address).
/////////////////////////////////////////////////////////////////////////////////////////////
//struct

typedef struct 
{
	u16		ChannelId;
	u16		PanId;
	EmberEUI64	PhyId; 
	u16		DeviceId;
	u8		NetStatus; 
	u8		DeviceStatus;
} DeviceInfo,*p_DeviceInfo;

typedef struct{
      u8 head;
      u8 cmd;
      u8 len; //interpan ���127�ֽ�
      void * data;
      u8 sum;
      u8 end;
} mobile_frame_t, *p_mobile_frame_t;

typedef struct{
      u8 frameType;
      u16 dstPan;
      EmberEUI64 dstPhyId;
      mobile_frame_t _frame;
}MobileCmdFrame;

typedef struct{
     u16 ChannelId;
     u16 PanId;
}NetInfo;

static int curDeviceInfoListIndex=0; 
static DeviceInfo default_DeviceInfo;

static int8u cmdBuf=0x00;
static int8u flicker_status=0x00;
static int8u lock_status=0x00;
static int8u lockDeviceIndex=0xff;

////////////////////////////////////////////////////////////////////////////////////////////
/*************************************
**��ȡУ��� cmd+len+data
*************************************/
u8 getCheckSum(mobile_frame_t * frame);
/*************************************
**����У��� cmd+len+data ��=sum
*************************************/
int8u checkSum(int8u * msg);
/************************************
**�ֳ��豸������ʾ ��
************************************/
void mobile_help();
//function define

void mobile_serch_channel();

u8 mobile_sendMsg(MobileCmdFrame *frame);

EmberStatus mobile_sendACK(MobileCmdFrame *frame,int8u *cmd);
EmberStatus mobile_senderr(MobileCmdFrame *frame,u8 cmd);
EmberStatus mobile_sendack(MobileCmdFrame *frame,u8 cmd);

void mobilCallbackHandler(MobileCmdFrame *m_frame,int8u * msgbuffer);
int getInfoIndex(int16u deviceId);
void mobileCommandHandler(void);
void mobileInit(void);
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sinkFormNetwork
//* ����        : Э��������
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void MobileFormNetwork(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void CoordinatorTick ( void )
//* ����        : coordinator tick handler.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void MobileTick ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : coordinatorExceptionHandler
//* ����        : Э�������쳣״���Ĵ���
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void MobileExceptionHandler(void);

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
void  MobileTimeEventHandler ( void );

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
void MobileInit(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sinkCommandHandler ( void )
//* ����        : ��������Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void MobileCommandHandler(void);


void ezspMobileIncomingMessageHandler ( EmberIncomingMessageType type, EmberApsFrame *apsFrame, EmberMessageBuffer message );
void ezspMobileMessageSentHandler ( EmberOutgoingMessageType type,
                                    int16u indexOrDestination,
                                    EmberApsFrame *apsFrame,
                                    EmberMessageBuffer message,
                                    EmberStatus status );
void emberMobileStackStatusHandler ( EmberStatus status );


#endif