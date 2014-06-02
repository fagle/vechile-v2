// *******************************************************************
//  sink.c
// *******************************************************************
#include "network.h"
#include "sink.h"
#include "security.h" 
#include "console.h"
#include "frame.h"
 
#ifdef SINK_APP
// *******************************************************************
//const and globles ��������ȫ�ֱ���
// Define the concentrator type for use when sending "many-to-one"
// route requests.
//
// In EmberZNet 2.x and earlier a node sending this type of request
// was referred to as an "aggregator". In the ZigBee specification and
// EmberZNet 3 it is referred to as a "concentrator".
// 
// EMBER_HIGH_RAM_CONCENTRATOR is used when the caller has enough
// memory to store source routes for the whole network.
// EMBER_LOW_RAM_CONCENTRATOR is used when the concentrator has
// insufficient RAM to store all outbound source routes.
//u16 concentratorType = EMBER_LOW_RAM_CONCENTRATOR;

#ifdef ENABLE_VEHICLE 
static u8      lamp_rept[((MAXCARDS >> 0x03) + 0x01)];         // for card state report bit maps
#else
static u8      lamp_rept[((MAXLAMPS >> 0x03) + 0x01)];        // for state report bit maps
#endif
static u16     lamp_addr[MAXLAMPS];                           // logic address list of lamps
road_t         road_info;                                     // sum information of each road 

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ezspCoordinatorMessageSentHandler
//* ����        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void ezspCoordinatorMessageSentHandler ( EmberOutgoingMessageType type,
                                         u16 indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         EmberMessageBuffer message,
                                         EmberStatus status )
{
    if (apsFrame->clusterId == send1.curmsg->id)   // sequence sometime error, use cluster id to ensure
    {
        Debug("\r\n[coord]send status = %02x, id = %02x", status, apsFrame->clusterId);
        if (status != EMBER_SUCCESS)               // send the command fail.
        {
            if (++ single_info.retry <= 0x02)
                sea_sendmsg(&send1, MULTICAST, send1.curmsg->dest, send1.curmsg->id, send1.curmsg->size, send1.curmsg->body);   // send it again
        }
        else
            single_info.retry = 0x00;
    }
}

