#ifndef __GPRS_H__
#define __GPRS_H__

////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"
#include "FreeRTOS.h"
#include "Semphr.h"

#define ININ_COMM_NUM                  0x06

#define GPRS_TIMEOUT      0x200000    // get response data waiting time
#define GPRS_RESPSIZE     0x800
#define MAX_CMD_SIZE      0x100

#define GPRS_CONN_STATE_CLOSE          0x00   // �ر�
#define GPRS_CONN_STATE_OPEN           0x01
#define GPRS_CONN_STATE_READ           0x02
#define GPRS_CONN_STATE_WAIT           0x03   // ��
#define GPRS_CONN_STATE_CONTROL        0x04   // ͨ����
#define GPRS_CONN_STATE_SEND           0x05   // ͨ����
#define GPRS_CONN_STATE_RECV           0x06   // ͨ����
#define GPRS_CONN_STATE_WRONG          0x07   // ����

#define GPRS_IDLE                      0x00
#define GPRS_SENDCMD                   0x01
#define GPRS_SENDPOST                  0x02
#define GPRS_SENDGET                   0x03
#define GPRS_SENDMSG                   0x04

#define EXTRASIZE                      0x06

//#define LEE_GPRS

#define RESPONSE_OF_IPSEND    (0x15)   // 0D 0A 25 49 50 53 45 4E 44 3A 31 2C 31 35 20 0D 0A 4F 4B 0D 0A
#define GPRSCMDSIZE           (0x30)
#define MAXRETRY              (0x02)   // (0x30)

////////////////////////////////////////////////////////////////////////////////////////////////
// in function sea_parseresponse() of gprs_info.staus 
// const char * cmd_resp[12] = {"OK", "ready", "CONNECT", "IPSEND", "REC", "IPDR", "> ","ERROR", "FAIL", "RING", "CMTI", "CPMS"};
// const u8 * cmd_resp[CMDRESPSIZE] = {"ready", "CONNECT", "IPSEND", "IPDR", "TCP", "OK", "REC", "> ", "CMTI", "ERROR"};
#define GPRSREADY             (0x0001)
#define GPRSCONNECT           (0x0002)
#define GPRSIPSEND            (0x0004)
#define GPRSIPDR              (0x0008)
#define GPRSTCP               (0x0010)
#define GPRSOK                (0x0020)
#define GPRSREC               (0x0040)
#define GPRSGREAT             (0x0080)
#define GPRSERROR             (0x0100)
#define GPRSCMTI              (0x0200)

////////////////////////////////////////////////////////////////////////////////////////////////
// idle state 
// const char * idle_resp[] = { "IPDR", "IPCLOSE", "RING", "CMTI", "CPMS", "OK", "TCP", "IPDATA" };
#define IDLEIPDR              (0x0001)
#define IDLEIPCLOSE           (0x0002)
#define IDLERING              (0x0004)
#define IDLECMTI              (0x0008)
#define IDLECPMS              (0x0010)
#define IDLEOK                (0x0020)
#define IDLETCP               (0x0040)
#define IDLEIPDATA            (0x0080)

////////////////////////////////////////////////////////////////////////////////////////////////
// the result of map command send  
#define ATE0BIT               (0x00000001)
#define SLEEPBIT              (0x00000002)
#define SIMBIT                (0x00000004)
#define IOMODEBIT             (0x00000008)
#define CGDCONTBIT            (0x00000010)
#define ETCPIPBIT             (0x00000020)
#define IPOPENBIT             (0x00000040)
#define IPDQBIT               (0x00000080)
#define IPDRBIT               (0x00000100)
#define IPCLOSEBIT            (0x00000200)
#define CNMIBIT               (0x00000400)
#define CMGRBIT               (0x00000800)
#define IPSENDBIT             (0x00001000)
#define CPMSBIT               (0x00002000)
#define CMGFBIT               (0x00004000)
#define CMGSBIT               (0x00008000)
#define CMGDBIT               (0x00010000)
#define POSTBIT               (0x00020000)
#define GETBIT                (0x00040000)
#define DELBIT                (0x00080000)
#define TCPQBIT               (0x00100000)
#define ATHBIT                (0x00200000)

