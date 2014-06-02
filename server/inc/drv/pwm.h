#ifndef __PWM_H__
#define __PWM_H__

#include "stm32f10x.h"

#define TIMPERIOD (35999)

#define CHENNEL1  (0x01)
#define CHENNEL2  (0x02)
#define CHENNEL3  (0x04)
#define CHENNEL4  (0x08)

////////////////////////////////////////////////////////////////////////////////
typedef struct 
{ 
   TIM_TypeDef  * timer;
   u16            period;

   GPIO_TypeDef * ch1_gpio_name; 
   u16            ch1_gpio_pin;

   GPIO_TypeDef * ch2_gpio_name; 
   u16            ch2_gpio_pin;

   GPIO_TypeDef * ch3_gpio_name; 
   u16            ch3_gpio_pin;

   GPIO_TypeDef * ch4_gpio_name; 
   u16            ch4_gpio_pin;
} time_info_t, *ptime_info_t;

typedef struct 
{
   u16   pwm1;
   u16   pwm2;
   u16   pwm3;
   u16   pwm4;
} pwm_info_t, *ppwm_info_t;

/*******************************************************************************
* Function Name  : void PWM_Configuration ( void )
* Description    : pwm gpio and time2 configuration.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM_Configuration ( void );

/*******************************************************************************
* Function Name  : void sea_adjustpwm ( u8 index, u16 pulse )
* Description    : adjust pwm output.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_adjustpwm ( u8 index, u16 pulse );

/*******************************************************************************
* Function Name  : u16 sea_getperiod ( void )
* Description    : get time period.
* Input          : None
* Output         : timer's period
* Return         : None
*******************************************************************************/
u16 sea_getperiod ( void );

#endif // __PWM_H__ 
////////////////////////////////////////////////////////////////////////////////
