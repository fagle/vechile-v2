#include <stdio.h>
#include "tsl2561.h"
#include "key.h"
#include "stm32f10x.h"

static GPIO_InitTypeDef GPIO_InitStructure;                 
u8 DataBufferTx[9];
u8 DataBufferRx[9];

void static tsl_dealy ( s16 Nus )
{
    while (Nus --)
    {
	nop();
    }
}

void I2C_Trans_Start ( void )
{
    TSL2561_DAT_OUT;
    TSL2561_DAT_H;
    nop();              // TSL2561tsl_dealy1();	
    TSL2561_CLK_H;	
    tsl_dealy(0x08);    // 本为5...//1.3us; //t(BUF),at least 1.3us needed
    TSL2561_DAT_L;
    tsl_dealy(0x05);    // 本为3...//0.827us; //t(HDSTA),at least 0.6us needed
    TSL2561_CLK_L;
    tsl_dealy(0x08);    // t(Low),at least 1.3us needed;
}

s16 I2C_Write_Byte ( u8 b )
{
    s16 i = 0x00;
    u8  c = 0x00;
    
    TSL2561_DAT_OUT;
    for (i = 0x00; i < 0x08; i ++)
    {
        if (b & 0x80)
        {
            TSL2561_DAT_H;
        }
        else 
        {
            TSL2561_DAT_L;
        }
		
        b = b << 0x01;
        TSL2561_CLK_H;
        tsl_dealy(0x08);      // 5...//t(HIGH), at least 0.6us needed
        TSL2561_CLK_L;
        tsl_dealy(11);        // 8...//1.6us//t(LOW), at least 1.3us needed				
    }

    TSL2561_DAT_IN;	      // (SUDAT),at least 100ns needed
    TSL2561_CLK_H;	
    tsl_dealy(0x08);          // 5...//0.828us;//t(HIGH), at least 0.6us needed
    
    c = TSL2561_DATA == 0x01 ? 0x01 : 0x00;  // (!c)应该是c，c表是ACK位是1，如果正常的话，要为0
    TSL2561_CLK_L;
    tsl_dealy(11);            // 8...//t(LOW), at least 1.3us needed
    
    return !c;
}

void I2C_Trans_Stop ( void )
{
    TSL2561_DAT_OUT;
    TSL2561_DAT_L;
    TSL2561_CLK_H;	
    tsl_dealy(0x08);        // 本为5...//1.3us; //t(BUF),at least 1.3us needed
    TSL2561_DAT_H;
    tsl_dealy(0x05);        // 本为3...//0.827us; //t(HDSTA),at least 0.6us needed
    TSL2561_CLK_L;
    tsl_dealy(0x08);        // t(Low),at least 1.3us needed;
}	 


u8 I2C_Read_Byte ( void )
{
    u8 i, temp;
	
    TSL2561_DAT_IN;
    for (temp = 0x00, i = 0x00; i < 0x08; i ++)
    {
        temp = temp << 0x01;
        TSL2561_CLK_H;
        tsl_dealy(10);
        if (TSL2561_DATA == 0x01)
            temp |= 0x01;
        TSL2561_CLK_L;
        tsl_dealy(13);
    }

    return temp;
}

void I2C_Ack_L ( void )
{
    TSL2561_DAT_OUT;
    TSL2561_DAT_L;
    TSL2561_CLK_H;
    tsl_dealy(0x08);	 
    TSL2561_CLK_L;
    tsl_dealy(11);
}


void I2C_Ack_H ( void )
{
    TSL2561_DAT_OUT;
    TSL2561_DAT_H;
    TSL2561_CLK_H;
    tsl_dealy(0x08);
    TSL2561_CLK_L;
    tsl_dealy(11);
}

s16 I2C_SendData_TSL2561 ( u8 addr, u8 cmd, s16 len, u8 * ptr )
{
    s16 i = 0x00;

    I2C_Trans_Start();
    if (!I2C_Write_Byte(addr << 0x01))
        return 0x00;   // No ACK

    if (!I2C_Write_Byte(cmd))
        return 0x00;
	
    while (i < len)
    {		
        if (!I2C_Write_Byte(ptr[i ++]))
            return 0x00;
    }
    I2C_Trans_Stop();
    
    return 0x01;
}

s16 I2C_ReceivData ( u8 addr, u8 cmd, s16 len, u8 * ptr )
{
    s16 i = 0x00;        
        
    I2C_Trans_Start();
    if (!I2C_Write_Byte(addr << 0x01))
        return 0x00;  // No ACK

    if (!I2C_Write_Byte(cmd))
        return 0x00;        
    
    I2C_Trans_Start();
    if (!I2C_Write_Byte((addr << 0x01) | 0x01))
        return 0x00;  // No ACK
    while (i < len)
    {
        ptr[i++] = I2C_Read_Byte();
        I2C_Ack_L();	                
    }
    I2C_Trans_Stop();

    return 0x01;
}