////////////////////////////////////////////////////////////////////////////////////////////////
//
#define csCmdBegin            "<Cmd>\0"
#define csCmdEnd              "</Cmd>\0"
#define csOk                  "OK\0"
#define csError               "ERROR\0"
#define ccPageMark            '"'
#define csZoneCode            "86\0"

#define CMDRESPSIZE           (0x09)
#define IDLERESPSIZE          (0x08)

////////////////////////////////////////////////////////////////////////////////////////////////
//
#define set_gprsmap(type)     (gprs_info.map |= (type))
#define clr_gprsmap(type)     (gprs_info.map &= ~(type))
#define get_gprsmap(type)     (gprs_info.map & (type))

#define set_gprstatus(type)   (gprs_info.status |= (type))
#define clr_gprstatus(type)   (gprs_info.status &= ~(type))
#define get_gprstatus(type)   (gprs_info.status & (type))

////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define GPRS_RESET_PIN        GPIO_Pin_9
#define GPRS_TERMON_PIN       GPIO_Pin_8
#define GPRS_STATE_PIN        GPIO_Pin_5
#define GPRS_PORT             GPIOB

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef enum
{  eATE0 = 0x01, eSLEEP, eSIM, eIOMODE, eIPDDMODE, eCGDCONT, eETCPIP, eIPOPEN, eIPDQ, eIPDR, eIPCLOSE, eCNMI, eCMGR, 
   eIPSEND, eCPMS, eCMGF, eCMGS, eCMGD, ePOST, eSYSCHTIME, eGET, eDEL, eTCPQ, eATH, eIPCLOSE1, eIPCLOSE5, eBAUD, eCGACT, 
   eCGDISACT, eUNKNOW
} EGPRS;

typedef struct
{
    EGPRS id;
    u8 *  cmd;
} gprs_cmd_t, *pgprs_cmd_t;

typedef struct
{
    u8    in, out, cnt;         // list of commands
    u8    list[GPRSCMDSIZE];    // gprs command list
} cmd_t, *pcmd_t;

typedef struct
{
    u16   count;               // count response data count
    u8    buf[GPRS_RESPSIZE];  // gprs response buffer
} resp_t, *presp_t;

typedef struct 
{
    u16    status;              // status of gprs
    u8     curid;               // current send id
    u8     retry;               // send command retry times
    u8     sim;                 // count of sim card
    u8     ring;                // count of ring 
    u8     cmd[MAX_CMD_SIZE];   // cmd buffer
    cmd_t  send;
    cmd_t  fail;
    resp_t resp;
    u16    reset;
    u16    termon;
    u16    state;
    GPIO_TypeDef * port;
} gprs_info_t, *pgprs_info_t;  

typedef struct 
{
    u8  state;
    u8  pos;
    u8  str[0x10];
} tag_t, *ptag_t;

