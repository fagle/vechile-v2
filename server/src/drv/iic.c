#include "FreeRTOS.h"
#include "task.h"

#include "stm32f10x.h" 
#include "config.h"
#include "iic.h"
#include "serial.h"
#include "systicker.h"
#include "network.h"

/************************************************
* ������      : void I2C_delay(void)
* ����        : ��ʱ����
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void I2C_delay ( void ) 
{ 
   for (u8 i = 0x00; i < 0x08; i ++)  // ��������Ż��ٶ� ����������͵�5����д�� 
   {
       nop();
       nop();
   }
} 

/************************************************
* ������      : bool I2C_Start(void)
* ����        : start iic
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
bool I2C_Start ( void ) 
{ 
    SDA_H; 
    SCL_H; 
    I2C_delay(); 
    if(!SDA_read)return FALSE; //SDA��Ϊ�͵�ƽ������æ,�˳� 
    SDA_L; 
    I2C_delay(); 
    if(SDA_read) return FALSE; //SDA��Ϊ�ߵ�ƽ�����߳���,�˳� 
    SDA_L; 
    I2C_delay(); 
    return TRUE; 
} 

/************************************************
* ������      : bool I2C_Stop(void)
* ����        : stop iic
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void I2C_Stop ( void ) 
{ 
    SCL_L; 
    I2C_delay(); 
    SDA_L; 
    I2C_delay(); 
    SCL_H; 
    I2C_delay(); 
    SDA_H; 
    I2C_delay(); 
} 

#if 0
/************************************************
* ������      : bool I2C_Ack(void)
* ����        : ACK
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void I2C_Ack ( void ) 
{ 
    SCL_L; 
    I2C_delay(); 
    SDA_L; 
    I2C_delay(); 
    SCL_H; 
    I2C_delay(); 
    SCL_L; 
    I2C_delay(); 
} 

/************************************************
* ������      : I2C_NoAck(void)
* ����        : I2C_NoAck
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void I2C_NoAck ( void ) 
{ 
    SCL_L; 
    I2C_delay(); 
    SDA_H; 
    I2C_delay(); 
    SCL_H; 
    I2C_delay(); 
    SCL_L; 
    I2C_delay(); 
} 
#endif

/************************************************
* ������      : I2C_WaitAck(void)
* ����        : WaitAck
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static bool I2C_WaitAck ( void )   //����Ϊ:=1��ACK,=0��ACK 
{ 
    SCL_L; 
    I2C_delay(); 
    SDA_H; 
    I2C_delay(); 
    SCL_H; 
    I2C_delay(); 
    SCL_L; 
    return (SDA_read) ? FALSE : TRUE; 
} 

/************************************************
* ������      : void I2C_SendByte(u8 SendByte)
* ����        : SendByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void I2C_SendByte ( u8 SendByte ) //���ݴӸ�λ����λ// 
{ 
    u8 i = 0x08; 
    
    while (i --) 
    { 
        SCL_L; 
        I2C_delay(); 
        (SendByte & 0x80) ? (SDA_H) : (SDA_L);    
        SendByte <<= 0x01; 
        I2C_delay(); 
        SCL_H; 
        I2C_delay(); 
    } 
    SCL_L; 
} 

/************************************************
* ������      : u8 I2C_ReceiveByte(void)
* ����        : ReceiveByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static u8 I2C_ReceiveByte ( void )  //���ݴӸ�λ����λ// 
{  
    u8 i   = 0x08; 
    u8 rec = 0x00; 
    
    SDA_H; 
    while (i--) 
    { 
        rec <<= 0x01;       
        SCL_L; 
        I2C_delay(); 
        SCL_H; 
        I2C_delay(); 
        if (SDA_read) 
            rec |= 0x01; 
    } 
    SCL_L; 
    return rec; 
} 

/************************************************
* ������      : void Write_PCF8365T(u8 Address, u8 Value) 
* ����        : ReceiveByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void Write_PCF8365T ( u8 address, u8 value ) 
{ 
    value = Hex2Bcd(value);
    I2C_Start();               // ������ 
    I2C_SendByte(0xa2);        // ������ַ,���λ0Ϊд���� 
    if (!I2C_WaitAck())
        return;
    I2C_SendByte(address);     // д��Ԫ��ַ 
    if (!I2C_WaitAck())
        return;
    I2C_SendByte(value);       // д���� 
    if (!I2C_WaitAck())
        return;                // �ȴ�Ӧ�� 
    I2C_Stop();                // ֹͣ���� 
} 

/************************************************
* ������      : void Read_PCF8365T(u8 Address, u8 Value) 
* ����        : ReceiveByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8  Read_PCF8365T ( u8 Address ) 
{ 
    u8  Temp; 
    
    I2C_Start();               // �������� 
    I2C_SendByte(0xA2);        // ������ַ,���λ0Ϊд���� 
    if (!I2C_WaitAck())        // �ȴ�Ӧ��  
        return 0x00;
    I2C_SendByte(Address);     // ��Ԫ��ַ 
    I2C_WaitAck();             // �ȴ�Ӧ�� 
    I2C_Start();               // �������� 
    I2C_SendByte(0xA3);        // AT24C02��ַ,���λ1������ 
    I2C_WaitAck();             // �ȴ�Ӧ�� 
    
    Temp = I2C_ReceiveByte();  // ������ 
    
    I2C_delay();        
    SDA_H; 
    SCL_L; 
    I2C_delay(); 
    I2C_Stop();                // ֹͣ���� 
    switch (Address)
    {
        case PCF8365T_Address_Second:
            Temp &= 0x7F;
            break;
        case PCF8365T_Address_Minute:
            Temp &= 0x7F;
            break;
        case PCF8365T_Address_Hour:
            Temp &= 0x3F;
            break;
        case PCF8365T_Address_Day:
            Temp &= 0x3F;
            break;
        case PCF8365T_Address_Weekday:
            Temp &= 0x07;
            break;
        case PCF8365T_Address_Month:
            Temp &= 0x1F;
            break;
        case PCF8365T_Address_Year:
            Temp &= 0xFF;
            break;
    }
    return Bcd2Hex(Temp);
} 

/************************************************
* ������      : void PCF8365T_ReadTime ( systime_t * time )
* ����        : data_time * time
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_ReadDate ( systime_t * time )
{
#ifdef PCF8365T_ENABLE 
    taskENTER_CRITICAL();
    {
        time->year   = 2000 + Read_PCF8365T(PCF8365T_Address_Year);
        time->mon    = Read_PCF8365T(PCF8365T_Address_Month);
        time->day    = Read_PCF8365T(PCF8365T_Address_Day);
        time->hour   = Read_PCF8365T(PCF8365T_Address_Hour);
        time->min    = Read_PCF8365T(PCF8365T_Address_Minute);
        time->sec    = Read_PCF8365T(PCF8365T_Address_Second);
    }
    taskEXIT_CRITICAL();
#else
#endif
}

/************************************************
* ������      : void PCF8365T_WriteDate ( systime_t time )
* ����        : write date time
* �������    : data_time time
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_WriteDate ( systime_t time )
{
#ifdef PCF8365T_ENABLE 
    taskENTER_CRITICAL();
    {
        Write_PCF8365T(PCF8365T_Address_Year, time.year % 100);     
        Write_PCF8365T(PCF8365T_Address_Month, time.mon);    
        Write_PCF8365T(PCF8365T_Address_Day, time.day);  
        Write_PCF8365T(PCF8365T_Address_Hour, time.hour);     
        Write_PCF8365T(PCF8365T_Address_Minute, time.min);    
        Write_PCF8365T(PCF8365T_Address_Second, time.sec);    
    
        Write_PCF8365T(0x00, 0x10);     //��ͨģʽ��оƬʱ�����С���Դ��λ����ʧЧ 
    }
    taskEXIT_CRITICAL();
#else
#endif    
}

/*********************************************************
* ������      : void PCF8365T_SyschTime ( u8 * time )
* ����        : ���ʱ��
* �������    : ʱ��
* �������    : 0x01:ʱ�䵽  0x00:ʱ��δ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void PCF8365T_SyschTime ( u8 * time )
{
    systime_t dt;    // 07-DC-05-18-0F-27-30
    
    dt.year = (time[0x00] * 0xff) * 0x0100 + time[0x01] & 0xff;
    dt.mon  = time[0x02] & 0xff;
    dt.day  = time[0x03] & 0xff;
    dt.hour = time[0x04] & 0xff;
    dt.min  = time[0x05] & 0xff;
    dt.sec  = time[0x06] & 0xff;
    if (dt.year < 2012 || dt.mon > 12 || dt.day > 31 || dt.hour > 23 || dt.min > 59 || dt.sec > 59)
    {
        sea_printf("\nserver date and time errors!");
        return;
    }
    
#ifdef PCF8365T_ENABLE 
    PCF8365T_WriteDate(dt);
    PCF8365T_ReadDate(sea_getsystime());
    sea_printf("\nSyc. server date %d-%d-%d, time %d:%d:%d.", sea_getsystime()->year,
                                                              sea_getsystime()->mon,
                                                              sea_getsystime()->day,
                                                              sea_getsystime()->hour,
                                                              sea_getsystime()->min,
                                                              sea_getsystime()->sec);
#else
    sea_getsystime()->year = dt.year;
    sea_getsystime()->mon  = dt.mon;
    sea_getsystime()->day  = dt.day;
    sea_getsystime()->hour = dt.hour;
    sea_getsystime()->min  = dt.min;
    sea_getsystime()->sec  = dt.sec;
#endif    
}

