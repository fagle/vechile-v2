/*
 * File: bootloader-uart.c
 * Description: em35x bootloader serial interface functions for a uart
 *
 *
 * Copyright 2009 by Ember Corporation. All rights reserved.                *80*
 */
//[[ Author(s): David Iacobone, diacobone@ember.com  ]]

#include PLATFORM_HEADER  // should be iar.h
#include "bootloader-common.h"
#include "bootloader-serial.h"
#include "stack/include/ember-types.h"
#include "hal.h"


/* Baud rate settings

Baud Rate  | SC1_UARTPER | SC1_UARTFRAC
---------------------------------------
300        | 40000       | 0
4800       | 2500        | 0
9600       | 1250        | 0
19200      | 625         | 0
38400      | 312         | 1
57600      | 208         | 1
115200     | 104         | 0
460800     | 26          | 0
921600     | 13          | 0

*/

// Function Name: serInit
// Description:   This function configures the UART. These are default 
//                settings and can be modified for custom applications.
// Parameters:    none
// Returns:       none
//
void serInit(void) 
{
  // see table above for alternative buad rate values
  // set uart to 115.2k baud
  SC1_UARTPER = 104;   // baud rate
  SC1_UARTFRAC   = 0;  // baud rate
  
  // set to 8 bits, no parity, 1 stop bit
  SC1_UARTCFG = BIT(SC_UART8BIT_BIT);
  SC1_MODE = SC1_MODE_UART;   // SC1 set to uart mode

  // configure IO pins to use UART
  halGpioConfig(PORTB_PIN(1),GPIOCFG_OUT_ALT);
  halGpioConfig(PORTB_PIN(2),GPIOCFG_IN);
  
  // init Timer 1 for use as serial input timeout timer
  // defaults to internal, 12MHz clock
  TIM1_CR1 = TIM1_CR1_RESET;      // reset/disable timer
  TIM1_PSC = 10;                  // prescaler
  TIM1_CR1 &= ~TIM_DIR;           //set dir bit to count up
  TIM1_CCR1 = TIM1_CCR1_RESET;    // set capture/compare to 0
  TIM1_CNT = TIM1_CNT_RESET;
}


void serPutFlush(void)
{
  while ( !(SC1_UARTSTAT & SC_UARTTXIDLE) ) 
  { /*wait for txidle*/ }
}

// wait for transmit free then send char
void serPutChar(int8u ch)
{
  while ( !(SC1_UARTSTAT & SC_UARTTXFREE))
  { /*wait for txfree*/ }
  SC1_DATA = ch;
}

void serPutStr(const char *str)
{
  while (*str) {
    serPutChar(*str);
    str++;
  }
}

void serPutBuf(const int8u buf[], int8u size)
{
  int16u i;

  for (i=0; i<size; i++) {
    serPutChar(buf[i]);
  }
}


void serPutHex(int8u byte)
{
  int8u val;
  val = ((byte & 0xF0) >> 4);
  serPutChar((val>9)?(val-10+'A'):(val+'0') );
  val = (byte & 0x0F);
  serPutChar((val>9)?(val-10+'A'):(val+'0') );
}

void serPutHexInt(int16u word)
{
  serPutHex(HIGH_BYTE(word));
  serPutHex(LOW_BYTE(word));
}

boolean serCharAvailable(void)
{
  if(SC1_UARTSTAT & SC_UARTRXVAL) {
    return TRUE;
  } else {
    return FALSE;
  }
}

// get char if available, else return error
BL_Status serGetChar(int8u* ch)
{
  if(serCharAvailable()) {
    *ch = SC1_DATA;
    return BL_SUCCESS;
  } else {
    return BL_ERR;
  }
}

// get chars until rx buffer is empty
void serGetFlush(void)
{
  int8u status = BL_SUCCESS;
  int8u tmp;
  do {
    status = serGetChar(&tmp);
  } while(status == BL_SUCCESS);
}

// If ch is NULL we just return when data is available
BL_Status serWaitChar(int8u* ch, int16u timeout)
{
  int8u status;
  
  INT_TIM1FLAG = 0xffffffff;   // clear flags
  TIM1_ARR = timeout;
  TIM1_CNT = 0;   // start value
  TIM1_CR1 |= TIM_CEN;   // Start timer running 

  do {
    halResetWatchdog();

    //[[
    #if 0 //ndef NO_RADIO
      //TODO OTA: abstract me better
      if (commState == COMM_RADIO){
        err = radGetChar(ch, timeout);
      } else if (commState == COMM_SERIAL)
    #endif
    //]]
    
    if(ch == NULL) {
      status = (serCharAvailable())? BL_SUCCESS : BL_ERR;
    } else {
      status = serGetChar(ch);
    }
    
    if (status == BL_SUCCESS) { 
      TIM1_CR1 &= ~TIM_CEN;            // stop timer, got a valid char
      return BL_SUCCESS;
    }
  } while ((timeout > TIM1_CNT) || (timeout==TIMEOUT_FOREVER));
  TIM1_CR1 &= ~TIM_CEN;            // stop timer
  return TIMEOUT;
}