#ifdef USE_BOOTLOADER_LIB
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bootloadUtilQueryResponseHandler
//* ����        : When a device sends out a bootloader query, the bootloader
//                query response messages are parsed by the bootloader-util
//                libray and and handled to this function. This application
//                simply prints out the EUI of the device sending the query
//                response.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void bootloadUtilQueryResponseHandler ( boolean bootloaderActive,
                                        u16 manufacturerId,
                                        u8 *hardwareTag,
                                        EmberEUI64 targetEui,
                                        u8 bootloaderCapabilities,
                                        u8 platform,
                                        u8 micro,
                                        u8 phy,
                                        u16 blVersion )
{
    Debug("RX [BL QUERY RESP] eui: ");
    printEUI64((EmberEUI64*)targetEui);
    Debug(" running %p\r\n", bootloaderActive ? "bootloader" : "stack");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : bootloadUtilLaunchRequestHandler
//* ����        : This function is called by the bootloader-util library 
//                to ask the application if it is ok to start the bootloader.
//                This happens when the device is meant to be the target of
//                a bootload. The application could compare the manufacturerId 
//                and/or hardwareTag arguments to known values to enure that 
//                the correct image will be bootloaded to this device.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
boolean bootloadUtilLaunchRequestHandler ( u16 manufacturerId,
                                           u8 *hardwareTag,
                                           EmberEUI64 sourceEui ) 
{
    // TODO: Compare arguments to known values.
    // TODO: Check for minimum required radio signal strength (RSSI).
    // TODO: Do not agree to launch the bootloader if any of the above conditions
    // are not met.  For now, always agree to launch the bootloader.
    return TRUE;
}
#endif // USE_BOOTLOADER_LIB

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printlampstate ( void )
//* ����        : print state of all lamps
//* �������    : �� 
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printlampstate ( void )
{
#ifndef ENABLE_GATEWAY    
    DBG("\r\nopen state of lamps:");
    for (u8 i = 0x00; i < road_info.map.size; i ++)
        Debug(" %02x", road_info.map.state[i]);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 sea_reporthandler ( plamp_t ptr, u16 sender)
//* ����        : lamp report handler
//* �������    : plamp_t ptr, u16 sender
//* �������    : true/flase
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8  sea_reporthandler ( plamp_t ptr, u16 sender )
{
    u16 number;
    
    if (ptr == NULL)      return 0x00;
    
    if (sys_info.ctrl.app == eVehicle)
        number = ptr->vehicle.number;
    else
        number = ptr->light.number;
    
    if (number > sys_info.ctrl.maxdev || number == 0x00)
        return 0x00;
    
    number --;
    road_info.error(ptr, sender);
    
#ifndef ENABLE_VEHICLE     
    status = ptr->light.status ? set_lampbitmap(road_info.map.state, number) : clr_lampbitmap(road_info.map.state, number);
#endif
    road_info.update(number, sender);
    
    return 0x01;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static void update_gatewaytable ( u16 address )
//* ����        : update gateway address table
//* �������    : u16 address
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void update_gatewaytable ( u16 address )             //����
{
#ifdef ENABLE_GATEWAY
    for (u16 i = 0x00; i < sys_info.maxdev; i ++)
    {
        if (road_info.map.addr[i] == address)
        {
            u8 j;
            for (j = 0x00; j < MAXGWSIZE; j ++)
            {
                if (network_info.gwaddr[j] == address)
                    return;
                if (!network_info.gwaddr[j])
                    break;
            }
            if (j < MAXGWSIZE)
                network_info.gwaddr[j] = address;
            return;
        }
    }
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : ezspCoordinatorStackStatusHandler(EmberStatus status)
//* ����        : this is called when the stack status changes
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void emberCoordinatorStackStatusHandler ( EmberStatus status )
{
    switch (status) 
    {
        case EMBER_NETWORK_UP:
            Debug("\r\nEVENT: stackStatus now EMBER_NETWORK_UP\r\n");
            {
                EmberStatus status;
                EmberNetworkParameters networkParams;
                status = emberGetNetworkParameters(&networkParams);
                if (status == EMBER_SUCCESS) 
                {
                    switch (network_info.method) 
                    {
                        case SINK_USE_NETWORK_INIT:
                            Debug("FORM : network started using network init\r\n");
                            break;
                        case SINK_FORM_NEW_NETWORK:
                            Debug("FORM : new network formed\r\n");
                            break;
                        case SINK_USE_SCAN_UTILS:
                            Debug("FORM : new network formed by scanning\r\n");
                            break;
                    }
                    Debug("\r\n     : channel 0x%02x, panid 0x%04x", networkParams.radioChannel, networkParams.panId);
                    Debug("\r\n     : expended id %s", networkParams.extendedPanId);
#ifdef ENABLE_GATEWAY    
                    sea_gwmsendframe(ICHP_GW_CHANNEL, sizeof(channel_t), (u8 *)&sys_info.channel);
#endif            
                }
            }
            sea_addmultigroup();  // Add the multicast group to the multicast table, this is done after the stack comes up
            break;
    
        case EMBER_NETWORK_DOWN:
            Debug("\r\nEVENT: stackStatus now EMBER_NETWORK_DOWN\r\n");
            break;

        case EMBER_JOIN_FAILED:
            Debug("\r\nEVENT: stackStatus now EMBER_JOIN_FAILED");
            break;

        default:
            Debug( "\r\nEVENT: stackStatus now 0x%02x\r\n", status);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : sinkFormNetwork
//* ����        : Э��������
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sinkFormNetwork ( void )
{
    // structure to store necessary network parameters of the node, (which are panId, enableRelay, radioTxPower, and radioChannel)
    EmberStatus status;

    // try and rejoin the network this node was previously a part of. if status is not EMBER_SUCCESS then the node didn't 
    // rejoin it's old network sink nodes need to be coordinators, so ensure we are a coordinator
    sensorCommonSetupSecurity();                               // Set the security keys and the security state - specific to this application, 
                                                               // all variants of this application (sink, sensor, sleepy-sensor, mobile-sensor) 
                                                               // need to use the same security setup. This function is in app/sensor/common.c. 
                                                               // This function should only be called when a network is formed as the act of setting 
                                                               // the key sets the frame counters to 0. On reset and networkInit this should not be called.
#ifdef USE_HARDCODED_NETWORK_SETTINGS                          // Bring the network up.++�̻��������
    Debug("\r\nFORM : attempting to form network");         // tell the user what is going on
    Debug("\r\n     : using channel 0x%2x, panid 0x%4x, ", network_info.params.radioChannel, network_info.params.panId);
    Debug("\r\n     : expended id %s", network_info.params.extendedPanId);
    
    network_info.method = SINK_FORM_NEW_NETWORK;               // set the mode we are using
    status = emberFormNetwork(&network_info.params);           // attempt to form the network
    if (status != EMBER_SUCCESS) 
    {
        Debug("\r\nERROR: from emberFormNetwork: 0x%x", status);
        assert(FALSE);
    }
#else                                                           // ++ɨ��ȷ���������
    network_info.method = SINK_USE_SCAN_UTILS;                  // set the mode we are using
    Debug("\r\nFORM : scanning for an available channel and panid\r\n");  // tell the user what is going on
    emberScanForUnusedPanId(EMBER_ALL_802_15_4_CHANNELS_MASK, 0x05);         // Use a function from app/util/common/form-and-join.c that scans 
                                                                             // and selects a quiet channel to form on. Once a PAN id and channel
                                                                             // are chosen, calls emberUnusedPanIdFoundHandler.
#endif  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static void sea_lamperror ( plamp_t ptr, u16 sender )
//* ����        : lamp error handler
//* �������    : plamp_t ptr, u16 sender
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void sea_lamperror ( plamp_t ptr, u16 sender )
{
    u8 fail;
    
    if (sys_info.ctrl.app == eVehicle)
    {
        fail = ptr->vehicle.fail;
        if (fail & WRONG_WAY)
        {
        }
        if (fail & MISS_CARD)
        {
        }
        if (fail & END_LINE)
        {
//            uartfrm.put(&uartfrm, ICHP_SV_END, sender, 0x01, &ptr->vehicle.card);
        }
        if (fail & MEET_OBSTACLE)
        {
        }
        if (fail & EMPTY_TABLE)
        {
        }
    }
    else
    {
        fail = ptr->light.fail;
        if (fail & LAMP_ER_OV)
        {
        }
        if (fail & LAMP_ER_OC)
        {
        }
        if (fail & LAMP_ER_PT)
        {
        }
        if (fail & LAMP_ER_SC)
        {
        }
        if (fail & LAMP_ER_CB)
        {
        }
    }
    if (fail & LAMP_ER_TM)
    {
        sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_DATE, sizeof(systime_t), &single_info.time);
    }
    if (fail & LAMP_ER_NP)
    {
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static u16 sea_getaddress ( u16 number )
//* ����        : get address
//* �������    : u16 number
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static u16 sea_getaddress ( u16 number )
{
    if (number == 0x00 || number > sys_info.ctrl.maxdev)
        return 0x00;
    
    return road_info.map.addr[number - 0x01];
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static u16 sea_getnumber ( u16 address )
//* ����        : get number
//* �������    : u16 address
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static u16 sea_getnumber ( u16 address )
{
    if (address == 0x00)
        return 0x00;
    
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
        if (road_info.map.addr[i] == address)
            return i + 0x01;
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static void sea_updatelogic ( u16 number, u16 sender )
//* ����        : update road state and address table
//* �������    : u16 number, u16 sender
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void sea_updatelogic ( u16 number, u16 sender )
{
    if (road_info.map.addr[number] != sender)
        road_info.map.addr[number] = sender;                 //it is a new node according to this net

    update_gatewaytable(sender);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sinkInit ( void )
//* ����        : ��ʼ��sink�ڵ���Ϣ
//*             : 
//*             : ������޽ڵ����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : Ӧ�ò㣺ԭ����Lamp_Configuration
//*------------------------------------------------*/
void sinkInit ( void ) 
{
    sea_memset(&road_info, 0x00, sizeof(road_t));
    sea_memset(lamp_addr, 0x00, sizeof(u16) * MAXLAMPS);
    
    road_info.map.max   = sys_info.ctrl.maxdev;
    road_info.map.addr  = lamp_addr;
#ifdef ENABLE_VEHICLE 
    road_info.map.size  = (MAXCARDS >> 0x03) + 0x01;
#else
    road_info.map.size  = ((sys_info.ctrl.maxdev / 0x08) + 0x01);
#endif    
    sea_memset(lamp_rept, 0x00, road_info.map.size); 
    road_info.map.state = lamp_rept;
    
    road_info.state     = OPEN_LAMPS;
    road_info.assign    = sea_lampnumberassign;                      // get lamp number by search and assign; 
    road_info.update    = sea_updatelogic;
    road_info.number    = sea_getnumber;
    road_info.address   = sea_getaddress;
    road_info.error     = sea_lamperror;
    
    sea_configprint();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sinkCommandHandler ( void )
//* ����        : ��������Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sinkCommandHandler ( void ) 
{   
    u8 ch;
    frame_t   fr;
    
    if (!serial_info.ept(&serial_info))
    {
        ch = serial_info.get(&serial_info);
        uartfrm.recv(&uartfrm, ch);
        if (uartfrm.state == PRT_IDLE)
            sea_roadkeyboardinput(ch);     
    }

    if (uartfrm.get(&uartfrm, &fr) != NULL)
        sea_parseroadframe(&fr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u16 sea_lampassign ( void )
//* ����        : assign lamp number by search empty number 
//* �������    : ��
//* �������    : number of lamp
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u16 sea_lampnumberassign ( void )
{
#ifndef ENABLE_GATEWAY    
    u16 num;
  
    for (num = 0x00; num < sys_info.ctrl.maxdev; num ++)
    {
        if (road_info.map.addr[num] == 0x00)
            return num + 0x01;
    }
    return 0xffff;
#else
    return DEFALUTLAMPNUM;
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif //SINK_APP

/////////////////////////////////////////////////////////////////////////////////////
