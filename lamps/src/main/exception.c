#include "network.h"
#include "sensor.h"
#include "sink.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//
extern int8u networkCount;  
extern NetworkInfo2 networkInfo2[MAXNETBUF];

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : coordinatorExceptionHandler
//* 功能        : 协调器对异常状况的处理
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void coordinatorExceptionHandler ( void )
{
    network_info.stackstate = emberNetworkState();
    
    switch (network_info.stackstate) 
    {
        case EMBER_NO_NETWORK:
            Debug( "\r\nsinkFormNetwork: form network.");
            sinkFormNetwork();
            break;
        case EMBER_JOINING_NETWORK:      // if in the middle of joining, do nothing
            break;
        case EMBER_JOINED_NETWORK:       // if already joined, turn allow joining on
            network_info.appstate = EMBER_JOINED_NETWORK;
            break;
        case EMBER_LEAVING_NETWORK:      // if leaving, do nothing
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : routerExceptionHandler
//* 功能        : 处理异常情况，比如“sensor目前不在网络中”
//*             : 在秒级别事件中调用(1 second)；
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :
//*------------------------------------------------*/
void routerExceptionHandler ( void )
{  
    static EmberStatus lastStatus = 100;
//    EmberStatus status;        
//    static u8  runstep = 0x00; 
//    static u8 joinTimes = 0x00;
//    static u16 counter = 0x00;
    
    scanModule();
    network_info.stackstate = emberNetworkState();

    if (lastStatus != network_info.stackstate)
    {
        switch (network_info.stackstate)
        {
            case EMBER_NO_NETWORK:
                DBG("\r\n[state]EMBER_NO_NETWORK");
                break;
            case EMBER_JOINING_NETWORK:
                DBG("\r\n[state]EMBER_JOINING_NETWORK");
                break;
            case EMBER_JOINED_NETWORK:
                DBG("\r\n[state]EMBER_JOINED_NETWORK");
                break;
            case EMBER_LEAVING_NETWORK:
                DBG("\r\n[state]EMBER_LEAVING_NETWORK");
                break;
            default:
                Debug("\r\n[state]unknow state");
                break;
        }
        lastStatus = network_info.stackstate;
    }

    switch (network_info.stackstate)
    {
        case EMBER_NO_NETWORK:
        {
            sensorJoinNetwork();
            break;
        }
        case EMBER_JOINED_NETWORK:
        {
            if (network_info.action == 0x01)
            {
                network_info.action = 0x00;
                emberLeaveNetwork();
            }
            break;
        }
    }      
    
    if (single_info.time.year <= 2000)
    {
        single_info.fail(SETFAILBIT, LAMP_ER_TM);
    }
    else  
    {
        if (single_info.fail(GETFAILBIT, LAMP_ER_TM))
            single_info.fail(CLRFAILBIT, LAMP_ER_TM);
    }

    if ((single_info.num() == 0x00 || single_info.num() > sys_info.ctrl.maxdev) && 
        !single_info.fail(GETFAILBIT, LAMP_ER_NP))
        single_info.fail(SETFAILBIT, LAMP_ER_NP);
    else if (single_info.num() > 0x00 && single_info.num() <= sys_info.ctrl.maxdev && 
        single_info.fail(GETFAILBIT, LAMP_ER_NP))
        single_info.fail(SETFAILBIT, LAMP_ER_NP);  
}

////////////////////////////////////////////////////////////////////////////////////

