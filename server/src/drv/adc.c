#include <math.h>
#include "stm32_dsp.h"
#include "table_fft.h"
#include "freeRTOS.h"
#include "task.h"
#include "adc.h"

#ifdef ADC_ENABLE
extern uint16_t TableFFT[];
long lBufIn[NPT];              // Complex input vector 
long lBufOut[NPT];             // Complex output vector
long lBufMag[NPT + NPT/2];     // Magnitude vector

u8 adc_timer = 0x00;
__IO uint16_t ADCConvertedValue;

static void timer1_config ( void )
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
    TIM_TimeBaseStructure.TIM_Period        = 5624;          // 72M/((599+1)*(999+1)) = 120Hz
    TIM_TimeBaseStructure.TIM_Prescaler     = 0x03;          // 72M/((5624+1)*(3+1)) = 3.2k
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;    
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    /* Clear TIM1 update pending flag[清除TIM1溢出中断标志] */
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);

    /* Enable TIM1 Update interrupt [TIM1溢出中断允许]*/
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);  

    /* TIM1 enable counter [允许tim1计数]*/
    TIM_Cmd(TIM1, ENABLE);
}
#endif // ADC_ENABLE

/****************************************************************
*   Calculate powermag
*   计算各次谐波幅值;  先将lBufOut分解成实部(X)和虚部(Y)，然后计算赋值(sqrt(X*X+Y*Y)
*****************************************************************/
void dsp_asm_powerMag ( void )
{  
#ifdef ADC_ENABLE
    s16 lX,lY;  
    u32 i;  
    
    for (i = 0x00; i < NPT / 2; i ++)  
    {    
        lX  = (lBufOut[i] << 16) >> 16;    
        lY  = (lBufOut[i] >> 16);    
        {    
            float X    = NPT * ((float)lX) /32768;    
            float Y    = NPT * ((float)lY) /32768;    
            float Mag  = sqrt(X*X + Y*Y)/NPT;    
            lBufMag[i] = (u32)(Mag * 65536);    
        }  
    }
#endif  // ADC_ENABLE
}

/*******************************************************************************
* Function Name  : u16 ADC_ConvertedValue ( void )
* Description    : converte ADC value.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 ADC_ConvertedValue ( void )
{
#ifdef ADC_ENABLE
    u8 i = 0x00;
    
    vTaskSuspendAll();
    for (i = 0x00; i < NPT; i ++)
    {
        timer1_config();
        adc_timer = 0x00;
        while (!adc_timer) ;
        lBufIn[i] = (s16)ADCConvertedValue << 16;
    }
    xTaskResumeAll();
    
    cr4_fft_64_stm32(lBufOut, lBufIn, NPT);
    dsp_asm_powerMag();
    
    return lBufMag[0x01];
#else
    return 0x00;    
#endif  // ADC_ENABLE
}

/*******************************************************************************
* Function Name  : float ADC_ConvertedFreq ( void )
* Description    : converte ADC Freq.
* Input          : None
* Output         : ADC frequency
* Return         : None
*******************************************************************************/
float ADC_ConvertedFreq ( void )
{
#ifdef ADC_ENABLE
    int i = 0x00;
    int max = 0x00, max_i = 0x00;
    
    vTaskSuspendAll();
    for (i = 0x00; i < NPT; i ++)
    {
        timer1_config();
        adc_timer = 0x00;
        while (!adc_timer) ;
        lBufIn[i] = (s16)ADCConvertedValue << 16;
    }
    xTaskResumeAll();
    
    cr4_fft_1024_stm32(lBufOut, lBufIn, NPT);
    dsp_asm_powerMag();
    for (i = 0x01; i < NPT / 2; i ++)
    {
        if(lBufMag[i] > max)
        {    
            max = lBufMag[i];
            max_i = i;
        }
    }
    return (float)max_i * 0.1171875;    // (* 120hz/1024)
#else
    return 0.00;
#endif   // ADC_ENABLE
}

////////////////////////////////////////////////////////////////////////////////////
