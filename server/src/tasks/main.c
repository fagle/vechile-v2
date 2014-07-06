#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_it.h"
#include "flash.h"
#include "system.h"
#include "network.h"
#include "usrTasks.h"
#include "adc.h"
#include "gpio.h"
#include "netconf.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
serial_info_t serial_info[MAXUSART];         
frame_info_t  consfrm1, w108frm1;
#ifdef LWIP_ENABLE     
frame_info_t  tcpfrm1;
#endif
rep_info_t    rep_info;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void SYS_Configuration ( void )
//* 功能        : system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void SYS_Configuration ( void )
{
    sea_memcpy(&sys_info, sea_flashread(0x00, sizeof(sys_info_t)), sizeof(sys_info_t));
    if (sys_info.mark != SYSMARK || sys_info.size != sizeof(sys_info_t) || 
        sys_info.ctrl.maxdev != MAXLAMPS)
        sea_writedefaultsysinfo();

    sea_memset(&dyn_info, 0x00, sizeof(dyn_info_t)); 
    dyn_info.semserial = xSemaphoreCreateMutex();
    dyn_info.serial    = 0x01;
    
    sea_memset(dyn_info.addr, 0x00, sys_info.ctrl.maxdev * sizeof(addr_t));
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
        dyn_info.addr[i].dev.num = i + 0x01;
    
    sys_info.card.cnt = 0x00;
    while (*((u16 *)sea_flashreadcards(sys_info.card.cnt ++)) != EMPTYFLASH && sys_info.card.cnt < 0x80) ;
    sys_info.card.cnt --;

    
    sea_initframe(&consfrm1, CONSOLE_COM);
    sea_initframe(&w108frm1, W108_COM);
#ifdef LWIP_ENABLE     
    sea_initcpfrm(&tcpfrm1, TCP_COM);
#endif
    
    sea_initmsg();
    sea_printsysinfo();
}

/*******************************************************************************
* Function Name  : pserial_info_t get_serialinfo ( USART_TypeDef * uart )
* Description    : get the pointer of structure serial_info.
* Input          : None
* Output         : pointer of serial_info
* Return         : None
*******************************************************************************/
pserial_info_t get_serialinfo ( USART_TypeDef * uart )
{
    if (uart == TCP_COM)
        return &serial_info[TCP_COM_INX];
    else if(uart == W108_COM)
        return &serial_info[W108_COM_INX];
    else if(uart == CONSOLE_COM)
        return &serial_info[CONSOLE_COM_INX];
    
    return NULL;
}

