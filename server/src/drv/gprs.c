#include <stdio.h>
#include <stdarg.h>
#include "FreeRTOSConfig.h"
#include "gprs.h"
#include "serial.h"
#include "network.h"
#include "http.h"
#include "frame.h"

const u8 * cmd_resp[CMDRESPSIZE]   = {"ready", "CONNECT", "IPDATA", "IPDR", "TCP", "OK", "REC", "> ", "ERROR"};
const u8 * idle_resp[IDLERESPSIZE] = { "IPDR", "IPCLOSE", "RING", "CMTI", "CPMS", "OK", "TCP", "IPDATA" };
const gprs_cmd_t gprs_cmd[] = { {eATE0, "ate0\r"},
                                {eSLEEP, "at%sleep=0\r"},
                                {eSIM, "at%tsim\r"},
                                {eIOMODE, "at%iomode=1,1,0\r"},
                                {eIPDDMODE, "at%ipddmode=0\r"},
                                {eCGDCONT, "at+cgdcont=1,\"IP\",\"CMNET\"\r"},
                                {eETCPIP, "at%ETCPIP=,\r"},
                                {eIPOPEN, "at%%ipopen=\"TCP\",\"%d.%d.%d.%d\",%d\r"},
                                {eIPDQ, "at%ipdq\r"},
                                {eIPDR, "at%ipdr\r"},
                                {eIPCLOSE, "at%ipclose\r"},
                                {eCNMI, "at+cnmi=2,1\r"},
                                {eCMGR, "at+CMGR=1\r"},                         // read message
                                {eIPSEND, "at%ipsend=\""},
                                {eCPMS, "at+CPMS=\"ME\",\"ME\",\"ME\"\r"},
                                {eCMGF, "at+CMGF=1\r"},
                                {eCMGS, "at+cmgs=\"%s\"\r%s"},                  // %s telephone, %s msg, %c(0x1a ctrl+z) send message end
                                {eCMGD, "at+cmgd=1,4\r"},
                                {ePOST, "POST /Util/PostCmd.aspx HTTP/1.1\r\nHost:%s\r\nContent-Length: %d\r\n\r\n\0"},
                                {eSYSCHTIME,"POST /Util/PostCmd.aspx HTTP/1.1\r\nHost:%s\r\nContent-Length: \0"},
                                {eGET, "GET /Util/GetCmd.aspx?roadId=%d HTTP/1.1\r\nHost:%s\r\n\r\n\0"},
                                {eDEL, "at%ipdd=0,2\r"},
                                {eTCPQ, "at%ipopen?\r"},
                                {eATH, "ath\r"},
                                {eIPCLOSE1, "at%ipclose=1\r"},                  // disconnect server
                                {eIPCLOSE5, "at%ipclose=5\r"},                  // logout network
                                {eBAUD, "at+ipr=%d\r"},                         // set baud rate
                                {eCGACT, "at+cgact=1,1\r"},
                                {eCGDISACT, "at+cgact=0,1\r"},
                                {eUNKNOW, NULL} };

///////////////////////////////////////////////////////////////////////////////////////////////////
//
gprs_info_t  gprs_info;
presp_t      resp1 = &gprs_info.resp; 
pcmd_t       send1 = &gprs_info.send; 
pcmd_t       fail1 = &gprs_info.fail; 
tag_t        web_lable, ch_conver, message_sym;

extern       frame_info_t gprsfrm1;

