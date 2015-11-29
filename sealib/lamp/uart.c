#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"

#if defined(EZSP_UART) && \
    !defined(EMBER_SERIAL1_RTSCTS) && \
    !defined(EMBER_SERIAL1_XONXOFF)
  #error EZSP-UART requires either RTS/CTS or XON/XOFF flow control!
#endif

#ifdef EMBER_SERIAL1_XONXOFF
  #if EMBER_SERIAL1_MODE != EMBER_SERIAL_FIFO
  #error "Illegal serial port 1 configuration"
  #endif

  static void halInternalUart1ForceXon(void); // forward declaration

  static int8s xcmdCount;         // number of XON/XOFFs sent to host:
                                  // XOFFs count -1, XONs count +1, 0 = XON state
  static int8u xonXoffTxByte;     // if non-zero, transmitted ahead of queued data
  static int8u xonTimer;          // time since data rx'ed from the host (1/4 secs)

  #define ASCII_XON         0x11  // requests host to pause sending 
  #define ASCII_XOFF        0x13  // requests host to resume sending
  #define XON_REFRESH_TIME  8     // delay between repeat XONs (1/4 sec units)
  #define XON_REFRESH_COUNT 3     // max number of repeat XONs to send after 1st

  // Define thresholds for XON/XOFF flow control in terms of queue used values
  // Take into account the 4 byte transmit FIFO
  #if (EMBER_SERIAL1_RX_QUEUE_SIZE == 128)
    #define XON_LIMIT       16    // send an XON
    #define XOFF_LIMIT      96    // send an XOFF
  #elif (EMBER_SERIAL1_RX_QUEUE_SIZE == 64)
    #define XON_LIMIT       8
    #define XOFF_LIMIT      36
  #elif (EMBER_SERIAL1_RX_QUEUE_SIZE == 32)
    #define XON_LIMIT       2 
    #define XOFF_LIMIT      8
  #else
    #error "Serial port 1 receive buffer too small!"
  #endif
#endif  // EMBER_SERIAL1_XONXOFF

#if !defined(EM_SERIAL1_DISABLED)
  //The following registers are the only SC1-UART registers that need to be
  //saved across deep sleep cycles.  All other SC1-UART registers are 
  //reenabled or restarted using more complex init or restart alrogithms.
  static int32u  SC1_UARTPER_SAVED;
  static int32u  SC1_UARTFRAC_SAVED;
  static int32u  SC1_UARTCFG_SAVED;
#endif//!defined(EM_SERIAL1_DISABLED)  
  
////////////////////// SOFTUART Pin and Speed definitions //////////////////////
//use a logic analyzer and trial and error to determine these values if
//the SysTick time changes or you want to try a different baud
//These were found using EMU 0x50
#define FULL_BIT_TIME_PCLK  0x4E0  //9600 baud with FLKC @ PCLK(12MHz)
#define START_BIT_TIME_PCLK 0x09C  //9600 baud with FLKC @ PCLK(12MHz)
#define FULL_BIT_TIME_SCLK  0x9C0  //9600 baud with FLKC @ SCLK(24MHz)
#define START_BIT_TIME_SCLK 0x138  //9600 baud with FLKC @ SCLK(24MHz)
//USE PB6 (GPIO22) for TXD
#define CONFIG_SOFT_UART_TX_BIT() \
  GPIO_PCCFGH = (GPIO_PCCFGH&(~PC6_CFG_MASK)) | (1 << PC6_CFG_BIT)
#define SOFT_UART_TX_BIT(bit)  GPIO_PCOUT = (GPIO_PCOUT&(~PC6_MASK))|((bit)<<PC6_BIT)
//USE PB7 (GPIO23) for RXD
#define CONFIG_SOFT_UART_RX_BIT() \
  GPIO_PCCFGH = (GPIO_PCCFGH&(~PC7_CFG_MASK)) | (4 << PC7_CFG_BIT)
#define SOFT_UART_RX_BIT  ((GPIO_PCIN&PC7)>>PC7_BIT)
////////////////////// SOFTUART Pin and Speed definitions //////////////////////

#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
  void halInternalRestartUart1Dma ( void );
#endif

#if defined(EMBER_SERIAL1_RTSCTS) && (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
  void halInternalUart1RxCheckRts ( void );
