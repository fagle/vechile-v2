#include "network.h"

const u8 *MenuList = "ABCDEFHLPU\0";
key_info_t  key_info;
extern systime_t adjust_time;

#ifdef USE_TOOL_KEIL
__asm void nop(void)
{
    NOP
	BX lr
}
#else
void  nop(void)
{
  __asm("nop");
}
#endif

#if 0
/************************************************
* 函数名      : void key_delay(void)
* 功能        : 延时
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void key_delay ( u16 times )
{
    extern void IwdgReload(void);
  
    while (times--)
    {
        for (int i = 0x00; i < 0x3ff; i ++)
        {
            nop();
            nop();
            nop();
        }
        IwdgReload();
    }
}
#endif

#if 1
led_info_t  led_info;
/*******************************************************************************
* Function Name  : void  sea_leddisplay ( void )
* Description    : display a string
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  sea_leddisplay ( void )
{
}
#endif

/*******************************************************************************
* Function Name  : u8 sea_scankey ( void )
* Description    : Read the GPIO line value connected to the key
* Input          : key
* Output         : GPIO line value
* Return         : None
*******************************************************************************/
u8 sea_scankey ( void )
{
    for (u8 i = 0x00; i < key_info.num; i ++)
    {
        if (!GPIO_ReadInputDataBit(key_info.port, key_info.pin[i]))
            return i + 0x01;
    }
    return 0x00;
} 

/*******************************************************************************
* Function Name  : u8 sea_iskeyempty ( void )
* Description    : is key empty.
* Input          : None
* Output         : key empty state
* Return         : None
*******************************************************************************/
u8 sea_iskeyempty ( void )
{
    return key_info.cnt == 0x00;
}

/*******************************************************************************
* Function Name  : u8 sea_readkey ( void )
* Description    : Inserts a delay time.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
u8 sea_readkey ( void )
{
    u8 key = eKeyNull;
    
    if (key_info.cnt)
    {
        key_info.cnt --;
        key = key_info.keys[key_info.out];
        key_info.out ++;
        if (key_info.out >= KEYSIZE)
            key_info.out = 0x00;
    }
    return key;
}

/*******************************************************************************
* Function Name  : void sea_putkey ( u8 key )
* Description    : Inserts a delay time.
* Input          : u8 key.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_putkey ( u8 key )
{ 
    key_info.keys[key_info.in] = key;
    key_info.cnt ++;
    key_info.in ++;
    if (key_info.in >= KEYSIZE)
        key_info.in = 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
