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
extern frame_info_t consfrm1, w108frm1, tcpfrm1;     // for console protocol

/////////////////////////////////////////////////////////////////////////////////////////////
client_info_t  client;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_resetetherif ( void )
//* 功能        : reset ethernet interface
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_resetetherif ( void )
{
//    SPI2_Configuration();  
    enc28j60Init(sys_info.local.mac);
    enc28j60clkout(0x01); 			
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static int sea_createsocket ( void )
//* 功能        : tcp client task
//* 输入参数    : 无
//* 输出参数    : socket
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static int sea_createsocket ( void )
{
    struct sockaddr_in server;
    int sock;
    int optval = 0x01;
    u8  iptxt[20];
    u8  retry = 0x00;
    
    sea_resetetherif();                                // reset enc28j60 chip.
    server.sin_family      = AF_INET;
#if 0
#ifdef HOME_IPADDR    
    server.sin_addr.s_addr = inet_addr("192.168.0.100");//test 192.168.1.19DEFDOMAIN
#else
    server.sin_addr.s_addr = inet_addr("192.168.1.100");//test 192.168.1.19DEFDOMAIN
#endif
    server.sin_port        = htons(8080);
#else
    sprintf((char *)iptxt, "%d.%d.%d.%d", sys_info.ip[0x00], sys_info.ip[0x01], sys_info.ip[0x02], sys_info.ip[0x03]);
    server.sin_addr.s_addr = inet_addr((char *)iptxt);   
    server.sin_port        = htons(sys_info.port);
#endif
    
    if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0x00)) == -1)   // IPPROTO_TCP)) == -1)
        return -1;     
    
    lwip_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    while (lwip_connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
    {
        lwip_close(sock);                
        if (++ retry >= 0x03)
            return -1;
        vTaskDelay(1 / portTICK_RATE_MS);
        if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0x00)) == -1)   //  IPPROTO_TCP)) == -1)
            return -1;
        lwip_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }   
    return sock;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vTCPClientTask ( void *pvParameters )
//* 功能        : tcp client task
//* 输入参数    : void *pvParameters
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vTCPClientTask ( void *pvParameters )
{
    vu32 ticker = 0x00;
    
    client.conn  = 0x00;
    client.sock  = -1;
    client.retry = 0x00;
    while (1)
    {
        if (client.retry < 0x100)
        {
            if (client.conn == 0x00)
            {
                if (client.sock >= 0x00)
                {
                    lwip_close(client.sock);
                    client.sock = -1;
                }
              
                if ((client.sock = sea_createsocket()) < 0x00)
                {
                    client.retry ++;
                    continue;
                }
                client.conn  = 0x01;
                client.retry = 0x00;
                sea_printf("\nconnected！");
            }
            else
            {
                client.recv.len = lwip_recv(client.sock, client.recv.buf, BUF_SIZE, 0x00);
                if (client.recv.len < 0x00)
                    client.conn = 0x00;
	        else if (client.recv.len > 0x00)
                {
                    u16 i = 0x00;
                    client.recv.buf[client.recv.len ++] = '\n';
                    client.recv.buf[client.recv.len ++] = '\r';
                    taskENTER_CRITICAL();
                    while (client.recv.len --)
                        sea_putbyte(get_serialinfo(tcpfrm1.uart), client.recv.buf[i ++]);
                    taskEXIT_CRITICAL();
                }
            }
        }
        else
        {
            sea_printf("\ncan't find server!");
            vTaskDelay(client.retry / portTICK_RATE_MS);
            client.retry >>= 0x01;
        }
        
        vTaskDelay(1 / portTICK_RATE_MS);
        ticker ++;
    }
}

/*******************************************************************************
* Function Name  : portBASE_TYPE xAreTCPClientTasksStillRunning  ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
portBASE_TYPE xAreTCPClientTasksStillRunning ( void )
{
    portBASE_TYPE xReturn = 0x00;

    return xReturn;
}
