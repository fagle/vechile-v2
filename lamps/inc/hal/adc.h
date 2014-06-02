/** @file /hal/micro/adc.h
 * @brief Header for A/D converter.
 *
 * See @ref adc for documentation.
 *
 * <!-- Copyright 2008 by Ember Corporation.  All rights reserved.       *80*-->
 */
 
/** @addtogroup adc
 * Sample A/D converter driver.
 *
 * See adc.h for source code.
 *
 * @note EM35x ADC driver support is preliminary and essentailly untested -
 * please do not attempt to use this ADC driver on this platform.
 *
 * @note Except for the EM35x, the EmberZNet stack does use these functions.
 *
 * To use the ADC system, include this file and ensure that
 * ::halInternalInitAdc() is called whenever the microcontroller is
 * started.  Call ::halInternalSleepAdc() to sleep the module and
 * ::halInternalInitAdc() to wake up the module.
 *
 * A "user" is a separate thread of execution and usage.  That is,
 * internal Ember code is one user and clients are a different user.
 * But a client that is calling the ADC in two different functions
 * constitutes only one user, as long as the ADC access is not
 * interleaved.
 *
 * @note This code does not allow access to the continuous reading mode of
 * the ADC, which some clients may require.
 *
 * Many functions in this file return an ::EmberStatus value.  See
 * error-def.h for definitions of all ::EmberStatus return values.
 *
 *@{
 */

#ifndef __ADC_H__
#define __ADC_H__

#define ADC_CH0   (0x01)
#define ADC_CH1   (0x02)
#define ADC_CH2   (0x03)
#define ADC_SCALE (45)              // 0~11475, convert to 0~255

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief ADC functions employ a user ID to keep different
 * users separate.  
 *
 * Avoid many users because each user requires some
 * amount of state storage.
 *
 * @sa ::NUM_ADC_USERS
 */
enum ADCUser
#else
// A type for the ADC User enumeration.
typedef int8u ADCUser;
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
    ADC_USER_LQI  = 0x00,  // LQI User ID. 
    ADC_USER_APP  = 0x01,  // Application User ID 
    ADC_USER_APP2 = 0x02   // Application User ID 
};

/** @brief Be sure to update ::NUM_ADC_USERS if additional users are added
 * to the ::ADCUser list.
 */
//#define NUM_ADC_USERS 3 // make sure to update if the above is adjusted
#define NUM_ADC_USERS    (0x02)    // default value is 0x03 // make sure to update if the above is adjusted


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief ADC reference voltages for the AVR Atmega microcontrollers.
 */
enum ADCReferenceType
#else
// A type for the AVR Atmega reference enumeration.
typedef int8u ADCReferenceType;
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  /** AREF pin reference. */
  ADC_REF_AREF = 0x00,
  /** AVCC pin reference. */
  ADC_REF_AVCC = 0x40,
  /** Internal reference. */
  ADC_REF_INT  = 0xC0
};


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief ADC EM250 rates.
 */
enum ADCRateType
#else
// A type for the EM250 rate enumeration.
typedef int8u ADCRateType;
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  /** Rate 32 us, 5 effective bits in ADC_DATA[15:11] */
  ADC_CONVERSION_TIME_US_32   = 0x0,
  /** Rate 64 us, 6 effective bits in ADC_DATA[15:10] */
  ADC_CONVERSION_TIME_US_64   = 0x1,
  /** Rate 128 us, 7 effective bits in ADC_DATA[15:9] */
  ADC_CONVERSION_TIME_US_128  = 0x2,
  /** Rate 256 us, 8 effective bits in ADC_DATA[15:8] */
  ADC_CONVERSION_TIME_US_256  = 0x3,
  /** Rate 512 us, 9 effective bits in ADC_DATA[15:7] */
  ADC_CONVERSION_TIME_US_512  = 0x4,
  /** Rate 1024 us, 10 effective bits in ADC_DATA[15:6] */
  ADC_CONVERSION_TIME_US_1024 = 0x5,
  /** Rate 2048 us, 11 effective bits in ADC_DATA[15:5] */
  ADC_CONVERSION_TIME_US_2048 = 0x6,
  /** Rate 4096 us, 12 effective bits in ADC_DATA[15:4] */
  ADC_CONVERSION_TIME_US_4096 = 0x7,
};


