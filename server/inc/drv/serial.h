
#ifndef __SERIAL_COMMS_H__
#define __SERIAL_COMMS_H__

#include "FreeRTOS.h"
#include "stm32f10x.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////////
// for freeRTOS
typedef void * xComPortHandle;

typedef enum
{ 
	serCOM1, 
	serCOM2, 
	serCOM3, 
	serCOM4, 
	serCOM5, 
	serCOM6, 
	serCOM7, 
	serCOM8 
} eCOMPort;

typedef enum 
{ 
	serNO_PARITY, 
	serODD_PARITY, 
	serEVEN_PARITY, 
	serMARK_PARITY, 
	serSPACE_PARITY 
} eParity;

typedef enum 
{ 
	serSTOP_1, 
	serSTOP_2 
} eStopBits;

typedef enum 
{ 
	serBITS_5, 
	serBITS_6, 
	serBITS_7, 
	serBITS_8 
} eDataBits;

typedef enum 
{ 
	ser50,		
	ser75,		
	ser110,		
	ser134,		
	ser150,    
	ser200,
	ser300,		
	ser600,		
	ser1200,	
	ser1800,	
	ser2400,   
	ser4800,
	ser9600,		
	ser19200,	
	ser38400,	
	ser57600,	
	ser115200
} eBaud;

xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength );
xComPortHandle xSerialPortInit( eCOMPort ePort, eBaud eWantedBaud, eParity eWantedParity, eDataBits eWantedDataBits, eStopBits eWantedStopBits, unsigned portBASE_TYPE uxBufferLength );
void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength );
signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, portTickType xBlockTime );
signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, portTickType xBlockTime );
portBASE_TYPE xSerialWaitForSemaphore( xComPortHandle xPort );
void vSerialClose( xComPortHandle xPort );

///////////////////////////////////////////////////////////////////////////////////
//
#define COM_BUFSIZE            (0x100)
#define UARTTIMEOUT            (0x10000u)
#define PRINTFBUF              (0x100)

#define MAXUSART               (0x03)
#define GPS_COM                (USART2)
#define GPRS_COM               (USART2)
#define OTDR_COM               (USART3)
#define CPLD_COM               (UART4)//
#define CONSOLE_COM            (USART1)

#define W108_COM               (USART2)
#define WIFI_COM               (USART3)

#define CONSOLE_COM_INX        (0x00)
#define GPS_COM_INX            (0x01)
#define GPRS_COM_INX           (0x01)
#define OTDR_COM_INX           (0x02)

#define W108_COM_INX           (0x01)
#define WIFI_COM_INX           (0x02)

//////////////////////////////////////////////////////////////////////////////////
/***************  Serial peripheral define values ******************** */
#define USART_WORDLENGTH 	  USART_WordLength_8b
#define USART_STOPBITS 		  USART_StopBits_1
#define USART_PARITY 	          USART_Parity_No
#define USART_HARDWAREFLOWCONTROL USART_HardwareFlowControl_None
#define USART_MODE 	          USART_Mode_Rx | USART_Mode_Tx
#define USART_CLOCK 		  USART_Clock_Disable
#define USART_CPOL_VALUE 	  USART_CPOL_Low
#define USART_CPHA_VALUE	  USART_CPHA_2Edge
#define USART_LASTBIT	 	  USART_LastBit_Disable

//////////////////////////////////////////////////////////////////////////////////
/***************  USART1 define values ******************** */
#define USART1_NAME                USART1 
#define USART1_BAUDRATE 	   115200

#define USART1_IRQ_CHANNEL         USART1_IRQn    // Description: Usart irq channel 

#define USART1_TX_GPIO_NAME        GPIOA                // Description: Usart TX, RX gpios
#define USART1_TX_GPIO_PIN         GPIO_Pin_9           // GPIO_Pin_9
#define USART1_TX_GPIO_APB2        RCC_APB2Periph_GPIOA // (RCC_APB2Periph_USART1 )//| RCC_APB2Periph_GPIOB)

#define USART1_RX_GPIO_NAME        GPIOA
#define USART1_RX_GPIO_PIN         GPIO_Pin_10          // GPIO_Pin_10
#define USART1_RX_GPIO_APB2        RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA // (RCC_APB2Periph_USART1 )//| RCC_APB2Periph_GPIOB)

//////////////////////////////////////////////////////////////////////////////////
/***************  USART2 define values ******************** */
#define USART2_NAME                USART2 // USART2
#define USART2_BAUDRATE 	   115200 // 9600

#define USART2_IRQ_CHANNEL         USART2_IRQn    // Description: Usart irq channel 

