//S1:PA8时钟,PA9数据

//#include "stm32f10x_type.h"
#include "sht11.h"
#include "key.h"
#include "stm32f10x.h"
//#include "stm32f10x_systick.h"
//#include "stm32f10x_tim.h"

int Time_1;
int Time_2;
int Time_3;
//extern int Timer_ms;
u8 reg_stat=0;
const u8 Table[256] = {  //CRC查找表
 0, 49, 98, 83, 196, 245, 166, 151, 185, 136, 219, 234, 125, 76, 31, 46,
 67, 114, 33, 16, 135, 182, 229, 212, 250, 203, 152, 169, 62, 15, 92, 109,
 134, 183, 228, 213, 66, 115, 32, 17, 63, 14, 93, 108, 251, 202, 153, 168,
 197, 244, 167, 150, 1, 48, 99, 82, 124, 77, 30, 47, 184, 137, 218, 235,
 61, 12, 95, 110, 249, 200, 155, 170, 132, 181, 230, 215, 64, 113, 34, 19,
 126, 79, 28, 45, 186, 139, 216, 233, 199, 246, 165, 148, 3, 50, 97, 80,
 187, 138, 217, 232, 127, 78, 29, 44, 2, 51, 96, 81, 198, 247, 164, 149,
 248, 201, 154, 171, 60, 13, 94, 111, 65, 112, 35, 18, 133, 180, 231, 214,
 122, 75, 24, 41, 190, 143, 220, 237, 195, 242, 161, 144, 7, 54, 101, 84,
 57, 8, 91, 106, 253, 204, 159, 174, 128, 177, 226, 211, 68, 117, 38, 23,
 252, 205, 158, 175, 56, 9, 90, 107, 69, 116, 39, 22, 129, 176, 227, 210,
 191, 142, 221, 236, 123, 74, 25, 40, 6, 55, 100, 85, 194, 243, 160, 145,
 71, 118, 37, 20, 131, 178, 225, 208, 254, 207, 156, 173, 58, 11, 88, 105,
 4, 53, 102, 87, 192, 241, 162, 147, 189, 140, 223, 238, 121, 72, 27, 42,
 193, 240, 163, 146, 5, 52, 103, 86, 120, 73, 26, 43, 188, 141, 222, 239,
 130, 179, 224, 209, 70, 119, 36, 21, 59, 10, 89, 104, 255, 206, 157, 172};
static GPIO_InitTypeDef GPIO_InitStructure;     


#ifndef WIFI_BRD_001
/*
 * Function name: sht11_delay
 * Description	: delay function
 * Arguments	: None
 * Return value	: None
 */
static void sht11_delay ( void )
{
    int k;
    
    for (k = 0x00; k < 0x03f; k ++)
    {
	    nop();
        nop();
        nop();
    }
}

/*
 * Function name: u8 sht11_state ( void )
 * Description	: transmission start sequence
 * Arguments	: None
 * Return value	: None
 */
u8 sht11_state ( void )
{
    SHT75_DAT_IN;
    return SHT75_DATA;
}

/*
 * Function name: s_Trans_Start
 * Description	: transmission start sequence
 * Arguments	: None
 * Return value	: None
 */
void s_Trans_Start(void)
{	
	SHT75_DAT_OUT;		// dir output	
	SHT75_DAT_H;
	SHT75_CLK_L;	
	sht11_delay();	
	SHT75_CLK_H;		// clk high
	sht11_delay();
	SHT75_DAT_L;			// data low
	sht11_delay();
	SHT75_CLK_L;		// clk low
	sht11_delay();
	SHT75_CLK_H;		// clk high
	sht11_delay();
	SHT75_DAT_H;
	sht11_delay();
	SHT75_CLK_L;		// clk low
	sht11_delay();
}
/*
 * Function name: s_Write_SHT11_Byte
 * Description	: Write a byte to sht11 sensor.
 * Arguments	: b - data
 * Return value	: result
 */