static s16 TSL2561TEST ( u8 addr, u8 cmd, s16 len, u8 * src, u8 * dst )
{
    s16 i = 0x00;

    I2C_Trans_Start();
    if (!I2C_Write_Byte(addr << 0x01))
        return 0x00;

    if (!I2C_Write_Byte(cmd))
        return 0x00;
	
    while (i < len)
    {		
        if (!I2C_Write_Byte(src[i++]))
            return 0x00;
    }
    I2C_Trans_Stop();
	
    tsl_dealy(0x05);
    I2C_Trans_Start();
    if (!I2C_Write_Byte(addr << 0x01))
        return 0x00;

    if (!I2C_Write_Byte(cmd))
        return 0x00;

    I2C_Trans_Start();
    if (!I2C_Write_Byte((addr << 0x01) | 0x01))
        return 0x00;
    dst[0] = I2C_Read_Byte();
    I2C_Ack_L();
    I2C_Trans_Stop();

    return 0x01;
}

s16 TSL2561PowerUp ( void )
{
    u8 temp = PowerUp;
    return I2C_SendData_TSL2561(SlaveAddress, PowerUpCode, 0x01, &temp);
}

s16 TSL2561PowerUpWithDetection ( u8 *Response )
{
    u8 temp = PowerUp;
    
    if (!TSL2561TEST(SlaveAddress, PowerUpCode, 0x01, &temp, Response))
        return 0x00;
    return 0x01;
}

s16 TSL2561PowerDown(void){
	DataBufferTx[0]=PowerDown;
	return I2C_SendData_TSL2561(SlaveAddress, PowerDownCode, 1, DataBufferTx);
}

s16 TSL2561SetThresholdLow(s16 Threshold){
	DataBufferTx[0]=Threshold&0x00FF;
	DataBufferTx[1]=(Threshold&0xFF00)>>8;
	return I2C_SendData_TSL2561(SlaveAddress, ThreshLowLowRegCode, 2, DataBufferTx);
}

s16 TSL2561SetThresholdHigh(s16 Threshold){
	DataBufferTx[0]=Threshold&0x00FF;
	DataBufferTx[1]=(Threshold&0xFF00)>>8;
	return I2C_SendData_TSL2561(SlaveAddress, ThreshHighLowRegCode, 2, DataBufferTx);



}	

/*************************************************************************************
InterruptLogic:  
				should be one of these: IntrDisabled,IntrLevelInt,IntrSMBAlert,
										 IntrTestMode
InterruptRate:
				should be one of these: EveryADCCycle,AnyOutsideValue,TowPeriods,
 										ThreePeriods,FourPeriods,FivePeriods,
										SixePeriodS,SevenPeriods,EightPeriods,
										NinePeriods,TenPeriods,ElevenPeriods,
										TwelvePeriods,ThirteenPeriods,FourteenPeriods,
										FifteenPeriods
**************************************************************************************/											 										  
s16 TSL2561SetInterruptCtrlReg(u8 InterruptLogic, u8 InterruptRate){
		u8 temp=0;
		temp&=IntrMask;
		temp&=PersistMask;
		temp|=InterruptLogic|InterruptRate;
		DataBufferTx[0]=temp;
		return I2C_SendData_TSL2561(SlaveAddress, InterruptRegCode, 1, DataBufferTx);			
}


/***************************************************************************************
Fetch Part Number ID and Revision Number ID respectly filled in *PartNumberID and
*revisionNumberID
***************************************************************************************/
s16 TSL2561ReadID(u8 *PartNumberID, u8 *RevisionNumberID){
		 if(I2C_ReceivData(SlaveAddress, IDRegCode, 1, DataBufferRx)){
			 *PartNumberID=(DataBufferRx[0]&REVNOMask)>>4;
			 *RevisionNumberID=(DataBufferRx[0]&PARTNOMask);
			 return 1;
		 }
		 return 0; //Command failed; No ACK;
}
/**********************************************************************************
ChannelNum: shoule be 0 or 1
*LightIntensity : fetch the Channel Number;
**********************************************************************************/
s16 TSL2561ReadADC(s16 ChannelNum, u16 *LightIntensity){
		if(ChannelNum==0){
			if(I2C_ReceivData(SlaveAddress, Data0LowRegCode, 2, DataBufferRx))
                        {                                                               
				*LightIntensity=(DataBufferRx[1]<<8)|DataBufferRx[0];
				return 1;
			}
			return 0;//No Ack;				
		}
		
		if(ChannelNum==1){
			if(I2C_ReceivData(SlaveAddress, Data1LowRegCode, 2, DataBufferRx)){                                                               
				*LightIntensity=(DataBufferRx[1]<<8)|DataBufferRx[0];
				return 1;	
			}
			return 0;//No ACK;
		}
		
		return 2;//Wrong ChannelNum;	 	
}

/**********************************************************************************
GAIN: should be GainSetLow or GainSetHigh
Manual: should be ManualSet or ManualClr
INTEG: should be IntegScal0034, IntegScal0252 or IntegScal1000
**********************************************************************************/
s16 TSL2561SetTimmingReg(u8 GAIN, u8 Manual, u8 INTEG){
		u8 temp=0;
		temp&=GainClrMask&ManualMask&IntegMask;
		temp|=GAIN|Manual|INTEG;
		DataBufferTx[0]=temp;
		return I2C_SendData_TSL2561(SlaveAddress, TimingRegCode, 1, DataBufferTx);
}

