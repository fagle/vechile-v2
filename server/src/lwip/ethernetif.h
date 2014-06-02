#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

/////////////////////////////////////////////////////////////////////////////////////
//
#include "config.h"


#define emacETHADDR0 0x00
#define emacETHADDR1 0xbd
#define emacETHADDR2 0x33
#define emacETHADDR3 0x06
#define emacETHADDR4 0x68
#define emacETHADDR5 0x22

#define netifMTU				( 1514 )
#define netifINTERFACE_TASK_STACK_SIZE          ( 350 )
#define netifINTERFACE_TASK_PRIORITY		( configMAX_PRIORITIES - 0x03 )   // ( configMAX_PRIORITIES - 5 )
//#define netifGUARD_BLOCK_TIME			( 250 )
#define IFNAME0 'S'
#define IFNAME1 'T'

#define RXMAXSIZE                               (1520)   // * 0x04)
#define TXMAXSIZE                               (1500)   //  << 0x01)

/////////////////////////////////////////////////////////////////////////////////////
//
err_t ethernetif_init                ( struct netif *netif );
void ethernetif_input             ( void * vParameters );     // err_t ethernetif_input(struct netif *netif);
struct netif *ethernetif_register ( void );
int ethernetif_poll               ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_resetetherif ( void )
//* ����        : reset ethernet interface
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_resetetherif             ( void );

///////////////////////////////////////////////////////////////////////////////////////
//
#endif 
