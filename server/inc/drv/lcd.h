
///////////////////////////////////////////////////////////////////////////////////////
//
#ifndef __LCD_H__
#define __LCD_H__

#include "config.h"
#include "systicker.h"

    #define LCD_PIN_CS    GPIO_Pin_0
    #define LCD_PIN_RESET GPIO_Pin_1
    #define LCD_PIN_RS    GPIO_Pin_2
    #define LCD_PIN_SCLK  GPIO_Pin_4
    #define LCD_PIN_MOSI  GPIO_Pin_3


#define cs1_lcd_low   GPIO_WriteBit(GPIOC,LCD_PIN_CS,Bit_RESET); lcd_info.state &= ~LCD_CSBIT  // Pull down the chip selected pin
#define cs1_lcd_up    GPIO_WriteBit(GPIOC,LCD_PIN_CS,Bit_SET); lcd_info.state |= LCD_CSBIT   // Pull up the chip selected pin

#define reset_lcd_low GPIO_WriteBit(GPIOC,LCD_PIN_RESET,Bit_RESET)  // Reset pin is set to be "0"
#define reset_lcd_up  GPIO_WriteBit(GPIOC,LCD_PIN_RESET,Bit_SET)  // Reset pin is set to be "1"

#define rs_lcd_low    GPIO_WriteBit(GPIOC,LCD_PIN_RS,Bit_RESET);lcd_info.state &= ~LCD_RSBIT  // select data register
#define rs_lcd_up     GPIO_WriteBit(GPIOC,LCD_PIN_RS,Bit_SET);lcd_info.state |= LCD_RSBIT   // select command register

#define sclk_lcd_low  GPIO_WriteBit(GPIOC,LCD_PIN_SCLK,Bit_RESET) // sclk pin is set to be "0"
#define sclk_lcd_up   GPIO_WriteBit(GPIOC,LCD_PIN_SCLK,Bit_SET) // sclk pin is set to be "1"

#define mosi_lcd_low   GPIO_WriteBit(GPIOC,LCD_PIN_MOSI,Bit_RESET) // SPI_MOSI is set to be "0"
#define mosi_lcd_up    GPIO_WriteBit(GPIOC,LCD_PIN_MOSI,Bit_SET) // SPI_MOSI is set to be "1"

///////////////////////////////////////////////////////////////////////////////////////
//
#define MAXX          (0x80)
#define MAXY          (0x40)
#define CHARH         (0x08)
#define CHARW         (0x06)
#define MAXCOL        (MAXX / CHARW)
#define MAXROW        (MAXY / CHARH)

#define LCD_MODE      0x03
#define LCD_UPDATE    0x04
#define LCD_UPDATE_ZH 0x05
#define LCD_MODIFY    0x08
#define LCD_CSBIT     0x10
#define LCD_RSBIT     0x20
#define LCD_LIGHT     0x40
#define LCD_REVS      0x80

#ifdef DISPBUF_ENABLE
#define LCDDISPBUF    ((MAXX * MAXY) >> 0x03)    // 1024
#else
#define LCDDISPBUF    (MAXROW * MAXCOL)          // 168
#endif // DISPBUF_ENABLE
#define LCDPRINTFBUF  22

#define LCDRESET      (0xe2)    // 软复位 
#define LCDVOLT       (0x2c)    // 升压步聚1 
#define CONTRAST      (0x25)    // 粗调对比度, 可设置范围0x20~0x27 
#define MINICONTRAST  (0x81)    // 微调对比度 
#define CONTRASTVAL   (0x10)    // 0x1a,微调对比度的值, 可设置范围0x00~0x3f 
#define LCDBIAS       (0xa2)    // 1/9偏压比, bias 
#define LCDROW        (0xc8)    // 行扫描顺序, 从上到下 
#define LCDCOL        (0xa0)    // 列扫描顺序, 从左到右 
#define LCDSTART      (0x40)    // 起始行, 第一行开始 
#define LCDOPEN       (0xaf)    // 开显示 
#define LCDCLOSE      (0xae)    // close
#define LCDNOR        (0xa6)    // normal display
#define LCDREV        (0xa7)    // reverse display

