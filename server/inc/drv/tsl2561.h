#ifndef __TSL2561_H__
#define __TSL2561_H__

///////////////////////////////////////////////////////////////////////////////////
//
#include "config.h"
#include "stm32f10x.h" 

///////////////////////////////////////////////////////////////////////////////////
//
#define	SlaveAddressGND	  0x29
#define	SlaveAddressFloat 0x39
#define	SlaveAddressVDD   0x49

#define SlaveAddress      0x49
#define SMBAlertAddress	  0x0C

///////////////////////////////////////////////////////////////////////////////////
// Register Address Define
#define	CtrlRegAdd	        0x00
#define	TimingRegAdd	        0x01
#define	ThreshLowLowRegAdd	0x02		
#define	ThreshLowHighRegAdd	0x03
#define	ThreshHighLowRegAdd	0x04
#define	ThreshHighHighRegAdd	0x05
#define	InterruptRegAdd 	0x06
#define	CRCRegAdd		0x08
#define	IDRegAdd                0x0A
#define BlockWriteAdd		0x0B
#define	Data0LowRegAdd		0x0C
#define	Data0HighRegAdd 	0x0D
#define	Data1LowRegAdd		0x0E
#define	Data1HighRegAdd 	0x0F

///////////////////////////////////////////////////////////////////////////////////
// About COMMAND Register
#define	IntClearClrMask	0xBF
#define	AddressClrMask	0xF0

///////////////////////////////////////////////////////////////////////////////////
// About Control Register
#define	PowerUp		0x03
#define	PowerDown	0x00

///////////////////////////////////////////////////////////////////////////////////
// About Timing Register
#define	GainClrMask	0xEF
#define	GainSetLow	0x00
#define	GainSetHigh	0x10
#define	ManualMask	0xF7
#define	ManualSet	0x08
#define	ManualClr	0x00
#define IntegMask	0xFC
#define IntegScal0034	0x00	//NOMINAL INTEGRATION TIME 13.7ms 101ms 402ms
#define	IntegScal0252	0x01
#define	IntegScal1000	0x02

///////////////////////////////////////////////////////////////////////////////////
// About Interrupt Control Register
#define	IntrMask	0xCF
#define	IntrDisabled	0x00
#define	IntrLevelInt	0x10
#define	IntrSMBAlert	0x20
#define	IntrTestMode	0x30

#define	PersistMask	0xF0
#define	EveryADCCycle	0x00
#define	AnyOutsideValue	0x01
#define	TowPeriods	0x02
#define	ThreePeriods	0x03
#define	FourPeriods	0x04
#define	FivePeriods	0x05
#define	SixePeriodS	0x06
#define	SevenPeriods	0x07
#define	EightPeriods	0x08
#define	NinePeriods	0x09
#define	TenPeriods	0x0A
#define	ElevenPeriods	0x0B
#define	TwelvePeriods   0x0C
#define	ThirteenPeriods	0x0D
#define	FourteenPeriods	0x0E
#define	FifteenPeriods	0x0F

///////////////////////////////////////////////////////////////////////////////////
// About ID Register
#define PARTNOMask	0x0F
#define REVNOMask	0xF0

///////////////////////////////////////////////////////////////////////////////////
//
#define T	0x0000
#define FN	0x0000
#define CL      0x0000
#define CS	0x0001