/*******************************************************************************
* Function Name  : void UART_Configuration ( void )
* Description    : Initialize the console and gprs communication channel.
*                  console -> uart1
*                  gprs   ->  uart2
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART_Configuration ( void )
{
    extern const SERIAL_InitTypeDef uart1_params, uart2_params, uart3_params, uart4_params, uart5_params;
    
    for (u8 i = 0x00; i < MAXUSART; i ++)
        sea_memset(&serial_info[i], 0x00, sizeof(serial_info_t));
    
    SERIAL_Init(&uart1_params);
    SERIAL_Init(&uart2_params);
    
    reset_serialinfo(get_serialinfo(TCP_COM));        // tcp/ip used
    reset_serialinfo(get_serialinfo(W108_COM));       // why ? must do it, 2014/01/17
    reset_serialinfo(get_serialinfo(CONSOLE_COM));    // why ? must to it, 2014/01/17
}

/*******************************************************************************
* Function Name  : void GPIO_Configuration ( void )
* Description    : Init/Configure the GPIOx for landscape
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration ( void )
{
#ifdef LANDSCAPE_MODE     
    GPIO_InitTypeDef GPIO_InitStructure; 

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;       
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif   // LANDSCAPE_MODE 
    
#ifdef VEHICLE_MODE                
    GPIO_InitTypeDef GPIO_InitStructure; 

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;         // reset w108 of zigbee module
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
    GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif  //  VEHICLE_MODE
}  

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void prvSetupHardware ( void )
//* 功能        : system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void prvSetupHardware ( void )
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);

    while (RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET) ;
    
    *((unsigned portLONG *)0x40022000) = 0x02     ;         // 2 wait states required on the flash. 

    RCC_HCLKConfig(RCC_SYSCLK_Div1);	                    // HCLK = SYSCLK 
    RCC_PCLK2Config(RCC_HCLK_Div1); 	                    // PCLK2 = HCLK 
    RCC_PCLK1Config(RCC_HCLK_Div2);	                    // PCLK1 = HCLK/2 
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);    // PLLCLK = 8MHz * 9 = 72 MHz. 

    RCC_PLLCmd(ENABLE);	                                    // Enable PLL. 

    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) ;   // Wait till PLL is ready. 
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	            // Select PLL as system clock source. 
    while (RCC_GetSYSCLKSource() != 0x08) ;                 // Wait till PLL is used as system clock source. 

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);                       // ADCCLK = PCLK2/4, 9MHz
        
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);      // Enable DMA1 clock 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1 , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
			   RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 | RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM5 | RCC_APB1Periph_CAN1, ENABLE);

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);          // Set the Vector Table base address at 0x08000000 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);         // 抢占优先级4位，从优先级0位
	
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	    // Configure HCLK clock as SysTick clock source. 
}

/*******************************************************************************
* Function Name  : TICK_Configuration ( void )
* Description    : Configures System Ticker
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TICK_Configuration ( void )
{
#ifdef PCF8365T_ENABLE 
    sea_memset(&systime, 0x00, sizeof(systime_t));
#else
    const systime_t deftm = { 2014, 2, 2, 10, 19, 18, 0 };
    sea_memcpy(&systime, (void *)&deftm, sizeof(systime_t));
#endif
}

/************************************************
* 函数名      : void I2C_Configuration ( void )
* 功能        : 配置iic引脚
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void I2C_Configuration ( void )
{
#ifdef IIC_ENABLE    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = RTC_SCL_PIN | RTC_SDA_PIN; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
}

/*******************************************************************************
* Function Name  : void IWDG_Configuration ( void )
* Description    : Configures watch dog.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Configuration ( void )
{
#ifdef WATCH_DOG    
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  // Enable write access to IWDG_PR and IWDG_RLR registers 

    IWDG_SetPrescaler(IWDG_Prescaler_256);         // IWDG counter clock: 32KHz(LSI) / 256 = 0.125KHz(8ms) 
    IWDG_SetReload(WDLOADCNT);                     // Set counter reload value to 500 x 8ms = 4000ms
    IWDG_ReloadCounter();                          // Reload IWDG counter 
  
    IWDG_Enable();                                 // 使能独立看门狗
#endif    
}

/*******************************************************************************
* Function Name  : void LED_Configuration ( void )
* Description    : Init/Configure the GPIO x led
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Configuration ( void )
{
#ifdef WIFI_MODE
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(GPIOC, &GPIO_InitStructure);             // 输出高电平，光耦不亮，断电
        
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;       // PB13:reset w108 pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          
    GPIO_Init(GPIOB, &GPIO_InitStructure);
        
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;       // PB0:current detection ADC pin
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;          
    GPIO_Init(GPIOB, &GPIO_InitStructure);
        
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);        //
    GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);       // 输出高电平, power off
#endif    
        
#ifdef HDU_ENABLE
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(GPIOA, &GPIO_InitStructure);            
        
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;      // GPIO_Mode_IPU;   
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
}  

/*******************************************************************************
* Function Name  : void KEY_Configuration ( void )
* Description    : init keyborad 
	                REMINDER: Have to make clear the pull-up/down 
	                issue under comment below
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void KEY_Configuration ( void )
{
    extern key_info_t key_info;
#ifdef KEY_ENABLE    
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;      // GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;      // GPIO_Mode_IPU;   
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif   // KEY_ENABLE
    sea_memset(&key_info, 0x00, sizeof(key_info_t));
} 

/*******************************************************************************
* Function Name  : ADC_Configuration
* Description    : Initializes the ADC.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_Configuration ( void )
{
#ifdef ADC_ENABLE
    extern __IO uint16_t ADCConvertedValue;

    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY - 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_7Cycles5);
    
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#endif  // ADC_ENABLE
}


/*******************************************************************************
* Function Name  : void SPI2_Configuration ( void )
* Description    : spi2 configuration.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_Configuration ( void )
{
#ifdef LWIP_ENABLE     
   SPI_InitTypeDef  SPI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
#ifdef MACINT_ENABLE
   NVIC_InitTypeDef NVIC_InitStructure;
   EXTI_InitTypeDef EXTI_InitStructure;

   NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;           // Enable the EXTI1 Interrupt, DI1 input
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x00;
   NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
#endif    
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     // SPI2 Periph clock enable 
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);      //main.c里面做过了 若移植请加上
  
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1; 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
    
#ifdef MACINT_ENABLE
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    
   EXTI_InitStructure.EXTI_Line    = EXTI_Line1;
   EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);
#endif    
   
   //ENC28J60的相关管脚
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12; 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
   
   GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  // Configure SPI2 pins: NSS, SCK, MISO and MOSI 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
  
   SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;      // SPI2 configuration 
   SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;
   SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;
   SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
   SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial     = 0x07;

   SPI_Init(SPI2, &SPI_InitStructure);
   SPI_Cmd(SPI2, ENABLE);                             // Enable SPI2 
#endif
}

#ifndef LWIP_ENABLE     
/*******************************************************************************
* Function Name  : static void MEM_Configuration ( void )
* Description    : Initializes Tasks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void MEM_Configuration ( void )
{
    extern void mem_init  ( void );
    extern void memp_init ( void );
    mem_init();
    memp_init();
}
#endif

/*******************************************************************************
* Function Name  : static void TASK_Configuration ( void )
* Description    : Initializes Tasks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void TASK_Configuration ( void )
{
    xTaskCreate(vCommandTask, (s8 *)"Command Task", 512, NULL, mainCMD_TASK_PRIORITY - 0x01, (xTaskHandle *)NULL);
    xTaskCreate(vW108MsgTask, (s8 *)"W108 Task", 512, NULL, mainW108_TASK_PRIORITY - 0x01, (xTaskHandle *)NULL);
    xTaskCreate(vMiscellaneaTask, (s8 *)"Misc Task", 512, NULL, mainMISC_TASK_PRIORITY - 0x01, (xTaskHandle *)NULL);
#ifdef LWIP_ENABLE     
    xTaskCreate(vTCPRecvTask, (s8 *)"TCPClient Task", 512, NULL, mainTCP_TASK_PRIORITY - 0x01, (xTaskHandle *)NULL);
    xTaskCreate(vTCPSendTask, (s8 *)"TCPSend Task", 512, NULL, mainTCP_TASK_PRIORITY - 0x01, (xTaskHandle *)NULL);
#endif
}

int main ( void )
{
    prvSetupHardware();
    GPIO_Configuration();
    UART_Configuration(); 
    I2C_Configuration();
    SPI2_Configuration();               // configure ethernet (spi2) 
    TICK_Configuration();
    IWDG_Configuration();               // IWDG配置。在systick.c中喂狗：IWDG_ReloadCounter();
    LED_Configuration();
    KEY_Configuration();
    SYS_Configuration();
    ADC_Configuration();
    CAN_Configuration();
    
#ifdef LWIP_ENABLE     
    LWIP_Configuration();               // Initilaize the LwIP stack 
#else
    MEM_Configuration();
#endif
    
    TASK_Configuration();
    vTaskStartScheduler();             // Start the scheduler.
	
    return 0x00;	               // Will only get here if there was not enough heap space to create the idle task.
}

//////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef  DEBUG
void assert_failed ( unsigned portCHAR* pcFile, unsigned portLONG ulLine )
{
    for( ;; )
    {
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
