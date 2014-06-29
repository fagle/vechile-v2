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
#include "lwip/sockets.h"
#include "enc28j60.h"

/////////////////////////////////////////////////////////////////////////////////////////////
extern frame_info_t   consfrm1, w108frm1, tcpfrm1;     // for console protocol
extern client_info_t  client;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vTCPSendTask ( void *pvParameters )
//* 功能        : tcp client task
//* 输入参数    : void *pvParameters
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vTCPSendTask ( void *pvParameters )
{
    vu32     ticker = 0x00L;

    while (1)
    {       
        ticker ++;  

#ifdef LWIP_ENABLE
        if (client.send.len && client.conn)
        { 
            client.sem = 0x01;
            if (lwip_send(client.sock, client.send.buf, client.send.len, 0x00) < 0x00)
                client.conn = 0x00;
            else if (client.count)
                client.count --;
            client.send.len = 0x00;
            client.sem      = 0x00;
        }
#endif

        vTaskDelay(1 / portTICK_RATE_MS);
    }
}

/*******************************************************************************
* Function Name  : portBASE_TYPE xAreTCPSendTasksStillRunning  ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
portBASE_TYPE xAreTCPSendTasksStillRunning ( void )
{
    portBASE_TYPE xReturn = 0x00;

    return xReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////