#ifdef XAP2B
/** @brief EM250 ADC channel values for ::ADCChannelType.
 */
enum
{
  /** Channel 0 : ADC0 on GPIO[4] */
  ADC_SOURCE_ADC0_GND     = 0x0,
  /** Channel 1 : ADC1 on GPIO[5] */
  ADC_SOURCE_ADC1_GND     = 0x1,
  /** Channel 2 : ADC2 on GPIO[6] */
  ADC_SOURCE_ADC2_GND     = 0x2,
  /** Channel 3 : ADC3 on GPIO[7] */
  ADC_SOURCE_ADC3_GND     = 0x3,
  /** Channel 4 : Battery (VDD_PADS, 2.1-3.6V pad supply)*/
  ADC_SOURCE_VDD_PADS_GND = 0x4,
  /** Channel 5 : Regulator (VDD, 1.8V core supply */
  ADC_SOURCE_VDD_GND      = 0x5,
  /** Channel 7 : VSS (0V) */
  ADC_SOURCE_VGND         = 0x7,
  /** Channel 8 : VREF */
  ADC_SOURCE_VREF         = 0x8,
  /** Channel 9 : (Differential) ADC0-ADC1 on GPIO[4]-GPIO[5] */
  ADC_SOURCE_ADC0_ADC1    = 0x9,
  /** Channel 10: (Differential) ADC2-ADC3 on GPIO[6]-GPIO[7] */
  ADC_SOURCE_ADC2_ADC3    = 0xA
};
#endif // XAP2B

#if defined (CORTEXM3)
  /** Channel 0 : ADC0 on PB5 */
#define ADC_MUX_ADC0    0x0
  /** Channel 1 : ADC0 on PB6 */
#define ADC_MUX_ADC1    0x1
  /** Channel 2 : ADC0 on PB7 */
#define ADC_MUX_ADC2    0x2
  /** Channel 3 : ADC0 on PC1 */
#define ADC_MUX_ADC3    0x3
  /** Channel 4 : ADC0 on PA4 */
#define ADC_MUX_ADC4    0x4
  /** Channel 5 : ADC0 on PA5 */
#define ADC_MUX_ADC5    0x5
  /** Channel 8 : VSS (0V) - not for high voltage range */
#define ADC_MUX_GND     0x8
  /** Channel 9 : VREF/2 (0.6V) */
#define ADC_MUX_VREF2   0x9
  /** Channel A : VREF (1.2V)*/
#define ADC_MUX_VREF    0xA
  /** Channel B : Regulator/2 (0.9V) - not for high voltage range */
#define ADC_MUX_VREG2   0xB