#else
  #define halInternalUart1RxCheckRts()
#endif

// Allow some code to be disabled (and flash saved) if
//  a port is unused or in low-level driver mode
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_UNUSED)
  #define EM_SERIAL0_DISABLED
#endif
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_LOWLEVEL)
  #error Serial 0 (Virtual Uart) does not support LOWLEVEL mode
#endif
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_UNUSED) || \
    (EMBER_SERIAL1_MODE == EMBER_SERIAL_LOWLEVEL)
  #define EM_SERIAL1_DISABLED
#endif

// Define receive interrupts and errors for FIFO mode
#ifndef EMBER_SERIAL1_IGNORE_ERRORS
  // Interrupts enabled for uart receiver
  #define INT_SC1CFG_RX_INTS_FIFO   ( INT_SCRXVAL    | \
                                      INT_SCRXOVF    | \
                                      INT_SC1FRMERR  | \
                                      INT_SC1PARERR  )
  // Errors enabled for uart receiver
  #define SC1_UARTSTAT_ERRORS_FIFO  ( SC_UARTFRMERR  | \
                                      SC_UARTRXOVF   | \
                                      SC_UARTPARERR  )
#else // Disable framing and parity, but not overflow, errors
  #define INT_SC1CFG_RX_INTS_FIFO   ( INT_SCRXVAL    | \
                                      INT_SCRXOVF    )
  #define SC1_UARTSTAT_ERRORS_FIFO  ( SC_UARTRXOVF )
#endif

#define INVALID -1 // frequency error exceeds +/-0.5%, given sysclock

// Save flash if ports are undefined
#if     !defined(EM_SERIAL1_DISABLED)

  const int32s baudSettings[] = {
      // Negative values indicate 0.5 fractional bit should be set
     40000,      // 300     0.00%(40000.00)
     20000,      // 600     0.00%(20000.00)
    -13333,      // 900    -0.01%(13333.50 but desire 13333.33; slow by 0.01%)
     10000,      // 1200    0.00%(10000.00)
      5000,      // 2400    0.00% (5000.00)
      2500,      // 4800    0.00% (2500.00)
      1250,      // 9600    0.00% (1250.00)
      -833,      // 14.4k  -0.02% ( 833.50 but desire 833.33; slow by 0.02%)
       625,      // 19.2k   0.00% ( 625.00)
      -416,      // 28.8k  +0.04% ( 416.50 but desire 416.67; fast by 0.04%)
      -312,      // 38.4k   0.00% ( 312.50)
       240,      // 50.0k   0.00% ( 240.00)
      -208,      // 57.6k  -0.08% ( 208.50 but desire 208.33; slow by 0.08%)
       156,      // 76.8k  +0.16% ( 156.00 but desire 156.25; fast by 0.16%)
       120,      // 100.0k  0.00% ( 120.00)
       104,      // 115.2k +0.16% ( 104.00 but desire 104.17; fast by 0.16%)
        52,      // 230.4k +0.16% (  52.00 but desire  52.08; fast by 0.16%)
        26,      // 460.8k +0.16% (  26.00 but desire  26.04; fast by 0.16%)
       #ifdef EMBER_SERIAL_BAUD_CUSTOM
         EMBER_SERIAL_BAUD_CUSTOM, //Hook for custom baud rate, see BOARD_HEADER
       #else
        13,      // 921.6k +0.16% (  13.00 but desire  13.02; fast by 0.16%)
       #endif
  };

#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
////////////////////////////////////////////////////////////////////////////////
//
static u8 buffer_dma[EMBER_SERIAL1_QUEUE_SIZE];
static const int16u fifoSize = EMBER_SERIAL1_QUEUE_SIZE;
extern uart_t  uart_rx, uart_tx;
#endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)

#endif//!defined(EM_SERIAL1_DISABLED)

