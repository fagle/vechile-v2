#include "FreeRTOS.h"
#include "task.h"

#include "stm32f10x.h" 
#include "config.h"
#include "iic.h"
#include "serial.h"
#include "systicker.h"
#include "network.h"

/************************************************
* 函数名      : void I2C_delay(void)
* 功能        : 延时函数
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void I2C_delay ( void ) 
{ 
   for (u8 i = 0x00; i < 0x08; i ++)  // 这里可以优化速度 ，经测试最低到5还能写入 
   {
       nop();
       nop();
   }
} 

/************************************************
* 函数名      : bool I2C_Start(void)
* 功能        : start iic
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
bool I2C_Start ( void ) 
{ 
    SDA_H; 
    SCL_H; 
    I2C_delay(); 
    if(!SDA_read)return FALSE; //SDA线为低电平则总线忙,退出 
    SDA_L; 
    I2C_delay(); 
    if(SDA_read) return FALSE; //SDA线为高电平则总线出错,退出 
    SDA_L; 
    I2C_delay(); 
    return TRUE; 
} 

/************************************************
* 函数名      : bool I2C_Stop(void)
* 功能        : stop iic
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : bool I2C_Ack(void)
* 功能        : ACK
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : I2C_NoAck(void)
* 功能        : I2C_NoAck
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : I2C_WaitAck(void)
* 功能        : WaitAck
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static bool I2C_WaitAck ( void )   //返回为:=1有ACK,=0无ACK 
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
* 函数名      : void I2C_SendByte(u8 SendByte)
* 功能        : SendByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void I2C_SendByte ( u8 SendByte ) //数据从高位到低位// 
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
* 函数名      : u8 I2C_ReceiveByte(void)
* 功能        : ReceiveByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static u8 I2C_ReceiveByte ( void )  //数据从高位到低位// 
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
* 函数名      : void Write_PCF8365T(u8 Address, u8 Value) 
* 功能        : ReceiveByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void Write_PCF8365T ( u8 address, u8 value ) 
{ 
    value = Hex2Bcd(value);
    I2C_Start();               // 起动总线 
    I2C_SendByte(0xa2);        // 器件地址,最低位0为写操作 
    if (!I2C_WaitAck())
        return;
    I2C_SendByte(address);     // 写单元地址 
    if (!I2C_WaitAck())
        return;
    I2C_SendByte(value);       // 写数据 
    if (!I2C_WaitAck())
        return;                // 等待应答 
    I2C_Stop();                // 停止总线 
} 

/************************************************
* 函数名      : void Read_PCF8365T(u8 Address, u8 Value) 
* 功能        : ReceiveByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8  Read_PCF8365T ( u8 Address ) 
{ 
    u8  Temp; 
    
    I2C_Start();               // 启动总线 
    I2C_SendByte(0xA2);        // 器件地址,最低位0为写操作 
    if (!I2C_WaitAck())        // 等待应答  
        return 0x00;
    I2C_SendByte(Address);     // 单元地址 
    I2C_WaitAck();             // 等待应答 
    I2C_Start();               // 重启总线 
    I2C_SendByte(0xA3);        // AT24C02地址,最低位1读操作 
    I2C_WaitAck();             // 等待应答 
    
    Temp = I2C_ReceiveByte();  // 读数据 
    
    I2C_delay();        
    SDA_H; 
    SCL_L; 
    I2C_delay(); 
    I2C_Stop();                // 停止总线 
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
* 函数名      : void PCF8365T_ReadTime ( systime_t * time )
* 功能        : data_time * time
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : void PCF8365T_WriteDate ( systime_t time )
* 功能        : write date time
* 输入参数    : data_time time
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
    
        Write_PCF8365T(0x00, 0x10);     //普通模式、芯片时钟运行、电源复位功能失效 
    }
    taskEXIT_CRITICAL();
#else
#endif    
}

/*********************************************************
* 函数名      : void PCF8365T_SyschTime ( u8 * time )
* 功能        : 检测时间
* 输入参数    : 时间
* 输出参数    : 0x01:时间到  0x00:时间未到
* 修改记录    : 无
* 备注        : 无
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

