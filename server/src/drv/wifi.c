#include <stdio.h>
#include <stdarg.h>
#include "FreeRTOSConfig.h"
#include "network.h"
#include "gprs.h"
#include "mem.h"
#include "serial.h"
#include "iic.h"
#include "frame.h"
#include "lamp.h"
#include "http.h"
#include "cmd.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
const wifi_cmd_t wifi_cmd[] = { {eWIFIRESET,  0x01, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xf4, 0xff, "reset wifi module"},
                                {eGETCONFIG,  0x02, 0x00, 0x0b, 0x00, 0x00, 0x00, 0xf2, 0xff, "get wifi configuration"},
                                {eSETCONFIG,  0x03, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x53, 0xff, "set wifi configuration"},
                                {eSETDNS,     0x52, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x4f, 0xff, "set wifi dns"},
                                {eGETDNS,     0x53, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xa2, 0xff, "get wifi dns"},
                                {eGETUART,    0x62, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x93, 0xff, "get wifi uart mode"},
                                {eSETUART,    0x61, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x93, 0xff, "set wifi uart mode"},
                                
                                {eGETNAME,    0x46, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xAF, 0xFF, "get wifi mode name"},
                                {eSETNAME,    0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, "set wifi mode name"},
                                {eGETVER,     0x09, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xEC, 0xFF, "get hardware and software version"},
                                {eGETMAC,     0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xE9, 0xFF, "get wifi module mac address"},
                                {eSTART,      0x05, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xF0, 0xFF, "wifi module start tcp/ip link via get status cmd"},
                                {eGETIP,      0x40, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xB5, 0xFF, "get ip address via dhcp"},
                                {eCLOSETCP,   0x44, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xB1, 0xFF, "close socket"},
                                {eOPENTCP,    0x45, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x6C, 0xFF, "open socket"},
                                {eSTOPWIFI,   0x4A, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xAB, 0xFF, "stop wifi link"},
                                {eCNTWIFI,    0x4B, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xAA, 0xFF, "connect wifi link"},
                                {eSCANAP,     0x04, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xF1, 0xFF, "scaning wifi access point"},
                                {eSCANCMP,    0x50, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x85, 0xFF, "get the access point singal"},
                                {eSENDDATA,   0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, "send data with wifi"},
                                {eRECVDATA,   0x07, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xEE, 0xFF, "recieve data from wifi"},
                                {eGETSTATUS,  0x08, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xED, 0xFF, "get wifi module current status"},
                                {(EWIFI)0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NULL} };
wifi_info_t wifi_info;

extern frame_info_t gprsfrm1;

////////////////////////////////////////////////////////////////////////////////////////
//
static u16 EM380C_CalcSum ( const void *data, u32 len )
{
    u32  cksum = 0x00;
    __packed u16 *p = (u16 *)data;

    while (len > 0x01)
    {
        cksum += *p++;
	len   -= 0x02;
    }
    
    if (len)
        cksum += *(u8 *)p;
    cksum  = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return ~cksum;
}

static int EM380C_CheckSum ( const void *data, u16 len )	
{
    __packed u16 *sum;
    u8 *p = (u8 *)data;
	
    p += len - 0x02;
    sum = (u16 *)p;

    return (EM380C_CalcSum(data, len - 0x02) == *sum) ? 0x01 : 0x00;
}

void sea_printfwifinfo ( void )
{
    sea_printf("\nwifi configuration as following:");
    sea_printf("\nwifi work mode: %d", wifi_info.cfg.wifi_mode);
    sea_printf("\nlocal ip address: %s", wifi_info.cfg.local_ip_addr);
    sea_printf("\nremote ip address: %s", wifi_info.cfg.remote_ip_addr);
    sea_printf("\nnetwork mask: %s", wifi_info.cfg.net_mask);
    sea_printf("\ngateway ip address: %s", wifi_info.cfg.gateway_ip_addr);
    sea_printf("\ntcp port: %d, connect mode: %d, tcp mode: %d, dhcp: %d", wifi_info.cfg.portH << 0x08 | wifi_info.cfg.portL,
                                               wifi_info.cfg.connect_mode, wifi_info.cfg.use_udp, wifi_info.cfg.use_dhcp); 
    sea_printf("\nuart baud, dma, cts, parity, len and stop: %d, %d, %d, %d, %d, %d",  wifi_info.cfg.UART_buadrate,
                                               wifi_info.cfg.DMA_buffersize,
                                               wifi_info.cfg.use_CTS_RTS,
                                               wifi_info.cfg.parity,
                                               wifi_info.cfg.data_length,
                                               wifi_info.cfg.stop_bits);
    sea_printf("\nssid %s, wep len %d, security mode:%d, io control: %d", wifi_info.cfg.wifi_ssid, 
                                               wifi_info.cfg.wifi_wepkeylen,
                                               wifi_info.cfg.sec_mode,
                                               wifi_info.cfg.IO_Control);
    if (sys_info.route.mode == Secure_WPA_WPA2_PSK)
       sea_printf("\nwpa2 security key %s", wifi_info.cfg.wpa_psk);
    else if (strlen((char *)sys_info.route.key))
        sea_printf("\nwep security key %s", wifi_info.cfg.wifi_wepkey);   
}

