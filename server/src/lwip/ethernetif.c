/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include <string.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/err.h"
#include "netif/etharp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "enc28j60.h"
#include "network.h"
#include "ethernetif.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
static u8 Rx_Data_Buf[RXMAXSIZE + 0x01];
static u8 Tx_Data_Buf[TXMAXSIZE];

/////////////////////////////////////////////////////////////////////////////////////////////
//
struct ethernetif
{
    struct eth_addr *ethaddr;
};
static struct ethernetif ethernetif;
#ifdef MACINT_ENABLE
static struct netif * enc_netif = NULL;
#endif

//////////////////////////////////////////////////////////////////////////////////////
//
/*-----------------------------------------------------------*/
     //网卡初始化函数
static void low_level_init ( struct netif *netif )
{
    netif->hwaddr_len = 0x06;
    netif->hwaddr[0]  = sys_info.local.mac[0x00];   // emacETHADDR0;
    netif->hwaddr[1]  = sys_info.local.mac[0x01];   // emacETHADDR1;
    netif->hwaddr[2]  = sys_info.local.mac[0x02];   // emacETHADDR2;
    netif->hwaddr[3]  = sys_info.local.mac[0x03];   // emacETHADDR3;
    netif->hwaddr[4]  = sys_info.local.mac[0x04];   // emacETHADDR4;
    netif->hwaddr[5]  = sys_info.local.mac[0x05];   // emacETHADDR5;
    netif->mtu        = netifMTU;                   // netifMTU;  // maximum transfer unit, 最大传输单元 
    netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;   // broadcast capability 

	/* Initialise the EMAC.  This routine contains code that polls status bits.  
	If the Ethernet cable is not plugged in then this can take a considerable 
	time.  To prevent this starving lower priority tasks of processing time we
	lower our priority prior to the call, then raise it back again once the
	initialisation is complete. */
    enc28j60Init(netif->hwaddr);
    enc28j60clkout(0x01); 				// change clkout from 6.25MHz to 12.5MHz

    // 任务入口函数   创建任务的名字  任务堆栈的长度   给任务的参数  优先级  句柄反馈使用任务的
#ifdef MACINT_ENABLE
    enc_netif = netif;
#else
    xTaskCreate(ethernetif_input, (s8 * )"ETH_INT", netifINTERFACE_TASK_STACK_SIZE, netif, netifINTERFACE_TASK_PRIORITY, (xTaskHandle *)NULL);
#endif
}		

/*-----------------------------------------------------------*/
/* low_level_output(): Should do the actual transmission of the packet. The 
 * packet is contained in the pbuf that is passed to the function. This pbuf 
 * might be chained. //链路层发送函数
 */
static err_t low_level_output ( struct netif *netif, struct pbuf *p )
{
    struct pbuf *q;
    int i = 0x00;   
    err_t xReturn = ERR_OK;

    for (q = p; q != NULL; q = q->next) 
    {
        sea_memcpy(&Tx_Data_Buf[i], (u8_t*)q->payload, q->len); // ping -t 192.168.1.18
   	i = i + q->len;
    }
    taskENTER_CRITICAL();
    enc28j60PacketSend(i, Tx_Data_Buf); //发送数据包
    taskEXIT_CRITICAL();
    
#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */

    return xReturn;
}

/*-----------------------------------------------------------*/
/* low_level_input(): Should allocate a pbuf and transfer the bytes of the 
 * incoming packet from the interface into the pbuf. 
 */
static struct pbuf *low_level_input ( struct netif *netif )
{
    struct pbuf *q, *p = NULL;
    u16 size = 0x00; 
    int i    = 0x00;
			
#ifndef MACINT_ENABLE
    taskENTER_CRITICAL();
#endif
    size = enc28j60PacketReceive(RXMAXSIZE, Rx_Data_Buf);     // 返回接收到的数据包长度	
    if (size == 0x00) 
        goto home;   

    p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (p != NULL) 
    {
        for (q = p; q != NULL; q = q->next) 
        {	 
            sea_memcpy((u8_t*)q->payload, (u8_t*)&Rx_Data_Buf[i], q->len);
             i = i + q->len;
        }
        if (i != p->tot_len)
            goto home;   
    }
#ifndef MACINT_ENABLE
    taskEXIT_CRITICAL();
#endif
    return p;
home:
#ifndef MACINT_ENABLE
    taskEXIT_CRITICAL();
#endif
    return NULL;   
}

/*-----------------------------------------------------------*/
/*
 * ethernetif_output(): This function is called by the TCP/IP stack when an 
 * IP packet should be sent. It calls the function called low_level_output() 
 * to do the actual transmission of the packet.
 */
static err_t ethernetif_output ( struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr )
{
    return etharp_output( netif, p, ipaddr );    // resolve hardware address, then send (or queue) packet 
}

#ifdef MACINT_ENABLE
/*-----------------------------------------------------------*/
/*
 * ethernetif_input_isr(): This function should be called when a packet is ready to 
 * be read from the interface. It uses the function low_level_input() that 
 * should handle the actual reception of bytes from the network interface.
 */
void ethernetif_input_isr ( void )
{
    struct pbuf  *p;

    if (enc_netif == NULL)
        return;
    
    if ((p = low_level_input(enc_netif)) != NULL)       
    {
        if (enc_netif->input(p, enc_netif) != ERR_OK)
        {
            pbuf_free(p);
            p = NULL;
        }
    }
}
#else
/*-----------------------------------------------------------*/
/*
 * ethernetif_input(): This function should be called when a packet is ready to 
 * be read from the interface. It uses the function low_level_input() that 
 * should handle the actual reception of bytes from the network interface.
 */
void  ethernetif_input ( void * pvParameters )
{  	
    struct pbuf  *p;
    struct netif *netif = (struct netif *)pvParameters;

    for (;;)
    {
        if ((p = low_level_input(netif)) != NULL)                   // no packet could be read, silently ignore this 
        {
            if (netif->input(p, netif) != ERR_OK)
            {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                pbuf_free(p);
                p = NULL;
            }
        }
        vTaskDelay(2 / portTICK_RATE_MS);
    }
}
#endif

static void arp_timer ( void *arg )
{
    ( void ) arg;

    etharp_tmr();
    sys_timeout( ARP_TMR_INTERVAL, arp_timer, NULL );
}

err_t ethernetif_init ( struct netif * netif )
{
    netif->state       = &ethernetif;
    netif->name[0]     = IFNAME0;
    netif->name[1]     = IFNAME1;
    netif->output      = ethernetif_output;
    netif->linkoutput  = low_level_output;
    ethernetif.ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

    low_level_init(netif);
    etharp_init();
    sys_timeout( ARP_TMR_INTERVAL, arp_timer, NULL );

    return ERR_OK;
}