int s_Write_SHT11_Byte(unsigned char b)
{
	int k,m;
	for(k=0;k<8;k++)
	{
		if(b & 0x80)
		{
		   SHT75_DAT_H;		// data
		}
		else
		{
			SHT75_DAT_L;		// data
		}		
		SHT75_CLK_H;		// clk high
		sht11_delay();
		b <<= 1;
		SHT75_CLK_L;		// clk low
	}	
	SHT75_DAT_IN;
	sht11_delay();
	SHT75_CLK_H;	
	for(m=0;m<1000;m++)
	{
		sht11_delay();
		if(SHT75_DATA==0x00) break;
	}	
	if(m<900)
	{
		SHT75_CLK_L;
		return 1;
	}else
	{
		SHT75_CLK_L;
		return 0;
	}
}

/*
 * Function name: s_Read_SHT11_Byte
 * Description	: Read a byte from sht11 sensor.
 * Arguments	: none
 * Return value	: the data read from sht11
 */
unsigned char s_Read_SHT11_Byte(void)
{
	int k;
	unsigned char rByte=0;
	SHT75_DAT_IN;			// change dir
	for(k=0;k<8;k++)
	{
		rByte<<=1;
		SHT75_CLK_H;
		sht11_delay();
		if(SHT75_DATA==0x01)
		{
			rByte |= 1;
		}
		SHT75_CLK_L;
		sht11_delay();
	}	
	return rByte;	
}
/*
 * Function name: s_Write_SHT11_Reg
 * Description	: Write register of sht11 sensor.
 * Arguments	: r - data
 * Return value	: result
 */
int s_Write_SHT11_Reg(unsigned char r)
{	
	s_Trans_Start();
	return s_Write_SHT11_Byte(r);	
}
/*
 * Function name: s_Write_SHT11_Reg
 * Description	: Write register of sht11 sensor.
 * Arguments	: r - data
 * Return value	: result
 *
 **************************************************************************************************
 *       _____________________________________________________         ________
 * DATA:                                                      |_______|
 *          _    _    _    _    _    _    _    _    _        ___     ___
 * SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
 *
 *************************************************************************************************
 */
void s_Reset_SHT11(void)
{
	int i;
	SHT75_DAT_OUT;	
	SHT75_DAT_H;
	sht11_delay();
	SHT75_CLK_L;          //Initial state	
	for(i=0;i<9;i++)                  //9 SCK cycles
	{
            SHT75_CLK_H;
	    sht11_delay();
	    SHT75_CLK_L;
	    sht11_delay();	
	}	
	s_Trans_Start();                   //transmission start
}	

int sht11_gethum(u16 * h)
{
        u8 SHT_CRC,CACU_CRC; //分别用来记录从SHT75读取的CRC和自己计算出来的CRC
        u16 hum_val=0; //用来记录湿度值,12位有效
        u16 tmp_byte; //用来记录每次读取的8个字节
        u8 crc_rev; //用来记录SHT读得的CRC的反转值
        u8 or_1; //用来进行"或"1操作
        int m;
        CACU_CRC=0;
        or_1=0x80;
        crc_rev=0;
       
        //CACU_CRC = Table[(CACU_CRC^0x05)]; //进行第一次CRC，0x05为湿度读取命令字节
//	s_Trans_Start();
//	s_Write_SHT11_Byte(RD_SHT11_HUM);
//        
//        for(m=0;m<0xdFF00;m++)
//        {
//             nop();
//        }
        CACU_CRC = Table[(CACU_CRC^0x05)]; //进行第一次CRC，0x05为湿度读取命令字节
        tmp_byte = s_Read_SHT11_Byte();
        CACU_CRC = Table[(CACU_CRC^tmp_byte)]; //进行第二次CRC
        hum_val |= tmp_byte<<8;

        sht11_delay();
	s_Ack_SHT11_L();

        tmp_byte = s_Read_SHT11_Byte();
        CACU_CRC = Table[(CACU_CRC^tmp_byte)]; //进行第三次CRC
        hum_val |= tmp_byte;

        sht11_delay();
	s_Ack_SHT11_L();

        tmp_byte = s_Read_SHT11_Byte();              //读取校验
        for(m=0;m<8;m++)
        {
          if((tmp_byte &0x01)==0x01)
            crc_rev |= or_1;
          tmp_byte>>=1;
          or_1>>=1;
        }
        SHT_CRC = crc_rev; //SHT_CRC反转
        sht11_delay();
	s_Ack_SHT11_L();
        *h =hum_val;
        if(CACU_CRC==SHT_CRC)
	  return  1;
        else
          return 0;	
}

