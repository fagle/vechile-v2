// *******************************************************************
//  sensor.c
// *******************************************************************
#include "network.h"
#include "sensor.h"
#include "common.h"
#include "flash.h"
#include "form-and-join-adapter.h"

#ifdef SENSOR_APP

//////////////////////////////////////////////////////////////////////////////////////////
//
extern u8 networkCount;  
extern NetworkInfo2 networkInfo2[MAXNETBUF];

static u8 joinInitial = 0x01;
u8 scanAction = 0x00;      // 0 no action   1 start scan

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sensorRejoinNetwork ( void )
//* 功能        : sensor attemp to rejoin network
//* 输入参数    : 无
//* 输出参数    : rejoin status
//* 修改记录    : 无
//* 备注        : ++ray:重复
//*------------------------------------------------*/
EmberStatus sensorRejoinNetwork ( void )
{
    EmberStatus status;
    
    sensorCommonSetupSecurity();
    network_info.params.joinMethod  = EMBER_USE_NWK_REJOIN;      // added by renyu
    status = emberJoinNetwork(network_info.type, &network_info.params);
    Debug("\r\n[sensor]rejoin network status,%02x", status);
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspRouterMessageSentHandler
//* 功能        : router发送数据后，自动调用；
//              : 可统计发送失败数量；
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 ++ray:重复
//* 备注        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//*------------------------------------------------*/
void ezspRouterMessageSentHandler ( EmberOutgoingMessageType type,
                                    u16 indexOrDestination,
                                    EmberApsFrame *apsFrame,
                                    EmberMessageBuffer message,
                                    EmberStatus status )
{ 
    if (apsFrame->clusterId == send1.curmsg->id)   // if the message failed, update fail counter and print a warning
    {            
        if (status != EMBER_SUCCESS) 
        {   
            DBG("\r\nROUTER send status = %02x, id = %02x, seq%d", status, apsFrame->clusterId, apsFrame->profileId);
            //sea_sendmsg(&send1, UNICAST, send1.curmsg->dest, send1.curmsg->id, send1.curmsg->size, send1.curmsg->body);   // send it again
            clr_lampmode(LAMP_NETWORK);
            if (++ single_info.retry > NWPK_TOLERANCE) 
            {
                DBG("\r\nERROR: %d too many data msg failures, looking for new sink", single_info.retry);
                single_info.retry   = 0x00;
                network_info.action = 0x01;
                network_info.appstate = EMBER_NO_NETWORK;
            }
        }
        else 
        {
            DBG("\r\nROUTER send ok, seq%d", apsFrame->profileId);
            single_info.retry   = 0x00;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberStackStatusHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无 
//* 备注        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberRouterStackStatusHandler ( EmberStatus status )
{
    EmberNetworkParameters networkParams;

    switch (status) 
    {
        case EMBER_NETWORK_UP:
            DBG("\r\nEVENT: stackStatus now EMBER_NETWORK_UP");
#ifdef ENABLE_GATEWAY
            sea_gwmsendframe(ICHP_GW_NETWORK, 0x01, &network_info.stackstate);
#endif
            
            
    
            if (emberGetNetworkParameters(&networkParams) == EMBER_SUCCESS) //ensure that printf the right network parameters
            {
                DBG("\r\nSENSOR APP: network joined");
                DBG("\r\n          : channel %d, panid 0x%04x", networkParams.radioChannel, networkParams.panId);
                DBG("\r\n          : expended id %s", networkParams.extendedPanId);
            }
            
            if(network_info.params.radioChannel == 26)  // for interpan
            {
                DBG("\r\nform interpan network");         
            }
            else  // for network
            { 
                if (network_info.params.panId != networkParams.panId || network_info.params.radioChannel != networkParams.radioChannel)
                {
                    DBG("\r\n[sensor]attempt to join, %d, 0x%04x",network_info.params.radioChannel,network_info.params.panId);
                    DBG("\r\n[sensor]but join, %d, 0x%04x",networkParams.radioChannel,networkParams.panId);
                    network_info.action = 0x01;
                }
                else
                {
                     if (sys_info.channel.panid != network_info.params.panId ||  
                        sys_info.channel.channel != network_info.params.radioChannel)    
                    {
                        DBG("\r\n[sensor] update new network parameters to flash memory.");
                        sys_info.channel.panid   = network_info.params.panId;  
                        sys_info.channel.channel = network_info.params.radioChannel;
                        sea_updatesysconfig();
                    }
                    joinInitial = 1;
                    
                    network_info.appstate = EMBER_JOINED_NETWORK;
                    network_info.intpstate = 0x00;
                    
                    sea_sendmsg(&send1, UNICAST, 0x00, ICHP_SV_RPT, sizeof(lamp_t), &single_info.lamp); 
                }
            }
            sea_addmultigroup();    // Add the multicast group to the multicast table - this is done after the stack comes up
            break;

        case EMBER_NETWORK_DOWN:
            network_info.stackstate = EMBER_NO_NETWORK;
            network_info.appstate = EMBER_NO_NETWORK;
            DBG("\r\nEVENT: stackStatus now EMBER_NETWORK_DOWN");
#ifdef RANGE_TEST 
            single_info.on();
#endif
            break;
        case EMBER_JOIN_FAILED:
            DBG("\r\nEVENT: stackStatus now EMBER_JOIN_FAILED!");
            //sensorRejoinNetwork();
#ifdef RANGE_TEST
            single_info.on();
#endif
            break;
        case EMBER_NO_BEACONS:
            DBG("\r\nEVENT: stackStatus now EMBER_NO_BEACONS");
            if (++ single_info.retry > NWPK_TOLERANCE)
                single_info.retry = 0x00;
            break;
        default:
            Debug( "\r\nEVENT: stackStatus now 0x%02x", status);
            break;
    }
}

#ifdef USE_BOOTLOADER_LIB
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilQueryResponseHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :// When a device sends out a bootloader query, the bootloader
//                  query response messages are parsed by the bootloader-util
//                  libray and and handled to this function. This application
//                  simply prints out the EUI of the device sending the query
//                  response.
//*------------------------------------------------*/
void bootloadUtilQueryResponseHandler(boolean bootloaderActive,
                                      u16 manufacturerId,
                                      u8 *hardwareTag,
                                      EmberEUI64 targetEui,
                                      u8 bootloaderCapabilities,
                                      u8 platform,
                                      u8 micro,
                                      u8 phy,
                                      u16 blVersion)
{
    Debug("\r\nRX [BL QUERY RESP] eui: ");
    printEUI64((EmberEUI64*)targetEui);
    Debug("\r\n running %p",  bootloaderActive ? "bootloader" : "stack");
}
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilLaunchRequestHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : This function is called by the bootloader-util library
//                to ask the application if it is ok to start the bootloader.
//                This happens when the device is meant to be the target of
//                a bootload. The application could compare the manufacturerId
//                and/or hardwareTag arguments to known values to enure that
//                the correct image will be bootloaded to this device.
//*------------------------------------------------*/
boolean bootloadUtilLaunchRequestHandler(u16 manufacturerId,
                                         u8 *hardwareTag,
                                         EmberEUI64 sourceEui) 
{
    // TODO: Compare arguments to known values. TODO: Check for minimum required radio signal strength (RSSI).
    // TODO: Do not agree to launch the bootloader if any of the above conditions are not met.  For now, always agree to launch the bootloader.
    return TRUE;
}
#endif // USE_BOOTLOADER_LIB

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sensorJoinNetwork ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sensorJoinNetwork ( void )
{
    static u8 joinTimes = 0;
//    static u8 scanTimes = 0;
    int8s currentIndex = -1;
    u8 paramReady = 0;
    EmberStatus status;
    u8 i;
    
    Debug("\r\n ~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
    if (joinInitial == 1)
    {
        joinInitial = 0;
        single_info.avail   = 0x05;
//        scanTimes = 3;
        single_info.scan = 0x01;
        joinTimes = 0;
        networkInfo2[0].index = 100;
        sea_memset(&network_info.params, 0x00, sizeof(EmberNetworkParameters));            //update rejoin parameters
        sea_memcpy(network_info.params.extendedPanId, DEF_EXPAN, EXTENDED_PAN_ID_SIZE);
        network_info.params.panId        = sys_info.channel.panid;                  
        network_info.params.radioTxPower = sys_info.channel.power;
        network_info.params.radioChannel = sys_info.channel.channel;
        network_info.params.joinMethod   = EMBER_USE_MAC_ASSOCIATION;
    }
    
#ifdef USE_HARDCODED_NETWORK_SETTINGS
    network_info.params.joinMethod   = EMBER_USE_MAC_ASSOCIATION;      // EMBER_USE_NWK_REJOIN;           // EMBER_USE_MAC_ASSOCIATION;, modified by renyu 2011/09/25
    Debug("\r\nSENSOR APP: joining network");              // tell the user what is going on
    Debug("\r\n          : channel 0x%02x, panid 0x%04x", network_info.params.radioChannel, network_info.params.panId);
    Debug("\r\n          : expended id %s", network_info.params.extendedPanId);
    sensorCommonSetupSecurity();
    status = emberJoinNetwork(network_info.type, &network_info.params);   // attempt to join the network
    if (status != EMBER_SUCCESS) 
        Debug("\r\nerror returned from emberJoinNetwork: 0x%x", status);
    else 
        Debug( "\r\nwaiting for stack up ...");
#else 

    // Use a function from app/util/common/form-and-join.c that scans and selects a beacon that has:
    // 1) allow join=TRUE, 2) matches the stack profile that the app is using, 3) matches the extended PAN ID passed 
    // in unless "0" is passed Once a beacon match is found, emberJoinableNetworkFoundHandler  is called.
    //emberScanForJoinableNetwork(EMBER_ALL_802_15_4_CHANNELS_MASK,(u8*) extendedPanId);
    
    if (single_info.avail > 0)
    {
        if(sys_info.channel.channel >= 11 && sys_info.channel.channel <= 26)
        {
            single_info.avail --;
            Debug("\r\n[join]default param,times = %d", single_info.avail);
            network_info.params.joinMethod   = EMBER_USE_MAC_ASSOCIATION;
            paramReady = 1;                   //if parameter ready
        }
        else
        {
            single_info.avail = 0x00;
        }
    }
    else
    {
        if(single_info.scan == 0x01)        //Initial
        {
            single_info.scan = 0x00;
            scanAction = 1;
        }
        
        if(isScanComplete())
        {
            Debug("\r\n[join]scan complete, networkCount = %d",networkCount);
            for(i=0;i<networkCount;i++)
            {
                Debug("\r\n[param]%d,%4x,%d",networkInfo2[i].channel,networkInfo2[i].panid,networkInfo2[i].index);
            }
            if(networkCount == 0)
            {
                scanAction = 1;
                //DBG("\n[scan end]null");
                return 0x01;
            }
            
            Debug("\r\n[join]joinTimes = %d",joinTimes);
            if (joinTimes > 0)
            {
                joinTimes--;
                network_info.params.joinMethod   = EMBER_USE_MAC_ASSOCIATION;
                paramReady = 1;
            }
            else
            {  
                currentIndex = 0;
                for(i = 0;i < networkCount;i++)
                {
                    if(networkInfo2[i].index < networkInfo2[currentIndex].index)
                      currentIndex = i;
                }
                if(networkInfo2[currentIndex].index != 100)
                {
                    Debug("[join]currentIndex = %d",currentIndex);
                    networkInfo2[currentIndex].index = 100;
                    joinTimes = 3;
                    
                    sea_memset(&network_info.params, 0x00, sizeof(EmberNetworkParameters));            //update rejoin parameters
                    sea_memcpy(network_info.params.extendedPanId, DEF_EXPAN, EXTENDED_PAN_ID_SIZE);
                    network_info.params.panId        = networkInfo2[currentIndex].panid;
                    network_info.params.radioTxPower = sys_info.channel.power;
                    network_info.params.radioChannel = networkInfo2[currentIndex].channel;
                    network_info.params.joinMethod   = EMBER_USE_MAC_ASSOCIATION;
                    paramReady = 1;
                }
                else
                {
                    Debug("\r\n[join]join all");
                    scanAction = 1;

                    return 0x00;
            
                }
            }
        }
    }
        
    if(paramReady == 1)
    {
        paramReady = 0;
        sensorCommonSetupSecurity();
        Debug("\r\n[sensor]attempt join, %d, 0x%4x", network_info.params.radioChannel, network_info.params.panId);
        status = emberJoinNetwork(EMBER_ROUTER, &network_info.params);
        Debug("\r\n[sensor]join network, status = %02x", status);
    }
    return 0x01;
#endif // USE_HARDCODED_NETWORK_SETTINGS
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void scanModule ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void scanModule ( void )
{
    if (scanAction == 0x01)
    {
        scanSet();
        scanAction = 0;
    }
    scaningStatus();
    if (isScanRunning())  
    {            
        emberScanForJoinableNetwork(EMBER_ALL_802_15_4_CHANNELS_MASK, DEF_EXPAN);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
static u16 sea_getnumber ( void )
{    
    return sys_info.dev.num;
}

static u8 sea_getype ( void )
{
    if (sys_info.ctrl.app == eVehicle)
        return sys_info.dev.type;
    return LAMPIDST;
}

static u8 sea_getfail ( u8 op, u8 fail )
{
    if (op == SETFAILBIT)
    {
        if (sys_info.ctrl.app == eVehicle)
        {
            single_info.lamp.vehicle.fail |= fail;
            return single_info.lamp.vehicle.fail;
        }
        else
        {
            single_info.lamp.light.fail |= fail;
            return single_info.lamp.light.fail;
        }
    }
    else if (op == CLRFAILBIT)
    {
        if (sys_info.ctrl.app == eVehicle)
        {
            single_info.lamp.vehicle.fail &= ~fail;
            return single_info.lamp.vehicle.fail;
        }
        else
        {
            single_info.lamp.light.fail &= ~fail;
            return single_info.lamp.light.fail;
        }
    }

    if (sys_info.ctrl.app == eVehicle)
        return single_info.lamp.vehicle.fail & fail;
    return single_info.lamp.light.fail & fail;
}

static u8 sea_getstatus ( void )
{
    if (sys_info.ctrl.app == eVehicle)
        return single_info.lamp.vehicle.status;
    return single_info.lamp.light.status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sensorInit
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : init common state for sensor nodes
//*------------------------------------------------*/
void sensorInit ( void ) 
{
    if (sys_info.ctrl.app == eVehicle)
    {
#ifdef ENABLE_VEHICLE
        vehicleInit();
#endif    
    }
    else
    {
        single_info.ctrl &= ~LAMP_CHANGE;
        Debug("\r\n single_info.ctrl = %4x",single_info.ctrl);
        single_info.lamp.light.number = sys_info.dev.num;
        if (sys_info.ctrl.config & LAMP_CF_OPEN)
        {
            single_info.lamp.light.percent = MAXPERCENT;
            single_info.on(&single_info.lamp.light.percent);
        }
        else
        {
            single_info.lamp.light.percent = 0x00;
            single_info.off(&single_info.lamp.light.percent);
        }
    }
    single_info.num    = sea_getnumber;
    single_info.type   = sea_getype;
    single_info.fail   = sea_getfail;
    single_info.status = sea_getstatus;
    
    mobileInit();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sensorCommandHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void sensorCommandHandler ( void )
{
    frame_t   fr;
    
    if (!serial_info.ept(&serial_info))
    {
        u8 ch  = serial_info.get(&serial_info);

        carInfo.ms = 0x00;                 // rs485 receiving data busy, not send msg.
        uartfrm.recv(&uartfrm, ch);
        if (uartfrm.state == PRT_IDLE)
            sea_lampkeyboardinput(ch);
    }

    if (uartfrm.get(&uartfrm, &fr) != NULL)
        sea_parselampframe(&fr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif