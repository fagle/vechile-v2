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
* ������      : void sea_setledoff ( void )
* ����        : power off
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_setledoff ( void );

/************************************************
* ������      : void sea_setledon ( void )
* ����        : ��ʱ
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_setledon ( void );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

#endif  // __GPIO_H__
////////////////////////////////////////////////////////////////////////////////
