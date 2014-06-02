// *******************************************************************
//  app-mobile.c
// *******************************************************************
#include "network.h"
#include "mobile.h"
#include "form-and-join.h"

DeviceInfo deviceInfoList[DEVICE_INFO_LIST_SIZE];
locate_t   curPosition={204048, 188840};

void init_DeviceInfo ( void )
{
  EmberNetworkParameters networkParams;
  if (emberGetNetworkParameters(&networkParams) == EMBER_SUCCESS)
  {
     default_DeviceInfo.ChannelId=networkParams.radioChannel;
     default_DeviceInfo.PanId=networkParams.panId;
  }
  else
  {
     default_DeviceInfo.ChannelId=sys_info.channel.channel;
     default_DeviceInfo.PanId=sys_info.channel.panid;
  }
  //MEMSET(&(default_DeviceInfo.PhyId),'1',EUI64_SIZE);
  MEMCOPY(&(default_DeviceInfo.PhyId),emberGetEui64(),EUI64_SIZE);
  default_DeviceInfo.DeviceId = single_info.num();
  default_DeviceInfo.NetStatus=0x01;
  default_DeviceInfo.DeviceStatus=0x01;
}

void updateDeviceInfo(void)
{
    EmberNetworkParameters networkParams;
    if (emberGetNetworkParameters(&networkParams) == EMBER_SUCCESS)
    {
       default_DeviceInfo.ChannelId=networkParams.radioChannel;
       default_DeviceInfo.PanId=networkParams.panId;
    }
    else
    {
       default_DeviceInfo.ChannelId=sys_info.channel.channel;
       default_DeviceInfo.PanId=sys_info.channel.panid;
    }
    
    //MEMSET(&(default_DeviceInfo.PhyId),'1',EUI64_SIZE);
    MEMCOPY(&(default_DeviceInfo.PhyId),emberGetEui64(),EUI64_SIZE);
    
    default_DeviceInfo.DeviceId=single_info.num();
    
    default_DeviceInfo.NetStatus=network_info.appstate;  // 0x00 EMBER_N0_NETWORK   0x02 EMBER_JOINED_NETWORK
    default_DeviceInfo.DeviceStatus=single_info.status();
}

void mobileInit(void)
{
  curDeviceInfoListIndex=0; 

}
void mobileAdjustChannel(int8u value)
{
  
  //  if(emberLeaveNetwork() == EMBER_SUCCESS)
  //  {
      network_info.params.radioChannel=value;
      network_info.params.panId=DEFAULT_MOBILE_PANID;
  //    if(emberFormNetwork(&network_info.params)==EMBER_SUCCESS)
 //     {
        sys_info.channel.panid   = DEFAULT_MOBILE_PANID;   
        sys_info.channel.channel = value; 
        Debug("The device in channel %d is ready\n",value);
        sea_updatesysconfig();
//      }

//    }
    
}
int8u checkSum(int8u * msg)
{
  int8u i=0;
  int8u *p=msg;
  int8u sum=0;
  int8u len=*(p+2);
  for(i=1;i<len+3;i++)
  {
    sum+=*(++p);
  }
  sum=sum&0xff;
  
  if(sum==*(p+1))
  {
    return 0x01;
  }
  return 0x00;
}

/*****************************************************
***���ܣ�*** ���������ַ�Ƿ��Ѿ���������
***������***
***���أ�***
******************************************************/
int checkInList ( EmberEUI64 longAddress )
{
    int _i;
    
    for(_i = 0x00; _i < DEVICE_INFO_LIST_SIZE; _i++)
    {
        if (!sea_memcomp(deviceInfoList[_i].PhyId, longAddress, EUI64_SIZE))
            return _i;
    }
    return DEVICE_INFO_LIST_SIZE; 
}

EmberEUI64* getPhyIdInList ( int16u deviceId )
{
    int _i;
    
    for (_i = 0x00; _i < DEVICE_INFO_LIST_SIZE; _i++)
    {
        if (deviceInfoList[_i].DeviceId == deviceId)
            return &deviceInfoList[_i].PhyId;
    }
    return NULL; 
}

