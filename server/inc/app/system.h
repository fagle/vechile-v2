#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define WDLOADCNT    (500)    // (349)

///////////////////////////////////////////////////////////////////////////////////
//
enum
{
   /** Cause for the reset is not known.*/
   RESET_UNKNOWN,
   /** A low level was present on the reset pin.*/
   RESET_EXTERNAL,
   /** The supply voltage was below the power-on threshold.*/   
   RESET_POWERON,
   /** The Watchdog is enabled, and the Watchdog Timer period expired.*/
   RESET_WATCHDOG,
   /** The Brown-out Detector is enabled, and the supply voltage was 
    * below the brown-out threshold.*/
   RESET_BROWNOUT,
   /** A logic one was present in the JTAG Reset Register. */ 
   RESET_JTAG,
   /** A self-check within the code failed unexpectedly. */
   RESET_ASSERT,
   /** The return address stack (RSTACK) went out of bounds. */
   RESET_RSTACK,
   /** The data stack (CSTACK) went out of bounds. */
   RESET_CSTACK,
   /** The bootloader deliberately caused a reset. */
   RESET_BOOTLOADER,
   /** The PC wrapped (rolled over) */
   RESET_PC_ROLLOVER,
   /** The software deliberately caused a reset */
   RESET_SOFTWARE,
   /** Protection fault or privilege violation */
   RESET_PROTFAULT,
   /** Flash write failed verification */
   RESET_FLASH_VERIFY_FAIL,
   /** Flash write was inihibited, address was already written */
   RESET_FLASH_WRITE_INHIBIT,
   /** Application bootloader reports bad upgrade image in EEPROM */
   RESET_BOOTLOADER_IMG_BAD
};

enum
{
   SLEEPMODE_IDLE,
   SLEEPMODE_RESERVED,
   SLEEPMODE_POWERDOWN,
   SLEEPMODE_POWERSAVE
};

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY                       ( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainWIFI_TASK_PRIORITY                          ( tskIDLE_PRIORITY + 1 )
#define mainGPRS_TASK_PRIORITY                          ( tskIDLE_PRIORITY + 1 )
#define mainW108_TASK_PRIORITY                          ( tskIDLE_PRIORITY + 1 )
#define mainTCP_TASK_PRIORITY                           ( tskIDLE_PRIORITY + 1 )
#define mainARC_TASK_PRIORITY                           ( tskIDLE_PRIORITY + 1 )
#define mainMISC_TASK_PRIORITY                          ( tskIDLE_PRIORITY + 1 )
#define mainCMD_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY                       ( tskIDLE_PRIORITY )

/* Constants related to the LCD. */
#define mainMAX_LINE						( 240 )
#define mainROW_INCREMENT					( 24 )
#define mainMAX_COLUMN						( 20 )
#define mainCOLUMN_START					( 319 )
#define mainCOLUMN_INCREMENT 				( 16 )

/* The maximum number of message that can be waiting for display at any one
time. */
#define mainLCD_QUEUE_SIZE					( 3 )

/* The check task uses the sprintf function so requires a little more stack. */
#define mainCHECK_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN						25

/* The time between cycles of the 'check' task. */
#define mainCHECK_DELAY						( ( portTickType ) 5000 / portTICK_RATE_MS )

/* The number of nano seconds between each processor clock. */
#define mainNS_PER_CLOCK ( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Baud rate used by the comtest tasks. */
#define mainCOM_TEST_BAUD_RATE		( 115200 )

/* The LED used by the comtest tasks. See the comtest.c file for more
information. */
#define mainCOM_TEST_LED			( 3 )


/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures NVIC and Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration ( void );

/*******************************************************************************
* Function Name  : void INT_Configuration ( void )
* Description    : Configures external or inter interrupt.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void INT_Configuration ( void );

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration ( void );

/*******************************************************************************
* Function Name  : void sea_reboot ( const char * msg )
* Description    : display system error message and reboot
* Input          : const char * msg
* Output         : None
* Return         : None
*******************************************************************************/
void sea_reboot ( const char * msg );

void halPowerDown ( void );  
void halSleep     ( u8 sleepmode );
void halPowerUp   ( void );

/*******************************************************************************
* Function Name  : void IWDG_Configuration ( void )
* Description    : Configures watch dog.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Configuration ( void );

/*******************************************************************************
* Function Name  : void IwdgReload ( void )
* Description    : feed watch dog 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IwdgReload ( void );

/*******************************************************************************
* Function Name  : void hal260IsAwakeIsr ( bool isAwake )
* Description    : restores from STOP mode
* Input          : None
* Return         : None
*******************************************************************************/
void hal260IsAwakeIsr ( bool isAwake );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void delay ( u16 times )
//* 功能        : delay times
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void delay ( u16 times );

/*******************************************************************************
* Function Name  : void ShowCopyrights ( void )
* Description    : show copyrights
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ShowCopyrights ( void );

#endif // __MICRO_H__ 

/////////////////////////////////////////////////////////////////////////////////////////////////////