void TSL2561ReadQuery ( u16 *Lux )
{
    u16 LightIntensity0 = 0xFFFF;
    u16 LightIntensity1 = 0xFFFF;
    
    TSL2561ReadADC(0, &LightIntensity0);
    TSL2561ReadADC(1, &LightIntensity1);
    if(LightIntensity0 != 0xFFFF&&LightIntensity1 != 0xFFFF)       
    *Lux=CalculateLux(GainSetHigh>>4, IntegScal1000, LightIntensity0, LightIntensity1, T);
    else
      *Lux=1800;
}

void TSL2561Init ( void )
{
    TSL2561PowerUp();
    TSL2561SetTimmingReg(GainSetHigh, ManualClr, IntegScal1000);
    TSL2561SetInterruptCtrlReg(IntrLevelInt, EveryADCCycle);
}

// lux equation approximation without floating pos16 calculations
//////////////////////////////////////////////////////////////////////////////
// Routine: u16 CalculateLux(u16 ch0, u16 ch0, s16 iType)
//
// Description: Calculate the approximate illuminance (lux) given the raw
// channel values of the TSL2560. The equation if implemented
// as a piece.wise linear approximation.
//
// Arguments: u16 iGain . gain, where 0:1X, 1:16X
// u16 tInt . s16egration time, where 0:13.7mS, 1:100mS, 2:402mS,
// 3:Manual
// u16 ch0 . raw channel value from channel 0 of TSL2560
// u16 ch1 . raw channel value from channel 1 of TSL2560
// u16 iType . package type (T or CS)
//
// Return: u16 . the approximate illuminance (lux)
//
//////////////////////////////////////////////////////////////////////////////
u32 CalculateLux (u16 iGain, u16 tInt, u16 ch0, u16 ch1, s16 iType)
{
//........................................................................
// first, scale the channel values depending on the gain and s16egration time
// 16X, 402mS is nominal.
// scale if s16egration time is NOT 402 msec
    u32 chScale;
    u32 channel1;
    u32 channel0;
    u32 ratio1 = 0;
    u32 ratio;
    u16 b, m;
    u32 temp;
    u32 lux;
    
    switch (tInt)
    {
        case 0: // 13.7 msec
            chScale = CHSCALE_TINT0;
            break;
        case 1: // 101 msec
            chScale = CHSCALE_TINT1;
            break;
        default: // assume no scaling
            chScale = (1 << CH_SCALE);
            break;
    }

    // scale if gain is NOT 16X
    if (!iGain) 
      chScale = chScale << 0x04; // scale 1X to 16X
    channel0 = (ch0 * chScale) >> CH_SCALE;        // scale the channel values
    channel1 = (ch1 * chScale) >> CH_SCALE;

    //........................................................................
    // find the ratio of the channel values (Channel1/Channel0)
    // protect against divide by zero

    if (channel0 != 0) 
        ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
    ratio = (ratio1 + 1) >> 1;      // round the ratio value, is ratio <= eachBreak ?

    switch (iType)
    {
        case 0: // T, FN and CL package
            if ((ratio > 0x00) && (ratio <= K1T))
            {
                b = B1T; m = M1T;
            }
            else if (ratio <= K2T)
            {
                b = B2T; m = M2T;
            }
            else if (ratio <= K3T)
            {
                b = B3T; m = M3T;
            }
            else if (ratio <= K4T)
            {
                b = B4T; m = M4T;
            }
            else if (ratio <= K5T)
            {
                b = B5T; m = M5T;
            }
            else if (ratio <= K6T)
            {
                b = B6T; m = M6T;
            }
            else if (ratio <= K7T)
            {
                b = B7T; m = M7T;
            }
            else if (ratio > K8T)
            {
                b = B8T; m = M8T;
            }
            break;
        case 1:  // CS package
            if ((ratio > 0x00) && (ratio <= K1C))
            {
                b = B1C; m = M1C;
            }
            else if (ratio <= K2C)
            {
                b = B2C; m = M2C;
            }
            else if (ratio <= K3C)
            {
                b = B3C; m = M3C;
            }
            else if (ratio <= K4C)
            {
                b = B4C; m = M4C;
            }
            else if (ratio <= K5C)
            {
                b = B5C; m = M5C;
            }
            else if (ratio <= K6C)
            {
                b = B6C; m = M6C;
            }
            else if (ratio <= K7C)
            {
                b = B7C; m = M7C;
            }
            else if (ratio > K8C)
            { 
                b = B8C; m = M8C;
            }
            break;
    }

    temp = ((channel0 * b)-(channel1 * m));
    if ((s32)temp < 0x00)                      // do not allow negative lux value
        temp = 0x00;
    temp += (0x01 << (LUX_SCALE - 0x01));      // round lsb (2^(LUX_SCALE.1))
    lux = temp >> LUX_SCALE;                   // strip off fractional portion
    
    return(lux);
}