#define USART2_TX_GPIO_NAME        GPIOA                // Description: Usart TX, RX gpios
#define USART2_TX_GPIO_PIN         GPIO_Pin_2           // GPIO_Pin_2
#define USART2_TX_GPIO_APB2        RCC_APB2Periph_GPIOA // (RCC_APB2Periph_USART2 )//| RCC_APB2Periph_GPIOB)

#define USART2_RX_GPIO_NAME        GPIOA
#define USART2_RX_GPIO_PIN         GPIO_Pin_3           // GPIO_Pin_3
#define USART2_RX_GPIO_APB2        RCC_APB2Periph_GPIOA // (RCC_APB2Periph_USART2 )//| RCC_APB2Periph_GPIOB)

//////////////////////////////////////////////////////////////////////////////////
/***************  USART3 define values ******************** */
#define USART3_NAME                USART3 // USART3
#define USART3_BAUDRATE 	   115200 // 9600

#define USART3_IRQ_CHANNEL         USART3_IRQn    // Description: Usart irq channel 

#define USART3_TX_GPIO_NAME        GPIOB                // Description: Usart TX, RX gpios
#define USART3_TX_GPIO_PIN         GPIO_Pin_10          // GPIO_Pin_10
#define USART3_TX_GPIO_APB2        RCC_APB2Periph_GPIOB // (RCC_APB2Periph_USART2 )//| RCC_APB2Periph_GPIOB)

#define USART3_RX_GPIO_NAME        GPIOB
#define USART3_RX_GPIO_PIN         GPIO_Pin_11          // GPIO_Pin_11
#define USART3_RX_GPIO_APB2        RCC_APB2Periph_GPIOB // (RCC_APB2Periph_USART3 )//| RCC_APB2Periph_GPIOB)
/***************  USART4 define values ******************** */
#define USART4_NAME                UART4   // USART4
#define USART4_BAUDRATE 	   115200  // 9600

#define USART4_IRQ_CHANNEL         UART4_IRQn    // Description: Usart irq channel 

#define USART4_TX_GPIO_NAME        GPIOC                // Description: Usart TX, RX gpios
#define USART4_TX_GPIO_PIN         GPIO_Pin_10          // GPIO_Pin_10
#define USART4_TX_GPIO_APB2        RCC_APB2Periph_GPIOC // (RCC_APB2Periph_USART4 )//| RCC_APB2Periph_GPIOC)

#define USART4_RX_GPIO_NAME        GPIOC
#define USART4_RX_GPIO_PIN         GPIO_Pin_11          // GPIO_Pin_11
#define USART4_RX_GPIO_APB2        RCC_APB2Periph_GPIOC // (RCC_APB2Periph_USART4 )//| RCC_APB2Periph_GPIOC)

//////////////////////////////////////////////////////////////////////////////////
/***************  USART4 define values ******************** */
#define USART5_NAME                UART5 // USART4
#define USART5_BAUDRATE 	   115200 // 9600

#define USART5_IRQ_CHANNEL         UART5_IRQn    // Description: Usart irq channel 

#define USART5_TX_GPIO_NAME        GPIOC                // Description: Usart TX, RX gpios
#define USART5_TX_GPIO_PIN         GPIO_Pin_12          // GPIO_Pin_10
#define USART5_TX_GPIO_APB2        RCC_APB2Periph_GPIOC // (RCC_APB2Periph_USART4 )//| RCC_APB2Periph_GPIOC)

#define USART5_RX_GPIO_NAME        GPIOD
#define USART5_RX_GPIO_PIN         GPIO_Pin_2          // GPIO_Pin_11
#define USART5_RX_GPIO_APB2        RCC_APB2Periph_GPIOD // (RCC_APB2Periph_USART4 )//| RCC_APB2Periph_GPIOC)

/** Description: USB Virtual com GPIO USB pullup */
#define BOARD_HAS_SERIAL_1        0x01
#define USART_USB_GPIO_NAME       GPIOB
#define USART_USB_GPIO_PIN        GPIO_Pin_0
#define USART_USB_GPIO_APB2       RCC_APB2Periph_GPIOB
#define BOARD_HAS_USB_VIRTUAL_COM_PULL_UP 0x01 // reset button allows usb re-enumeration 
#define BOARD_HAS_USB_VIRTUAL_COM 0x01         // REva-STM32 supports usb virtual com 

////////////////////////////////////////////////////////////////////////////////
enum { PARITY_NONE = 0x00, PARITY_ODD, PARITY_EVEN };

