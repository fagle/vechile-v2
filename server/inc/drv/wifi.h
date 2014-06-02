#ifndef __WIFI_H__
#define __WIFI_H__

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
#define     __I     volatile const          
#define     __O     volatile                
#define     __IO    volatile             

/////////////////////////////////////////////////////////////////////////////////////////
//
#define PING_DEFAULT_URL	        "www.sina.com\n"
#define PING_DEFAULT_PACKAGE_LENGTH     32

#define STATION                         0x01
#define AP                              0x02
#define STATIONAP                       0x03

#define	TCP_Server                      0x0
#define TCP_Client                      0x1

#define DHCP_Disable                    0x0
#define DHCP_Enable	                0x1
#define TCP_mode                        0x0
#define UDP_mode	                0x1

#define BaudRate_9600                   0x0
#define BaudRate_19200                  0x1
#define BaudRate_38400                  0x2 
#define BaudRate_57600                  0x3 
#define BaudRate_115200                 0x4
#define BaudRate_230400                 0x5
#define BaudRate_460800                 0x6
#define BaudRate_921600                 0x7 

#define buffer_2bytes                   0x0
#define buffer_16bytes                  0x1
#define buffer_32bytes	                0x2
#define buffer_64bytes	                0x3
#define buffer_128bytes	                0x4
#define buffer_256bytes                 0x5
#define buffer_512bytes                 0x6 
 
#define WordLength_8b                   0x0
#define WordLength_9b                   0x1

#define StopBits_1                      0x0                         
#define StopBits_0_5                    0x1                         
#define StopBits_2                      0x2                         
#define StopBits_1_5                    0x3                         

#define Parity_No                       0x0                         
#define Parity_Even                     0x1                         
#define Parity_Odd                      0x2  

#define HardwareFlowControl_None        0x0                         
#define HardwareFlowControl_RTS_CTS     0x1

#define Secure_WEP           		0x0                         
#define Secure_WPA_WPA2_PSK             0x1
#define Secure_None                     0x2

#define IO1_Normal			0x0
#define IO1_Frame_Control	 	0x1
#define IO1_485_TX_Enable               0x2

#define DNS_Disable                     0x0
#define DNS_Enable                      0x1

#define EM380ERROR                      -1
                         
#define WIFICMDSIZE                     (0x0a)
#define WIFILISTSIZE                    (0x30)

#define WIFI_STATE_PIN                  GPIO_Pin_5
#define WIFI_PORT                       GPIOB

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command code
#define EMSP_CMD_RESET			0x0001	// reset module which is composed of STM32 and AW-GH320
#define EMSP_CMD_GET_CONFIG		0x0002	// get wifi module configuration, ip address, tcp/udp, port etc.
#define EMSP_CMD_SET_CONFIG		0x0003	// set wifi module configuration, ip address, tcp/udp, port etc.
#define EMSP_CMD_SCAN_AP		0x0004	// scan ap
#define EMSP_CMD_START			0x0005	// start module
#define EMSP_CMD_SEND_DATA		0x0006	// send data to module, and the data will be send out use the WIFI
#define EMSP_CMD_RECV_DATA		0x0007	// receive data from module
#define EMSP_CMD_GET_STATUS		0x0008	// get module status
#define EMSP_CMD_GET_VER		0x0009	// get module hardware & firmware version
//#define	EMSP_CMD_GET_RF_POWER	        0x000A  // Get the value of the RF power
//#define	EMSP_CMD_SET_RF_POWER	        0x000B  // Set the value of the RF power
#define EMSP_CMD_GET_MAC  		0x000C	// get 88w8686's MAC address

#define EMSP_CMD_SET_DNS		0x0052	// set wifi dns
#define EMSP_CMD_GET_DNS		0x0053	// get wifi dns
#define EMSP_CMD_GET_UART		0x0062	// get wifi uart mode
#define EMSP_CMD_SET_UART		0x0061	// set wifi uart mode
#define EMSP_CMD_GET_NAME		0x0046	// get wifi mode name
#define EMSP_CMD_SET_NAME		0x0047	// set wifi mode name
#define EMSP_CMD_GET_IP 		0x0040	// get ip address via dhcp
#define EMSP_CMD_CLOSE_TCP		0x0044	// close socket
#define EMSP_CMD_OPEN_TCP		0x0045	// open socket
#define EMSP_CMD_STOP_WIFI		0x004A	// stop wifi link
#define EMSP_CMD_CONNECT		0x004B	// connect wifi link
#define EMSP_CMD_CMP    		0x0050	// get the access point singal