//extern u16 CalculateLux(u16 iGain, u16 tInt, u16
//ch0, u16 ch1, s16 iType);
#define LUX_SCALE       14       // scale by 2^14
#define RATIO_SCALE     9        // scale ratio by 2^9
//...................................................
// Integration time scaling factors
//...................................................
#define CH_SCALE        10       // scale channel values by 2^10
#define CHSCALE_TINT0   0x7517   // 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1   0x0fe7   // 322/81 * 2^CH_SCALE
//...................................................
// T, FN, and CL Package coefficients
//...................................................
// For Ch1/Ch0=0.00 to 0.50
// Lux/Ch0=0.0304.0.062*((Ch1/Ch0)^1.4)
// piecewise approximation
// For Ch1/Ch0=0.00 to 0.125:
// Lux/Ch0=0.0304.0.0272*(Ch1/Ch0)
//
// For Ch1/Ch0=0.125 to 0.250:
// Lux/Ch0=0.0325.0.0440*(Ch1/Ch0)
//
// For Ch1/Ch0=0.250 to 0.375:
// Lux/Ch0=0.0351.0.0544*(Ch1/Ch0)
//
// For Ch1/Ch0=0.375 to 0.50:
// Lux/Ch0=0.0381.0.0624*(Ch1/Ch0)
//
// For Ch1/Ch0=0.50 to 0.61:
// Lux/Ch0=0.0224.0.031*(Ch1/Ch0)
//
// For Ch1/Ch0=0.61 to 0.80:
// Lux/Ch0=0.0128.0.0153*(Ch1/Ch0)
//
// For Ch1/Ch0=0.80 to 1.30:
// Lux/Ch0=0.00146.0.00112*(Ch1/Ch0)
//
// For Ch1/Ch0>1.3:
// Lux/Ch0=0
//...................................................
#define K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be // 0.0272 * 2^LUX_SCALE
#define K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1 // 0.0440 * 2^LUX_SCALE
#define K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b // 0.0544 * 2^LUX_SCALE
#define K4T 0x0100 // 0.50 * 2^RATIO_SCALE
#define B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe // 0.0624 * 2^LUX_SCALE
#define K5T 0x0138 // 0.61 * 2^RATIO_SCALE
#define B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc // 0.0310 * 2^LUX_SCALE
#define K6T 0x019a // 0.80 * 2^RATIO_SCALE
#define B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb // 0.0153 * 2^LUX_SCALE
#define K7T 0x029a // 1.3 * 2^RATIO_SCALE
#define B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012 // 0.00112 * 2^LUX_SCALE
#define K8T 0x029a // 1.3 * 2^RATIO_SCALE
#define B8T 0x0000 // 0.000 * 2^LUX_SCALE
#define M8T 0x0000 // 0.000 * 2^LUX_SCALE

//...................................................
// CS package coefficients
//...................................................
// For 0 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0315.0.0593*((Ch1/Ch0)^1.4)
// piecewise approximation
// For 0 <= Ch1/Ch0 <= 0.13
// Lux/Ch0 = 0.0315.0.0262*(Ch1/Ch0)
// For 0.13 <= Ch1/Ch0 <= 0.26
// Lux/Ch0 = 0.0337.0.0430*(Ch1/Ch0)
// For 0.26 <= Ch1/Ch0 <= 0.39
// Lux/Ch0 = 0.0363.0.0529*(Ch1/Ch0)
// For 0.39 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0392.0.0605*(Ch1/Ch0)
// For 0.52 < Ch1/Ch0 <= 0.65
// Lux/Ch0 = 0.0229.0.0291*(Ch1/Ch0)
// For 0.65 < Ch1/Ch0 <= 0.80
// Lux/Ch0 = 0.00157.0.00180*(Ch1/Ch0)
// For 0.80 < Ch1/Ch0 <= 1.30
// Lux/Ch0 = 0.00338.0.00260*(Ch1/Ch0)
// For Ch1/Ch0 > 1.30
// Lux = 0
//...................................................
#define K1C 0x0043 // 0.130 * 2^RATIO_SCALE
#define B1C 0x0204 // 0.0315 * 2^LUX_SCALE
#define M1C 0x01ad // 0.0262 * 2^LUX_SCALE
#define K2C 0x0085 // 0.260 * 2^RATIO_SCALE
#define B2C 0x0228 // 0.0337 * 2^LUX_SCALE
#define M2C 0x02c1 // 0.0430 * 2^LUX_SCALE
#define K3C 0x00c8 // 0.390 * 2^RATIO_SCALE
#define B3C 0x0253 // 0.0363 * 2^LUX_SCALE
#define M3C 0x0363 // 0.0529 * 2^LUX_SCALE
#define K4C 0x010a // 0.520 * 2^RATIO_SCALE
#define B4C 0x0282 // 0.0392 * 2^LUX_SCALE
#define M4C 0x03df // 0.0605 * 2^LUX_SCALE
#define K5C 0x014d // 0.65 * 2^RATIO_SCALE
#define B5C 0x0177 // 0.0229 * 2^LUX_SCALE
#define M5C 0x01dd // 0.0291 * 2^LUX_SCALE
#define K6C 0x019a // 0.80 * 2^RATIO_SCALE
#define B6C 0x0101 // 0.0157 * 2^LUX_SCALE
#define M6C 0x0127 // 0.0180 * 2^LUX_SCALE
#define K7C 0x029a // 1.3 * 2^RATIO_SCALE
#define B7C 0x0037 // 0.00338 * 2^LUX_SCALE
#define M7C 0x002b // 0.00260 * 2^LUX_SCALE
#define K8C 0x029a // 1.3 * 2^RATIO_SCALE
#define B8C 0x0000 // 0.000 * 2^LUX_SCALE
#define M8C 0x0000 // 0.000 * 2^LUX_SCALE

