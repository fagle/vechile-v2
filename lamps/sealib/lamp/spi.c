#include "spi.h"
#include "serial.h"
#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : SPI_Configuration
//* ����        : SPI��ʼ��
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void SPI_Configuration ( void )
{
    if (network_info.type == EMBER_COORDINATOR)       // ����-Э����  
    {
        SC2_MODE       = 0x02;                        // SC2ʹ��SPI
        SC2_SPICFG    |= SC_SPIMST;                   // SPI��ģʽ
        SC2_TWICTRL2  |= SC_TWIACK;
  
        SC2_RATEEXP   = 0x03;  // 0x01;                         // 0x03 ʱ��Ƶ������xMb/s  3 9, speed 100k
        SC2_RATELIN   = 0x09;                         // 0x09
  
        SC2_SPICFG   &= ~SC_SPIPOL_MASK;              // ֡��ʽѡ��
        SC2_SPICFG   &= ~SC_SPIPHA_MASK;
        SC2_SPICFG   &= ~SC_SPIORD_MASK;
        SC2_SPICFG   &= ~SC_SPIRPT_MASK;
        
        halGpioConfig(PORTA_PIN(0x00), 0x9u);         // ��PA0��Ϊ������������죩
        halGpioConfig(PORTA_PIN(0x01), 0x8u);         // ��PA1��Ϊ����(����)
        halGpioConfig(PORTA_PIN(0x02), 0xBu);         // ��PA2��Ϊ������������죩����SCLK
        halGpioConfig(PORTA_PIN(0x03), 0x1u);         // PA3��ģʽ�²���,������Ϊ���
  
        GPIO_PASET_REG = (0x01 << PA1_BIT);           // PA1���� 
  
        Debug("\r\n---spi cfg(co)---");
    }
    else if (network_info.type == EMBER_ROUTER)       // ����-·��  
    {
        SC2_MODE       = 0x02;                        // SC2ʹ��SPI
        SC2_SPICFG    &= ~SC_SPIMST_MASK;             // SPI��ģʽ 
  
        SC2_SPICFG    &= ~SC_SPIPOL_MASK;             // ֡��ʽѡ��(�����豸���)
        SC2_SPICFG    &= ~SC_SPIPHA_MASK;
        SC2_SPICFG    &= ~SC_SPIORD_MASK;
        SC2_SPICFG    &= ~SC_SPIRPT_MASK;
        
        halGpioConfig(PORTA_PIN(0x00), 0x8u);         // ��PA0��Ϊ���루������
        halGpioConfig(PORTA_PIN(0x01), 0x9u);         // ��PA1��Ϊ������������죩
        halGpioConfig(PORTA_PIN(0x02), 0x8u);         // ��PA2��Ϊ����
        halGpioConfig(PORTA_PIN(0x03), 0x8u);         // ��PA3��Ϊ����SSEL
  
        GPIO_PASET_REG = (0x01 << PA0_BIT);           // PA0���� 
        GPIO_PASET_REG = (0x01 << PA2_BIT);           // PA2���� 
        GPIO_PACLR_REG = (0x01 << PA3_BIT);           // PA3���� 
  
        Debug("\r\n---spi cfg(zr)---");
    }
//    Debug("\r\nSC2_MODE: 0x%x, SC2_SPICFG: 0x%x, SC2_TWICTRL2: 0x%x", (u16)(SC2_MODE), (u16)(SC2_SPICFG), (u16)(SC2_TWICTRL2));
    Debug("\r\nGPIO_PACFGL: 0x%x", GPIO_PACFGL);
//    Debug("\r\nGPIO_PAOUT: 0x%x, GPIO_PAIN: 0x%x", GPIO_PAOUT, GPIO_PAIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void spi_start( void )
//* ����        : ��ʼSPI���䣬SSEL�����õ�
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void spi_start ( void )
{
//    Debug("\r\nspi_start:GPIO_PAOUT is %x", GPIO_PAOUT_REG);
    GPIO_PACLR_REG = (0x01 << PA3_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void spi_end( void )
//* ����        : ����SPI���䣬SSEL�����ø�
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void spi_end ( void )
{
//    Debug("\r\nspi end:GPIO_PAOUT is %x", GPIO_PAOUT_REG);
    GPIO_PASET_REG = (0x01 << PA3_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bool spi_iselect ( void )
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
bool spi_iselect ( void )
{
    if (GPIO_PAOUT & 0x08)
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bool spi_isnotempty ( void )
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : last function name : bool spi_isrxovf ( void )
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
//* ������      : static bool spi_ready ( void )
//* ����        : spi ready to read
//* �������    : ��
//* �������    : bool
//* �޸ļ�¼    : ��
//* ��ע        : last function name : bool spi_isrxovf ( void )
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
//* ������      : u8 spi_writebyte ( u16 data )
//* ����        : Write a byte over the SPI
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : u16 spi_readbyte ( void )
//* ����        : Read a byte over the SPI
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : u8 spi_readwrite ( u8 write_data )
//* ����        : Read and write a byte over the SPI
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 spi_readwrite ( u8 write_data )
{    
    SC2_DATA = write_data;                                                 // Write the 0xFF data 
    if (spi_notready())
        return SPIEMPTY;
    return SC2_DATA;                                                       // Read a data 
}

/////////////////////////////////////////////////////////////////////////////////////////////
