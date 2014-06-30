#include "network.h"

static canfrm_t out_frame, in_frame;

/*******************************************************************************
* Function Name  : CAN_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_Configuration ( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;     // Configure CAN pin: RX 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;      // Configure CAN pin: TX 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CAN_INIT(); 
}

/*******************************************************************************
* Function Name  : CAN_INIT ( void )
* Description    : initialize can information.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_INIT ( void )
{
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;

    CAN_DeInit(CAN1);          // CAN register init 
    CAN_StructInit(&CAN_InitStructure);

    CAN_InitStructure.CAN_TTCM = DISABLE;    // CAN cell init 
    CAN_InitStructure.CAN_ABOM = DISABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = ENABLE;//DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;  //CAN_Mode_Normal;  CAN_Mode_LoopBack

    CAN_InitStructure.CAN_SJW  = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1  = CAN_BS1_8tq;
    CAN_InitStructure.CAN_BS2  = CAN_BS2_7tq;
    CAN_InitStructure.CAN_Prescaler = 18;   // 125kbps =36M/((1+8+7)*18)
    CAN_Init(CAN1, &CAN_InitStructure);

    CAN_FilterInitStructure.CAN_FilterNumber         = 0x00;    // CAN filter init 
    CAN_FilterInitStructure.CAN_FilterMode           = CAN_FilterMode_IdMask;//CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale          = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh         = 0x0000;//0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow          = 0x0000;//
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);        // CAN FIFO0 message pending interrupt enable 
}

/*******************************************************************************
* Function Name  : void CAN_RxMessage ( void ) 
* Description    : can receive massage.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_RxMessage ( void ) 
{
    CanRxMsg RxMessage;

    sea_memset((void *)&RxMessage, 0x00, sizeof(CanRxMsg));
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    
//    if ((RxMessage.ExtId == CANRECVID) && (RxMessage.IDE == CAN_ID_EXT))
    {
        sea_memcpy(&in_frame, RxMessage.Data, CANFRAMELEN);
        in_frame.speed = (in_frame.speed >> 0x08 | in_frame.speed << 0x08) & 0xffff;
        in_frame.left  = (in_frame.left >> 0x08 | in_frame.left << 0x08) & 0xffff;
        in_frame.right = (in_frame.right >> 0x08 | in_frame.right << 0x08) & 0xffff;
        sea_printf("\nExtId, IDE: %04x, %04d", RxMessage.ExtId, RxMessage.IDE);
        sea_printf("\ntype, cmd, speed left and right: %02x, %02x, %04x, %04x, %04x", 
                   in_frame.type, in_frame.cmd, in_frame.speed, in_frame.left, in_frame.right); 
    }        
}

/*******************************************************************************
* Function Name  : void CAN_TxMessage ( frame_t * ptr ) 
* Description    : can send frame massage.
* Input          : frame_t * ptr
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_TxMessage ( canfrm_t * ptr )
{
    CanTxMsg TxMessage;
    u16      retry = CANRETRY;
    u8       TransmitMailbox;
  
    TxMessage.StdId = STDSENDID;      // 0x01e0;
    TxMessage.ExtId = CANSENDID;      // 0x5a5;
    TxMessage.RTR   = CAN_RTR_DATA;
    TxMessage.IDE   = CAN_ID_STD;       //2014Äê6ÔÂ27ÈÕ 14:41:40   CAN_ID_EXT
    sea_memcpy(TxMessage.Data, (char *)ptr, CANFRAMELEN);
    TxMessage.DLC   = CANFRAMELEN;

    sea_printf("\nsend frame ");
    taskENTER_CRITICAL();
    while (-- retry)
    {
        TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);
        if (CAN_TransmitStatus(CAN1, TransmitMailbox) == CANTXOK)
            break;
    }
    taskEXIT_CRITICAL();
    sea_printf("%s.", retry ? "ok" : "fail");
}

/*******************************************************************************
* Function Name  : void sea_canprint ( u8 cmd, u8 type, u16 speed, u16 left, u16 right )
* Description    : can send frame massage.
* Input          : u8 cmd, u8 type, u16 speed, u16 left, u16 right
* Output         : None
* Return         : None
*******************************************************************************/
void sea_canprint ( u8 cmd, u8 type, u16 speed, u16 left, u16 right )
{
    out_frame.cmd   = cmd;
    out_frame.type  = type;
    out_frame.speed = speed;
    out_frame.left  = left;
    out_frame.right = right;
    
    CAN_TxMessage(&out_frame);
}

////////////////////////////////////////////////////////////////////////////////
