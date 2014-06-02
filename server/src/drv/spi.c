#include "config.h"
#include "SPI.h"

/*******************************************************************************
* Function Name  : u8 SPI_ReadWriteByte ( u8 TxData )
* Description    : SPI2读写一字节数据.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u8 SPI_ReadWriteByte ( u8 TxData )
{
    u32 timeout = SPITIMEOUT;
    
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET && timeout --) ;   // Loop while DR register in not emplty 
    if (!timeout)
        return 0x00;
    SPI_I2S_SendData(SPI2, TxData);                                    // Send byte through the SPI2 peripheral 
    timeout = SPITIMEOUT;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET && timeout --) ;  // Wait to receive a byte 
    if (!timeout)
        return 0x00;
    return SPI_I2S_ReceiveData(SPI2);                                  // Return the byte read from the SPI bus 
}

#if 0
/*******************************************************************************
* Function Name  : void SPI2_Configuration ( void )
* Description    : spi2 configuration.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_Configuration ( void )
{
   SPI_InitTypeDef SPI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;

   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     // SPI2 Periph clock enable 
   //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);      //main.c里面做过了 若移植请加上
  
   //ENC28J60的相关管脚
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12; 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
   
   GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  // Configure SPI2 pins: NSS, SCK, MISO and MOSI 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
  
   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      // SPI2 configuration 
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;

   SPI_Init(SPI2, &SPI_InitStructure);
   SPI_Cmd(SPI2, ENABLE);                             // Enable SPI2 
}

#endif

///////////////////////////////////////////////////////////////////////////////////////

