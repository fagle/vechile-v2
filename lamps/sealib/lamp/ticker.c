
#include "network.h"
#include "lamp.h"

static systime_t   *  systime = NULL;

/*******************************************************************************
* Function Name  : TICK_Configuration ( void )
* Description    : Configures System Ticker
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void TICK_Configuration ( void )
{
    sea_memset(&single_info.time, 0x00, sizeof(systime_t));
    systime = &single_info.time;
    
    //tickEvent.status = EMBER_EVENT_MS_TIME;             // EMBER_EVENT_MS_TIME;
    emberEventControlSetActive(millTickEvent);   
    emberEventControlSetActive(secTickEvent);              // emberEventControlSetActive(minTickEvent);
    //emberEventControlSetActive(permitJoinEvent);
}


/*******************************************************************************
* Function Name  : millTickHook
* Description    : 毫秒级事件处理 + 计时（偏慢一点点），循环
* Input          : None
* Output         : 
* Return         : None
*******************************************************************************/
static u8 workhour = 0x02;
void millTickHook ( void )
{
    sea_feedwatchdog();
    if (systime == NULL)    return;  
    
    if (workhour)
        msTimeEventHandler();
    
    if (++ systime->tick >= ONESECOND)
    {
        systime->tick = 0x00;
        if (++ systime->sec >= 60)
        {
            systime->sec = 0x00;                        // systime->sec - 60;
            if (++ systime->min >= 60)
            {
                systime->min  = 0x04;                   // 经测试，每小时慢了4'10''
                systime->sec += 10; 
                if (workhour)
                    hourTimeEventHandler();             // sea_timehourevent();
                if (++ systime->hour >= 24)
                {
                    systime->hour = 0x00;
                    if (++ systime->day >= 30)
                    {
                        systime->day = 0x00;
                        if (++ systime->mon >= 12)
                        {
                            systime->mon = 0x00;
                            systime->year ++;
                        }
                    }
                }
                if ((sys_info.ctrl.config | LAMP_CF_OPEN) && ~(sys_info.ctrl.config & LAMP_CF_NETWORK))   
                {
                    if (workhour)
                        workhour --;
                }
            }
        }
    }
    
    if (workhour)
        emberEventControlSetDelayMS(millTickEvent, 0x01);
}

/*******************************************************************************
* Function Name  : secTickHook ( void )
* Description    : 秒级事件处理程序,循环
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void secTickHook ( void )
{
    secTimeEventHandler();
    emberEventControlSetDelayQS(secTickEvent, 0x04);   // 250ms x 4, // single_info.period second event 
}

/*******************************************************************************
* Function Name  : void sea_getsystime ( systime_t * time )
* Description    : Inserts a delay time.
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_getsystime ( systime_t * time )
{
    if (systime == NULL)     
        return;
    sea_memcpy((char *)time, (char *)&systime, sizeof(systime_t));
}

/*******************************************************************************
* Function Name  : u32  sea_getinterval ( systime_t time )
* Description    : Inserts a delay time. 计算经过了多长的时间间隔；
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
u32  sea_getinterval ( systime_t time )
{
    if (systime == NULL)     
        return 0x00L;
    return systime->tick - time.tick;
}

#if 0   // 移植不用 in em3018 or em3118, only for em260.
/*******************************************************************************
* Function Name  : void sea_setimevent ( u8 index, u16 time, TMHANDLE handle )
* Description    : set time event and function
* Input          : u16 time, TMHANDLE handle
* Output         : None
* Return         : None
*******************************************************************************/
void sea_setimevent ( u8 index, u16 time, TMHANDLE handle )
{
    if (index >= MAXTIMEVENT)
        return;
    
    time_event[index].time = time;
    if (time < 0x02)
        time_event[index].time = 0x02;
    time_event[index].handle = handle;
}

/*******************************************************************************
* Function Name  : void TICK_Disabled ( void )
* Description    : disable System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void TICK_Disable ( void )
{
    SysTick_CounterCmd(SysTick_Counter_Disable);
}

/*******************************************************************************
* Function Name  : void TICK_Enable ( void )
* Description    : enable System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void TICK_Enable ( void )
{
    SysTick_CounterCmd(SysTick_Counter_Enable);
}

/*******************************************************************************
* Function Name  : void sea_delayms ( u32 time )
* Description    : Inserts a delay time.
* Input          : time: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_delayms ( u32 time )
{
    u32 tick = systime->tick;

    if (systime == NULL)     return;
    while (time --)          // 要延时的毫秒数
    {
	while (tick == systime->tick) 
        {
            extern void delay ();
            u32 cnt = SysTick_GetCounter();
            delay(0x01);
            if (cnt == SysTick_GetCounter())
                break;
        }
	tick = systime->tick;
    }
    SysTick_CounterCmd(SysTick_Counter_Clear);     // Clear SysTick Counter 
}

/*******************************************************************************
* Function Name  : halTimer1Isr ( void )
* Description    : TIM1中断服务程序;
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void halTimer1Isr ( void )
{ 
//    INT_CFGSET &= ~INT_TIM1;   // stop the interrupts
    
    TimingTickHook();  
//    INT_TIM1FLAG = 0xffffffff; // clear interrupt
    TIM1_CNT    = 0x00;
    TIM1_ARR    = TICKCNT;     // 417 = 1ms  417000
//    INT_TIM1CFG = INT_TIMUIF;  // enable the Timer 1, CNT ?= TOP interrupt  (Update interrupt enable)
//    INT_CFGSET |= INT_TIM1;    // enable top level timer interrupts
}
#endif

///////////////////////////////////////////////////////////////////////////////////////
