#ifndef __LED_H__
#define __LED_H__

///////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

#define LED_LIGHT    (0x01)
#define LED_POWER    (0x02)
#define LED_OTHER    (0x04)

#define LEDSIZE      (0x05)
#define LEDLIGHT     (0x10)

#define LEDIDEL      (0x00)
#define LEDTIME      (0x01)
#define LEDMENU      (0x02)
#define LEDLAMP      (0x03)
#define LEDSET       (0x04)
#define LEDPWOFF     (0x05)
#define LEDDOT       (0xf8)

#define STATEMASK    (0x07)

///////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8  state;
    u8  index;
    u8  time;
    u8  digital[LEDSIZE];
} led_info_t, *pled_info_t;

/*******************************************************************************
* Function Name  : void LED_Configuration ( void )
* Description    : Init/Configure the GPIO x led
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Configuration ( void );

/*******************************************************************************
* Function Name  : LED_Display ( void )
* Description    : display a string
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Display ( void );

/*******************************************************************************
* Function Name  : LED_Clear ( void )
* Description    : display a string
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Clear ( void );

/*******************************************************************************
* Function Name  : void LED_SetState ( u8 state )
* Description    : display a string
* Input          : u8 state
* Output         : None
* Return         : None
*******************************************************************************/
void LED_SetState ( u8 state );

/*******************************************************************************
* Function Name  : u8 LED_GetState ( void )
* Description    : display a string
* Input          : None
* Output         : led display state
* Return         : None
*******************************************************************************/
u8 LED_GetState ( void );

/*******************************************************************************
* Function Name  : u8 * LED_GetBuffer ( void )
* Description    : display a string
* Input          : None
* Output         : led display buffer
* Return         : None
*******************************************************************************/
u8 * LED_GetBuffer ( void );

/*******************************************************************************
* Function Name  : void LED_SetCursor ( u8 cursor )
* Description    : display cursor
* Input          : u8 cursor
* Output         : None
* Return         : None
*******************************************************************************/
void LED_SetCursor ( u8 cursor );

/*******************************************************************************
* Function Name  : void  sea_leddisplay ( void )
* Description    : display a string
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  sea_leddisplay ( void );

/*******************************************************************************
* Function Name  : void  sea_setledbuffer ( void )
* Description    : display a string
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  sea_setledbuffer ( void );

/************************************************
* 函数名      : void sea_setledoff ( void )
* 功能        : power off
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_setledoff ( void );

/************************************************
* 函数名      : void sea_setledon ( void )
* 功能        : 延时
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_setledon ( void );

///////////////////////////////////////////////////////////////////////////////////
//
extern led_info_t  led_info;

///////////////////////////////////////////////////////////////////////////////////
//
#endif // __LED_H__ 


