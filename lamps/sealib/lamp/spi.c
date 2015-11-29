#include "spi.h"
#include "serial.h"
#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : SPI_Configuration
//* 功能        : SPI初始化
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void SPI_Configuration ( void )
{
    if (network_info.type == EMBER_COORDINATOR)       // 网关-协调器  
    {
        SC2_MODE       = 0x02;                        // SC2使用SPI
        SC2_SPICFG    |= SC_SPIMST;                   // SPI主模式
        SC2_TWICTRL2  |= SC_TWIACK;
  
        SC2_RATEEXP   = 0x03;  // 0x01;                         // 0x03 时钟频率设置xMb/s  3 9, speed 100k
        SC2_RATELIN   = 0x09;                         // 0x09
  
        SC2_SPICFG   &= ~SC_SPIPOL_MASK;              // 帧格式选择
        SC2_SPICFG   &= ~SC_SPIPHA_MASK;
        SC2_SPICFG   &= ~SC_SPIORD_MASK;
        SC2_SPICFG   &= ~SC_SPIRPT_MASK;
        
        halGpioConfig(PORTA_PIN(0x00), 0x9u);         // 将PA0设为复用输出（推挽）
        halGpioConfig(PORTA_PIN(0x01), 0x8u);         // 将PA1设为输入(上拉)
        halGpioConfig(PORTA_PIN(0x02), 0xBu);         // 将PA2设为复用输出（推挽）特殊SCLK
        halGpioConfig(PORTA_PIN(0x03), 0x1u);         // PA3主模式下不用,这里设为输出
  
        GPIO_PASET_REG = (0x01 << PA1_BIT);           // PA1上拉 
  
        Debug("\r\n---spi cfg(co)---");
    }
    else if (network_info.type == EMBER_ROUTER)       // 网关-路由  
    {
        SC2_MODE       = 0x02;                        // SC2使用SPI
        SC2_SPICFG    &= ~SC_SPIMST_MASK;             // SPI从模式 
  
        SC2_SPICFG    &= ~SC_SPIPOL_MASK;             // 帧格式选择(从主设备获得)
        SC2_SPICFG    &= ~SC_SPIPHA_MASK;
        SC2_SPICFG    &= ~SC_SPIORD_MASK;
        SC2_SPICFG    &= ~SC_SPIRPT_MASK;
        
        halGpioConfig(PORTA_PIN(0x00), 0x8u);         // 将PA0设为输入（上拉）
        halGpioConfig(PORTA_PIN(0x01), 0x9u);         // 将PA1设为服用输出（推挽）
        halGpioConfig(PORTA_PIN(0x02), 0x8u);         // 将PA2设为输入
        halGpioConfig(PORTA_PIN(0x03), 0x8u);         // 将PA3设为输入SSEL
  
        GPIO_PASET_REG = (0x01 << PA0_BIT);           // PA0上拉 
        GPIO_PASET_REG = (0x01 << PA2_BIT);           // PA2上拉 
        GPIO_PACLR_REG = (0x01 << PA3_BIT);           // PA3下拉 
  
        Debug("\r\n---spi cfg(zr)---");
    }
//    Debug("\r\nSC2_MODE: 0x%x, SC2_SPICFG: 0x%x, SC2_TWICTRL2: 0x%x", (u16)(SC2_MODE), (u16)(SC2_SPICFG), (u16)(SC2_TWICTRL2));
    Debug("\r\nGPIO_PACFGL: 0x%x", GPIO_PACFGL);
//    Debug("\r\nGPIO_PAOUT: 0x%x, GPIO_PAIN: 0x%x", GPIO_PAOUT, GPIO_PAIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void spi_start( void )
//* 功能        : 开始SPI传输，SSEL引脚置低
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void spi_start ( void )
{
//    Debug("\r\nspi_start:GPIO_PAOUT is %x", GPIO_PAOUT_REG);
    GPIO_PACLR_REG = (0x01 << PA3_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void spi_end( void )
//* 功能        : 结束SPI传输，SSEL引脚置高
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void spi_end ( void )
{
//    Debug("\r\nspi end:GPIO_PAOUT is %x", GPIO_PAOUT_REG);
    GPIO_PASET_REG = (0x01 << PA3_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool spi_iselect ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool spi_iselect ( void )
{
    if (GPIO_PAOUT & 0x08)
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool spi_isnotempty ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : last function name : bool spi_isrxovf ( void )
//*------------------------------------------------*/
bool spi_isnotempty ( void )
{
//    if ((SC2_SPISTAT & SC_SPIRXOVF_MASK) >> SC_SPIRXOVF_BIT == 0x01)
    if (((SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 0x01))
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static bool spi_ready ( void )
//* 功能        : spi ready to read
//* 输入参数    : 无
//* 输出参数    : bool
//* 修改记录    : 无
//* 备注        : last function name : bool spi_isrxovf ( void )
//*------------------------------------------------*/
static bool spi_notready ( void )
{
    vu32 timeout = SPITIMEOUT;

//    while ((SC2_SPISTAT & SC_SPITXIDLE) != SC_SPITXIDLE && timeout --) ;
//    while ((SC2_SPISTAT & SC_SPIRXVAL) !=  SC_SPIRXVAL)   // Wait until the Rx FIFO buffer will become not empty 
    while ((SC2_SPISTAT & SC_SPIRXVAL) !=  SC_SPIRXVAL  && --timeout )   // Wait until the Rx FIFO buffer will become not empty 
        sea_feedwatchdog();
    
//    return (bool)(FALSE);
    Debug("timeout%8x",timeout);
    
    return (bool)(timeout == 0x00 ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 spi_writebyte ( u16 data )
//* 功能        : Write a byte over the SPI
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 spi_writebyte ( u8 data )
{
    SC2_DATA = data;
    if (spi_notready())
        return SPIEMPTY;
    return SC2_DATA;     // Read a data
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 spi_readbyte ( void )
//* 功能        : Read a byte over the SPI
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 spi_readbyte ( void )
{
    SC2_DATA = SPIEMPTY;                                                   // Write the 0xFF data 
    if (spi_notready())
    {
        return SPIEMPTY;
    }
    return SC2_DATA;                                                       // Read a data 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 spi_readwrite ( u8 write_data )
//* 功能        : Read and write a byte over the SPI
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 spi_readwrite ( u8 write_data )
{    
    SC2_DATA = write_data;                                                 // Write the 0xFF data 
    if (spi_notready())
        return SPIEMPTY;
    return SC2_DATA;                                                       // Read a data 
}

/////////////////////////////////////////////////////////////////////////////////////////////
