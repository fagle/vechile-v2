

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"

#include "ethernetif.h"
#include "netconf.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "network.h"

///////////////////////////////////////////////////////////////////////////////////////
//
typedef enum
{
  DHCP_START = 0x00,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
} DHCP_State_TypeDef;

#define MAX_DHCP_TRIES 5

///////////////////////////////////////////////////////////////////////////////////////
//
struct netif xnetif; // network interface structure 

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatenetaddr ( void )
//* 功能        : update client network address
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatenetaddr ( void )
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

    IP4_ADDR(&ipaddr, sys_info.local.ip[0x00], sys_info.local.ip[0x01], sys_info.local.ip[0x02], sys_info.local.ip[0x03]);
    IP4_ADDR(&netmask, sys_info.local.mk[0x00], sys_info.local.mk[0x01], sys_info.local.mk[0x02], sys_info.local.mk[0x03]);
    IP4_ADDR(&gw, sys_info.local.gw[0x00], sys_info.local.gw[0x01], sys_info.local.gw[0x02], sys_info.local.gw[0x03]);
    netif_set_addr(&xnetif, &ipaddr , &netmask, &gw);
}

///////////////////////////////////////////////////////////////////////////////////////
//
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LWIP_Configuration ( void )
{
#if 0
//    const u8 DEFLOCIP[IPSIZE]   = { 10, 1, 130, 105 };   // local ip address
//    const u8 DEFLOCMK[IPSIZE]   = { 255, 255, 255, 0 };   // local mask
//    const u8 DEFLOCGW[IPSIZE]   = { 10, 1, 130, 254 };     // local gateway ip
#ifdef HOME_IPADDR    
    const u8 DEFLOCIP[IPSIZE]   = { 192, 168, 0, 110 };   // local ip address
    const u8 DEFLOCMK[IPSIZE]   = { 255, 255, 255, 0 };   // local mask
    const u8 DEFLOCGW[IPSIZE]   = { 192, 168, 0, 1 };     // local gateway ip
#else
    const u8 DEFLOCIP[IPSIZE]   = { 192, 168, 1, 110 };   // local ip address
    const u8 DEFLOCMK[IPSIZE]   = { 255, 255, 255, 0 };   // local mask
    const u8 DEFLOCGW[IPSIZE]   = { 192, 168, 1, 1 };     // local gateway ip
#endif
#endif
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
#ifdef USE_DHCP
    u8     iptab[4];
    u8     iptxt[20];
#endif
    tcpip_init( NULL, NULL );          // Create tcp_ip stack thread 

  /* IP address setting & display on STM32_evalboard LCD*/
#ifdef USE_DHCP
    ipaddr.addr  = 0x00;
    netmask.addr = 0x00;
    gw.addr      = 0x00;
#else
#if 1  
    IP4_ADDR(&ipaddr, sys_info.local.ip[0x00], sys_info.local.ip[0x01], sys_info.local.ip[0x02], sys_info.local.ip[0x03]);
    IP4_ADDR(&netmask, sys_info.local.mk[0x00], sys_info.local.mk[0x01], sys_info.local.mk[0x02], sys_info.local.mk[0x03]);
    IP4_ADDR(&gw, sys_info.local.gw[0x00], sys_info.local.gw[0x01], sys_info.local.gw[0x02], sys_info.local.gw[0x03]);
#else
    IP4_ADDR(&ipaddr, DEFLOCIP[0x00], DEFLOCIP[0x01], DEFLOCIP[0x02], DEFLOCIP[0x03]);
    IP4_ADDR(&netmask, DEFLOCMK[0x00], DEFLOCMK[0x01], DEFLOCMK[0x02], DEFLOCMK[0x03]);
    IP4_ADDR(&gw, DEFLOCGW[0x00], DEFLOCGW[0x01], DEFLOCGW[0x02], DEFLOCGW[0x03]);
#endif
#endif
    /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))

       Adds your network interface to the netif_list. Allocate a struct netif and pass a pointer to this structure 
    as the first argument. Give pointers to cleared ip_addr structures when using DHCP, or fill them with sane numbers 
    otherwise. The state pointer may be NULL. The init function pointer must point to a initialization function for
    your ethernet netif interface. The following code illustrates it's use.*/

    netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    netif_set_default(&xnetif);     //  Registers the default network interface. 
    netif_set_up(&xnetif);          //  When the netif is fully configured this function must be called.
}

#ifdef USE_DHCP
/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
void vDHCPClientTask ( void * pvParameters )
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    uint32_t IPaddress;
    uint8_t iptab[4];
    uint8_t iptxt[20];
    uint8_t DHCP_state;
    DHCP_state = DHCP_START;

    for (;;)
    {
        switch (DHCP_state)
        {
            case DHCP_START:
                dhcp_start(&xnetif);
                IPaddress = 0;
                DHCP_state = DHCP_WAIT_ADDRESS;
                break;

            case DHCP_WAIT_ADDRESS:
                /* Read the new IP address */
                IPaddress = xnetif.ip_addr.addr;

                if (IPaddress!=0)
                {
                    DHCP_state = DHCP_ADDRESS_ASSIGNED;
                    dhcp_stop(&xnetif);
                    /* end of DHCP process: LED1 stays ON*/
                    STM_EVAL_LEDOn(LED1);
                    vTaskDelete(NULL);
                }
                else
                {
                    /* DHCP timeout */
                    if (xnetif.dhcp->tries > MAX_DHCP_TRIES)
                    {
                        DHCP_state = DHCP_TIMEOUT;
                        dhcp_stop(&xnetif);

                        IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
                        IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
                        IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
                        netif_set_addr(&xnetif, &ipaddr , &netmask, &gw);
                        vTaskDelete(NULL);
                    }
                }
                break;
        }
        vTaskDelay(250);     // wait 250 ms 
    }
}
#endif  /* USE_DHCP */


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
