
#ifndef __KEY_H__
#define __KEY_H__

////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

////////////////////////////////////////////////////////////////////////////////////////////
//
//#define Model_Btn 9
//#define Num1_Btn 8
//#define Num2_Btn 7
//#define Adjust_Btn 6
//#define Switch_Btn 5
//#define Submit_Btn 2

//#define NULL_KEY     0x00

//#define OPEN_KEY     0x06
//#define CLOSE_KEY    0x07

//#define NUM0_KEY     0x01
//#define NUM1_KEY     0x02
//#define NUM2_KEY     0x03
//#define NUM3_KEY     0x04
//#define SWITCH_KEY   0x05

//#define SwitchOpen   0x31
//#define SwitchClose  0x30

#define KEYSIZE      (0x10)
//#define KEYDELAY     (0x100)
//#define MENUSIZE     (0x0a)
#define MAXKEYS      (0x0a)

////////////////////////////////////////////////////////////////////////////////////////////
//
typedef enum { eKeyNull = 0x00, eKeyNum0, eKeyNum1, eKeyNum2, eKeyNum3, eKeyNum4, eKeyNum5,
               eKeyNum6, eKeyNum7, eKeyNum8, eKeyNum9 } EKEY;

////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8  state;
    u8  time;
    u8  last;
    u8  num;
    u8  cnt;
    u8  in, out;
    u8  keys[KEYSIZE];
    u16 pin[MAXKEYS]; 
    GPIO_TypeDef * port;
} key_info_t;

////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef USE_TOOL_KEIL
__asm void nop(void);
#else
void  nop(void);
#endif

/*******************************************************************************
* Function Name  : u8 sea_scankey ( void )
* Description    : Read the GPIO line value connected to the key
* Input          : key
* Output         : GPIO line value
* Return         : None
*******************************************************************************/
u8 sea_scankey ( void );

/*******************************************************************************
* Function Name  : u8 sea_readkey ( void )
* Description    : Inserts a delay time.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
u8 sea_readkey ( void );

/*******************************************************************************
* Function Name  : void sea_putkey ( u8 key )
* Description    : Inserts a delay time.
* Input          : u8 key.
* Output         : None
* Return         : None
*******************************************************************************/
void sea_putkey ( u8 key );

/*******************************************************************************
* Function Name  : u8 sea_iskeyempty ( void )
* Description    : is key empty.
* Input          : None
* Output         : key empty state
* Return         : None
*******************************************************************************/
u8 sea_iskeyempty ( void );

/*******************************************************************************
* Function Name  : void KEY_Configuration ( void )
* Description    : init keyborad 
	                REMINDER: Have to make clear the pull-up/down 
	                issue under comment below
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void KEY_Configuration ( void );

/*******************************************************************************
* Function Name  : void KeyCommandHandler ( u8 key )
* Description    : keybord command handler
* Input          : u8 key
* Output         : None
* Return         : None
*******************************************************************************/
void KeyCommandHandler ( u8 key );

/******************************************************************************
* 函数名      : void keyboardinput ( char ch )
* 功能        : keyboard input
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
void keyboardinput ( char ch );

////////////////////////////////////////////////////////////////////////////////////////////
//
extern key_info_t  key_info;

////////////////////////////////////////////////////////////////////////////////////////////
#endif // __KEY_H__ 