#define EMSP_CMD_PING_TEST              0xA005  // ping test

////////////////////////////////////////////////////////////////////////////////////////////////
// in function sea_parseresponse() of wifi_info.control 
#define WIFICOMMAND           (0x0001)
#define WIFIRECVRESP          (0x0002)
#define WIFIRECVDATA          (0x0004)
#define WIFIIDLE              (0x0008)
#define WIFITCP               (0x0010)
#define WIFIOK                (0x0020)
#define WIFIREC               (0x0040)
#define WIFIGREAT             (0x0080)
#define WIFIERROR             (0x0100)
#define WIFICMTI              (0x0200)

/////////////////////////////////////////////////////////////////////////////////////////
//
#define EMSP_CONFIG_MODE                0x00
#define EMSP_DTU_MODE                   0x01

#define set_wifistatus(type)   (wifi_info.control |= (type))
#define clr_wifistatus(type)   (wifi_info.control  &= ~(type))
#define get_wifistatus(type)   (wifi_info.control  & (type))

/////////////////////////////////////////////////////////////////////////////////////////
//
typedef enum
{	
    TCP_stop      = 0x00,
    TCP_listening = 0x01,
    TCP_connected = 0x02,
    TCP_unknown   = 0xFF,
} ETCPStatus;

typedef enum
{
    WIFI_disconnected = 0x00,
    WIFI_connected    = 0x05,
    WIFI_unknown      = 0xFF,
} EWIFIStatus;

typedef enum
{
    TCP_client = 0x0,
    TCP_server = 0x1
} EM380C_Mode_TypeDef;

typedef enum
{   eWIFIRESET = 0x01, eGETCONFIG, eSETCONFIG, eSETDNS, eGETDNS, eGETUART, eSETUART, eGETNAME, eSETNAME, 
    eGETVER, eGETMAC, eSTART, eGETIP, eCLOSETCP, eOPENTCP, eSTOPWIFI, eCNTWIFI, eSCANAP, eSCANCMP, 
    eSENDDATA, eRECVDATA, eGETSTATUS
} EWIFI;

/////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    EWIFI id;
    u8    cmd[WIFICMDSIZE - 0x02];
    u8  * help;
} wifi_cmd_t, *pwifi_cmd_t;

typedef struct
{
    u8    in, out, cnt;         // list of commands
    EWIFI list[WIFILISTSIZE];    // wifi command list
} wifi_cmd_list_t, *pwifi_cmd_list_t;

typedef struct
{
    u8 UART_buadrate;	        // 0:9600, 1:19200, 2:38400, 3:57600, 4:115200
    u8 DMA_buffersize;	        // 0:2, 1:16, 2:32, 3:64, 4:128, 5:256, 6:512
    u8 use_CTS_RTS;		// 0:disale, 1:enable
    u8 parity;			// 0:none, 1:even parity, 2:odd parity
    u8 data_length;		// 0:8, 1:9
    u8 stop_bits;		// 0:1, 1:0.5, 2:2, 3:1.5
} EM380C_Uart;

typedef struct
{
    u32 enable;
    u8  name[64];
    u8  ip[16];
} dns_t, *pdns_t;

typedef struct
{
    u8  udp;
    u8  connect;
    u8  portH;
    u8  portL;
    u8  domain[64];
} socket_t, *psocket_t;

typedef struct
{
    u8 wifi_mode;		// Wlan802_11IBSS(0), Wlan802_11Infrastructure(1)
    u8 wifi_ssid[32];	        // 
    u8 wifi_wepkey[16];	        // 40bit and 104 bit
    u8 wifi_wepkeylen;	        // 5, 13

    u8 local_ip_addr[16];	// if em380 is server, it is server's IP;	if em380 is client, it is em380's default IP(when DHCP is disable)
    u8 remote_ip_addr[16];	// if em380 is server, it is NOT used;		if em380 is client, it is server's IP
    u8 net_mask[16];	        // 255.255.255.0
    u8 gateway_ip_addr[16];     // gateway ip address
    u8 portH;			// High Byte of 16 bit
    u8 portL;			// Low Byte of 16 bit
    u8 connect_mode;	        // 0:server  1:client
    u8 use_dhcp;		// 0:disale, 1:enable
    u8 use_udp;			// 0:use TCP,1:use UDP

    u8 UART_buadrate;	        // 0:9600, 1:19200, 2:38400, 3:57600, 4:115200
    u8 DMA_buffersize;	        // 0:2, 1:16, 2:32, 3:64, 4:128, 5:256, 6:512
    u8 use_CTS_RTS;		// 0:disale, 1:enable
    u8 parity;			// 0:none, 1:even parity, 2:odd parity
    u8 data_length;		// 0:8, 1:9
    u8 stop_bits;		// 0:1, 1:0.5, 2:2, 3:1.5

    u8 IO_Control;		// 0 - 255
    u8 sec_mode;                // 0 = wep, 1=wpa psk, 2=none
    u8 wpa_psk[32];
} EM380C_parm_TypeDef;

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////
//
__packed typedef struct
{									
   uint8_t url[64];
   uint32_t pingPackageLength; 
} CMD_PING_TEST_INPUT_Typedef;