// Forward prototype for TX
void halInternalUart1TxIsr ( void );
void halInternalUart1RxIsr ( int16u );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : halInternalUartInit
//* 功能        : uart初始化
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
#if (!defined(EM_SERIAL0_DISABLED) || !defined(EM_SERIAL1_DISABLED))
EmberStatus halInternalUartInit ( int8u port, SerialBaudRate rate, SerialParity parity, int8u stopbits )
{
    int32u tempcfg;

#if !defined(EM_SERIAL0_DISABLED)
    if (port == 0x00) 
    {
        // Nothing special to do since the debug channel handles this
        return EMBER_SUCCESS;
    }
#endif//!defined(EM_SERIAL0_DISABLED)

#if !defined(EM_SERIAL1_DISABLED)
#ifdef SOFTUART
    //make sure the TX bit starts at idle high
    SOFT_UART_TX_BIT(1);
    CONFIG_SOFT_UART_TX_BIT();
    CONFIG_SOFT_UART_RX_BIT();
#else //SOFTUART
    if (port == 0x01) 
    { 
        if ((rate >= sizeof(baudSettings) / sizeof(*baudSettings)) ||
          (baudSettings[rate] == INVALID) ) 
        {
            return EMBER_SERIAL_INVALID_BAUD_RATE;
        }

        // Negative indicates set .5 fractional bit
        if (baudSettings[rate] < 0x00) 
        {
            SC1_UARTPER  = -baudSettings[rate];
            SC1_UARTFRAC = 0x01;
        } 
        else 
        {
            SC1_UARTPER  = baudSettings[rate];
            SC1_UARTFRAC = 0x00;
        }

        // Default is always 8 data bits irrespective of parity setting,
        // according to Lee, but hack overloads high-order nibble of stopbits to
        // allow user to specify desired number of data bits:  7 or 8 (default).
        if (((stopbits & 0xF0) >> 0x04) == 0x07) 
        {
            tempcfg = 0x00;
        } 
        else 
        {
            tempcfg = SC_UART8BIT;
        }
      
        if (parity == PARITY_ODD) 
        {
            tempcfg |= SC_UARTPAR | SC_UARTODD;
        } 
        else if (parity == PARITY_EVEN) 
        {
            tempcfg |= SC_UARTPAR;
        }

        if ((stopbits & 0x0F) >= 0x02) 
        {
            tempcfg |= SC_UART2STP;
        }
        SC1_UARTCFG = tempcfg;
        SC1_MODE = SC1_MODE_UART;

#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
        // Make the RX Valid interrupt level sensitive (instead of edge)
        SC1_INTMODE = SC_RXVALLEVEL;    // SC_SPIRXVALMODE;
        // Enable just RX interrupts; TX interrupts are controlled separately
        INT_SC1CFG |= (INT_SCRXVAL   |
                       INT_SCRXOVF   |
                       INT_SC1FRMERR |
                       INT_SC1PARERR);
        INT_SC1FLAG = 0xFFFF; // Clear any stale interrupts
        INT_CFGSET  = INT_SC1;
#ifdef EMBER_SERIAL1_XONXOFF
        halInternalUart1ForceXon();
#endif
#elif (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
        {
          SC1_RXBEGA =  (int32u)&buffer_dma;                            // setup the addresses for receive DMA
          SC1_RXENDA = (int32u)(&buffer_dma + fifoSize / 0x02 - 0x01);
          SC1_RXBEGB = (int32u)(&buffer_dma + fifoSize / 0x02);
          SC1_RXENDB = (int32u)(&buffer_dma + fifoSize - 0x01);
          
          SC1_DMACTRL = (SC_RXLODA | SC_RXLODB);                   // activate DMA
        }
#ifndef EZSP_UART
        INT_SC1CFG |= (INT_SCRXOVF   |
                       INT_SC1FRMERR |
                       INT_SC1PARERR);
#endif
        // The receive side of buffer mode does not require any interrupts. The transmit side of buffer mode requires interrupts.
        // Enable the top level SC1 interrupt for the transmit side.
        INT_SC1FLAG = 0xFFFF;                                     // Clear any stale interrupts
        INT_CFGSET  = INT_SC1;                                    // Enable top-level interrupt

#ifdef EMBER_SERIAL1_RTSCTS
        INT_SC1CFG |= (INT_SCRXULDA | INT_SCRXULDB);
        SC1_UARTCFG |= (SC_UARTFLOW | SC_UARTRTS);
#endif 
#endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
        return EMBER_SUCCESS;
    }
#endif //SOFTUART
#endif//!defined(EM_SERIAL1_DISABLED)

    return EMBER_SERIAL_INVALID_PORT;
}
#endif//(!defined(EM_SERIAL0_DISABLED) || !defined(EM_SERIAL1_DISABLED))

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : halInternalStartUartTx(int8u port)
//* 功能        : Uart开始发送
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void halInternalStartUartTx ( int8u port )
{   
//    if (uart_tx.ept(&uart_tx))
//        return;
    ATOMIC
    (
        INT_SC1CFG |= (INT_SCTXFREE | INT_SCTXIDLE);          // Enable TX interrupts
        halInternalUart1TxIsr();                              // Pretend we got a tx interrupt
    )
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalStopUartTx(int8u port)
//* 功能        : Uart停止 发送
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void halInternalStopUartTx(int8u port)
{
  // Nothing for port 0 (virtual uart)

  #if !defined(EM_SERIAL1_DISABLED)
    if (port == 1) 
    {
        // Disable TX Interrupts
        INT_SC1CFG &= ~(INT_SCTXFREE | INT_SCTXIDLE);    
    }
  #endif//!defined(EM_SERIAL1_DISABLED)
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : halInternalForceWriteUartData
//* 功能        : 强制性的发送数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : //full blocking, no queue overflow issues, can be used in or out of int context
//                  does not return until character is transmitted.
//*------------------------------------------------*/
EmberStatus halInternalForceWriteUartData(int8u port, int8u *data, int8u length)
{
  #if !defined(EM_SERIAL0_DISABLED)
    if (port == 0) {
      emDebugSendVuartMessage(data, length);
      return EMBER_SUCCESS;
    }
  #endif//!defined(EM_SERIAL0_DISABLED)

  #if !defined(EM_SERIAL1_DISABLED)  
      //if the port is configured, go ahead and transmit
      if ( (port == 1) && (SC1_MODE == SC1_MODE_UART) ) {
        while (length--) {
          //spin until data register has room for more data
          while ((SC1_UARTSTAT&SC_UARTTXFREE)!=SC_UARTTXFREE) {}
          SC1_DATA = *data;
          data++;
        }
  
        //spin until TX complete (TX is idle)
        while ((SC1_UARTSTAT&SC_UARTTXIDLE)!=SC_UARTTXIDLE) {}
  
        return EMBER_SUCCESS;
      }
  #endif//!defined(EM_SERIAL1_DISABLED)

  return EMBER_SERIAL_INVALID_PORT;
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalWaitUartTxComplete(int8u port)
//* 功能        : 等待发送数据完毕
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : blocks until the text actually goes out
//*------------------------------------------------*/
void halInternalWaitUartTxComplete(int8u port)
{
  halResetWatchdog();

  // Nothing to do for port 0 (virtual uart)

  #if !defined(EM_SERIAL1_DISABLED) 
    if ( port == 1 ) 
    {
      while ( !(SC1_UARTSTAT & SC_UARTTXIDLE) ) {}
      return;
    }
  #endif//!defined(EM_SERIAL1_DISABLED) 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalUartRxPump(int8u port)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halInternalUartRxPump(int8u port)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalPowerDownUart(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/  
void halInternalPowerDownUart(void)
{
  #if !defined(EM_SERIAL1_DISABLED)
    SC1_UARTPER_SAVED = SC1_UARTPER;
    SC1_UARTFRAC_SAVED = SC1_UARTFRAC;
    SC1_UARTCFG_SAVED = SC1_UARTCFG;
  #endif//!defined(EM_SERIAL1_DISABLED)
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalPowerUpUart(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halInternalPowerUpUart(void)
{
  #if !defined(EM_SERIAL1_DISABLED)
    SC1_UARTPER = SC1_UARTPER_SAVED;
    SC1_UARTFRAC = SC1_UARTFRAC_SAVED;
    SC1_UARTCFG = SC1_UARTCFG_SAVED;

    SC1_MODE = SC1_MODE_UART;
    
    #if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
      //make the RX Valid interrupt level sensitive (instead of edge)
      //SC1_INTMODE = SC_SPIRXVALMODE;
      //enable just RX interrupts; TX interrupts are controlled separately
      INT_SC1CFG |= (INT_SCRXVAL   |
                     INT_SCRXOVF   |
                     INT_SC1FRMERR |
                     INT_SC1PARERR);
      INT_SC1FLAG = 0xFFFF; // Clear any stale interrupts
      INT_CFGSET = INT_SC1;
    #elif (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
      halInternalRestartUart1Dma();
    #endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
  #endif//!defined(EM_SERIAL1_DISABLED)
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalRestartUart(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halInternalRestartUart ( void )
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : halSc1Isr（）
//* 功能        : 注册的中断服务程序
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void halSc1Isr ( void )
{
    int32u interrupt;

    // this read and mask is performed in two steps otherwise the compiler
    // will complain about undefined order of volatile access
    interrupt  = INT_SC1FLAG;
    interrupt &= INT_SC1CFG;
  
#if !defined(EM_SERIAL1_DISABLED)
    while (interrupt != 0x00) 
    {
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO) && defined(EMBER_SERIAL1_RTSCTS)
        // In FIFO mode with RTS flow control, don't clear Rx interrupts here - let
        // the receive ISR do it since it must leave an interrupt pending when the
        // software FIFO is full.
        INT_SC1FLAG = interrupt & ~INT_SC1CFG_RX_INTS_FIFO;
#else
        INT_SC1FLAG = interrupt; // acknowledge the interrupts early
#endif

        // RX events
        if (interrupt & (INT_SCRXVAL   | // RX has data
                         INT_SCRXOVF   | // RX Overrun error
                         INT_SCRXFIN   | // RX done [TWI]
                         INT_SCNAK     | // RX Nack [TWI]
                         INT_SCRXULDA  | // RX DMA A has data
                         INT_SCRXULDB  | // RX DMA B has data
                         INT_SC1FRMERR | // RX Frame error
                         INT_SC1PARERR)) // RX Parity error
        {
            halInternalUart1RxIsr(interrupt);
        }
    
        // TX events
        if (interrupt & (INT_SCTXFREE | // TX has room
                         INT_SCTXIDLE | // TX idle (more room)
                         INT_SCTXUND  | // TX Underrun [SPI/TWI]
                         INT_SCTXFIN  | // TX complete [TWI]
                         INT_SCCMDFIN | // TX Start/Stop done [TWI]
                         INT_SCTXULDA | // TX DMA A has room
                         INT_SCTXULDB)) // TX DMA B has room
        {
            halInternalUart1TxIsr();
        }
    
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
        interrupt = INT_SC1FLAG;
        interrupt &= INT_SC1CFG;
#endif // (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
    }
#endif//!defined(EM_SERIAL1_DISABLED)
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus halInternalForceReadUartByte(int8u port, int8u* dataByte)
//* 功能        : 强制性的等待接收数据
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : Useful for waiting on serial port characters before interrupts have been
//*                  turned on.
//*------------------------------------------------*/
EmberStatus halInternalForceReadUartByte ( int8u port, int8u* dataByte )
{
    EmberStatus err = EMBER_SUCCESS;

#if !defined(EM_SERIAL0_DISABLED)
    if (port == 0x00) 
    {
        EmSerialFifoQueue *q = emSerialRxQueues[0];
        ATOMIC
        (
            if (q->used == 0) 
            {
                WAKE_CORE = WAKE_CORE_FIELD;
            }
            if (q->used > 0) 
            {
                *dataByte = FIFO_DEQUEUE(q, emSerialRxQueueMasks[0]);
            } 
            else 
            {
                err = EMBER_SERIAL_RX_EMPTY;
            }
        )
    }
#endif//!defined(EM_SERIAL0_DISABLED)

#if !defined(EM_SERIAL1_DISABLED)
    if ( port == 1 ) 
    {
        if ( SC1_UARTSTAT & SC_UARTRXVAL ) 
        {
            *dataByte = (int8u) SC1_DATA;
        }
        else 
        {
            err = EMBER_SERIAL_RX_EMPTY;
        }
    }
#endif//!defined(EM_SERIAL1_DISABLED)
    return err;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalWaitUartTxComplete(int8u port)
//* 功能        : Debug Channel calls this ISR to push up data it has received
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void halStackReceiveVuartMessage(int8u *data, int8u length)
{
  #if !defined(EM_SERIAL0_DISABLED)
    EmSerialFifoQueue *q = emSerialRxQueues[0];

    while (length--) {
      // Since queue sizes are powers-of-two the Mask is the same as
      //  the queue size minus 1, so is safe to use in this comparison
      if ((q->used < emSerialRxQueueMasks[0])) {
        FIFO_ENQUEUE(q,*data++,emSerialRxQueueMasks[0]);
      } else {    
        // save error code & location in queue
        if (emSerialRxError[0] == EMBER_SUCCESS) {
          emSerialRxErrorIndex[0] = q->head;
          emSerialRxError[0] = EMBER_SERIAL_RX_OVERFLOW;
        } else {
          // Flush back to previous error location & update value
          q->head = emSerialRxErrorIndex[0];
          emSerialRxError[0] = EMBER_SERIAL_RX_OVERFLOW;
          q->used = ((q->head - q->tail) & emSerialRxQueueMasks[0]);
        }
        return;  // no sense in trying to enqueue the rest
      }
    }
  #else  //!defined(EM_SERIAL0_DISABLED)
    return;  // serial 0 not used, drop any input
  #endif //!defined(EM_SERIAL0_DISABLED)
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : softwareUartTxByte(int8u byte)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#ifdef SOFTUART
//this requires use of the SysTick counter and will destory interrupt latency!
static void softwareUartTxByte(int8u byte)
{
  int8u i;
  int16u fullBitTime;
  
  if (CPU_CLKSEL) {
    fullBitTime = FULL_BIT_TIME_SCLK;
  } else {
    fullBitTime = FULL_BIT_TIME_PCLK;
  }

  ATOMIC(
    ST_RVR = fullBitTime; //set the SysTick reload value register
    //enable core clock reference and the counter itself
    ST_CSR = (ST_CSR_CLKSOURCE | ST_CSR_ENABLE);
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
    
    //go low for start bit
    SOFT_UART_TX_BIT(0); //go low for start bit
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
    
    //loop over all 8 data bits transmitting each
    for (i=0;i<8;i++) {
      SOFT_UART_TX_BIT(byte&0x1); //data bit
      while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
      byte = (byte>>1);
    }

    SOFT_UART_TX_BIT(1); //stop bit
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time

    //disable SysTick
    ST_CSR = 0;
  )
}
#endif //SOFTUART


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static int8u softwareUartRxByte(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#ifdef SOFTUART
//this requires use of the SysTick counter and will destory interrupt latency!
static int8u softwareUartRxByte(void)
{
  int8u i;
  int8u bit;
  int8u byte = 0;
  int16u startBitTime, fullBitTime;
  
  if (CPU_CLKSEL) {
    startBitTime = START_BIT_TIME_SCLK;
    fullBitTime = FULL_BIT_TIME_SCLK;
  } else {
    startBitTime = START_BIT_TIME_PCLK;
    fullBitTime = FULL_BIT_TIME_PCLK;
  }

  ATOMIC(
    INTERRUPTS_ON();
    //we can only begin receiveing if the input is idle high
    while (SOFT_UART_RX_BIT != 1) {}
    //now wait for our start bit
    while (SOFT_UART_RX_BIT != 0) {}
    INTERRUPTS_OFF();
    
    //set reload value such that move to the center of an incoming bit
    ST_RVR = startBitTime;
    ST_CVR = 0; //writing the current value will cause it to reset to zero
    //enable core clock reference and the counter itself
    ST_CSR = (ST_CSR_CLKSOURCE | ST_CSR_ENABLE);
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 0.5bit time
    //set reload value such that move 1bit time
    ST_RVR = fullBitTime;
    ST_CVR = 0; //writing the current value will cause it to reset to zero
    
    //loop 8 times recieving all 8 bits and building up the byte
    for (i=0;i<8;i++) {
      while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
      bit = SOFT_UART_RX_BIT; //get the data bit
      bit = ((bit&0x1)<<7);
      byte = (byte>>1)|(bit);
    }

    //disable SysTick
    ST_CSR = 0;
  )
  return byte;
}
#endif //SOFTUART

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalUartFlowControl(int8u port)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO) && defined(EMBER_SERIAL1_XONXOFF)
void halInternalUartFlowControl(int8u port)
{
  if (port == 1) {
    int8u used = emSerialRxQueues[1]->used;
    int8u time;
    time = halCommonGetInt16uQuarterSecondTick();
    if (used) {
      xonTimer = time;
    }
    // Send an XON if the rx queue is below the XON threshold
    // and an XOFF had been sent that needs to be reversed
    if ( (xcmdCount == -1) && (used <= XON_LIMIT) ) {
      halInternalUart1ForceXon();
    } else if ( (used == 0) && 
                ((int8u)(time - xonTimer) > XON_REFRESH_TIME) && 
                (xcmdCount < XON_REFRESH_COUNT) ) {
      halInternalUart1ForceXon();
    }
  }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void halInternalUart1ForceXon(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#ifdef EMBER_SERIAL1_XONXOFF
static void halInternalUart1ForceXon(void)
{
    ATOMIC
    (
        if (xonXoffTxByte == 0x00) 
        {   // send an XON only if an XOFF is not pending
            xonXoffTxByte = ASCII_XON;
            halInternalStartUartTx(1);
        }
    )
    xonTimer = halCommonGetInt16uQuarterSecondTick();
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalRestartUart1Dma(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
void halInternalRestartUart1Dma ( void )
{
    //reload all defaults addresses - they will be adjusted below if needed
    SC1_RXBEGA = (int32u)&buffer_dma;
    SC1_RXENDA = (int32u)(&buffer_dma + fifoSize / 0x02 - 0x01);
    SC1_RXBEGB = (int32u)(&buffer_dma + fifoSize / 0x02);
    SC1_RXENDB = (int32u)(&buffer_dma + fifoSize - 0x01);
    
    SC1_DMACTRL = (SC_RXLODA | SC_RXLODB);                   // activate DMA
}
#endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalUart1RxCheckRts(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#if defined(EMBER_SERIAL1_RTSCTS) && (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
void halInternalUart1RxCheckRts(void)
{
  // Verify RTS is controlled by SW (not AUTO mode), and isn't already asserted.
  if ((SC1_UARTCFG & (SC_UARTFLOW | SC_UARTAUTO | SC_UARTRTS)) == SC_UARTFLOW) {
    // Assert RTS if the rx queue tail is in an active (or pending) DMA buffer,
    // because this means the other DMA buffer is empty.
    ATOMIC (
      if ( ( (emSerialRxQueues[1]->tail < EMBER_SERIAL1_RX_QUEUE_SIZE/2) &&
             (SC1_DMACTRL & SC_RXLODA) ) ||
           ( (emSerialRxQueues[1]->tail >= EMBER_SERIAL1_RX_QUEUE_SIZE/2) 
              && (SC1_DMACTRL & SC_RXLODB) ) ) {
          SC1_UARTCFG |= SC_UARTRTS;          // assert RTS
      }
    )
  }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halInternalUart1RxIsr(int16u causes)
//* 功能        : causes:启动该Isr的原因
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#if !defined(EM_SERIAL1_DISABLED)
void halInternalUart1RxIsr ( int16u causes )
{
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
    if (causes & INT_SCRXOVF)           
    {
        while (SC1_UARTSTAT & SC_UARTRXVAL)
            serial_info.put(&serial_info, (int8u)SC1_DATA & 0xff);
    }
    else if (SC1_UARTSTAT & SC_UARTRXVAL) 
        serial_info.put(&serial_info, (int8u)SC1_DATA & 0xff);
        
#elif (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
    //Load all of the hardware status, then immediately reset so we can process
    //what happened without worrying about new data changing these values.
    //We're in an error condition anyways, so it is ok to have the DMA disabled
    //for a while (less than 80us, while 4 bytes @ 115.2kbps is 350us)
    {
        int16u status   = SC1_DMASTAT;
        int16u intSrc   = causes;              // interrupts acknowledged at the start of the master SC1 ISR
        int16u cntA     = SC1_RXCNTA;
        int16u cntB     = SC1_RXCNTB;
        int8u errorType = EMBER_SUCCESS;

        for (u8 i = 0x00; i < cntA; i ++)
            uart_rx.put(&uart_rx, buffer_dma[i]);
        for (u8 i = 0x00; i < cntB; i ++)
            uart_rx.put(&uart_rx, buffer_dma[i + fifoSize / 0x02]);
        
        SC1_DMACTRL = SC_RXDMARST;            // to clear error, state fully captured, DMA reset, now we process error and restart
        if (intSrc & INT_SCRXOVF)             // Read the data register four times (into a temp variable) to clear the RXOVERRUN condition and 
        {                                     // empty the FIFO, giving us 4 bytes worth of time (from this point) to reenable the DMA.
            errorType = SC1_DATA;
            errorType = SC1_DATA;
            errorType = SC1_DATA;
            errorType = SC1_DATA;
            if (status & (SC_RXFRMA | SC_RXFRMB | SC_RXPARA | SC_RXPARB)) 
                halInternalRestartUart1Dma();
        }
    }
#endif //(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
}
#endif//!defined(EM_SERIAL1_DISABLED)

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : halInternalUart1TxIsr
//* 功能        : 发送中断服务子程序
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
// If called outside of an ISR, it should be from within an ATOMIC block.
void halInternalUart1TxIsr ( void )
{
#if 0  
    while (!uart_tx.ept(&uart_tx))
    {
        if (SC1_UARTSTAT & SC_UARTTXFREE)
            SC1_DATA = uart_tx.get(&uart_tx);
    }
#endif
}
#elif   (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
// If called outside of an ISR, it should be from within an ATOMIC block.
void halInternalUart1TxIsr ( void )
{
    while (uart_tx.cnt) 
    {
        if (!(SC1_DMACTRL & SC_TXLODA) && !(SC1_DMASTAT & SC_TXACTA))  // Channel A is available
        {
            if (uart_tx.out + uart_tx.cnt < EMBER_SERIAL1_QUEUE_SIZE)
            {
                SC1_TXBEGA  = (int32u)&uart_tx.buffer[uart_tx.out];
                SC1_TXENDA  = (int32u)&uart_tx.buffer[uart_tx.out + uart_tx.cnt];
                INT_SC1FLAG = INT_SCTXULDA;    // Ack if pending
                SC1_DMACTRL = SC_TXLODA;
                uart_tx.cnt = 0x00;
                uart_tx.out += uart_tx.cnt;
            }
            else
            {
                SC1_TXBEGA  = (int32u)&uart_tx.buffer[uart_tx.out];
                SC1_TXENDA  = (int32u)&uart_tx.buffer[EMBER_SERIAL1_QUEUE_SIZE - 0x01];
                INT_SC1FLAG = INT_SCTXULDA;    // Ack if pending
                SC1_DMACTRL = SC_TXLODA;
                uart_tx.cnt = uart_tx.out + uart_tx.cnt - EMBER_SERIAL1_QUEUE_SIZE;
                uart_tx.out = 0x00;
            }
        } 
        else if (!(SC1_DMACTRL & SC_TXLODB) && !(SC1_DMASTAT & SC_TXACTB))  // Channel B is available
        {
            if (uart_tx.out + uart_tx.cnt < EMBER_SERIAL1_QUEUE_SIZE)
            {
                SC1_TXBEGB  = (int32u)&uart_tx.buffer[uart_tx.out];
                SC1_TXENDB  = (int32u)&uart_tx.buffer[uart_tx.out + uart_tx.cnt];
                INT_SC1FLAG = INT_SCTXULDB; // Ack if pending
                SC1_DMACTRL = SC_TXLODB;
                uart_tx.cnt = 0x00;
                uart_tx.out += uart_tx.cnt;
            }
            else
            {
                SC1_TXBEGB  = (int32u)&uart_tx.buffer[uart_tx.out];
                SC1_TXENDB  = (int32u)&uart_tx.buffer[EMBER_SERIAL1_QUEUE_SIZE - 0x01];
                INT_SC1FLAG = INT_SCTXULDB; // Ack if pending
                SC1_DMACTRL = SC_TXLODB;
                uart_tx.cnt = uart_tx.out + uart_tx.cnt - EMBER_SERIAL1_QUEUE_SIZE;
                uart_tx.out = 0x00;
            }
         } 
        else 
            break;
    } // while ( uart_tx.cnt > 0x00 )
}
#endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
/////////////////////////////////////////////////////////////////////////////////////////////

