#include <stdio.h>
#include <stdarg.h>
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

#ifdef LWIP_ENABLE     
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u8 sea_tcpsendbyte ( USART_TypeDef * uart, u8 ch )
//* 功能        : data output
//* 输入参数    : USART_TypeDef * uart, u8 ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u8 sea_tcpsendbyte ( USART_TypeDef * uart, u8 ch )
{ return 0x01; }
 
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static s16 sea_tcpsendframe ( struct frm_t frm, u8 cmd, u8 len, u16 road, const u8 * str )
//* 功能        : data output
//* 输入参数    : struct frm_t * frm, u8 cmd, u8 len, u16 road, const u8 * str
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static s16 sea_tcpsendframe ( struct frm_t * frm, u8 cmd, u8 len, u16 road, const u8 * str )
{
    u8   ch, sum;

    if (!len)   return 0x01;
    taskENTER_CRITICAL();
    client.send.len = 0x00;
    client.send.buf[client.send.len ++] = PREFIX;    
    client.send.buf[client.send.len ++] = cmd;
    client.send.buf[client.send.len ++] = len;      
    client.send.buf[client.send.len ++] = (road >> 0x08) & 0xff;
    client.send.buf[client.send.len ++] = road & 0xff;
    sum = cmd + len + (road & 0xff) + ((road >> 0x08) & 0xff);  
    for (ch = 0x00; ch < len; ch ++)
    {
        client.send.buf[client.send.len ++] = str[ch];
        sum += (str[ch] & 0xff);
    }
    client.send.buf[client.send.len ++] = sum;
    client.send.buf[client.send.len]    = 0x00;
    taskEXIT_CRITICAL();
    client.count ++;
    return 0x01;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_tcprintframe ( struct frm_t * frm, const u8 * str, ... )
//* 功能        : print frame infmation
//* 输入参数    : struct frm_t * frm, const u8 * str, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_tcprintframe ( struct frm_t * frm, const u8 * str, ... )
{
    va_list v_list;
    
    sea_memset(frm->buf, 0x00, FRPRINTBUF);
    va_start(v_list, str);     
    vsprintf((char *)frm->buf, (char const *)str, v_list); 
    va_end(v_list);
    taskENTER_CRITICAL();
    client.send.len = sea_strlen(frm->buf);
    sea_memcpy(client.send.buf, frm->buf, client.send.len);
    taskEXIT_CRITICAL();
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_initcpfrm ( pframe_info_t ptr,  USART_TypeDef * uart )
//* 功能        : initialize tcp frame information structure body
//* 输入参数    : pframe_info_t ptr,  USART_TypeDef * uart
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_initcpfrm ( pframe_info_t ptr,  USART_TypeDef * uart )
{
#ifdef LWIP_ENABLE     
    sea_initframe(&tcpfrm1, TCP_COM);
    tcpfrm1.put      = sea_tcpsendframe;      
    tcpfrm1.print    = sea_tcprintframe;            
    tcpfrm1.sendbyte = sea_tcpsendbyte;
#endif
}

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
    
    sea_resetetherif();                                         // reset enc28j60 chip.
    server.sin_family      = AF_INET;
    sprintf((char *)iptxt, "%d.%d.%d.%d", sys_info.ip[0x00], sys_info.ip[0x01], sys_info.ip[0x02], sys_info.ip[0x03]);
    server.sin_addr.s_addr = inet_addr((char *)iptxt);   
    server.sin_port        = htons(sys_info.port);
    
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
//* 函数名      : void vTCPRecvTask ( void *pvParameters )
//* 功能        : tcp client task
//* 输入参数    : void *pvParameters
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vTCPRecvTask ( void *pvParameters )
{
    vu32   ticker = 0x00;
    
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
                client.count = 0x00;
                client.sem   = 0x00;
                sea_printf("\nconnected！");
            }
            else
            {
                if (client.sem == 0x00)
                    client.recv.len = lwip_recv(client.sock, client.recv.buf, BUF_SIZE, 0x00);
                if (client.recv.len < 0x00)
                    client.conn = 0x00;
                else if (client.recv.len > 0x00)
                {
                    u16 i = 0x00;
                    client.recv.buf[client.recv.len ++] = '\n';
                    client.recv.buf[client.recv.len ++] = '\r';
                    while (client.recv.len --)
                        sea_putbyte(get_serialinfo(tcpfrm1.uart), client.recv.buf[i ++]);
                }
            }
        }
        else
        {
            vTaskDelay(client.retry / portTICK_RATE_MS);
            client.retry >>= 0x01;
        }
        
        vTaskDelay(1 / portTICK_RATE_MS);
        ticker ++;
    }
}

/*******************************************************************************
* Function Name  : portBASE_TYPE xAreTCPRecvTasksStillRunning  ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
portBASE_TYPE xAreTCPRecvTasksStillRunning ( void )
{
    portBASE_TYPE xReturn = 0x00;

    return xReturn;
}
