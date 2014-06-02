#ifndef __SPI_H__
#define __SPI_H__

////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

//下面的定义3.50与3.22的库不一样
//----------------------------------------------

#define SPITIMEOUT        0x8000000
#define SPInet_ReadWrite  SPI_ReadWriteByte


/*******************************************************************************
* Function Name  : void SPI1_Init ( void )
* Description    : spi1 configuration.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_Init ( void );

/*******************************************************************************
* Function Name  : void SPI2_Configuration ( void )
* Description    : spi2 configuration.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_Configuration ( void );

/*******************************************************************************
* Function Name  : u8 SPI_ReadWriteByte ( u8 TxData )
* Description    : SPI1/2读写一字节数据.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u8 SPI_ReadWriteByte ( u8 TxData );

////////////////////////////////////////////////////////////////////////////////////
//
#endif