/*
 * Function name: sht11_getem
 * Description	: Get temperature form sht11 sensor.
 * Arguments	: t - a pointer point temperature value
 * Return value	: operation result
 */
int sht11_getemp(u16 * t)
{
        u8 SHT_CRC,CACU_CRC; //分别用来记录从SHT75读取的CRC和自己计算出来的CRC
        u16 temp_val=0; //用来记录湿度值,12位有效
        u16 tmp_byte; //用来记录每次读取的8个字节
        u8 crc_rev; //用来记录SHT读得的CRC的反转值
        u8 or_1; //用来进行"或"1操作
	int m;
        CACU_CRC=0;
        or_1=0x80;
        crc_rev=0;

       // CACU_CRC = Table[(CACU_CRC^0x03)]; //进行第一次CRC，0x05为湿度读取命令字节
//	s_Trans_Start();	
//	s_Write_SHT11_Byte(RD_SHT11_TEMP);
//
//        for(m=0;m<0xdff00;m++)
//        {
//            nop();
//        }
        
        CACU_CRC = Table[(CACU_CRC^0x03)]; //进行第一次CRC，0x05为湿度读取命令字节
	tmp_byte = s_Read_SHT11_Byte();  //读取高8位
        CACU_CRC = Table[(CACU_CRC^tmp_byte)]; //进行第二次CRC
        temp_val |= tmp_byte<<8;

	sht11_delay();
	s_Ack_SHT11_L();

	tmp_byte = s_Read_SHT11_Byte();   //读取低8位
        CACU_CRC = Table[(CACU_CRC^tmp_byte)]; //进行第三次CRC
        temp_val |= tmp_byte;
	
        s_Ack_SHT11_L();
	sht11_delay();

	tmp_byte = s_Read_SHT11_Byte();   //读取校验
        for(m=0;m<8;m++)
        {
          if((tmp_byte &0x01)==0x01)
            crc_rev |= or_1;
          tmp_byte>>=1;
          or_1>>=1;
        }
        SHT_CRC = crc_rev; //SHT_CRC反转

	s_Ack_SHT11_H();
	sht11_delay();

        *t= temp_val;
	if(CACU_CRC==SHT_CRC)
	  return  1;
        else
          return 0;
	
}

/*
 * Function name: s_Ack_SHT11_L
 * Description	: low level acknowledge to sht11.
 * Arguments	: none
 * Return value	: none
 *
 *Data    _______________
 *				---
 *SCLK    ______|   |____
 */
void s_Ack_SHT11_L(void)
{
	SHT75_DAT_OUT;
	SHT75_DAT_L;			// ack
	sht11_delay();
	SHT75_CLK_H;		// clk high	
	sht11_delay();
	SHT75_CLK_L;		// clk low
}

/*
 * Function name: s_Ack_SHT11_H
 * Description	: high level acknowledge to sht11.
 * Arguments	: none
 * Return value	: none
 *
 *         ----------------
 * Data
 * 				  ---
 * SCLK    ______|   |____
 */
void s_Ack_SHT11_H(void)
{
	SHT75_DAT_OUT;
	SHT75_DAT_H;			// ack
	sht11_delay();
	SHT75_CLK_H;		// clk high	
	sht11_delay();
	SHT75_CLK_L;		// clk low	
}

#endif