///////////////////////////////////////////////////////////////////////////////////
//
// S1:PB6Ê±ÖÓ,PB7Êý¾Ý
//typedef unsigned short     s16 us1616_t;
//typedef us1616_t u16;
//typedef unsigned          char us168_t;
//typedef us168_t  u8;
#ifndef USE_TOOL_KEIL
//typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
#endif

////////////////////////////////////////////////////////////////////////////////////////////
// PB12,CLK line of S1
#define TSL2561_CLK_H   GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET)
#define TSL2561_CLK_L   GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET)

////////////////////////////////////////////////////////////////////////////////////////////
// PB14,Data line of S1
#define TSL2561_DAT_H     GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET)
#define TSL2561_DAT_L     GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET)


#define TSL2561_DATA       GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)
#define TSL2561_DAT_IN     GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;               \
                           GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	      \
                           GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;     \
                           GPIO_Init(GPIOB, &GPIO_InitStructure);
#define TSL2561_DAT_OUT    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;               \
                           GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	      \
                           GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          \
                           GPIO_Init(GPIOB, &GPIO_InitStructure);

////////////////////////////////////////////////////////////////////////////////////////////
// Define operation Command
//#define RD_SHT11_REG		0x07 // read reg
//#define RD_SHT11_TEMP			0x03 // get temperature
//#define RD_SHT11_HUM		0x05 // get humidity
//#define WT_SHT11_REG		0x06 // write status reg
//#define SHT11_RESET			0x1E // soft reset
//
//#define	MEASURE_TEMP		0x01
//#define	MEASURE_HUMI		0x02
//FlagStatus TSL2561_s16_irq_flag = RESET;

////////////////////////////////////////////////////////////////////////////////////////////
// 
#define PowerUpCode	       (0x80 | CtrlRegAdd)
#define PowerDownCode          (0x80 | CtrlRegAdd)
#define TimingRegCode          (0x80 | TimingRegAdd)
#define ThreshLowLowRegCode    (0x80 | ThreshLowLowRegAdd)
#define	ThreshLowHighRegCode   (0x80 | ThreshLowHighRegAdd)
#define ThreshHighLowRegCode   (0x80 | ThreshHighLowRegAdd)
#define ThreshHighHighRegCode  (0x80 | ThreshHighHighRegAdd)
#define InterruptRegCode       (0x80 | InterruptRegAdd)
#define IDRegCode	       (0x80 | IDRegAdd)
#define Data0LowRegCode        (0xC0 | Data0LowRegAdd)     // reading Data and Clear Interrupt//0x80|Data0LowRegAdd
#define Data0HighRegCode       (0xC0 | Data0HighRegAdd)    // 0x80 | Data0HighRegAdd
#define Data1LowRegCode        (0xC0 | Data1LowRegAdd)     // 0x80 | Data0HighRegAdd
#define Data1HighRegCode       (0xC0 | Data1HighRegAdd)    // 0x80 | Data1HighRegAdd

////////////////////////////////////////////////////////////////////////////////////////////
// Function defination
s16 TSL2561PowerDown(void);
s16 TSL2561PowerUp(void);
s16 TSL2561PowerUpWithDetection(u8 *Response);
s16 TSL2561SetThresholdLow(s16 Threshold);
s16 TSL2561SetThresholdHigh(s16 Threshold);
s16 TSL2561SetInterruptCtrlReg(u8 InterruptLogic, u8 InterruptRate);
s16 TSL2561ReadID(u8 *PartNumberID, u8 *RevisionNumberID);
s16 TSL2561ReadADC(s16 ChannelNum, u16 *LightIntensity);
s16 TSL2561SetTimmingReg(u8 GAIN, u8 Manual, u8 INTEG);

void TSL2561Init ( void );

void I2C_Trans_Start(void);
void I2C_Trans_Stop(void);
void I2C_Ack_L(void);
void I2C_Ack_H(void);
s16 I2C_Write_Byte(u8 b);
u8 I2C_Read_Byte(void);

s16 I2C_ReceivData ( u8 addr, u8 cmd, s16 len, u8 * ptr );
s16 I2C_SendData_TSL2561 ( u8 addr, u8 cmd, s16 len, u8 * ptr );
void TSL2561ReadQuery ( u16 *Lux );

u32 CalculateLux ( u16 iGain, u16 tInt, u16 ch0, u16 ch1, s16 iType );

////////////////////////////////////////////////////////////////////////////////////////////
//
#endif

