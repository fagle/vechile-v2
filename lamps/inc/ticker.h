
#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "ember.h"
#include "hal.h"
#include "serial.h"
#include "security.h"
  
#ifdef SINK_APP
#define ONEMICROSEN//HUDMICROSEN
#else
#define ONEMICROSEN
#endif

//////////////////////////////////////////////////////////////////////////////////////
//
#ifdef  ONEMICROSEN
#define PRESCALER       (0x0f)     // CK_CNT =  PCLK/4096 = 12 MHz/4096 = 2929.6875 Hz
#define TICKCNT         (0xffff)   // SysTick interrupt each 1000 Hz with HCLK equal to 72MHz
//#define ONESECOND       (15000)
#define ONESECOND       (1000)
#endif

#ifdef  TENMICROSEN
#define TICKCNT         (90000)  // SysTick interrupt each 100 Hz with HCLK equal to 72MHz
#define ONESECOND       (100)
#endif
  
#ifdef  HUDMICROSEN
#define TICKCNT         (900000) // SysTick interrupt each 10 Hz with HCLK equal to 72MHz
#define ONESECOND       (10)
#endif

#define MAXTIMEVENT     (0x02)
  
///////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u16  year;
    u8   mon;
    u8   day;
    u8   hour;
    u8   min;
    u8   sec;
    u32  tick;
} systime_t, *psystime_t;

//////////////////////////////////////////////////////////////////////////////////////
//
typedef void (*TMHANDLE)(void);
typedef struct
{
    u16      time;
    TMHANDLE handle;
} time_event_info, *ptime_event_info;

/*******************************************************************************
* Function Name  : TICK_Configuration ( void )
* Description    : Configures System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TICK_Configuration  ( void );

/*******************************************************************************
* Function Name  : millTickHook
* Description    : 毫秒级事件处理 + 计时（偏慢一点点）
* Input          : None
* Output         : 
* Return         : None
*******************************************************************************/
void millTickHook ( void );

/*******************************************************************************
* Function Name  : secTickHook ( void )
* Description    : 秒级事件处理程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void secTickHook ( void );

/*******************************************************************************
* Function Name  : minTickHook ( void )
* Description    : 分钟级事件处理程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void minTickHook(void);

/*******************************************************************************
* Function Name  : permitJoinHook ( void )
* Description    : 协调器允许加入网络
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void permitJoinHook(void);

/*******************************************************************************
* Function Name  : void sea_delayms ( u32 time )
* Description    : Inserts a delay time.
* Input          : time: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_delayms ( u32 time );

/*******************************************************************************
* Function Name  : void sea_getsystime ( systime_t * time )
* Description    : Inserts a delay time.
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_getsystime  ( systime_t * time );

/*******************************************************************************
* Function Name  : u32  sea_getinterval ( systime_t time )
* Description    : Inserts a delay time. 计算经过了多长的时间间隔；
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
u32  sea_getinterval ( systime_t time );

/*******************************************************************************
* Function Name  : void sea_setimevent ( u8 index, u16 time, TMHANDLE handle )
* Description    : set time event and function
* Input          : u16 time, TMHANDLE handle
* Output         : None
* Return         : None
*******************************************************************************/
//void sea_setimevent ( u8 index, u16 time, TMHANDLE handle );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

////////////////////////////////////////////////////////////////////////
#endif // __SYSTICK_H__ 