////////////////////////////////////////////////////////////////////////////////////////////////
// 
/************************************************
* ������      : void GPRS_Configuration ( void )
* ����        : gprsģ��ĳ�ʼ������ ,���в���
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Configuration ( void );

/************************************************
* ������      : void GPRS_SoftwareReset ( void )
* ����        : gprsģ��ĳ�ʼ������ ,���в���
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SoftwareReset ( void );

/************************************************
* ������      : u8 GPRS_State ( void )
* ����        : get gprs state
* �������    : None
* �������    : gprs state
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_State ( void );

/************************************************
* ������      : void GPRS_Reset ( void )
* ����        : reset gprs modle em310
* �������    : None
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Reset ( void );

/*******************************************************************************
* Function Name  : static void GPRS_PowerOnOff ( void )
* Description    : power on gprs modle
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPRS_PowerOnOff ( void );

/************************************************
* ������      : void GPRS_Init(void)
* ����        : init the gprs module
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Init(void);

/************************************************
* ������      : void GPRS_ConnectServer ( void )
* ����        : Connect Server
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ConnectServer ( void );

/************************************************
* ������      : void GPRS_ClearCommand ( pcmd_t ptr )
* ����        : clear command list
* �������    : AT_Command
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ClearCommand ( pcmd_t ptr );

/************************************************
* ������      : u8 GPRS_IsCommandEmpty ( pcmd_t ptr )
* ����        : commands list is empty
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IsCommandEmpty ( pcmd_t ptr );

/************************************************
* ������      : u8 GPRS_IsSameCommand ( pcmd_t ptr, EGPRS cmd )
* ����        : the same commands in list
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IsSameCommand ( pcmd_t ptr, EGPRS cmd );

/************************************************
* ������      : EGPRS GPRS_GetSendCommand ( pcmd_t ptr )
* ����        : commands list is empty
* �������    : None
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
EGPRS GPRS_GetSendCommand ( pcmd_t ptr );

/************************************************
* ������      : void GPRS_SendCommand ( pcmd_t ptr, EGPRS id )
* ����        : send command��������ʧ�ܣ��ظ������ĴΣ�
* �������    : AT_Command
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SendCommand ( pcmd_t ptr, EGPRS id );

/************************************************
* ������      : void GPRS_SendCommandFinish ( pcmd_t ptr )
* ����        : send command finished 
* �������    : None
* �������    : None
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_SendCommandFinish ( pcmd_t ptr );

/************************************************
* ������      : void GPRS_ReadPage ( void )
* ����        : read page content
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ReadPage ( void );

/************************************************
* ������      : void GPRS_ResetResponse ( void )
* ����        : reset response buffer
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ResetResponse ( void );

/*********************************************************
* ������      : void GPRS_SyschTimeServer ( void )
* ����        : ���ʱ��
* �������    : ʱ��
* �������    : 0x01:ʱ�䵽  0x00:ʱ��δ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void GPRS_SyschTimeServer ( void );

/************************************************
* ������      : void GPRS_Printf ( const char * cmd, ... )
* ����        : send command to uart
* �������    : const char * cmd, ...
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_Printf ( const char * cmd, ... );

/************************************************
* ������      : u8 GPRS_IpsendString ( u8 * pointer )
* ����        : ʹ��ipsend��������
* �������    : u8 * pointer
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendString ( u8 * pointer );

/************************************************
* ������      : u8 GPRS_Ipsend( u8 * pointer )
* ����        : ����һ������
* �������    : �ַ���
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend ( u8 * pointer );

/************************************************
* ������      : u8 u8 GPRS_Ipsend2Byte ( u8 ch )
* ����        : ʹ��ipsend����2�ֽ�����
* �������    : �ֽ�
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend2Byte ( u8 ch );

/************************************************
* ������      : void GPRS_ReadData(void)
* ����        : ����ipdr����congEM310�ж�������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void GPRS_ReadData(void);

/************************************************
* ������      : u8 GPRS_Ipsend( u8 * pointer )
* ����        : ����һ������
* �������    : �ַ���
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend ( u8 * pointer );

/************************************************
* ������      : void sea_putgprsbyte ( u8 ch )
* ����        : parse gprs receive message
* �������    : u8 ch
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_putgprsbyte ( u8 ch );

/************************************************
* ������      : pgprs_cmd_t sea_getgprscmd ( u8 id )
* ����        : get gprs command structure body
* �������    : u8 id
* �������    : pointer of command body
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
pgprs_cmd_t sea_getgprscmd ( u8 id );

/************************************************
* ������      : u8 sea_parseresponse ( u8 * resp )
* ����        : parse gprs receive message
* �������    : u8 * resp
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/

void sea_parseserverpage ( u8 ch );
char sea_parsewebcommand ( char ch, const char * mark,  ptag_t tag );

////////////////////////////////////////////////////////////////////////////////////////////
//
extern gprs_info_t gprs_info;
extern presp_t     resp1;
extern pcmd_t      send1; 
extern pcmd_t      fail1; 

////////////////////////////////////////////////////////////////////////////////////////////
//
#endif
