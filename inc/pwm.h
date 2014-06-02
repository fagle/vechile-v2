#ifndef __PWM_H__
#define __PWM_H__

#include "config.h"
#include "ember.h"
#include "hal.h"



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : PWM_Configuration
//* 功能        : PWM模块初始化
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void PWM_Configuration(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : adjustPWM
//* 功能        : 调整PWM输出值
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
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

