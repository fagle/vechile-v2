
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef SENSOR_APP

#if (NUM_ADC_USERS > 8)
  #error NUM_ADC_USERS must not be greater than 8, or int8u variables in adc.c must be changed
#endif

static int16u adcData;             // conversion result written by DMA
static int8u adcPendingRequests;   // bitmap of pending requests
volatile static int8u adcPendingConversion; // id of pending conversion
static int8u adcReadingValid;      // bitmap of valid adcReadings
static int16u adcReadings[NUM_ADC_USERS];
static int16u adcConfig[NUM_ADC_USERS];
static boolean adcCalibrated;
static int16s Nvss;
static int16s Nvdd;

/* Modified the original ADC driver for enabling the ADC extended range mode required for 
   supporting the STLM20 temperature sensor.
   NOTE: 
   The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC 
   (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
   the temperature values 
*/
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
static int16s Nvref;
static int16s Nvref2;
#endif // ENABLE_ADC_EXTENDED_RANGE_BROKEN 
static int16u adcStaticConfig;

void halAdcSetClock ( boolean slow )
{
    //slow ? (adcStaticConfig |= ADC_1MHZCLK_MASK) : (adcStaticConfig &= ~ADC_1MHZCLK_MASK);
}

void halAdcSetRange ( boolean high )
{
    //high ? (adcStaticConfig |= (ADC_HVSELP_MASK | ADC_HVSELN_MASK)) : (adcStaticConfig &= ~(ADC_HVSELP_MASK | ADC_HVSELN_MASK));
}

boolean halAdcGetClock ( void )
{
    //return (adcStaticConfig & ADC_1MHZCLK_MASK) ? TRUE : FALSE;
    return TRUE;
}

boolean halAdcGetRange ( void )
{
    //return (adcStaticConfig & ((ADC_HVSELP_MASK | ADC_HVSELN_MASK))) ? TRUE : FALSE;
    return TRUE;
}

// Define a channel field that combines ADC_MUXP and ADC_MUXN
#define ADC_CHAN        (ADC_MUXP | ADC_MUXN)
#define ADC_CHAN_BIT    ADC_MUXN_BIT

void halAdcIsr ( void )
{
    Debug("goto halAdcIsr");
    INT_ADCFLAG = 0xFFFF;
}

/*void halAdcIsr ( void )
{
    int8u i;
    int8u conversion = adcPendingConversion; // fix 'volatile' warning; costs no flash

    // make sure data is ready and the desired conversion is valid
    if ((INT_ADCFLAG & INT_ADCULDFULL) && (conversion < NUM_ADC_USERS)) 
    {
        adcReadings[conversion] = adcData;
        adcReadingValid |= BIT(conversion);       // mark the reading as valid
        if (adcPendingRequests)                   // setup the next conversion if any
        {
            for (i = 0x00; i < NUM_ADC_USERS; i++) 
            {
                if (BIT(i) & adcPendingRequests) 
                {
                    adcPendingConversion = i;     // set pending conversion
                    adcPendingRequests ^= BIT(i); //clear request: conversion is starting
                    ADC_CFG = adcConfig[i]; 
                    break; //conversion started, so we're done here (only one at a time)
                }
            }
        } 
        else 
        {                                         // no conversion to do
            ADC_CFG = 0x00;                       // disable adc
            adcPendingConversion = NUM_ADC_USERS; // nothing pending, so go "idle"
        }
    }
    INT_ADCFLAG = 0xFFFF;
    asm("DMB");
}*/