/************************************************
* 函数名      : void EM380C_ChangeServer ( u16 port, const u8 * domain )
* 功能        : change remote server
* 输入参数    : u16 port, const u8 * domain 
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void EM380C_ChangeServer ( u16 port, const u8 * domain )
{
    wifi_info.sock.portH  = (port >> 0x08) & 0xff;  
    wifi_info.sock.portL  = port & 0xff;
    wifi_info.cfg.portH   = wifi_info.sock.portH;
    wifi_info.cfg.portL   = wifi_info.sock.portL;
    memcpy(wifi_info.sock.domain, domain, strlen((char *)domain));
    memcpy(wifi_info.cfg.remote_ip_addr, domain, strlen((char *)domain));
}

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
s8 EM380C_SetMode ( u8 mode )
{
    switch (mode)
    {
        case EMSP_DTU_MODE:      
           GPIO_WriteBit(wifi_info.port, wifi_info.lpg, Bit_SET);
	   break;
        case EMSP_CONFIG_MODE:   
           GPIO_WriteBit(wifi_info.port, wifi_info.lpg, Bit_RESET);
	   break;
	default:
	   return -1;
    }
    return 0x00;
}

/************************************************
* 函数名      : void EM380C_ResetResponse ( void )
* 功能        : reset response buffer
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void EM380C_ResetResponse ( void )
{
    GPRS_ResetResponse();

    wifi_info.control = WIFIIDLE;
    wifi_info.count   = 0x00;
    wifi_info.hdr     = NULL;
}

/************************************************
* 函数名      : s8 EM380C_SendCommand( u8 id, u16 len, void * data )
* 功能        : send command to uart
* 输入参数    : EWIFI id, u16 len, void * data
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
s8 EM380C_SendCommand ( u8 id, u16 len, void * data )
{
    emsp_header * hdr;
    u16  checksum;
    s8   ret = 0x00;

    EM380C_SetMode(EMSP_CONFIG_MODE);
    EM380C_ResetResponse();
    set_wifistatus(WIFICOMMAND);
    clr_wifistatus(WIFIIDLE);
    hdr = (emsp_header *)wifi_cmd[id - 0x01].cmd;
    sea_memcpy(gprs_info.cmd, (void *)wifi_cmd[id - 0x01].cmd, WIFICMDSIZE - 0x02);
    if (hdr->size == WIFICMDSIZE)
    {
        gprs_info.cmd[WIFICMDSIZE - 0x02] = 0xff;
        gprs_info.cmd[WIFICMDSIZE - 0x01] = 0xff;
        ret = UartSendSizeString(gprsfrm1.uart, gprs_info.cmd, WIFICMDSIZE);
    }
    else if (hdr->size == 0x00)
    {
        hdr->size = len + sizeof(emsp_header) + sizeof(emsp_footer);
        hdr->checksum = EM380C_CalcSum(hdr, sizeof(emsp_header) - 0x02);
        memcpy(&gprs_info.cmd[WIFICMDSIZE - 0x02], data, len);
        checksum = EM380C_CalcSum(data, len); 
        gprs_info.cmd[hdr->size - 0x02] = checksum & 0xff;
        gprs_info.cmd[hdr->size - 0x01] = (checksum >> 0x08) & 0xff;
        ret = UartSendSizeString(gprsfrm1.uart, gprs_info.cmd, hdr->size);
    }
    else if (hdr->size == len + WIFICMDSIZE)
    {
        memcpy(&gprs_info.cmd[WIFICMDSIZE - 0x02], data, len);
        checksum = EM380C_CalcSum(data, len); 
        gprs_info.cmd[WIFICMDSIZE - 0x02] = 0x01;
        gprs_info.cmd[hdr->size - 0x02] = checksum & 0xff;
        gprs_info.cmd[hdr->size - 0x01] = (checksum >> 0x08) & 0xff;
        ret = UartSendSizeString(gprsfrm1.uart, gprs_info.cmd, hdr->size);
    }
    else
        sea_printf("\nsend command %04x error.", id);
    
    if (ret)
    {
        delay(0x200);
        EM380C_SetMode(EMSP_DTU_MODE);
        return ret;
    }
    clr_wifistatus(WIFICOMMAND);
    set_wifistatus(WIFIIDLE);
    return -1;
}

/************************************************
* 函数名      : void EM380C_Printf ( const char * cmd, ... )
* 功能        : send command to uart
* 输入参数    : const char * cmd, ...
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void EM380C_Printf ( const char * cmd, ... )
{
    va_list v_list;

    memset(gprs_info.cmd, 0x00, PRINTFBUF);
    va_start(v_list, format);      
    vsprintf((char *)gprs_info.cmd, cmd, v_list); 
    va_end(v_list);
    
    UartSendString(gprsfrm1.uart, gprs_info.cmd);
}

/************************************************
* 函数名      : static void sea_putresponse ( u8 ch )
* 功能        : put char to response buffer
* 输入参数    : u8 ch
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void sea_putwifiresponse ( u8 ch )
{
    resp1->buf[resp1->count ++] = ch;
    if (resp1->count >= GPRS_RESPSIZE)
        memset(resp1, 0x00, sizeof(resp_t));
}

static s8 sea_parsewifihead ( const emsp_header *hdr )
{
    if (!EM380C_CheckSum(hdr, sizeof(emsp_header)) || hdr == NULL) 
    {
        sea_printf("\nsum error:cmd %04x, size %04x, result %04x, sum %04x", hdr->cmdcode, hdr->size, hdr->result, hdr->checksum);
        EM380C_ResetResponse();
        return -1;
    }
	
    clr_wifistatus(WIFICOMMAND);
    switch (hdr->cmdcode) 
    {
        case EMSP_CMD_RESET:		     // no response		
            sea_printf("\nreset.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_GET_CONFIG:	     // will recveive wifi_parm structure body, size = 0xa9
            sea_printf("\nget config.");   
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_SET_CONFIG:	     // will recveive wifi_parm structure body, size = 0xa9
            sea_printf("\nset config.");   
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_GET_STATUS:	     // get status of tcp	
            sea_printf("\nget status.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_START:				
            sea_printf("\ntcp start.");      // no response
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_GET_MAC:               // get mac address, size = 0x10
            sea_printf("\nset wifi dns.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_SEND_DATA:             // send data, result = 0x00, send error	
            sea_printf("\nhost send data to web server.");
            if (hdr->result == 0x00)
                set_wifistatus(WIFIERROR);
            break;
        case EMSP_CMD_RECV_DATA:             // receive data, result = 0x00, no data	
            sea_printf("\nhost receive data from web server");
            if (hdr->result == 0x00)
                set_wifistatus(WIFIIDLE);
            else
            {
                wifi_info.count = hdr->result;
                set_wifistatus(WIFIRECVDATA);
            }
            break;
        case EMSP_CMD_SCAN_AP:	             // return ssid, rssi string, count = number of found ap
            sea_printf("\nscan ap and return scan result to host.");
            wifi_info.count = hdr->result;
            set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_GET_VER:               // get version, size = 0x0e
            sea_printf("\nget version.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_SET_DNS:               // no response
            sea_printf("\nset wifi dns.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_GET_DNS:               // get dns server infomation, size = 0x5e
            sea_printf("\nget wifi dns.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;  
        case EMSP_CMD_GET_UART:              // get uart mode
            sea_printf("\nget uart.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_SET_UART:              // no response
            sea_printf("\nset uart.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_GET_NAME:              // get device name, size = 40
            sea_printf("\nget wifi mode name.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_SET_NAME:              // no response
            sea_printf("\nset wifi mode name.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_GET_IP:                // get ip address, mask and gateway, size = 0x3a
            sea_printf("\nget ip address via dhcp.");
            if (hdr->result == 0x01)
                set_wifistatus(WIFIRECVRESP);
            break;
        case EMSP_CMD_CLOSE_TCP:             // no response
            sea_printf("\nclose socket.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_OPEN_TCP:              // no response
            sea_printf("\nopen socket.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_STOP_WIFI:             // no response
            sea_printf("\nstop wifi link.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_CONNECT:               // no response
            sea_printf("\nconnect wifi link.");
            set_wifistatus(WIFIIDLE);
            break;
        case EMSP_CMD_CMP:                   // get ap singal
            sea_printf("\nget the access point singal.");
            wifi_info.count = hdr->result;
            set_wifistatus(WIFIRECVRESP);
            break;
        default:
            set_wifistatus(WIFIIDLE);
            sea_printf("\nunknow command.");
            return -1;
    }
    return 0x00;
}

static void sea_parseresponse ( u8 ch )
{
    if (!wifi_info.hdr)
        return;
    
    if (resp1->count >= wifi_info.hdr->size)
    {
        switch (wifi_info.hdr->cmdcode) 
        {
            case EMSP_CMD_GET_CONFIG:	     // will recveive wifi_parm structure body, size = 0xa9
                break;
            case EMSP_CMD_SET_CONFIG:	     // will recveive wifi_parm structure body, size = 0xa9
                break;
            case EMSP_CMD_GET_STATUS:	     // get status of tcp	
                memcpy(&wifi_info.status, &resp1->buf[WIFICMDSIZE - 0x02], sizeof(wifi_state_t));
                sea_printf("\ntcp status %d, wifi status %d", wifi_info.status.tcp, wifi_info.status.wifi);
                break;
            case EMSP_CMD_GET_MAC:           // get mac address, size = 0x10
                break;
            case EMSP_CMD_SCAN_AP:	     // return ssid, rssi string, count = number of found ap
                break;
            case EMSP_CMD_GET_VER:           // get version, size = 0x0e
                break;
            case EMSP_CMD_GET_DNS:           // get dns server infomation, size = 0x5e
                break;  
            case EMSP_CMD_GET_UART:          // get uart mode
                break;
            case EMSP_CMD_GET_NAME:          // get device name, size = 40
                break;
            case EMSP_CMD_GET_IP:            // get ip address, mask and gateway, size = 0x3a
                break;
            case EMSP_CMD_CMP:               // get ap singal
                break;
        }
    }
}

/************************************************
* 函数名      : void sea_putwifibyte ( u8 ch )
* 功能        : parse wifi receive message
* 输入参数    : u8 ch
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_putwifibyte ( u8 ch )
{
//    sea_printf("%c", ch);
    sea_putwifiresponse(ch);
    if (get_wifistatus(WIFIIDLE))
    {
        sea_parseserverpage(ch);
        if (resp1->count >= wifi_info.hdr->size)
            EM380C_ResetResponse();
    }
    else if (resp1->count >= WIFICMDSIZE && get_wifistatus(WIFICOMMAND))
    {
        if (resp1->count >= WIFICMDSIZE)
        { 
            wifi_info.hdr = (emsp_header *)resp1->buf;
            sea_parsewifihead(wifi_info.hdr);
        }
    }
    else   // if (get_wifistatus(WIFIRECVDATA | WIFIRECVRESP))
    {
        if (get_wifistatus(WIFIRECVDATA))
        {
            sea_parseserverpage(ch);
        }
        else if (get_wifistatus(WIFIRECVRESP))
        {
            sea_printf("%c", ch);
            sea_parseresponse(ch);
        }
        
        if (resp1->count >= wifi_info.hdr->size)
            EM380C_ResetResponse();
    }
}

/************************************************
* 函数名      : u8 wifi_ReportFrame ( u8 id, u8 size, u16 road_id, u8 * body )
* 功能        : 向wifi发送帧
* 输入参数    : u8 id, u8 size, u16 road_id, u8 * body
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 wifi_ReportFrame ( u8 id, u8 size, u16 road, u8 * body )
{
    u16 length;
    
    sea_printf("\nPOST message ");
    length = (size + EXTRASIZE)*2;             // (pointer[2] + FRAME_HEAD_CHECKSUM )*2;   /// ???? renyu 12/05/23

    HTTP_IpsendPostSize(length);
    
    FRAME_PostFrame(id, size, road, body);
    
    return 0x01;      
}

/************************************************
* 函数名      : EM380C_parm_TypeDef * EM380C_DefaultWifiParm ( void )
* 功能        : write defalut wifi moudel parameter to flash
* 输入参数    : 无
* 输出参数    : EM380C_parm_TypeDef *
* 修改记录    : 无
* 备注        : 无
*************************************************/  
EM380C_parm_TypeDef * EM380C_DefaultWifiParm ( void )
{
    sea_memset(&wifi_info.cfg, 0x00, sizeof( EM380C_parm_TypeDef));    
//    strcpy((char *)wifi_parm.local_ip_addr, "192.168.1.105");
//    strcpy((char*)wifi_parm.remote_ip_addr, "192.168.1.104");	// 210.32.24.139:6181	
//    strcpy((char *)wifi_parm.net_mask, "255.255.255.0");
//    strcpy((char *)wifi_parm.gateway_ip_addr, "192.168.1.1");
    sprintf((char *)wifi_info.cfg.local_ip_addr, "%d.%d.%d.%d", sys_info.local.ip[0], sys_info.local.ip[1], sys_info.local.ip[2], sys_info.local.ip[3]);
    sprintf((char *)wifi_info.cfg.remote_ip_addr, "%d.%d.%d.%d", sys_info.ip[0], sys_info.ip[1], sys_info.ip[2], sys_info.ip[3]);
    sprintf((char *)wifi_info.cfg.net_mask, "%d.%d.%d.%d", sys_info.local.mk[0], sys_info.local.mk[1], sys_info.local.mk[2], sys_info.local.mk[3]);
    sprintf((char *)wifi_info.cfg.gateway_ip_addr, "%d.%d.%d.%d", sys_info.local.gw[0], sys_info.local.gw[1], sys_info.local.gw[2], sys_info.local.gw[3]);
    wifi_info.cfg.wifi_mode      = STATION;     // AP
//    wifi_parm.portH          = (6181 >> 0x08) & 0xff;  // 1024 above
//    wifi_parm.portL          = 6181 & 0xff;
    wifi_info.cfg.portH          = (sys_info.port >> 0x08) & 0xff;  
    wifi_info.cfg.portL          = sys_info.port & 0xff;
    wifi_info.cfg.connect_mode   = TCP_Client;
    wifi_info.cfg.use_dhcp       = DHCP_Disable;
    wifi_info.cfg.use_udp        = TCP_mode;
    wifi_info.cfg.UART_buadrate  = BaudRate_115200;
    wifi_info.cfg.DMA_buffersize = buffer_256bytes;
    wifi_info.cfg.use_CTS_RTS    = HardwareFlowControl_None;
    wifi_info.cfg.parity         = Parity_No;			
    wifi_info.cfg.data_length    = WordLength_8b;	
    wifi_info.cfg.stop_bits      = StopBits_1;	
    wifi_info.cfg.IO_Control     = IO1_Normal;
    
    memcpy(wifi_info.cfg.wifi_ssid, sys_info.route.ssid, strlen((char *)sys_info.route.ssid));
    wifi_info.cfg.wifi_wepkeylen = sys_info.route.size;
    wifi_info.cfg.sec_mode       = sys_info.route.mode;    // Secure_WPA_WPA2_PSK;
    if (sys_info.route.mode == Secure_WPA_WPA2_PSK)
        memcpy((char *)wifi_info.cfg.wpa_psk, sys_info.route.key, strlen((char *)sys_info.route.key));
    else if (strlen((char *)sys_info.route.key))
        memcpy(wifi_info.cfg.wifi_wepkey, sys_info.route.key, strlen((char *)sys_info.route.key));   
    
    wifi_info.sock.portH        = (sys_info.port >> 0x08) & 0xff;  
    wifi_info.sock.portL        = sys_info.port & 0xff;
    wifi_info.sock.connect      = TCP_Client;
    wifi_info.sock.udp          = TCP_mode;
    sprintf((char *)wifi_info.sock.domain, "%d.%d.%d.%d", sys_info.ip[0], sys_info.ip[1], sys_info.ip[2], sys_info.ip[3]);
    
    return &wifi_info.cfg;
}

///////////////////////////////////////////////////////////////////////////////////////