/////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    int32_t pingRresult[10];   
}  CMD_PING_TEST_RETURN_Typedef;

typedef struct
{
    u16 cmdcode;	// Command code
    u16 size;		// size of the packet, including the header and footer
    u16 result;		// Result code, set by module, This field is used only the response packet. Set 0 in the request packet
    u16 checksum;	// check sum of the header
} emsp_header;

typedef struct 
{
    u16 checksum;	// check sum of the packet body( not include the header)
} emsp_footer;

/////////////////////////////////////////////////////////////////////////////////////////
//
#define RNAMESIZE     (20)    // wireless router name length
#define RLISTSIZE     (10)    // size of route list 

typedef struct
{
    u16  wifi;
    u16  tcp;
} wifi_state_t;

typedef struct 
{
    u8   name[RNAMESIZE];
    u8   signal;
} rlist_t; 

typedef struct
{
    u32      version;
    u8       mac[0x06];
    u8       control;
    u8       mode;
    u8       count;
    rlist_t  route[RLISTSIZE];
    dns_t    dns;
    socket_t sock;
    wifi_state_t  status;
    emsp_header  * hdr;
    EM380C_parm_TypeDef cfg;
    u16      lpg;
    GPIO_TypeDef * port;
} wifi_info_t, *pwifi_info_t;

/////////////////////////////////////////////////////////////////////////////////////////
//
s8   recv_cmd         ( u8 *recvbuf );

s8   EM380C_SendCmd   ( u16 cmd );
s8   EM380C_SendData  ( const u8 * data, u16 len );

u8   wifi_ReportFrame ( u8 id, u8 size, u16 road, u8 * body );

void sea_printfwifinfo ( void );

/************************************************
* 函数名      : EM380C_parm_TypeDef * EM380C_DefaultWifiParm ( void )
* 功能        : write defalut wifi moudel parameter to flash
* 输入参数    : 无
* 输出参数    : EM380C_parm_TypeDef *
* 修改记录    : 无
* 备注        : 无
*************************************************/  
EM380C_parm_TypeDef * EM380C_DefaultWifiParm ( void );

/************************************************
* 函数名      : s8 EM380C_SetMode ( u8 mode )
* 功能        : brief Set up Wi-Fi module to be DTU module or Config module. 
*               note only used under module Firmware 0105F1XX. 
*               configuration in command mode, if change wifi set.
* 输入参数    : u8 mode
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
s8 EM380C_SetMode ( u8 mode );

/************************************************
* 函数名      : void EM380C_Printf ( const char * cmd, ... )
* 功能        : send command to uart
* 输入参数    : const char * cmd, ...
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void EM380C_Printf ( const char * cmd, ... );

/************************************************
* 函数名      : s8 EM380C_SendCommand( u8 id, u16 len, void * data )
* 功能        : send command to uart
* 输入参数    : EWIFI id, u16 len, void * data
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
s8 EM380C_SendCommand ( u8 id, u16 len, void * data );

/************************************************
* 函数名      : void sea_putwifibyte ( u8 ch )
* 功能        : parse wifi receive message
* 输入参数    : u8 ch
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_putwifibyte ( u8 ch );

/************************************************
* 函数名      : void EM380C_ChangeServer ( u16 port, const u8 * domain )
* 功能        : change remote server
* 输入参数    : u16 port, const u8 * domain 
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void EM380C_ChangeServer ( u16 port, const u8 * domain );

/////////////////////////////////////////////////////////////////////////////////////////
//
extern wifi_info_t wifi_info;

/////////////////////////////////////////////////////////////////////////////////////////
//
#endif  // __WIFI_H__