// ADC_SOURCE_<pos>_<neg> selects <pos> as the positive input and <neg> as  
// the negative input.
enum
{
  ADC_SOURCE_ADC0_VREF2  = ((ADC_MUX_ADC0  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC0_GND    = ((ADC_MUX_ADC0  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC1_VREF2  = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC1_GND    = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC2_VREF2  = ((ADC_MUX_ADC2  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC2_GND    = ((ADC_MUX_ADC2  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC3_VREF2  = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC3_GND    = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC4_VREF2  = ((ADC_MUX_ADC4  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC5_VREF2  = ((ADC_MUX_ADC5  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC1_ADC0   = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_ADC0),
  ADC_SOURCE_ADC0_ADC1   = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_ADC0),

  ADC_SOURCE_ADC3_ADC2   = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_ADC2),
  ADC_SOURCE_ADC2_ADC3   = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_ADC2),

  ADC_SOURCE_ADC5_ADC4   = ((ADC_MUX_ADC5  <<ADC_MUXN_BITS) + ADC_MUX_ADC4),

  ADC_SOURCE_GND_VREF2   = ((ADC_MUX_GND   <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VGND        = ((ADC_MUX_GND   <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_VREF_VREF2  = ((ADC_MUX_VREF  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VREF        = ((ADC_MUX_VREF  <<ADC_MUXN_BITS) + ADC_MUX_GND),

/* Modified the original ADC driver for enabling the ADC extended range mode required for 
   supporting the STLM20 temperature sensor.
   NOTE: 
   The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC 
   (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
   the temperature values 
*/
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
  ADC_SOURCE_VREF2_VREF2  = ((ADC_MUX_VREF2  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VREF2        = ((ADC_MUX_VREF2  <<ADC_MUXN_BITS) + ADC_MUX_GND),
#endif

  ADC_SOURCE_VREG2_VREF2 = ((ADC_MUX_VREG2 <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VDD_GND     = ((ADC_MUX_VREG2 <<ADC_MUXN_BITS) + ADC_MUX_GND)
};
#endif // defined (CORTEXM3)

/** @brief Atmega single-ended ADC channel values for ::ADCChannelType.
 */
enum
{
  /** Channel 0 : ADC0 */
  ADC_SOURCE_ADC0     = 0x0,
  /** Channel 1 : ADC1 */
  ADC_SOURCE_ADC1     = 0x1,
  /** Channel 2 : ADC2 */
  ADC_SOURCE_ADC2     = 0x2,
  /** Channel 3 : ADC3 */
  ADC_SOURCE_ADC3     = 0x3,
  /** Channel 4 : ADC4 */
  ADC_SOURCE_ADC4     = 0x4,
  /** Channel 5 : ADC5 */
  ADC_SOURCE_ADC5     = 0x5,
  /** Channel 5 : ADC6 */
  ADC_SOURCE_ADC6     = 0x6,
  /** Channel 7 : ADC7 */
  ADC_SOURCE_ADC7     = 0x7,
};

/** @brief A type for the EM250 channel enumeration 
 * (such as ::ADC_SOURCE_ADC0_GND) and the Atmega single-ended ADC channels
 * (such as ::ADC_SOURCE_ADC0).
 */
typedef int8u ADCChannelType;


/** @brief Initializes and powers-up the ADC.  Should also be
 * called to wake from sleep.  The ADC is required for EM250 stack operation
 * so this function must be called from halInit.
 */
void halInternalInitAdc(void);


/** @brief Shuts down the voltage reference and ADC system to
 * minimize power consumption in sleep.
 */
void halInternalSleepAdc(void);


/** @brief Starts an ADC conversion for the user specified by \c id.
 *
 * @appusage The application must set \c reference to the voltage
 * reference desired (see the ADC references enum, fixed at
 * ::ADC_REF_INT for the em250), set \c channel to the channel number
 * required (see the ADC channel enum), and set \c rate to reflect the
 * number of bits of accuracy desired (see the ADC rates enum, fixed
 * at ::ADC_CONVERSION_TIME_US_256 for the Atmega).
 *
 * @param id        An ADC user.
 * 
 * @param reference Voltage reference to use, chosen from enum
 * ::ADCReferenceType (fixed at ::ADC_REF_INT for the EM250).
 * 
 * @param channel   Microprocessor channel number.  For EM250 channels, see the
 * EM250 ADC channels enum.  For basic, single-ended Atmel channels, see the
 * Atmega single-ended ADC channels enum.  For more complex measurements on
 * Atmels (differential and amped channel numbers), see the Atmel datasheet
 * for your micro.
 * 
 * @param rate      EM250 rate number (see the ADC EM250 rate enum).
 * Fixed at ::ADC_CONVERSION_TIME_US_256 for AVR Atmega.
 * 
 * @return One of the following: 
 * - EMBER_ADC_CONVERSION_DEFERRED   if the conversion is still waiting 
 * to start.
 * - EMBER_ADC_CONVERSION_BUSY       if the conversion is currently taking 
 * place.
 * - EMBER_ERR_FATAL                 if a passed parameter is invalid.
 */
EmberStatus halStartAdcConversion(ADCUser id,
                                  ADCReferenceType reference,
                                  ADCChannelType channel,
                                  ADCRateType rate);

/** @brief Returns the status of a pending conversion
 * previously started by ::halStartAdcConversion().  If the conversion
 * is complete, writes the raw register value of the conversion (the unaltered
 * value taken directly from the ADC's data register) into \c value.
 *
 * @param id     An ADC user.
 *
 * @param value  Pointer to an int16u to be loaded with the new value. Take
 * note that the Atmel's ADC only generates 8-bit values which are loaded into
 * the lower 8 bits of \c value.
 * 
 * @return One of the following: 
 * - ::EMBER_ADC_CONVERSION_DONE       if the conversion is complete.
 * - ::EMBER_ADC_CONVERSION_DEFERRED   if the conversion is still waiting 
 * to start.
 * - ::EMBER_ADC_CONVERSION_BUSY       if the conversion is currently taking 
 * place.
 * - ::EMBER_ADC_NO_CONVERSION_PENDING if \c id does not have a pending 
 * conversion.
 */
EmberStatus halRequestAdcData(ADCUser id, int16u *value);


/** @brief Waits for the user's request to complete and then,
 * if a conversion was done, writes the raw register value of the conversion
 * (the unaltered value taken directly from the ADC's data register) into
 * \c value and returns ::EMBER_ADC_CONVERSION_DONE, or immediately
 * returns ::EMBER_ADC_NO_CONVERSION_PENDING.
 *
 * @param id     An ADC user.
 *
 * @param value  Pointer to an int16u to be loaded with the new value. Take
 * note that the Atmel's ADC only generates 8-bit values which are loaded into
 * the lower 8 bits of \c value.
 * 
 * @return One of the following: 
 * - ::EMBER_ADC_CONVERSION_DONE        if the conversion is complete.
 * - ::EMBER_ADC_NO_CONVERSION_PENDING  if \c id does not have a pending 
 * conversion.
 */
EmberStatus halReadAdcBlocking(ADCUser id, int16u *value);


/** @brief Calibrates or recalibrates the ADC system. 
 *
 * @appusage Use this function to (re)calibrate as needed. This function is
 * intended for the EM250 microcontroller, which requires proper calibration to calculate
 * a human readible value (a value in volts).  If the app does not call this
 * function, the first time (and only the first time) the function 
 * ::halConvertValueToVolts() is called, this function is invoked.  To
 * maintain accurate volt calculations, the application should call this
 * whenever it expects the temperature of the micro to change.
 *
 * @param id  An ADC user.
 *
 * @return One of the following: 
 * - ::EMBER_ADC_CONVERSION_DONE        if the calibration is complete.
 * - ::EMBER_ERR_FATAL                  if the calibration failed.
 */
EmberStatus halAdcCalibrate(ADCUser id);


/** @brief Convert the raw register value (the unaltered value taken
 * directly from the ADC's data register) into a signed fixed point value with
 * units 10^-4 Volts.  The returned value will be in the range -12000 to
 * +12000 (-1.2000 volts to +1.2000 volts).
 *
 * @appusage Use this function to get a human useful value.
 *
 * @param value  An int16u to be converted.
 * 
 * @return Volts as signed fixed point with units 10^-4 Volts. 
 */
int16s halConvertValueToVolts(int16u value);


/** @brief Calibrates Vref to be 1.2V +/-10mV.
 *
 *  @appusage This function must be called from halInternalInitAdc() before
 *  making ADC readings.  This function is not intended to be called from any
 *  function other than halInternalInitAdc().  This function ensures that the
 *  master cell voltage and current bias values are calibrated before
 *  calibrating Vref.
 */
void emberCalibrateVref(void);

#ifdef CORTEXM3
void halAdcSetClock(boolean fast);
void halAdcSetRange(boolean high);
boolean halAdcGetClock(void);
boolean halAdcGetRange(void);
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ADC_Configuration
//* 功能        : init ADC function
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ADC_Configuration();

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_getadcvalue ( u8 channel )
//* 功能        : get adc convert vlaue
//* 输入参数    : u8 channel
//* 输出参数    : adc value
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_getadcvalue ( u8 channel );



#endif // __ADC_H__



