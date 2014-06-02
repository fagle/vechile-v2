/////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "stm32f10x.h"

#define LAMPALLON               (0x3f00)
#define LAMPALLOFF              (0xc0ff)
#define FIRSTPIN                (0x08)
  
/*******************************************************************************
* Function Name  : void GPIO_Configuration ( void )
* Description    : Init/Configure the GPIOx for landscape
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration ( void );

/*******************************************************************************
* Function Name  : void sea_openlandscape ( u8 num )
* Description    : open landscape
* Input          : u8 num
* Output         : None
* Return         : None
*******************************************************************************/
void sea_openlandscape ( u8 num );

/*******************************************************************************
* Function Name  : void sea_closelandscape ( u8 num )
* Description    : close landscape
* Input          : u8 num
* Output         : None
* Return         : None
*******************************************************************************/
void sea_closelandscape ( u8 num );

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

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

#endif  // __GPIO_H__
////////////////////////////////////////////////////////////////////////////////
