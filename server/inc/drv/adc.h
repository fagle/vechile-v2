#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f10x.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////////////
//
#define PI2  6.28318530717959
#define NPT  64                          // NPT = No of FFT point

#define ADCTIMEOUT         0x3000000
#define ADC1_DR_Address    ((uint32_t)0x4001244C)

/*******************************************************************************
* Function Name  : ADC_Configuration
* Description    : Initializes the ADC.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_Configuration ( void );

/*******************************************************************************
* Function Name  : u16 ADC_ConvertedValue ( void )
* Description    : converte ADC value.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 ADC_ConvertedValue ( void );

/*******************************************************************************
* Function Name  : float ADC_ConvertedFreq ( void )
* Description    : converte ADC Freq.
* Input          : None
* Output         : ADC frequency
* Return         : None
*******************************************************************************/
float ADC_ConvertedFreq ( void );

#endif /* __ADC_H__ */


