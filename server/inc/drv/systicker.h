
#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "stm32f10x.h"

#define ONEMICROSEN   // TENMICROSEN    // ONEMICROSEN
  
//////////////////////////////////////////////////////////////////////////////////////
//
#ifdef  ONEMICROSEN
#define TICKCNT         (9000)   // SysTick interrupt each 1000 Hz with HCLK equal to 72MHz
#define ONESECOND       (1000)
#define KEYSCANTIME     (250)
#define GPRSENDTIME     (2000)
#endif

#ifdef  TENMICROSEN
#define TICKCNT         (90000)  // SysTick interrupt each 100 Hz with HCLK equal to 72MHz
#define ONESECOND       (100)
#define KEYSCANTIME     (20)
#endif
  
#ifdef  HUDMICROSEN
#define TICKCNT         (900000) // SysTick interrupt each 10 Hz with HCLK equal to 72MHz
#define ONESECOND       (10)
#define KEYSCANTIME     (2)
#endif

#define MAXTIMEVENT     (0x02)
  
///////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8 hour;
    u8 min;
    u8 sec;
} time_t, *ptime_t;

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
    u32      tick;
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
* Function Name  : TimingTickHook
* Description    : Decrements the TimingDelay variable.
* Input          : None
* Output         : TimingDelay
* Return         : None
*******************************************************************************/
void TimingTickHook      ( void );

/*******************************************************************************
* Function Name  : void sea_delayms ( u32 time )
* Description    : Inserts a delay time.
* Input          : time: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_delayms ( u32 time );

/*******************************************************************************
* Function Name  : psystime_t sea_getsystime  ( void )
* Description    : Inserts a delay time.
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
psystime_t sea_getsystime  ( void );

/*******************************************************************************
* Function Name  : void sea_setimevent ( u8 index, u16 time, TMHANDLE handle )
* Description    : set time event and function
* Input          : u16 time, TMHANDLE handle
* Output         : None
* Return         : None
*******************************************************************************/
void sea_setimevent ( u8 index, u16 time, TMHANDLE handle );

/*******************************************************************************
* Function Name  : void TimingTickHook ( void )
* Description    : Inserts a delay time. ���㾭���˶೤��ʱ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TimingTickHook ( void );

/*******************************************************************************
* Function Name  : void TICK_Disabled ( void )
* Description    : disable System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TICK_Disable ( void );

/*******************************************************************************
* Function Name  : void TICK_Enable ( void )
* Description    : enable System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TICK_Enable ( void );

/*******************************************************************************
* Function Name  : u32 sea_getms ( void )
* Description    : enable System Ticker
* Input          : None
* Output         : micro_second
* Return         : None
*******************************************************************************/
u32 sea_getms ( void );

/*******************************************************************************
* Function Name  : u32  sea_getinterval ( systime_t time )
* Description    : Inserts a delay time. ���㾭���˶೤��ʱ������
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
u32  sea_getinterval ( systime_t time );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u32 sea_caltime ( const time_t * tm )
//* ����        : calaculate time to second
//* �������    : const time_t * tm
//* �������    : None
//* �޸ļ�¼    : None
//* ��ע        : None
//*------------------------------------------------*/
u32 sea_caltime ( const time_t * tm );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : s8 sea_timecmp( const time_t * t1, const time_t * t2 )
//* ����        : compare time difference
//* �������    : const time_t * t1, const time_t * t2
//* �������    : None
//* �޸ļ�¼    : None
//* ��ע        : None
//*------------------------------------------------*/
s8 sea_timecmp ( const time_t * t1, const time_t * t2 );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : s8 sea_datecmp ( const systime_t * t1, const systime_t * t2 )
//* ����        : compare date and time difference
//* �������    : systime_t * t1, systime_t * t2
//* �������    : None
//* �޸ļ�¼    : None
//* ��ע        : None
//*------------------------------------------------*/
s8 sea_datecmp ( const systime_t * t1, const systime_t * t2 );

////////////////////////////////////////////////////////////////////////
//
extern systime_t systime;

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

////////////////////////////////////////////////////////////////////////
#endif // __SYSTICK_H__ 