///////////////////////////////////////////////////////////////////////////////////////
//
typedef struct 
{
    u8   state;
    u8   x, y;
    u8 * ascii;
    u8 * chinese;
    u8 * disp;
    u8 * displast;
} lcd_info_t, *plcd_info_t;    

enum e_index_zh_str
{
    i_road_name = 0,
	i_jsnl = 2,
	i_hdu_building3 = 6,
	i_lamps_sum = 10,
	i_road_num = 14,
	i_port = 17,
	i_soft_ver = 20,
	i_admin = 24,
	i_dns = 27,
	i_time =29,
	i_open_all_lamps = 31,
	i_shutdown = 37,
	i_lightness = 39,
	i_one_lamp = 41,
	i_adjust = 43,
	i_setting = 45,
	i_reboot = 47,
	i_poweron = 49,
	i_poweroff = 51,
	i_sys_info = 53
};
typedef struct
{
    u8 index;
	u8 nums;
}zh_str_t;

typedef struct
{
   u8 x;
   u8 y;
}lcd_oldxy_t;

extern const zh_str_t
s_road_name      ,
s_jsnl           ,
s_hdu_building3  ,
s_lamps_sum      ,
s_road_num       ,
s_port           ,
s_soft_ver       ,
s_admin          ,
s_dns            ,
s_time           ,
s_open_all_lamps ,
s_shutdown       ,
s_lightness      ,
s_one_lamp       ,
s_adjust         ,
s_setting        ,
s_reboot         ,
s_poweron        ,
s_poweroff       ,
s_sys_info       ,
s_humidity       ,
s_temperature    ;




///////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void LCD_Configuration  ( void )
//* 功能        : 格式化输出数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void LCD_Configuration  ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : plcd_info_t get_lcdinfo ( void )
//* 功能        : 格式化输出数据
//* 输入参数    : 无
//* 输出参数    : the pointer of plcd_info_t 
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
plcd_info_t get_lcdinfo ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_clrscreen ( void )
//* 功能        : 格式化输出数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_clrscreen      ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_putimage ( u8 sx, u8 sy, u8 ex, u8 ey, const u8 * pic )
//* 功能        : 格式化输出数据
//* 输入参数    : u8 sx, u8 sy, u8 ex, u8 ey, const u8 * pic
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_putimage       ( u8 sx, u8 sy, u8 ex, u8 ey, const u8 * pic );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_out_zh_char_12  ( u8 x, u8 y )
//* 功能        : 输出预先定义的中文字符串
//* 输入参数    : u8 sx,u8 sy,const zh_str_t st_zh
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_out_zh_char_12(u8 sx,u8 sy,const zh_str_t st_zh);
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_out_zh_num_12  ( u8 x, u8 y )
//* 功能        : 输出预先定义的中文数字
//* 输入参数    : u8 sx,u8 sy,const zh_str_t st_zh
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_out_zh_num_12(u8 sx,u8 sy,const u8 st_zh);
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_gotoxy  ( u8 x, u8 y )
//* 功能        : 格式化输出数据
//* 输入参数    : u8 x, u8 y
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_gotoxy         ( u8 x, u8 y );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_outtextxy  ( u8 x, u8 y, const u8 * str )
//* 功能        : 格式化输出数据
//* 输入参数    : u8 x, u8 y, const u8 * str
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_outtextxy      ( u8 x, u8 y, const u8 * str );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_outtext  ( const u8 * str )
//* 功能        : 格式化输出数据
//* 输入参数    : const u8 * str
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_outtext        ( const u8 * str );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_printf ( const char * format, ... )
//* 功能        : 格式化输出数据
//* 输入参数    : const char * format, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_printf ( const char * format, ... );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void lcd_update ( u8 update )
//* 功能        : 格式化输出数据
//* 输入参数    : u8 update
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void lcd_update ( u8 update );
void lcd_updatenum ( u8 update );
void LCD_Display(void);
void LCD_Display_num(systime_t time);

///////////////////////////////////////////////////////////////////////////////////////
//
#endif  // __LCD_H__