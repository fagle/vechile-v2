#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "serial.h"
#include "shrimp.h"
#include "usrtasks.h"
#include "frame.h"
#include "network.h"
#include "gprs.h"

frame_info_t gprsfrm1;

void ShrimpInit ( void )
{
#ifdef SHRIMP_MODE
    IrReceive_reset();
    framebuffer2date();
    shrimpstage_readfromflash();
#endif
}    

void ShrimpHandler ( void )
{
#ifdef SHRIMP_MODE
    u8 i, j;
    u8 IrDataHex[16];
    u8 bit = 0;

    shrimp_stage_handler();
    if(IrReceive_complete)
    {
        j = 0;
        memset(IrDataHex,0,16);
        sea_printf("Receive IrData:");
        for(i = 0; i < 128; i++)
            sea_printf("%d ",Ir_data[i]);
        sea_printf("\n");
        for(i = 1; i < 128; i=i+2)
        {
            if(Ir_data[i]>4400)
                sea_printf("Header ");
            else
            {
                if(Ir_data[i]>1500)
                {
                    sea_printf("1");
                    IrDataHex[j/8] = (IrDataHex[j/8]<<1) + 1;
                    j++;
                }
                else if(Ir_data[i]<700&&Ir_data[i]>400)
                {
                    sea_printf("0");
                    IrDataHex[j/8] = IrDataHex[j/8]<<1;
                    j++;
                }
                if(j%4==0)
                {
                    sea_printf(" ");
                }
            }
        }
        sea_printf("\n");
        sea_printf("IrData In Hex: ");
        for(i=0;i<j/8;i++)
            sea_printf("%02x ",IrDataHex[i]);
        sea_printf("\n");
        sea_printf("bit:%d\n",bit);
        if(bit<18)
        {//0 µ½ 100 µµÎ»
            watersw_setbits(BIT(bit));
            pwm_change((bit)/18*100);
            bit++;
        }
        else 
        {
            pwm_change((36 - bit)/18*100);
            watersw_resetbits(BIT(bit-18));
            bit++;
            if(bit==36)
                bit=0;
        }
        IrReceive_reset();
    }
#endif
}

void vGprsMsgTask ( void *pvParameters )
{
    ShrimpInit();
    
    while (1)
    {       
        ShrimpHandler();
        
        if (xSemaphoreTake(dyn_info.semgprs, portMAX_DELAY) == pdTRUE)
        {
            if (!GPRS_IsCommandEmpty(send1))
            {
                EGPRS id = GPRS_GetSendCommand(send1);
                pgprs_cmd_t ptr;
                
                ptr = sea_getgprscmd(id);
                GPRS_ResetResponse();
                if (ptr)
                {
                    switch (id)
                    {
                      case ePOST:
                        if (get_gprstatus(GPRSCONNECT))
                        {
#ifdef SHRIMP_MODE
                            u8 * rep_buffer;
                            rep_buffer = get_shrimp_frame();                            
                            shrimp_ReportFrame(SHRIMP_FRM_LENGTH, rep_buffer);
#endif                            
                            if (!sea_isreportempty())
                                GPRS_SendCommand(send1, ePOST);
                            else
                            {
                                GPRS_SendCommand(send1, eIPCLOSE1);
                                GPRS_SendCommand(send1, eIPCLOSE5);
                            }
                        }
                        else
                            if (!sea_isreportempty())
                            {                                
                                GPRS_ClearCommand(send1);
                                GPRS_Configuration();
                            }                           
                        break;
                      case eGET:
                        if (gprs_info.sim && get_gprstatus(GPRSCONNECT))
                            HTTP_IpsendGet();
                        else
                        {
                            GPRS_SendCommandFinish(send1);
                        }
                        break;
                      case eSYSCHTIME:
                        HTTP_ReportFrame(ICHP_SV_DATE, 0x00, sys_info.road, NULL);
                        break;
                      case eCGDCONT:
                        if(!get_gprstatus(GPRSCONNECT))
                        {
                            sea_printf("\n%s", ptr->cmd);
                            UartSendString(GPRS_COM, ptr->cmd);
                        }
                        break;
                      case eETCPIP:
                        if(!get_gprstatus(GPRSCONNECT))
                        {
                            sea_printf("\n%s", ptr->cmd);
                            UartSendString(GPRS_COM, ptr->cmd);
                        }
                        break;
                      case eIPOPEN:
                        if(!get_gprstatus(GPRSCONNECT))
                        {
                            sprintf((char *)gprs_info.cmd, (char const *)ptr->cmd, sys_info.ip[0], sys_info.ip[1], sys_info.ip[2], sys_info.ip[3], sys_info.port);
                            sea_printf("\n%s", gprs_info.cmd);
                            UartSendString(GPRS_COM, gprs_info.cmd);
                        }
                        break;
                      case eBAUD:
                        sprintf((char *)gprs_info.cmd, (char const *)ptr->cmd, USART5_BAUDRATE);
                        sea_printf("\n%s", gprs_info.cmd);
                        UartSendString(GPRS_COM, gprs_info.cmd);
                        break;
                      default:
                        sea_printf("\n%s", ptr->cmd);
                        UartSendString(GPRS_COM, ptr->cmd);
                        //sea_printf("\nrest %d cmd\n", send1->cnt-0x01);
                        break;
                    }                        
                    gprs_info.curid = id;
                }
            }
            else
                xSemaphoreGive(dyn_info.semgprs);
        }
        else
        {
            sea_printf("\ngprs task semaphore time out.");
            gprs_info.status = 0x00;                       // status reset, timeout;
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////