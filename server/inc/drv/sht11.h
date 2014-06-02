#ifndef __SHT11_H__
#define __SHT11_H__

////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

////////////////////////////////////////////////////////////////////////////////////
// PC12,CLK line of S1
#ifdef  WIFI_BRD_002
    #define SHT10_PIN_CLK      GPIO_Pin_12
    #define SHT10_PIN_DAT      GPIO_Pin_13
#elif defined WIFI_BRD_003
    #define SHT10_PIN_CLK      GPIO_Pin_6
    #define SHT10_PIN_DAT      GPIO_Pin_7
#else 
    #define SHT10_PIN_CLK      0
    #define SHT10_PIN_DAT      0
#endif

////////////////////////////////////////////////////////////////////////////////////
//
#ifdef WIFI_BRD_001
    #define SHT75_DAT_IN       
    #define SHT75_DAT_OUT      
    #define SHT75_CLK_H        
    #define SHT75_CLK_L       
    #define SHT75_DAT_H      
    #define SHT75_DAT_L      
#else
    #define SHT75_CLK_H        GPIO_WriteBit(GPIOC,SHT10_PIN_CLK,Bit_SET)                //GPIO_PASET= 0x0004//PA2=1 
    #define SHT75_CLK_L        GPIO_WriteBit(GPIOC,SHT10_PIN_CLK,Bit_RESET)                 //GPIO_PASET=0x0004 PA2=0
    
    /*PC13,Data line of S1*/
    
    #define SHT75_DAT_H        GPIO_WriteBit(GPIOC,SHT10_PIN_DAT,Bit_SET)  //PC13=1 
    #define SHT75_DAT_L        GPIO_WriteBit(GPIOC,SHT10_PIN_DAT,Bit_RESET) //PC13=0
    
    #define SHT75_DATA         GPIO_ReadInputDataBit(GPIOC,SHT10_PIN_DAT)     //read PC13 vlaue
    #define SHT75_DAT_IN       GPIO_InitStructure.GPIO_Pin   = SHT10_PIN_DAT;               \
                               GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		  \
                               GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;     \
                               GPIO_Init(GPIOC, &GPIO_InitStructure);
    #define SHT75_DAT_OUT      GPIO_InitStructure.GPIO_Pin   = SHT10_PIN_DAT;               \
                               GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		  \
                               GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          \
                               GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

////////////////////////////////////////////////////////////////////////////////////
//  Define operation Command
#define RD_SHT11_REG		0x07 // read reg
#define RD_SHT11_TEMP		0x03 // get temperature
#define RD_SHT11_HUM		0x05 // get humidity
#define WT_SHT11_REG		0x06 // write status reg
#define SHT11_RESET			0x1E // soft reset

#define	MEASURE_TEMP		0x01
#define	MEASURE_HUMI		0x02

////////////////////////////////////////////////////////////////////////////////////
// Function defination
void s_Trans_Start(void);
int s_Write_SHT11_Byte(unsigned char b);
unsigned char s_Read_SHT11_Byte(void);
int s_Write_SHT11_Reg(unsigned char r);
void s_Reset_SHT11(void);
void s_Ack_SHT11_L(void);
void s_Ack_SHT11_H(void);

int sht11_getemp ( u16 * t );
int sht11_gethum ( u16 * h );

/*
 * Function name: u8 sht11_state ( void )
 * Description	: transmission start sequence
 * Arguments	: None
 * Return value	: None
 */
u8 sht11_state ( void );

////////////////////////////////////////////////////////////////////////////////////
//
#endif
