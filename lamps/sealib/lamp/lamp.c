#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// 常量和全局变量
const systime_t  def_time = { 2013, 0x0c, 0x10, 0x00, 0x00, 0x00, 0x00L};
single_t single_info; 

/////////////////////////////////////////////////////////////////////////////////////////////
//ray:以下是本地化代码
//start:开/关灯、更新时间、更新灯号、调整输出百分比
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_openlamp ( void )
//* 功能        : 路由实现开灯功能;开完后要修改自己的数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_openlamp ( void * ptr )
{
    if (single_info.lamp.light.status == LAMP_OPEN)
        return;
  
    single_info.lamp.light.percent = ptr == NULL ? MAXPERCENT : *((u8 *)ptr);
    single_info.lamp.light.status = LAMP_OPEN;
    sea_updatepercent(single_info.lamp.light.percent);       //  GPIO_PACLR_REG = (1<<PA4_BIT);  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_closelamp ( void )
//* 功能        : 路由实现关灯功能
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_closelamp ( void * ptr )
{
    if (single_info.lamp.light.status == LAMP_CLOSE)
        return;
    
    single_info.lamp.light.percent = ptr == NULL ? 0x00 : *((u8 *)ptr);
    single_info.lamp.light.status  = LAMP_CLOSE;
    sea_updatepercent(single_info.lamp.light.percent);       //  GPIO_PACLR_REG = (1<<PA4_BIT);  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_togglelamp ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_togglelamp ( void * ptr )
{
    (single_info.lamp.light.status == LAMP_CLOSE) ? (sea_openlamp(ptr)) : (sea_closelamp(ptr)); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatetime ( psystime_t tm )
//* 功能        : update time of lamp
//* 输入参数    : psystime_t ptm
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatetime ( psystime_t ptm )
{
    sea_memcpy(&single_info.time, ptm, sizeof(systime_t));
    
    if (single_info.fail(GETFAILBIT, LAMP_ER_TM))
        single_info.fail(CLRFAILBIT, LAMP_ER_TM); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatepercent ( u8 percent )
//* 功能        : update output percent of lamp
//* 输入参数    : u8 percent
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatepercent ( u8 percent )
{
    single_info.lamp.light.percent = (percent >= MAXPERCENT) ? (MAXPERCENT) : (percent <= MINPERCENT ? (0x00) : (percent));
    adjustPWM(single_info.lamp.light.percent * 10);
    set_lampmode(LAMP_CHANGE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void init_singleinfo ( void )
//* 功能        : 初始化本地的状态信息
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void init_singleinfo ( void )
{
    sea_memset(&single_info, 0x00, sizeof(single_t));
    if (network_info.type == EMBER_COORDINATOR && network_info.devid == COORDID)
        sea_memcpy(&single_info.time, (void *)&def_time, sizeof(systime_t));
    else
        sea_memset(&single_info.time, 0x00, sizeof(systime_t));
    
    single_info.avail                        = 0x06;    
    single_info.address                      = 0x01;                        // 短地址    
    single_info.ctrl                         = 0x00;                        // control mode
    single_info.min                          = sys_info.ctrl.period;  
    single_info.sec                          = sys_info.ctrl.period;  
    single_info.count                        = 0x00;
    single_info.retry                        = 0x00;
    single_info.on                           = sea_openlamp;
    single_info.off                          = sea_closelamp;
    single_info.toggle                       = sea_togglelamp;
    
    single_info.lamp.light.manual            = TRUE;                        // 初始化为手动状态
    single_info.lamp.light.percent           = MAXPERCENT;
    single_info.lamp.light.cost              = 0x10;                        // 每小时耗1.2度电
    single_info.lamp.light.factor            = 0x08;
    single_info.lamp.light.voltage.in        = 200;                         // 电压值      
    single_info.lamp.light.voltage.out       = 200;
    single_info.lamp.light.current.in        = 0x0c;                        // 电流值
    single_info.lamp.light.current.out       = 0x0c;
    single_info.lamp.light.temperature.light = 0x36;
    single_info.lamp.light.temperature.trans = 0x0c;
    single_info.lamp.light.temperature.env   = 0x08;
    single_info.lamp.light.temperature.lux   = 0x30;
    single_info.lamp.light.fail              = LAMP_ER_NO;                  // no errors 
    single_info.lamp.light.number            = sys_info.dev.num;            // read from flash
}

/////////////////////////////////////////////////////////////////////////////////////