/************************************************
* ������      : static void GPRS_Delay ( void )
* ����        : ��ʱ
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void GPRS_Delay ( u16 times )
{
    vu16 i;
  
    while (times--)
        for (i = 0x00; i < 0x03ff; i ++) ;
}

/************************************************
* ������      : u8 GPRS_State ( void )
* ����        : get gprs state
* �������    : None
* �������    : gprs state
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_State ( void )
{
    return GPIO_ReadInputDataBit(gprs_info.port, gprs_info.state);   
}

/************************************************
* ������      : void GPRS_Reset ( void )
* ����        : reset gprs modle em310
* �������    : None
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Reset ( void )
{
    GPIO_WriteBit(gprs_info.port, gprs_info.reset, Bit_SET);                  // reset em310
    GPRS_Delay(0x500);
    GPIO_WriteBit(gprs_info.port, gprs_info.reset, Bit_RESET);
}

/************************************************
* ������      : u8 GPRS_IsCommandEmpty ( pcmd_t ptr )
* ����        : commands list is empty
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IsCommandEmpty ( pcmd_t ptr )
{
    return ptr->cnt == 0x00;
}

/************************************************
* ������      : EGPRS GPRS_GetSendCommand ( pcmd_t ptr )
* ����        : commands list is empty
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
EGPRS GPRS_GetSendCommand ( pcmd_t ptr )
{
    return (EGPRS)ptr->list[ptr->out];
}

/************************************************
* ������      : u8 GPRS_IsSameCommand ( pcmd_t ptr, EGPRS cmd )
* ����        : the same commands in list
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IsSameCommand ( pcmd_t ptr, EGPRS cmd )
{
    u8  pos, i, cnt;
    
    for (pos = ptr->out, cnt = i = 0x00; i < ptr->cnt; i ++, pos ++)
    {
        if (pos >= GPRSCMDSIZE)
            pos = 0x00;
        if (ptr->list[pos] == cmd)
            cnt ++;
    }
    return cnt >= 0x02 ? (0x01) : (0x00);
}

/************************************************
* ������      : void GPRS_ClearCommand ( pcmd_t ptr )
* ����        : clear command list
* �������    : AT_Command
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ClearCommand ( pcmd_t ptr )
{
    memset(ptr, 0x00, sizeof(cmd_t));  
}

/************************************************
* ������      : void GPRS_Printf ( const char * cmd, ... )
* ����        : send command to uart
* �������    : const char * cmd, ...
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Printf ( const char * cmd, ... )
{
    va_list v_list;
    
    memset(gprs_info.cmd, 0x00, PRINTFBUF);
    va_start(v_list, format);      
    vsprintf((char *)gprs_info.cmd, cmd, v_list); 
    va_end(v_list);
    
    UartSendString(gprsfrm1.uart, gprs_info.cmd);
}

/************************************************
* ������      : void GPRS_SendCommand ( pcmd_t ptr, EGPRS id )
* ����        : send command��������ʧ�ܣ��ظ������ĴΣ�
* �������    : AT_Command
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SendCommand ( pcmd_t ptr, EGPRS id )
{
    if (ptr->cnt <= GPRSCMDSIZE)
    {
        ptr->cnt ++;
        ptr->list[ptr->in ++] = id;
        if (ptr->in >= GPRSCMDSIZE)
            ptr->in = 0x00;
    }
}

/************************************************
* ������      : void GPRS_SendCommandFinish ( pcmd_t ptr )
* ����        : send command finished 
* �������    : None
* �������    : None
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SendCommandFinish ( pcmd_t ptr )
{
    if (ptr->cnt)
    {
        ptr->cnt --;
        ptr->out ++;
        if (ptr->out >= GPRSCMDSIZE)
            ptr->out = 0x00;
    }
    
    memset(&web_lable, 0x00, sizeof(tag_t));
    memset(&ch_conver, 0x00, sizeof(tag_t));
    memset(&message_sym, 0x00, sizeof(tag_t));
    gprs_info.curid  = eUNKNOW;
    gprs_info.retry  = 0x00;
    
    xSemaphoreGive(dyn_info.semgprs);
}

/************************************************
* ������      : void GPRS_ResetResponse ( void )
* ����        : reset response buffer
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ResetResponse ( void )
{
    memset(resp1, 0x00, sizeof(resp_t));
    memset(&web_lable, 0x00, sizeof(tag_t));
    memset(&ch_conver, 0x00, sizeof(tag_t));
}

/************************************************
* ������      : void GPRS_ReadPage ( void )
* ����        : read page content
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ReadPage ( void )
{   
    GPRS_SendCommand(send1, eCGDCONT);
    GPRS_SendCommand(send1, eETCPIP);        
    GPRS_SendCommand(send1, eIPOPEN);
    GPRS_SendCommand(send1, eDEL);    // ipddmode=0, auto delete packets
    GPRS_SendCommand(send1, eGET);
    GPRS_SendCommand(send1, eIPDR);    
}

/*******************************************************************************
* Function Name  : static void GPRS_PowerOnOff ( void )
* Description    : power on gprs modle
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPRS_PowerOnOff ( void )
{
    GPIO_WriteBit(gprs_info.port, gprs_info.termon, Bit_SET);                 // term on
    GPRS_Delay(0x300);
    GPIO_WriteBit(gprs_info.port, gprs_info.termon, Bit_RESET);
}  

/************************************************
* ������      : void sea_sendipopen ( void )
* ����        : set ip open command
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_sendipopen ( void )
{
    sprintf((char *)gprs_info.cmd, (char const *)gprs_cmd[eIPOPEN].cmd, sys_info.ip[0], sys_info.ip[1], sys_info.ip[2], sys_info.ip[3], sys_info.port);
    sea_printf("\n%s", gprs_info.cmd);
    UartSendString(GPRS_COM, gprs_info.cmd);
}

/************************************************
* ������      : u16 sea_parsecmdresponse ( const u8 * resp, const u8 * cmd_resp[], u8 size )
* ����        : parse gprs command receive message
* �������    : const u8 * resp, const u8 * cmd_resp[], u8 size
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u16 sea_parsecmdresponse ( const u8 * resp, const u8 * cmd_resp[], u8 size )
{
    u16 state = 0x00;
    u8 i;
    
    for (i = 0x00; i < size; i ++)
    {
        if (sea_memstr(resp, (const void *)cmd_resp[i], sea_strlen(resp)))
            state |= (0x01 << i);
    }
    return state;
}

/************************************************
* ������      : static void sea_putresponse ( u8 ch )
* ����        : put char to response buffer
* �������    : u8 ch
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void sea_putresponse ( u8 ch )
{
//    sea_printf("%c", ch);
    resp1->buf[resp1->count ++] = ch;
    if (resp1->count >= GPRS_RESPSIZE)
        memset(resp1, 0x00, sizeof(resp_t));
    
    if (sea_memstr(resp1->buf, "RING", strlen((char *)resp1->buf)))
    {             
        sea_printf("\nThere is a ring.");        
        memset(resp1, 0x00, sizeof(resp_t));
        GPRS_ClearCommand(send1);        
        GPRS_SendCommand(send1, eATH);         
        GPRS_ReadPage();
    }
    else if (sea_memstr(resp1->buf, "CMTI", strlen((char *)resp1->buf)))
    {
        memset(resp1, 0x00, sizeof(resp_t));
        GPRS_ClearCommand(send1);
        GPRS_SendCommand(send1, eCMGR);
        GPRS_SendCommand(send1, eCMGD);
        GPRS_SendCommand(send1, eCMGD);
    }
}

/************************************************
* ������      : char sea_parsewebcommand ( char ch, const char * mark, ptag_t tag )
* ����        : parse web commands received
* �������    : char ch, const char * mark, ptag_t tag
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
char sea_parsewebcommand ( char ch, const char * mark,  ptag_t tag )
{
    u8  len = strlen(mark);
    
    if (tag->pos)
    {
        tag->str[tag->pos ++] = ch;
        if (tag->pos >= len)
        {
            tag->str[tag->pos] = 0x00;
            if (memcmp((char *)tag->str, mark, len) == 0x00)
                return 0x01;
            tag->pos = 0x00;
        }
    }
    else if (ch == mark[0x00])
    {
        tag->str[0x00] = ch;
        tag->pos ++;
    }
    return 0x00;
}

/************************************************
* ������      : static void sea_parseserverpage ( u8 ch )
* ����        : parse gprs idle state receive message
* �������    : u8 ch
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_parseserverpage ( u8 ch )
{
    int  tmp;
    
    if (ch_conver.state)
    {
        if (ch >= 0x30)
        {
            ch_conver.str[ch_conver.pos ++] = ch;
            ch_conver.pos &= 0x01;
        }
        if (!ch_conver.pos)
        {
            sscanf((char *)ch_conver.str, "%2x", &tmp);
            tmp &= 0xff;
#ifdef GPRS_DEBUG                
            sea_printf("%c", tmp);     // hex to char
#endif
            if (web_lable.state)
            {
                if (sea_parsewebcommand(tmp, csCmdEnd, &web_lable))  // </Cmd> end tag of command. || tmp == 'C' || tmp == 'm' || tmp == 'd')
                {
                    web_lable.state = 0x00;
#ifdef GPRS_DEBUG                
                    sea_printf(" end.");
#endif
                }
                else 
                {
#ifdef GPRS_DEBUG                
                    sea_printf("%02x", tmp);
#endif
                    gprsfrm1.recv(&gprsfrm1, tmp);   // sea_putgprsframebyte(tmp);   // command protocol frame 
                }
            }
            else if (sea_parsewebcommand(tmp, csCmdBegin, &web_lable))
            {
                web_lable.state  = 0x01;
#ifdef GPRS_DEBUG                
                sea_printf("\nfound command begin : ");
#endif
            }
        }
    }
        
    if (ch == ccPageMark)    /// web page begin and end, '"')
    {
#ifdef GPRS_DEBUG                
        ch_conver.state ? sea_printf("\npage end.") : sea_printf("\npage begin : ");
#endif
        ch_conver.state = (ch_conver.state + 0x01) & 0x01;
        if (!ch_conver.state)
        {
            clr_gprstatus(GPRSIPDR);
            sea_printf("\nThere is a packet.");
        }
    }
}

/************************************************
* ������      : static void sea_parsetelmessage ( u8 ch )
* ����        : parse gprs idle state receive message
* �������    : u8 ch
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
static void sea_parsetelmessage ( u8 ch )
{
    static tag_t phone;
    int  tmp;
    
    sea_printf("%c", ch);
    if (message_sym.state == 0x00)
    {
        if (sea_parsewebcommand(ch, csZoneCode, &message_sym))
        {
            memset(&message_sym, 0x00, sizeof(tag_t));
            message_sym.state = 0x01;
            memset(&phone, 0x00, sizeof(tag_t));
        }
    }
    else if (message_sym.state == 0x01)
    {
        if (ch > 0x39 || ch < 0x30 || phone.pos >= 0x10)
        {
//#ifdef GPRS_DEBUG                
            sea_printf("\nphone number %s", phone.str);
//#endif
            message_sym.state = 0x02;
        }
        else
            phone.str[phone.pos ++] = ch;
    }
    else
    {
        if (message_sym.state == 0x03)
        {
            if ( ch == '<' || ch == '/') 
            {
                memset(&message_sym, 0x00, sizeof(tag_t));
                clr_gprstatus(GPRSREC);
                sea_printf("\nThere is a message.");
            }
            if (ch >= 0x30)
            {
#ifdef GPRS_DEBUG                
                sea_printf("%c", ch);
#endif
                ch_conver.str[ch_conver.pos ++] = ch;
                ch_conver.pos &= 0x01;
            }
            if (!ch_conver.pos)
            {
                sscanf((char *)ch_conver.str, "%2x", &tmp);
                gprsfrm1.recv(&gprsfrm1, (u8)tmp & 0xff);  // sea_putgprsframebyte(tmp); 
            }   
        }
        else if (sea_parsewebcommand(ch, csCmdBegin, &message_sym))
        {
            memset(&message_sym, 0x00, sizeof(tag_t));
            message_sym.state = 0x03;
        }
    }
}

/************************************************
* ������      : void sea_putgprsbyte ( u8 ch )
* ����        : parse gprs receive message
* �������    : u8 ch
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_putgprsbyte ( u8 ch )
{
    u16 len;
    
    sea_putresponse(ch);
    if (get_gprstatus(GPRSREC | GPRSIPDR))
    {
        get_gprstatus(GPRSIPDR) ? (sea_parseserverpage(ch)) : (sea_parsetelmessage(ch));
        return;
    }
    sea_printf("%c", ch);
    
    len = sea_strlen(resp1->buf);
    if (sea_memstr(resp1->buf, "IPDR", len))
    {
        memset(resp1, 0x00, sizeof(resp_t));
        clr_gprstatus(GPRSREC | GPRSIPDR);
        set_gprstatus(GPRSIPDR);
    }
    else if (sea_memstr(resp1->buf, "REC", len))
    {
        memset(resp1, 0x00, sizeof(resp_t));
        clr_gprstatus(GPRSREC | GPRSIPDR);
        set_gprstatus(GPRSREC);
    }
    else if (sea_memstr(resp1->buf, "IPCLOSE", len))
    {
        if (get_gprstatus(GPRSCONNECT))
        {
            sea_printf("\ntcp link will close.");
            clr_gprstatus(GPRSCONNECT);
        }
        memset(resp1, 0x00, sizeof(resp_t));
    }
    else if (sea_memstr(resp1->buf, "OK", len) || sea_memstr(resp1->buf, "ready", len))
    {  // || (sea_memstr(resp1->buf, "IPSEND",len)&&gprs_info.curid==ePOST)
        sea_printf("\nreceive OK or ready!");
        if (gprs_info.curid == eSIM)
        {
            if ( sea_memstr(resp1->buf, "SIM", sea_strlen(resp1->buf)))
            {
                gprs_info.sim = resp1->buf[gprs_info.sim + 0x03] - '0';
#ifdef GPRS_DEBUG                
                sea_printf("\nexist SIM count %d", gprs_info.sim);
#endif
                if (gprs_info.sim)
                {
//                    GPRS_SendCommand(send1, eETCPIP);
//                    GPRS_SendCommand(send1, eIPOPEN);
                }
                else
                {
                    sea_printf("\nERROR: no simcard, em310 reset!");
                    GPRS_Configuration();
                }
            }
        }
        memset(resp1, 0x00, sizeof(resp_t));
        if (get_gprstatus(GPRSREC | GPRSIPDR))
            clr_gprstatus(GPRSREC | GPRSIPDR);
        GPRS_SendCommandFinish(send1);
    }
    else if (sea_memstr(resp1->buf, "CONNECT", len))
    {    
        if (!get_gprstatus(GPRSCONNECT))
            set_gprstatus(GPRSCONNECT);
        GPRS_SendCommandFinish(send1);
        memset(resp1, 0x00, sizeof(resp_t));
    }
    else if (sea_memstr(resp1->buf, "ERROR", len))
    {    
        if (gprs_info.retry ++ > MAXRETRY)
        {
            GPRS_SendCommand(fail1, (EGPRS)gprs_info.curid);
            GPRS_SendCommandFinish(send1);
        }
        else
        {
            if (gprs_info.curid == eDEL || gprs_info.curid == eIPOPEN)
                GPRS_SendCommandFinish(send1);
            else
                gprs_info.curid = eUNKNOW;       // send the command again !!
        }
        memset(resp1, 0x00, sizeof(resp_t));
        xSemaphoreGive(dyn_info.semgprs);
    }
}

/************************************************
* ������      : pgprs_cmd_t sea_getgprscmd ( u8 id )
* ����        : get gprs command structure body
* �������    : u8 id
* �������    : pointer of command body
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
pgprs_cmd_t sea_getgprscmd ( u8 id )
{
    if (id < eUNKNOW)
        return (pgprs_cmd_t)&gprs_cmd[id];
    return NULL;
}

/************************************************
* ������      : void GPRS_ConnectServer ( void )
* ����        : Connect Server
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ConnectServer ( void )
{
    if (!get_gprstatus(GPRSCONNECT))
    {
        GPRS_SendCommand(send1, eCGDCONT);
        GPRS_SendCommand(send1, eETCPIP);        
        GPRS_SendCommand(send1, eIPOPEN);
    }
}

/*********************************************************
* ������      : void GPRS_SyschTimeServer ( void )
* ����        : ���ʱ��
* �������    : ʱ��
* �������    : 0x01:ʱ�䵽  0x00:ʱ��δ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void GPRS_SyschTimeServer ( void )
{
    GPRS_ConnectServer();
    GPRS_SendCommand(send1, eDEL);  
    GPRS_SendCommand(send1, eSYSCHTIME);
    GPRS_SendCommand(send1, eIPDR);
}

/************************************************
* ������      : void GPRS_SoftwareReset ( void )
* ����        : gprsģ��ĳ�ʼ������ ,���в���
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SoftwareReset ( void )
{
    UartSendString(GPRS_COM, "at%rst\r");
}

/************************************************
* ������      : u8 u8 GPRS_Ipsend2Byte ( u8 ch )
* ����        : ʹ��ipsend����2�ֽ�����
* �������    : �ֽ�
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend2Byte ( u8 ch )
{
#ifdef GPRS_MODE
    char temp[0x03] = { 0x00, 0x00, 0x00 };
    
    sprintf(temp, "%02x", ch); 
    UartSendByte(GPRS_COM, temp[0x00]);
    UartSendByte(GPRS_COM, temp[0x01]);
#elif defined (WIFI_MODE)
    UartSendByte(GPRS_COM, ch);
#endif
    return 0x01;
}

/************************************************
* ������      : u8 GPRS_IpsendString ( u8 * pointer )
* ����        : ʹ��ipsend��������
* �������    : u8 * pointer
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendString ( u8 * pointer )
{
    while (*pointer)
        GPRS_Ipsend2Byte(*pointer ++);
    return 0x01;
}

/************************************************
* ������      : u8 GPRS_Ipsend( u8 * pointer )
* ����        : ����һ������
* �������    : �ַ���
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend ( u8 * pointer )
{
    sea_printf("\nSend data!\n");

    reset_serialinfo(GPRS_COM);
    UartSendString(GPRS_COM, "at%ipsend=\"");
    GPRS_IpsendString(pointer);
    UartSendString(GPRS_COM,"\"\r");
    
    return 0x01;
}

////////////////////////////////////////////////////////////////////////////////////////