int getInfoIndex(int16u deviceId)
{
   int _i=0;
  for(_i=0;_i<DEVICE_INFO_LIST_SIZE;_i++)
  {
    if(deviceInfoList[_i].DeviceId==deviceId)
      return _i;
  }
  return 0xFF; 
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void mobile_help()
//* ����        : print help information 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void mobile_help()
{
    Debug("\r\n[Mobile Device Command Help]");
    Debug("\r\n? = help");
    Debug("\r\n0 = print searched channel number around device");
    Debug("\r\n1 = print all lamp info around device by searching");
    Debug("\r\n2 = change target lamp id");   
    Debug("\r\n3 = adjust lamp position info ");
    Debug("\r\n4 = control target lamp and make it flicker ");
    Debug("\r\n5 = make target lamp stop flickering");
    Debug("\r\n6 = change target lamp status(open/close)");
    Debug("\r\n7 = print system config");
    Debug("\r\na = make target lamp rejion net");  
    Debug("\r\nb = make target lamp leave net");  
    Debug("\r\nt = test mobile frame");  
    Debug("\r\nd = init mobile device");  
    Debug("\r\ne = modify radio index of channel ");  
    Debug("\r\nf = modify radio index of channel panid");  
    Debug("\r\ng = make target lamp reboot");      
    Debug("\r\ni = request target lamp infomation");   
    Debug("\r\nl = lock/unlock the target lamp");    
    Debug("\r\ns = search all lamp info around device");
    /*if(1==menu_table.MenuCount)
    {
      lcd_clrscreen ( ); 
      lcd_printf("\n0 = search lamp info");
      lcd_printf("\n1 = all searched info ");
      lcd_printf("\n2 = change lamp id");     
      lcd_printf("\n3 = lamp flicker ");
      lcd_printf("\n4 = lock/unlock lamp");
      lcd_printf("\n5 = lamp stop flickering");
      lcd_printf("\n6 = change lamp status(open/close)");
      lcd_printf("\n7 = system config");     
    }*/
    /*if(2==menu_table.MenuCount)
    {
      lcd_clrscreen ( );
      lcd_printf("\n8 = reboot lamp");
      lcd_printf("\n9 = modify channel ");  
      lcd_printf("\n10 = modify panid");  
      lcd_printf("\n11 = adjust position info");      
      lcd_printf("\n12 = request lamp infomation");        
      lcd_printf("\n13 = lamp rejion net");
      lcd_printf("\n14 = lamp leave net");        
      lcd_printf("\n15 = reboot mobile");       
    }*/
    /*if(3==menu_table.MenuCount)
    {
       u8 temp;
       char LAMPID[]="";      
       for(temp=0;temp<DEVICE_INFO_LIST_SIZE;temp++)
       {
         if(0!=deviceInfoList[temp].DeviceId)
          {
            sprintf(LAMPID,"%d",deviceInfoList[temp].DeviceId);                 
            lcd_outtextxy (lcd_info.x,lcd_info.y,LAMPID);
            lcd_info.x=0x00;
            lcd_info.y++;
          }
      }
    }*/

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      :void mobilCallbackHandler(MobileCmdFrame *m_frame,int8u * msgbuffer)
//* ����        : ·�ƺ��ֳ��豸���յ�������֡�Ĵ���
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void mobilCallbackHandler(MobileCmdFrame *m_frame,int8u * msgbuffer)
{
  int8u data_buf[MAXDATASIZE];
  int8u *p=msgbuffer;
  int list_index=0;
  EmberStatus status;   

  sys_info_t lampBuffer;
  if(*p==FRAME_HEAD)
  {
    if(0x01==checkSum(p))
      {
        mobile_sendACK(m_frame,(p+1));
        m_frame->_frame.head=*p++;
        m_frame->_frame.cmd=*p++;
        m_frame->_frame.len=*p++;
        MEMCOPY(&data_buf,p,m_frame->_frame.len);
        p+=m_frame->_frame.len;
        m_frame->_frame.data=data_buf;
        m_frame->_frame.sum=*p++;
        m_frame->_frame.end=*p;
        //*pcmdBuf=m_frame->_frame.cmd;
        m_frame->frameType = INTER_PAN_UNICAST;
        switch(m_frame->_frame.cmd)
        {        
        case INTP_SEARCH:
          Debug("receive a INTP_SEARCH Command \n");
          m_frame->_frame.cmd=INTP_SEARCH_RES;
          m_frame->_frame.len=DEVICEINFOSIZE;
          
          updateDeviceInfo();
          
          m_frame->_frame.data=&default_DeviceInfo;
          Debug("the data address %08x\n",m_frame->_frame.data);
          mobile_sendMsg(m_frame);
        break;
        case INTP_SEARCH_RES:
          Debug("receive a INTP_SEARCH_RES Command \n");
          //lcd_printf("\nreceive INTP_SEARCH_RES");
          list_index=checkInList(m_frame->dstPhyId);
          
          if(list_index==DEVICE_INFO_LIST_SIZE)
          {
              MEMCOPY(&deviceInfoList[curDeviceInfoListIndex],m_frame->_frame.data,DEVICEINFOSIZE);
              //MEMCOPY(&deviceInfoList[curDeviceInfoListIndex].PhyId,m_frame->dstPhyId,8);
              curDeviceInfoListIndex=(curDeviceInfoListIndex+1)%DEVICE_INFO_LIST_SIZE;
          }
          else
          {
            MEMCOPY(&deviceInfoList[list_index],m_frame->_frame.data,DEVICEINFOSIZE);
            //MEMCOPY(&deviceInfoList[curDeviceInfoListIndex].PhyId,m_frame->dstPhyId,8);
          }
          
        break;
        case INTP_REJION_NET:
          Debug("receive a INTP_REJION_NET Command \n");     
          network_info.stackstate = emberNetworkState();
          Debug("previous status = %d",network_info.stackstate);
          if( network_info.stackstate ==EMBER_NO_NETWORK)
          {
          sensorCommonSetupSecurity();
          network_info.params.joinMethod  = EMBER_USE_NWK_REJOIN;      // added by renyu
          status = emberJoinNetwork(network_info.type, &network_info.params);
          Debug("status = %d",status);         
          if (status != EMBER_SUCCESS||EMBER_INVALID_CALL) 
          {
              mobile_senderr(m_frame,m_frame->_frame.cmd);
              Debug("\r\nerror returned from emberJoinNetwork: 0x%x", status);
          }
          else 
          {
              mobile_sendack(m_frame,m_frame->_frame.cmd);
              Debug( "\nwaiting for stack up ...");
          } 
          }
          else
          {
            Debug( "\nNow there is a network,so rejoin network is a fault ...");
          }
        break;
        case INTP_LEAVE_NET:
          Debug("receive a INTP_LEAVE_NET Command \n");
          emberLeaveNetwork();
          status = emberNetworkState();
          Debug("status = %d",status);
           if (status != EMBER_SUCCESS) 
          {
              mobile_senderr(m_frame,m_frame->_frame.cmd);
              Debug("\r\nerror returned from emberLeaveNetwork: 0x%x", status);
          }
          else 
          {
              mobile_sendack(m_frame,m_frame->_frame.cmd);
              Debug( "\nwaiting for stack up ...");
          }
        break;
        case INTP_UPDATE_LAMP_ID:          
          Debug("receive a INTP_UPDATE_LAMP_ID Command \n");
          u16 number;
          MEMCOPY(&number,m_frame->_frame.data,2);
          if (number&&number <= sys_info.ctrl.maxdev)
                {
                    sys_info.dev.num = number;
                    sea_updatesysconfig();
                    Debug("\r\nthe lamp number has been changed, new lamp number is %d,the lamp will reboot just now ...",
                               sys_info.dev.num);
                }
        break;
        case INTP_UPDATE_LAMP_STATUS:
          {
          u8 lampStatus=0x00;
          Debug("receive a INTP_UPDATE_LAMP_STATUS Command \n");
          MEMCOPY(&lampStatus,m_frame->_frame.data,1);
          if(lampStatus == 0x01)
          {  
          Debug("INTP_UPDATE_LAMP_STATUS :%d \n",lampStatus);
          single_info.lamp.light.percent = 0x00;
          adjustPWM(0);
          }
        else
          { 
          Debug("INTP_UPDATE_LAMP_STATUS :%d \n",lampStatus);
          single_info.lamp.light.percent = 100;
          adjustPWM(1000);
          }        
        break;
          }
        case INTP_UPDATE_LATITUDE:
          Debug("receive a INTP_UPDATE_LATITUDE Command \n");
          locate_t CurLamp;
          sea_memcpy(&CurLamp, m_frame->_frame.data, sizeof(locate_t));
          sys_info.loc.longitude = CurLamp.longitude;
          sys_info.loc.latitude  = CurLamp.latitude;
          sea_updatesysconfig();
          Debug("\r\nthe lamp position has been changed, \nnew lamp position's longitude is %d,\nnew lamp position's latitude is %d",
                     sys_info.loc.longitude,sys_info.loc.latitude);
        break;
        case INTP_REPORT_TO_COR:
          Debug("receive a INTP_REPORT_TO_COR Command \n");
        break;
        case INTP_FLICKER:
          Debug("receive a INTP_FLICKER Command \n");
          flicker_status=0x01;
          Debug("\r\nlamp start to flicker!\n");
          break;
        case INTP_STOP_FLICKER:
          Debug("receive a INTP_STOP_FLICKER Command \n");
          flicker_status=0x00;
          Debug("\r\nlamp stop to flicker!\n");
        break;
        case INTP_GET_INFO:
          Debug("receive a INTP_GET_INFO Command \n");
          m_frame->_frame.cmd=INTP_GET_INFO_RES;
          m_frame->_frame.len=sizeof(lamp_t);
          m_frame->_frame.data=&sys_info;         
          Debug("lamp infomation is:%d\n",sys_info.road);
          Debug("lamp number is:%d\n",sys_info.dev.num);
          mobile_sendMsg(m_frame);
        break;
        case INTP_GET_INFO_RES:
          //lcd_printf("receive INTP_GET_INFO_RES");         
          MEMCOPY(&lampBuffer,m_frame->_frame.data,sizeof(sys_info));
          Debug("lamp infomation is:%d\n",lampBuffer.dev.num);   
        break;
        case INTP_REBOOT_LAMP:
          Debug("receive a INTP_REBOOT_LAMP Command \n"); 
          halReboot();
          break;
        case INTP_ACK:
          MEMCOPY(&cmdBuf,m_frame->_frame.data,1);
          Debug("receive a INTP_ACK Command ,the Origanl Command is %08x\n",cmdBuf);
          //lcd_printf("\nreceive INTP_ACK ");
          
        break;
        case INTP_ERR:
          MEMCOPY(&cmdBuf,m_frame->_frame.data,1);
          Debug("receive a INTP_ERR Command \n");
        break;
        default:
        break;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus mobile_sendACK(MobileCmdFrame *frame,int8u *cmd)
//* ����        : ����ACK����
//* �������    : ����֡������ֵ
//* �������    : ���ͷ���״̬
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
EmberStatus mobile_sendACK(MobileCmdFrame *frame,int8u *cmd)
{
  Debug("the cmd is %08x\n",*cmd);
  if(INTP_ACK==*cmd || INTP_ERR==*cmd  )
  {
  return 0;
  }
  frame->frameType=INTER_PAN_UNICAST;
  frame->_frame.head=FRAME_HEAD;
  frame->_frame.cmd=INTP_ACK;
  frame->_frame.len=0x01;
  frame->_frame.data=cmd;
  frame->_frame.sum=0x00;
  frame->_frame.end=FRAME_END;
  return mobile_sendMsg(frame);

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus mobile_senderr(MobileCmdFrame *frame,int8u *cmd)
//* ����        : ����INTP_ERR����
//* �������    : ����֡������ֵ
//* �������    : ���ͷ���״̬
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
 EmberStatus mobile_senderr(MobileCmdFrame *frame,u8 cmd)
{  
    frame->frameType=INTER_PAN_UNICAST;
    frame->_frame.head=FRAME_HEAD;
    frame->_frame.cmd=INTP_ERR;
    frame->_frame.len=0x01;
    frame->_frame.data=&cmd;
    frame->_frame.sum=0x00;
    frame->_frame.end=FRAME_END;
    return mobile_sendMsg(frame);

}

EmberStatus mobile_sendack(MobileCmdFrame *frame,u8 cmd)
{  
  
    frame->frameType=INTER_PAN_UNICAST;
    frame->_frame.head=FRAME_HEAD;
    frame->_frame.cmd=INTP_ACK;
    frame->_frame.len=0x01;
    frame->_frame.data=&cmd;
    frame->_frame.sum=0x00;
    frame->_frame.end=FRAME_END;
    return mobile_sendMsg(frame);

}



/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : EmberStatus mobile_sendMsg(MobileCmdFrame *frame)
//* ����        : ��������֡
//* �������    : ����֡
//* �������    : ���ͷ���״̬
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
EmberStatus mobile_sendMsg(MobileCmdFrame *frame)   
{
  int8u _buffer[INTERPANMAXSIZE];
  int8u *_p=_buffer;
  int8u _i=0;
  int8u _sum=0;
  MEMCOPY(_p,&(frame->_frame),3);
  _p+=3;
  MEMCOPY(_p,frame->_frame.data,frame->_frame.len);
  _p+=frame->_frame.len;
  for(_i=1;_i<frame->_frame.len+3;_i++)
  {
    _sum+=_buffer[_i];
  }
  _sum=_sum&0xff;
  *_p++=_sum;
  *_p=frame->_frame.end;
 // Debug("Send a InterPan Message ,the type is %08x,the Dest panId is %08x, the Phy Address is ",frame->frameType,frame->dstPan);

  return sea_SendInterPanMsg(frame->frameType,
                      frame->dstPan,
                      0x00, 
                      0x00,
                      frame->dstPhyId,
                      frame->_frame.len+5, 
                      _buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void MobileFormNetwork(void)
//* ����        : �ֳ��豸����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void MobileFormNetwork(void)
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
#ifdef USE_HARDCODED_NETWORK_SETTINGS                              // Bring the network up.++�̻��������
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
#else                                                               // ++ɨ��ȷ���������
        network_info.method = SINK_USE_SCAN_UTILS;                  // set the mode we are using
        Debug("\r\nFORM : scanning for an available channel and panid\r\n");  // tell the user what is going on
        emberScanForUnusedPanId(EMBER_ALL_802_15_4_CHANNELS_MASK, 0x05);      // Use a function from app/util/common/form-and-join.c that scans 
                                                                              // and selects a quiet channel to form on. Once a PAN id and channel
                                                                              // are chosen, calls emberUnusedPanIdFoundHandler.
#endif 
  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void MobileTick ( void )
//* ����        : Mobile tick handler.
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void MobileTick ( void )
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void MobileExceptionHandler(void)
//* ����        : �ֳ��豸���쳣״���Ĵ���
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : 
//*------------------------------------------------*/
void MobileExceptionHandler(void)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void  MobileTimeEventHandler ( void )
//* ����        : mobile��Ϊ·��ʵ�ֿ�/�صƹ���,�����Ҫ�޸��Լ�������;
//*             : mobile�ϱ�FULL��Ϣ��
//*             : mobile�ϱ�KEY��Ϣ��
//*             : ++comment by ray:ONESECOND����һ��
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void  MobileTimeEventHandler ( void )
{
  if (0x01==flicker_status)
  {    
    static int8u timer = 0x01;   
    if ((single_info.time.sec % timer) == 0x00 && single_info.sec != single_info.time.sec)   // 5 minute
    {
        single_info.sec = single_info.time.sec;
        if((single_info.time.sec % (2*timer)) == 0x00)
        {         
          single_info.lamp.light.percent=0;
          adjustPWM(0);
        }
        else
        {         
          single_info.lamp.light.percent = 100;
          adjustPWM(1000);
        }        
    }        
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void MobileInit(void)
//* ����        : ��ʼ��mobile�ڵ���Ϣ
//*             : 
//*             : ������޽ڵ����
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : // init common state for sensor and sink nodes
//*------------------------------------------------*/
void MobileInit(void)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void MobileCommandHandler ( void )
//* ����        : ��������Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void MobileCommandHandler(void)
{

}


void ezspMobileIncomingMessageHandler ( EmberIncomingMessageType type, EmberApsFrame *apsFrame, EmberMessageBuffer message )
{

}
void ezspMobileMessageSentHandler ( EmberOutgoingMessageType type,
                                    int16u indexOrDestination,
                                    EmberApsFrame *apsFrame,
                                    EmberMessageBuffer message,
                                    EmberStatus status )
{

}
void emberMobileStackStatusHandler ( EmberStatus status )
{

}
