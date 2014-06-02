#include "network.h"

extern void sea_printbuffer(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberIncomingMessageHandler
//* ����        : callback
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberIncomingMessageHandler ( EmberIncomingMessageType type, EmberApsFrame *apsFrame, EmberMessageBuffer message )
{
    if (apsFrame->clusterId == ICHP_LARGE_DATA)
    {   
//        u8   length = emberMessageBufferLength(message);
        u8 * ptr    = emberGetLinkedBuffersPointer(message, 0x00);
        u8   status = emberFragmentIncomingMessage(apsFrame, message);
        
        if (status)
            Debug("\r\n emberFragmentIncomingMessage() return true!");
        else
            Debug("\r\n call emberFragmentIncomingMessage() return false!");
    }
    else 
        network_info.incoming(type, apsFrame, message);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberMessageSentHandler
//* ����        : �����ͳɹ�ʱ���Զ�����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//*------------------------------------------------*/
void emberMessageSentHandler ( EmberOutgoingMessageType type,
                               int16u indexOrDestination,
                               EmberApsFrame *apsFrame,
                               EmberMessageBuffer message,
                               EmberStatus status )
{
    if (apsFrame->clusterId == ICHP_LARGE_DATA)
    {
        if (emberFragmentMessageSent(apsFrame, message, status))
            Debug("\r\nemberFragmentMessageSent() succeed!");
        else  
            Debug("\r\nemberFragmentMessageSent() failed!");
    }
    else 
        network_info.msgSent(type, indexOrDestination, apsFrame, message, status);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberFragmentMessageSentHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberFragmentMessageSentHandler ( EmberStatus status )
{
    Debug("\r\ncall emberFragmentMessageSentHandler()! status:0x%x",status);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void emberScanErrorHandler
//* ����        : callback
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void emberScanErrorHandler ( EmberStatus status )
{
#ifdef SINK_APP
    Debug("\r\n[EVENT]:(sink)could not find an available channel and panid - status: 0x%x", status);
#else
    Debug("\r\n[EVENT]:(sensor)could not find channel and panid - status: 0x%x", status);
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberStackStatusHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : �� ++ray:�ظ�
//* ��ע        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberStackStatusHandler ( EmberStatus status )
{
    network_info.stackStatus(status); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberUnusedPanIdFoundHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void emberUnusedPanIdFoundHandler ( EmberPanId panId, u8 channel )
{
#ifdef SINK_APP
    EmberNetworkParameters parameters;
    
    MEMCOPY(parameters.extendedPanId, sys_info.channel.expanid, EXTENDED_PAN_ID_SIZE);
    parameters.panId        = panId;
    parameters.radioTxPower = APP_POWER;
    parameters.radioChannel = channel;
    emberFormNetwork(&parameters);
#else
  //do nothing
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : emberJoinableNetworkFoundHandler
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        :
//*------------------------------------------------*/
void emberJoinableNetworkFoundHandler ( EmberZigbeeNetwork *networkFound, u8 lqi, int8s rssi )
{
#ifndef SINK_APP
    EmberNetworkParameters parameters;
    
    Debug("\r\n[call]: found joinable network, ");
    sea_memset(&parameters, 0x00, sizeof(EmberNetworkParameters));
    sea_memcpy(parameters.extendedPanId, networkFound->extendedPanId, EXTENDED_PAN_ID_SIZE);
    parameters.panId        = networkFound->panId;
    parameters.radioTxPower = APP_POWER;
    parameters.radioChannel = networkFound->channel;
    //parameters.joinMethod   = EMBER_USE_MAC_ASSOCIATION;
    parameters.joinMethod   = EMBER_USE_NWK_REJOIN;
    
    sea_memset(&network_info.params, 0x00, sizeof(EmberNetworkParameters));            //update rejoin parameters
    sea_memcpy(network_info.params.extendedPanId, networkFound->extendedPanId, EXTENDED_PAN_ID_SIZE);
    network_info.params.panId        = networkFound->panId;
    network_info.params.radioTxPower = APP_POWER;
    network_info.params.radioChannel = networkFound->channel;
    
    emberJoinNetwork(EMBER_ROUTER, &parameters);
#endif
}

/**@brief A callback invoked by the EmberZNet stack when a 
 * MAC passthrough message is received.  These are messages that
 * applications may wish to receive but that do 
 * not follow the normal ZigBee format.
 *
 * Examples of MAC passthrough messages are Embernet
 * messages and first generation (v1) standalone bootloader messages,
 * and SE (Smart Energy) InterPAN messages.
 * These messages are handed directly to the application and are not
 * otherwise handled by the EmberZNet stack.
 * Other uses of this API are not tested or supported at this time.
 * If the application includes this callback, it must define
 * EMBER_APPLICATION_HAS_MAC_PASSTHROUGH_HANDLER in its CONFIGURATION_HEADER
 * and set emberMacPassthroughFlags to indicate the kind(s) of messages
 * that the application wishes to receive.
 *
 * @param messageType    The type of MAC passthrough message received.
 * @param message        The MAC passthrough message that was received.
 */
void emberMacPassthroughMessageHandler ( EmberMacPassthroughType messageType, EmberMessageBuffer message )
{
#ifdef MOBILE_APP
    u8 inBuffer[73];//������ݰ�����
    InterPanHeader headerData;
    u8 appPayloadOffset;
    u8 i;
    u8 length;
    u8 appPayloadLength;
    MobileCmdFrame receive_frame;

    Debug("\r\n[label]receive message from mac");
    if(messageType == EMBER_MAC_PASSTHROUGH_SE_INTERPAN)
    {
        Debug("RX a SE_InterPan Message\n");
    }
    else
    {
        Debug("RX a InterPan Message\n");
    }

    //����interPan����,������ͷ ��appPayloadOffsetΪ�ز�����ƫ�Ƶ�ַ
    appPayloadOffset = parseInterPanMessage(message,      // the raw message
                                            0x00,         // no offset 
                                            &headerData); // header info
  
    //#ifdef INTER_PAN_DEBUG
    //�ж���ʲô���͵����ݰ����������鲥or�㲥
    if (appPayloadOffset != 0x00) 
    {
        Debug("RX inter PAN message(");
        if (headerData.messageType == INTER_PAN_UNICAST) 
        {
            Debug("uni");
        } 
        else if (headerData.messageType == INTER_PAN_BROADCAST) 
        {
            Debug("broad");
        } 
        else if (headerData.messageType == INTER_PAN_MULTICAST) 
        {
            Debug("multi");
        }
        Debug("cast):\r\n");
    
        receive_frame.frameType=headerData.messageType;
        receive_frame.dstPan=headerData.panId;
        MEMCOPY(receive_frame.dstPhyId,headerData.longAddress,8);
    
        //��ӡ��ͷ����
        // ԴPanID
        Debug("  source PAN ID:0x %x\r\n", headerData.panId);
        //Դ����ַ
        Debug("  src long addr:%2x%2x%2x%2x%2x%2x%2x%2x\r\n", 
                    headerData.longAddress[0], headerData.longAddress[1],
                    headerData.longAddress[2], headerData.longAddress[3],
                    headerData.longAddress[4], headerData.longAddress[5],
                    headerData.longAddress[6], headerData.longAddress[7]);
        //Դ�̵�ַ
        Debug(" source short ID :0x %x\r\n",headerData.shortAddress);
        //��ͷprofileID
        Debug("  profileId....:0x %x\r\n", headerData.profileId);
        //��ͷclusterID
        Debug("  clusterId....:0x %x\r\n", headerData.clusterId);
        //�㲥��groupId
        if (headerData.messageType == INTER_PAN_MULTICAST) 
        {
            Debug("  groupId......: %x\r\n", headerData.groupId);
        }
        // ��ȡ�ز����ݣ�����ʵ����
        length = emberMessageBufferLength(message);
        Debug("the message length is %d\n", length);
        appPayloadLength = length - appPayloadOffset;
        Debug("  payload (len %d) [0x", appPayloadLength); 
        for (i = appPayloadOffset; i < length; i ++) 
        {
            //��ӡ�յ�����ʵ����
            Debug("%2x ", emberGetLinkedBuffersByte(message, i));
            //�����ڴ˽��յ������ݷ�װ������֡
            //������������������
        }
        Debug("]\r\n"); 
   
        //#endif
        // ���յ�����ʵ���ݷ���inBuffer ����
        emberCopyFromLinkedBuffers(message, appPayloadOffset, &inBuffer, appPayloadLength);
    
        mobilCallbackHandler(&receive_frame,inBuffer);
        //network_info.incoming(0x10, (EmberApsFrame*)&headerData, message);           //0x10 is interpantype
    
        //����ACKȷ���յ���Ϣ
        /*if(emberGetLinkedBuffersByte(message, appPayloadOffset) != 97)
        {
            Debug("send the ack msg\n");
            sea_SendInterpanUnicast(headerData.shortAddress,headerData.panId,
                            headerData.profileId,headerData.clusterId,
                            0,headerData.longAddress,sizeof("a"),"a");
            //sea_SendInterPanMsg(INTER_PAN_UNICAST,headerData.panId,headerData.profileId,headerData.clusterId,headerData.longAddress,sizeof("a"),"a");
        }*/
    } 
    else 
    {
        //�յ�δ֪���ݵĴ�����
        length = emberMessageBufferLength(message);
        Debug("RX Inter-PAN (anonymous) [");
        for (i = 0x00; i < length; i ++) 
        {
            Debug("%x ",emberGetLinkedBuffersByte(message, i));
        }
        Debug("\r\n"); 
    }
#endif
}

/**@brief A callback invoked by the EmberZNet stack when the
 * MAC has finished transmitting a raw message.
 *
 * If the application includes this callback, it must define
 * EMBER_APPLICATION_HAS_RAW_HANDLER in its CONFIGURATION_HEADER.
 *
 * @param message  The raw message that was sent.
 * @param status   ::EMBER_SUCCESS if the transmission was successful,
 *                 or ::EMBER_DELIVERY_FAILED if not.
 */
void emberRawTransmitCompleteHandler ( EmberMessageBuffer message, EmberStatus status )
{
    Debug("\r\n call emberRawTransmitCompleteHandler()   status:0x%02x", status);  // 0x40:EMBER_MAC_NO_ACK_RECEIVED
}

#ifndef EMBER_APPLICATION_HAS_COUNTER_HANDLER
void emberCounterHandler ( EmberCounterType type, u8 data )
{
//    Debug("\r\ncounter handler type:%d, data:%d.", (u16)type, (u16)data);
    switch (type)
    {
        case EMBER_COUNTER_MAC_RX_BROADCAST:             // The MAC rececived a broadcast.
            break;
        case EMBER_COUNTER_MAC_TX_BROADCAST:             // The MAC transmitted a broadcast. 
            break;
        case EMBER_COUNTER_MAC_RX_UNICAST:               // The MAC received a unicast. 
            break;
        case EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS:       // The MAC successfully transmitted a unicast. 
            break;
        case EMBER_COUNTER_MAC_TX_UNICAST_RETRY:         // The MAC retried a unicast. 
            break;
        case EMBER_COUNTER_MAC_TX_UNICAST_FAILED:        // The MAC unsuccessfully transmitted a unicast. 
            break;
        case EMBER_COUNTER_APS_DATA_RX_BROADCAST:        // The APS layer rececived a data broadcast. 
            break;
        case EMBER_COUNTER_APS_DATA_TX_BROADCAST:        // The APS layer transmitted a data broadcast. 
            break;
        case EMBER_COUNTER_APS_DATA_RX_UNICAST:          // The APS layer received a data unicast. 
            break;
        case EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS:  // The APS layer successfully transmitted a data unicast.
            break;
        case EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY:    // The APS layer retried a data unicast.
            break;
        case EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED:   // The APS layer unsuccessfully transmitted a data unicast. 
            break;
        case EMBER_COUNTER_ROUTE_DISCOVERY_INITIATED:    // The network layer successfully submitted a new route discovery to the MAC. 
            break;
        case EMBER_COUNTER_NEIGHBOR_ADDED:               // An entry was added to the neighbor table. 
            break;
        case EMBER_COUNTER_NEIGHBOR_REMOVED:             // An entry was removed from the neighbor table. 
            break;
        case EMBER_COUNTER_NEIGHBOR_STALE:               // A neighbor table entry became stale because it had not been heard from. 
            break;
        case EMBER_COUNTER_JOIN_INDICATION:              // A node joined or rejoined to the network via this node. 
            break;
        case EMBER_COUNTER_CHILD_REMOVED:                // An entry was removed from the child table. 
            break;
        case EMBER_COUNTER_ASH_OVERFLOW_ERROR:           // EZSP-UART only. An overflow error occurred in the UART.
            break;
        case EMBER_COUNTER_ASH_FRAMING_ERROR:            // EZSP-UART only. A framing error occurred in the UART. 
            break;
        case EMBER_COUNTER_ASH_OVERRUN_ERROR:            // EZSP-UART only. An overrun error occurred in the UART.
            break;
        case EMBER_COUNTER_NWK_FRAME_COUNTER_FAILURE:    // NWK frame counter was not higher than the last message seen from that source. 
            break;
        case EMBER_COUNTER_APS_FRAME_COUNTER_FAILURE:    // APS frame counter was not higher than the last message seen from that source. 
            break;
        case EMBER_COUNTER_ASH_XOFF:                     // EZSP-UART only. An XOFF was transmitted by the UART. 
            break;
        case EMBER_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED:  // ASP layer the key is not authorized for use in APS data messages.
            break;
        case EMBER_COUNTER_NWK_DECRYPTION_FAILURE:       // A NWK encrypted message was received but dropped because decryption failed. 
            break;
        case EMBER_COUNTER_APS_DECRYPTION_FAILURE:       // An APS encrypted message was received but dropped because decryption failed. 
            break;
        case EMBER_COUNTER_TYPE_COUNT:                   // A placeholder giving the number of Ember counter types. 
            break;
    }
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