////////////////////////////////////////////////////////////////////////////////
//
typedef struct 
{ 
   USART_TypeDef * SERIAL_name;

   u32 USART_BaudRate;
   u16 USART_WordLength;
   u16 USART_StopBits;
   u16 USART_Parity;
   u16 USART_HardwareFlowControl;
   u16 USART_Mode;
   u16 USART_Clock;
   u16 USART_Cpol;
   u16 USART_Cpha;
   u16 USART_LastBit;
   
   u8  serial_irq_channel;

   GPIO_TypeDef * serial_TX_gpio_name; 
   u16 serial_TX_gpio_pin;
   u32 serial_TX_gpio_APB2Periph; 

   GPIO_TypeDef * serial_RX_gpio_name; 
   u16 serial_RX_gpio_pin;
   u32 serial_RX_gpio_APB2Periph; 
} SERIAL_InitTypeDef;

////////////////////////////////////////////////////////////////////////////////
//
typedef struct 
{
    u16   in, out;
    u8    buffer[COM_BUFSIZE];
} serial_info_t, *pserial_info_t;

/*******************************************************************************
* Function Name  : void SERIAL_Init ( const SERIAL_InitTypeDef * SERIAL_InitStructure )
* Description    : Init the serial gpios and peripheral 
* Input          : serials init structure with the gpio init parameters
* Output         : None
* Return         : None
*******************************************************************************/
void SERIAL_Init ( const SERIAL_InitTypeDef * ptr );

/*******************************************************************************
* Function Name  : pserial_info_t get_serialinfo ( USART_TypeDef * uart )
* Description    : get the pointer of structure serial_info.
* Input          : None
* Output         : pointer of serial_info
* Return         : None
*******************************************************************************/
pserial_info_t get_serialinfo ( USART_TypeDef * uart );

/*******************************************************************************
* Function Name  : bool sea_isempty ( pserial_info_t ptr )
* Description    : is empty of ring.
* Input          : None
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_isempty ( pserial_info_t ptr );  

/*******************************************************************************
* Function Name  : void sea_clearbuffer ( pserial_info_t ptr ) 
* Description    : clear ring buffer.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_clearbuffer ( pserial_info_t ptr );  

/*******************************************************************************
* Function Name  : uu8   sea_readbyte ( pserial_info_t ptr )
* Description    : get ch from serial_info ring.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
u8 sea_readbyte ( pserial_info_t ptr );  

/*******************************************************************************
* Function Name  : void sea_putbyte ( pserial_info_t ptr, u8  ch )
* Description    : Inserts ch in ring.
* Input          : u8 ch
* Output         : None
* Return         : None
*******************************************************************************/
void sea_putbyte ( pserial_info_t ptr, u8  ch );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printf ( const char * format, ... )
//* 功能        : 格式化输出数据
//* 输入参数    : const char * format, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printf ( const char * format, ... );

/*******************************************************************************
* Function Name  : u8 UartSendByte ( USART_TypeDef * uart, u8 ch )
* Description    : send a byte to uart 
* Input          : USART_TypeDef * uart, u8 ch 
* Output         : None
* Return         : None
*******************************************************************************/
u8 UartSendByte ( USART_TypeDef * uart, u8 ch );

/*******************************************************************************
* Function Name  : void UartSendString ( USART_TypeDef * uart, const u8 * str )
* Description    : print a string to the uart
* Input          : strf为发送数据的地址
* Output         : None
* Return         : None
*******************************************************************************/
void UartSendString ( USART_TypeDef * uart, const u8 * str );

/*******************************************************************************
* Function Name  : u8  UartSendSizeString ( USART_TypeDef * uart, u8 * str,  u16 size )
* Description    : print a string to the uart
* Input          : str为发送数据的地址
* Output         : None
* Return         : None
*******************************************************************************/
u8  UartSendSizeString ( USART_TypeDef * uart, const u8 * str, u16 size );

/*******************************************************************************
* Function Name  : u8 UartReadByte ( USART_TypeDef * uart )
* Description    : read a char from uart.
* Input          : USART_TypeDef * uart
* Output         : a char
* Return         : None
*******************************************************************************/
u8 UartReadByte ( USART_TypeDef * uart );

/*******************************************************************************
* Function Name  : u8 UartGetChar ( USART_TypeDef * uart )
* Description    : get a char from uart, no block.
* Input          : USART_TypeDef * uart
* Output         : a char or not
* Return         : None
*******************************************************************************/
u8 UartGetChar ( USART_TypeDef * uart );

/*******************************************************************************
* Function Name  : void UART_Configuration ( void )
* Description    : Initialize the console and gprs communication channel.
*                  console -> uart1
*                  gprs   ->  uart2
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART_Configuration ( void );

/*******************************************************************************
* Function Name  : void reset_serialinfo ( pserial_info_t ptr )
* Description    : reset serial_info body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void reset_serialinfo ( pserial_info_t ptr );

/////////////////////////////////////////////////////////////////////////////////////
//

extern serial_info_t  serial_info[MAXUSART]; 

////////////////////////////////////////////////////////////////////////////////
#endif

