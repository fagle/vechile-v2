#ifndef __PWM_H__
#define __PWM_H__

#include "config.h"
#include "ember.h"
#include "hal.h"



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : PWM_Configuration
//* ����        : PWMģ���ʼ��
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void PWM_Configuration(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : adjustPWM
//* ����        : ����PWM���ֵ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void adjustPWM(int Pluse);

void enablePWM_outChannel(void);

void disalePWM_outChannel(void);

void init_PWM_outChannel(void);

void PowerSupply_SW_port_init(void); 
void PowerSupply_SW_on(void);
void PowerSupply_SW1_on(void);
void PowerSupply_SW2_on(void);
void PowerSupply_SW_off(void);
void PowerSupply_SW1_off(void);
void PowerSupply_SW2_off(void);
#endif

