#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "network.h"

////////////////////////////////////////////////////////////////////////////////////
//
/************************************************
* 函数名      : static void printCommandHelp ( void )
* 功能        : print commands
* 输入参数    : None
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
static void printCommandHelp ( void )
{
/**************************     {eATE0, "ate0\r"},
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
    sea_printf("\nid: %d, cmd: %s", gprs_cmd[i].id, gprs_cmd[i].cmd);
  **************************************************************************/
}

void sea_gprsconfig  ( int argc, char * argv[] )
{
    int id;
    
    if (argc < 0x02)
    {
        printCommandHelp();
	return;
    }
    sscanf(argv[0x01], "%d", &id);
    GPRS_SendCommand(send1, (EGPRS)id);
}


////////////////////////////////////////////////////////////////////////////////////

