#include "network.h"
#include "pwm.h"
#include "frame.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t   uartfrm;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : PWM_Configuration
//* 功能        : PWM模块初始化
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void PWM_Configuration ( void )
{
#ifdef RF_TEST
    halGpioConfig(PORTA_PIN(6), 0X8u);    // 将PA6设为输入（上拉）
    return;
#endif
  
#ifndef EMZ3118  
    TIM1_OR    = 0x00;                    // use 12MHz clock
    TIM1_PSC   = 0x01;                    // 预分频2^10
    TIM1_EGR   = 0x01;
    TIM1_CCMR2 = 0x00;
    //  TIM_REMAPC1=0;
    TIM1_CCMR2 = (0x6 << TIM_OC3M_BIT);   // 将tim1的通道3设为pwm1模式
    halGpioConfig(PORTA_PIN(6), 0X9);     // 将PA6设为推免输出
    init_PWM_outChannel(); 
    PowerSupply_SW_port_init();
    //PowerSupply_SW_off();
    PowerSupply_SW_on();

    enablePWM_outChannel();
    
    TIM1_ARR   = 1000;        // period
    TIM1_CCR3  = 10;          // 占空比
    TIM1_CNT   = 0x00;
    ATOMIC
    (
        TIM1_CCER = TIM_CC3E; // enable output on channel 1  BUZZER_OUTPUT_ENABLE TIM_CCER
        TIM1_CR1 |= TIM_CEN;  // enable counting
    ) 
    
#else
    TIM2_OR    = 0x00;    // use 12MHz clock
    TIM2_PSC   = 0x02;    // 预分频2^10
    TIM2_EGR   = 0x01;
    TIM2_CCMR2 = 0x00;
    //  TIM_REMAPC1=0;
    TIM2_CCMR2 = (0x6 << TIM_OC3M_BIT);  // 将tim2的通道1设为pwm1模式
    halGpioConfig(PORTA_PIN(1), 0X9);    // 将PA0设为推免输出
  
    TIM2_ARR   = 1000;   // period
    TIM2_CCR3  = 10;     // 占空比
    TIM2_CNT   = 0;
    ATOMIC
    (
        TIM2_CCER = TIM_CC3E; // enable output on channel 1  BUZZER_OUTPUT_ENABLE TIM_CCER
        TIM2_CR1 |= TIM_CEN;  // enable counting
    )       
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : adjustPWM
//* 功能        : 调整PWM输出值
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void adjustPWM ( int Pluse )
{
#ifndef  EMZ3118 
//    sea_uartsendstring("\r\n adjust pluse = ");
//    printd32(Pluse);
    Pluse = 1000 - Pluse;
    if(Pluse >= 0 && Pluse <= 1000)
        TIM1_CCR3 = Pluse;
#else 
    
    TIM2_CCR3 = Pluse;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : enablePWM_outChannel
//* 功能        : PWM输出通道开启
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : add by dolphi
//*------------------------------------------------*/
void enablePWM_outChannel(void)
{
  GPIO_PCSET |= PC6;
  GPIO_PCSET |= PC7;
}

void disalePWM_outChannel(void)
{
  GPIO_PCCLR |= PC6;
  GPIO_PCCLR |= PC7;
}

void init_PWM_outChannel(void)
{
    halGpioConfig(PORTC_PIN(7), 0x1);
    halGpioConfig(PORTC_PIN(6), 0x1);     // 将PC6、PC7设置为推挽输出，是PWM的两路选通信号
}

void PowerSupply_SW_port_init(void)
{
    halGpioConfig(PORTB_PIN(0), 0x1);
    halGpioConfig(PORTA_PIN(7), 0x1);     // 将PB0、PA7设置为推挽输出，两路开关控制信号
}

void PowerSupply_SW_on(void)
{
    GPIO_PBSET |= PB0;
    GPIO_PASET |= PA7;
}

void PowerSupply_SW1_on(void)
{
    GPIO_PBSET |= PB0;
    //GPIO_PACLR |= PA7;
}

void PowerSupply_SW2_on(void)
{
    //GPIO_PBCLR |= PB0;
    GPIO_PASET |= PA7;
}

void PowerSupply_SW_off(void)
{
     GPIO_PBCLR |= PB0;
     GPIO_PACLR |= PA7;
}

void PowerSupply_SW1_off(void)
{
     GPIO_PBCLR |= PB0;
     //GPIO_PASET |= PA7;
}

void PowerSupply_SW2_off(void)
{
     //GPIO_PBSET |= PB0;
     GPIO_PACLR |= PA7;
}