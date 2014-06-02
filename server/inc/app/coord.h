
#ifndef __COORDINATOR_H__
#define __COORDINATOR_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "stm32f10x.h"
#include "network.h"

#define TIMEOUT             0x100000

#define SIZE_SINGLE_CMD     0x08//���ƿ��������
#define SIZE_ALL_PWM_CMD    0x07//����PWM����ĳ���
#define SIZE_SINGLE_PWM_CMD 0x09//��������PWM����ĳ���
#define SIZE_REQ_FULL_CMD   0x06//�����ϱ�ȫ����������ĳ���
    
#define SIZE_TIME_CMD       0x0B//len�ֶ�ֵ
#define SIZE_SYSTIME_CMD    0x0D//���������
#define SIZE_SYSTIME        0x07//�����ֶγ���
    
/*********************************************************
* ������      : void W108MsgHandlerTick ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void W108MsgHandler ( void );

/*********************************************************
* ������      : void GprsMsgHandlerTick ( void )
* ����        : �����ӷ������յ�������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void GprsMsgHandler ( void );

/*********************************************************
* ������      : TIME_SyschTimeServer
* ����        : ���ʱ��
* �������    : ʱ��
* �������    : 0x01:ʱ�䵽  0x00:ʱ��δ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void TIME_SyschTime( u8 * server_time );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

////////////////////////////////////////////////////////////////////////
#endif // __COORDINATOR_H__ 

