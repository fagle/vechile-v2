#include "http.h"
#include "gprs.h"
#include "serial.h"
#include "network.h"
#include "frame.h"

//const u8 * GET  = "GET /Util/GetCmd.aspx?roadId=34 HTTP/1.1\r\nHost:ecs.hdu.edu.cn\r\n\r\n\0";
//const u8 * POST = "POST /Util/PostCmd.aspx HTTP/1.1\r\nHost:ecs.hdu.edu.cn\r\nContent-Length: \0";
//const char * frmGET   = "GET /Util/GetCmd.aspx?roadId=%d HTTP/1.1\r\nHost:%s\r\n\r\n\0";
const char * frmGET   = "GET /Util/GetCmd.aspx?roadId=%d HTTP/1.1\r\nHost:%s\r\n\r\n\0";
const char * frmPOST  = "POST /Util/PostCmd.aspx HTTP/1.1\r\nHost:%s\r\nContent-Length: %d\r\n\r\n\0";
const char * frmMSG   = "at+cmgs=\"%s\"\r\0";     // "at+cmgs=\"%s\"\r\0";
const char * frmOPEN  = "[MSG] road %d, open time %02d:%02d%:%02d.\0";
const char * frmCLOSE = "[MSG] road %d, close time %02d:%02d%:%02d.\0";

/************************************************
* 函数名      : static void sea_sendmsgcontent ( const u8 * msg )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * msg
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void sea_sendmsgcontent ( const u8 * msg )
{
    u8 i;
    for ( i = 0x00; i < 0x08; i ++)    // why ??? 
        UartSendByte(GPRS_COM, 0x20);   
    while (*msg)
    {
        UartSendByte(GPRS_COM, *msg);
        msg ++;
    }
}

/************************************************
* 函数名      : void sea_sendopenmessage ( const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendopenmessage ( const u8 * phone )
{
    if (!phone)    return;
    
    sprintf((char *)gprs_info.cmd, frmMSG, phone);
    UartSendString(GPRS_COM, gprs_info.cmd);   
    sprintf((char *)gprs_info.cmd, frmOPEN, sys_info.ctrl.road, systime.hour, systime.min, systime.sec);
    sea_sendmsgcontent(gprs_info.cmd);
    UartSendByte(GPRS_COM, 0x1a);    
}

/************************************************
* 函数名      : void sea_sendclosemessage ( const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendclosemessage ( const u8 * phone )
{
    if (!phone)    return;
    
    sprintf((char *)gprs_info.cmd, frmMSG, phone);
    UartSendString(GPRS_COM, gprs_info.cmd);   
    sprintf((char *)gprs_info.cmd, frmCLOSE, sys_info.ctrl.road, systime.hour, systime.min, systime.sec);
    sea_sendmsgcontent(gprs_info.cmd);
    UartSendByte(GPRS_COM, 0x1a);    
}

/************************************************
* 函数名      : void sea_sendgprsmessage ( const u8 * msg, const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * msg, const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendgprsmessage ( const u8 * msg, const u8 * phone )
{
    if (!phone || !msg)    return;
    
    sprintf((char *)gprs_info.cmd, frmMSG, phone);
    UartSendString(GPRS_COM, gprs_info.cmd);   
    sea_sendmsgcontent(msg);
    UartSendByte(GPRS_COM, 0x1a);    
}

/************************************************
* 函数名      : u8 HTTP_IpsendPost ( void )
* 功能        : send post to web
* 输入参数    : 无
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
void HTTP_IpsendPost ( void )
{
    if (!sea_isreportempty())
    {
        u16 Content_Length;
    
        Content_Length = ((sizeof(lamp_t) + EXTRASIZE) * sea_reportnum()) * 0x02;
        UartSendString(GPRS_COM, "at%ipsend=\"");
        sprintf((char *)gprs_info.cmd, frmPOST, sys_info.domain, Content_Length);
        GPRS_IpsendString(gprs_info.cmd);
        
        while (sea_reportnum())
        {
            prep_t ptr = sea_getreport();
            FRAME_PostFrame( ICHP_SV_RPT_KEY , sizeof(lamp_t) , sys_info.ctrl.road , ptr->body);
        }
        UartSendString(GPRS_COM,"\"\r");
    }
}

/************************************************
* 函数名      : u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road_id, u8 * body )
* 功能        : 向gprs发送帧
* 输入参数    : u8 id, u8 size, u16 road_id, u8 * body
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road, u8 * body )
{
    u16 length;
    
    sea_printf("\nPOST message ");
    length = (size + EXTRASIZE) * 0x02;             // (pointer[2] + FRAME_HEAD_CHECKSUM )*2;   /// ???? renyu 12/05/23
    UartSendString(GPRS_COM, "at%ipsend=\"");
    HTTP_IpsendPostSize(length);
    
    FRAME_PostFrame(id, size, road, body);
    UartSendString(GPRS_COM, "\"\r");    
    
    return 0x01;      
}

/************************************************
* 函数名      : u8 HTTP_IpsendPost ( u16 len )
* 功能        : send post to web
* 输入参数    : u16 len
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_IpsendPostSize ( u16 len )
{
    sprintf((char *)gprs_info.cmd, frmPOST, sys_info.domain, len);
    sea_printf("length: %d",len);
    sea_printf("POST 命令：%s",gprs_info.cmd);
    return GPRS_IpsendString(gprs_info.cmd);
}

/************************************************
* 函数名      : u8 HTTP_IpsendGet ( void )
* 功能        : send get to web
* 输入参数    : 无
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_IpsendGet ( void )
{
    sprintf((char *)gprs_info.cmd, frmGET, sys_info.ctrl.road, sys_info.domain);
#ifdef GPRS_MODE
    UartSendString(GPRS_COM, "at%ipsend=\"");
#endif
    GPRS_IpsendString(gprs_info.cmd);
#ifdef GPRS_MODE
    UartSendString(GPRS_COM, "\"\r");   
#endif
    sea_printf("\nSend HTTP cmd GET: %s\n",gprs_info.cmd);
    return 0x01;
}

/////////////////////////////////////////////////////////////////////////////////////
