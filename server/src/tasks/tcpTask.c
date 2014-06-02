#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_it.h"
#include "flash.h"
#include "gprs.h"
#include "system.h"
#include "network.h"
#include "usrTasks.h"
#include "adc.h"
#include "gpio.h"
#include "lwip/sockets.h"
#include "enc28j60.h"

/////////////////////////////////////////////////////////////////////////////////////////////
client_info_t  client;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_resetetherif ( void )
//* ����        : reset ethernet interface
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_resetetherif ( void )
{
//    SPI2_Configuration();  
    enc28j60Init(sys_info.local.mac);
    enc28j60clkout(0x01); 			
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static int sea_createsocket ( void )
//* ����        : tcp client task
//* �������    : ��
//* �������    : socket
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static int sea_createsocket ( void )
{
    struct sockaddr_in server;
    int sock;
    int optval = 0x01;
    u8     iptxt[20];
    
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
        vTaskDelay(5 / portTICK_RATE_MS);
        if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0x00)) == -1)   //  IPPROTO_TCP)) == -1)
            return -1;
        lwip_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }   
    return sock;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void vTCPClientTask ( void *pvParameters )
//* ����        : tcp client task
//* �������    : void *pvParameters
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void vTCPClientTask ( void *pvParameters )
{
    vu32 ticker = 0x00;
    
    client.conn = 0x00;
    client.sock = -1;
    while (1)
    {
        if (client.conn == 0x00)
        {
            if (client.sock >= 0x00)
            {
                lwip_close(client.sock);
                client.sock = -1;
            }
              
            if ((client.sock = sea_createsocket()) < 0x00)
                continue;
            client.conn = 0x01;
            sea_printf("\n���ӷ������ɹ���");
        }
        else
        {
//            len = lwip_recv(client.s, client.recv.buf, BUF_SIZE, 0x00);
            client.recv.len = lwip_recv(client.sock, client.recv.buf, BUF_SIZE, 0x00);
            if (client.recv.len < 0x00)
                client.conn = 0x00;
	    else if (client.recv.len > 0x00)
            {
                client.recv.buf[client.recv.len] = 0x00;
            }
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
