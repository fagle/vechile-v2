#ifndef __MENU_H__
#define __MENU_H__


#include "config.h"
#include "systicker.h"

void Display_All_Lamp_command(void);
void LCD_Display_mainmenu(void);
void Display_Single_Lamp_command(void);
void Display_Update_Lightness_Command();
u16 InputLampId(u8 x,u8 y,u8 num);
void Auto_Adjust_Time ( void );
void Display_Time_Adjust_command( );

#endif  //__MENU_H__