// An internal support routine called from functions below. Returns the user number of the started conversion, or NUM_ADC_USERS otherwise.
ADCUser startNextConversion ( void )
{
    int8u i;
  
    ATOMIC 
    (
        if (adcPendingRequests && !(ADC_CFG & ADC_ENABLE))     // start the next requested conversion if any
        {
            for (i = 0x00; i < NUM_ADC_USERS; i++) 
            {
                if (BIT(i) & adcPendingRequests) 
                {
                    adcPendingConversion = i;     // set pending conversion
                    adcPendingRequests ^= BIT(i); // clear request
                    ADC_CFG = adcConfig[i];       // set the configuration to desired
                    INT_ADCFLAG = 0xFFFF;
                    INT_CFGSET = INT_ADC;
                }
            }
        } 
        else 
            i = NUM_ADC_USERS;
    )
    return i;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalInitAdc ( void )
//* 功能        : adc initial
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halInternalInitAdc ( void )
{
    adcPendingRequests   = 0x00;                     // reset the state variables
    adcPendingConversion = NUM_ADC_USERS;
    adcCalibrated        = FALSE;
    adcStaticConfig      = ADC_1MHZCLK | ADC_ENABLE; // init config: 1MHz, low voltage
    adcReadingValid      = 0x00;                     // set all adcReadings as invalid

    ADC_CFG     = 0x00;                              // disable ADC, turn off HV buffers
    ADC_OFFSET  = ADC_OFFSET_RESET;
    ADC_GAIN    = ADC_GAIN_RESET;
    ADC_DMACFG  = ADC_DMARST;
    ADC_DMABEG  = (int32u)&adcData;
    ADC_DMASIZE = 0x01;
    ADC_DMACFG  = (ADC_DMAAUTOWRAP | ADC_DMALOAD);

    //INT_ADCCFG  = INT_ADCULDFULL;                    // clear the ADC interrupts and enable
    INT_ADCFLAG = 0xFFFF;
    //INT_CFGSET  = INT_ADC;
    INT_ADCCFG = 0x0000;

    emberCalibrateVref();
    // Modified the original ADC driver for enabling the ADC extended range mode required for supporting the STLM20 temperature sensor.
    // NOTE: The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC (see STM32W108 errata). 
    //       As consequence, it is not reccomended to use this ADC driver for getting the temperature values 
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
    halAdcSetRange(TRUE);
#endif // ENABLE_ADC_EXTENDED_RANGE_BROKEN 
}

EmberStatus halStartAdcConversion ( ADCUser id, ADCReferenceType reference, ADCChannelType channel, ADCRateType rate )
{
    if (reference != ADC_REF_INT)
        return EMBER_ERR_FATAL;

    // save the chosen configuration for this user
    adcConfig[id] = (((rate << ADC_PERIOD_BIT) & ADC_PERIOD) | ((channel << ADC_CHAN_BIT) & ADC_CHAN) | adcStaticConfig);

    if (adcPendingRequests & BIT(id))       // if the user already has a pending request, overwrite params
        return EMBER_ADC_CONVERSION_DEFERRED;
    ATOMIC                                  // otherwise, queue the transaction
    (
        adcPendingRequests |= BIT(id);
        adcReadingValid    &= ~BIT(id);     // try and start the conversion if there is not one happening
    )
    if (startNextConversion() == id)
        return EMBER_ADC_CONVERSION_BUSY;
    return EMBER_ADC_CONVERSION_DEFERRED;
}

EmberStatus halRequestAdcData ( ADCUser id, int16u *value )
{
    // Both the ADC interrupt and the global interrupt need to be enabled, otherwise the ADC ISR cannot be serviced.
    boolean intsAreOff = (INTERRUPTS_ARE_OFF() || !(INT_CFGSET & INT_ADC) || !(INT_ADCCFG & INT_ADCULDFULL));
    EmberStatus stat;

    ATOMIC     // If interupts are disabled but the flag is set, manually run the isr..., FIXME -= is this valid???
    (
        if (intsAreOff && ((INT_CFGSET & INT_ADC) && (INT_ADCCFG & INT_ADCULDFULL))) 
            halAdcIsr();

        if (BIT(id) & adcReadingValid)    // check if we are done
        {
            *value = adcReadings[id];
            adcReadingValid ^= BIT(id);
            stat = EMBER_ADC_CONVERSION_DONE;
        } 
        else if (adcPendingRequests & BIT(id)) 
            stat = EMBER_ADC_CONVERSION_DEFERRED;
        else if (adcPendingConversion == id) 
            stat = EMBER_ADC_CONVERSION_BUSY;
        else 
            stat = EMBER_ADC_NO_CONVERSION_PENDING;
    )
    return stat;
}

EmberStatus halReadAdcBlocking ( ADCUser id, int16u *value )
{
    EmberStatus stat;

    do 
    {
#ifndef DISABLE_WATCHDOG
        halResetWatchdog();
#endif    
        stat = halRequestAdcData(id, value);
        if (stat == EMBER_ADC_NO_CONVERSION_PENDING)
            break;
    } while(stat != EMBER_ADC_CONVERSION_DONE);
    return stat;
}

EmberStatus halAdcCalibrate ( ADCUser id )
{
   EmberStatus stat;

   // Modified the original ADC driver for enabling the ADC extended range mode required for supporting the STLM20 temperature sensor.
   // NOTE: The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC (see STM32W108 errata). 
   // As consequence, it is not reccomended to use this ADC driver for getting the temperature values 
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
   if (halAdcGetRange())
   {
       halStartAdcConversion(id, ADC_REF_INT, ADC_SOURCE_VREF_VREF2, ADC_CONVERSION_TIME_US_4096);
       stat = halReadAdcBlocking(id, (int16u *)(&Nvref));
       if (stat == EMBER_ADC_CONVERSION_DONE) 
       {
           halStartAdcConversion(id, ADC_REF_INT, ADC_SOURCE_VREF2_VREF2, ADC_CONVERSION_TIME_US_4096);
           stat = halReadAdcBlocking(id, (int16u *)(&Nvref2));
       }
       if (stat == EMBER_ADC_CONVERSION_DONE) 
       {
           adcCalibrated = TRUE;
       }
       else 
       {
           adcCalibrated = FALSE;
           stat = EMBER_ERR_FATAL;
       }
       return stat;    
    }  
#endif // ENABLE_ADC_EXTENDED_RANGE_BROKEN 
    halStartAdcConversion(id, ADC_REF_INT, ADC_SOURCE_GND_VREF2, ADC_CONVERSION_TIME_US_4096);
    stat = halReadAdcBlocking(id, (int16u *)(&Nvss));
    if (stat == EMBER_ADC_CONVERSION_DONE) 
    {
        halStartAdcConversion(id, ADC_REF_INT, ADC_SOURCE_VREG2_VREF2, ADC_CONVERSION_TIME_US_4096);
        stat = halReadAdcBlocking(id, (int16u *)(&Nvdd));
    }
    if (stat == EMBER_ADC_CONVERSION_DONE) 
    {
        Nvdd -= Nvss;
        adcCalibrated = TRUE;
    } 
    else 
    {
        adcCalibrated = FALSE;
        stat = EMBER_ERR_FATAL;
    }
    return stat;
}

// Use the ratio of the sample reading to the of VDD_PADSA/2, known to be 900mV, to convert to 100uV units.
// FIXME: support external Vref use #define of Vref, ignore VDD_PADSA
// FIXME: support EM350 high voltage range  use Vref-Vref/2 to calibrate
// FIXME: check for mfg token specifying measured VDD_PADSA
int16s halConvertValueToVolts ( int16u value )
{
    int32s N;
    int16s V;
    int32s nvalue;

    if (!adcCalibrated) 
        halAdcCalibrate(ADC_USER_LQI);

    if (adcCalibrated) 
    {
        // Modified the original ADC driver for enabling the ADC extended range mode required for supporting the STLM20 temperature sensor.
        // NOTE: The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC (see STM32W108 errata). 
        //       As consequence, it is not reccomended to use this ADC driver for getting the temperature values 
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
        if (halAdcGetRange())    // High range.
        {  
            N = (((int32s)value + Nvref - 2 * Nvref2) << 16) / (0x02 * (Nvref - Nvref2));
            // Calculate voltage with: V = (N * VREF) / (2^16) where VDD = 1.2 volts Mutiplying by 1.2*10000 makes the result of this equation 100 uVolts
            V = (int16s)((N * 12000L) >> 16);
            if (V > 21000) 
                V = 21000;
        }
        else 
        { 
#endif // ENABLE_ADC_EXTENDED_RANGE_BROKEN 
            assert(Nvdd);
            nvalue = value - Nvss;    // Convert input value (minus ground) to a fraction of VDD/2.
            N = ((nvalue << 16) + Nvdd / 0x02) / Nvdd;
            // Calculate voltage with: V = (N * VDD/2) / (2^16) where VDD/2 = 0.9 volts Mutiplying by0.9*10000 makes the result of this 
            // equation 100 uVolts (in fixed point E-4 which allows for 13.5 bits vs millivolts which is only 10.2 bits).
            V = (int16s)((N * 9000L) >> 16);
            if (V > 11475)    // 12000), // 0~11475, convert to 0~255
                V = 11475;    // 12000;
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
        }
#endif // ENABLE_ADC_EXTENDED_RANGE_BROKEN 
    } 
    else 
        V = -32768;
    V = (V < 0x00) ? (0x00) : (V);
    return V;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ADC_Configuration
//* 功能        : init ADC function
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ADC_Configuration ( void )
{
    halInternalInitAdc();
    halGpioConfig(PORTB_PIN(5), 0x00);  // 将PB5设为模拟输出
    halGpioConfig(PORTB_PIN(6), 0x00);  // 将PB6设为模拟输出
    halGpioConfig(PORTB_PIN(7), 0x00);
}

 

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_getadcvalue ( u8 channel )
//* 功能        : get adc convert vlaue
//* 输入参数    : u8 channel
//* 输出参数    : adc value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_getadcvalue ( u8 channel )
{
    int16u adcValue = 0x00;
  
    adcData = 0x00;
    if (channel == ADC_CH0)
    {
        halStartAdcConversion(ADC_USER_APP, ADC_REF_INT, ADC_SOURCE_ADC0_VREF2, ADC_CONVERSION_TIME_US_4096);
        halReadAdcBlocking(ADC_USER_APP, &adcValue); // This blocks for a while, about 4ms.
    }
    else if (channel == ADC_CH1)
    {
        halStartAdcConversion(ADC_USER_APP, ADC_REF_INT, ADC_SOURCE_ADC1_VREF2, ADC_CONVERSION_TIME_US_4096);
        halReadAdcBlocking(ADC_USER_APP, &adcValue); // This blocks for a while, about 4ms.
    }
    adcValue = halConvertValueToVolts(adcValue);
 
      
    return adcValue;  
}

u16 DMA2[10];


void startADC0(void)
{
  ADC_CFG &= ~ADC_ENABLE;
  
  
  ADC_DMACFG = ADC_DMARST;
  ADC_DMABEG = (int32u)DMA2;
  ADC_DMASIZE = 10;
  ADC_DMACFG = ADC_DMALOAD|ADC_DMAAUTOWRAP;
  
  INT_ADCFLAG = 0xFFFF;
  
  ADC_CFG = ((ADC_CONVERSION_TIME_US_4096 << ADC_PERIOD_BIT) & ADC_PERIOD_MASK)
          | ((ADC_SOURCE_ADC0_VREF2 << ADC_CHAN_BIT) & ADC_CHAN)
          | ((1 << ADC_1MHZCLK_BIT) & ADC_1MHZCLK_MASK);
  ADC_CFG |= ADC_ENABLE;
}

void stopADC(void)
{
  ADC_CFG &= ~ADC_ENABLE;
}

#else    // SINK_APP, need function of halInternalPowerUpBoard()

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalInitAdc ( void )
//* 功能        : adc initial
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halInternalInitAdc ( void )
{}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif   // SENSOR_APP

