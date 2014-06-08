#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "network.h"

////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t w108frm1;

////////////////////////////////////////////////////////////////////////////////////
//
static void printCommandHelp ( void )
{
    sea_printf("\nw108 Usage: w108 -[codrxalinp] nn");
    sea_printf("\n  -o nn, start number of nn vehicle.");
    sea_printf("\n  -c nn, stop number of nn vehicle.");
    sea_printf("\n  -e nn, send end_line to nn vehicle.");
    sea_printf("\n  -b nn, send reboot to nn vehicle.");
    sea_printf("\n  -r n1 idx id act id act ..., set route table to vehicle.");
    sea_printf("\n  -n n1 num type, change vehicle from n1 to number and type.");
    sea_printf("\n  -i index mask, change vehicle radio index and mask.");
    sea_printf("\n  -a nn, change vehicle application type.");
    sea_printf("\n  -l nn, print nn route table.");
    sea_printf("\n  -d cc tt ss vv, dispatch vv vehicle to cc caller's station.");
    sea_printf("\n  -x mm bb, change vehicle max. and base devices.");
    sea_printf("\n  -p nn cc, change vehicle report period and config mode.");
}

void sea_w108config ( int argc, char * argv[] )
{
    s8  opt;
    int num;
    u8  tmp;
    frame_t  frm;

    if (argc < 0x02)
    {
        sea_printw108sysinfo();
	return;
    }

    if (argv[0x01][0x00] == '?' && argc == 0x02)
    {
        printCommandHelp();
        return;
    }
    
    while ((opt = sea_getopt(argc, argv, "coberxainpldsf1234g")) != -1) 
    {
        if (opt != 0x00 && cmdhd1.optarg != NULL)
        {
            switch (opt) 
            {
                case '1':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    sea_printf("\nsending ICHP_SV_OPEN");
                    w108frm1.put(&w108frm1, ICHP_SV_OPEN, 0x01, 0x00, &tmp);
                    break;
                }
                case '2':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    sea_printf("\nsending ICHP_SV_CLOSE");
                    w108frm1.put(&w108frm1, ICHP_SV_CLOSE, 0x01, 0x00, &tmp);
                    break;
                }
                case '3':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic && (argc & 0x01) == 0x00)
                    {
                        dyn_info.buffer[0x00] = tmp;                     // number of logic 
                        sscanf(argv[0x03], "%d", &num);           
                        dyn_info.buffer[0x01] = num & 0xff;              // index of route table
                        dyn_info.buffer[0x02] = (argc - 0x04) / 0x02;    // sum of route table
                        dyn_info.buffer[0x03] = (argc - 0x04) / 0x02;    // count of route table
                        for (u8 i = 0x00; i < argc - 0x04; i ++)
                        {
                            sscanf(argv[0x04 + i], "%d", &num);          // id and action
                            dyn_info.buffer[i + 0x04] = num & 0xff;
                        }
                        frm.cmd        = ICHP_PC_ROUTE;
                        frm.len        = argc;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        
                        sea_printf("\nlogic num %d",dyn_info.buffer[0x00]);
                        sea_printf("\naction num %d",(argc-4)/2);
                            
                        sea_memcpy(frm.body, dyn_info.buffer, frm.len);
                        
                        
                        if ((frm.len & 0x01) == 0x00)
                        {
                            if (rep_info.key[frm.body[0x00] - 0x01].vehicle.status == CAR_STOP)
                            {
                                msg_info.add(frm.body[0x00], frm.body[0x01], frm.body[0x03] * sizeof(action_t), &frm.body[0x04]);
                            }
                             else
                            {
                                sea_printf("status is not CAR_STOP");
                            }
                        }
                    }
                        
                    break;
                }
                case 'l':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    ppath_t ptr = msg_info.find(num);

                    if (ptr == NULL || ptr->cnt == 0x00)
                        sea_printf("\ncan not find table");
                    else
                    {
                        sea_printf("\nroute table of %d, cnt %d", tmp, ptr->cnt);
                        sea_printf("\n");
                        for (u8 i = 0x00; i < ptr->cnt; i ++)
                            sea_printf("%2x ", ptr->line[i].id);
                        sea_printf("\n");
                        for(u8 i = 0x00; i < ptr->cnt; i ++)
                            sea_printf("%2x ",ptr->line[i].action);
                    }
                    break;
                }
                case 'd':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    tmp = num & 0xff;  
                    if (dyn_info.addr[tmp - 0x01].logic)
                    {
                        frm.body[0x00] = tmp;                // caller's station number
                        sscanf(argv[0x03], "%d", &num);
                        frm.body[0x01] = num & 0xff;         // call vehicle type
                        sscanf(argv[0x04], "%d", &num);
                        frm.body[0x02] = num & 0xff;         // caller's status, waiting, assign, online
                        sscanf(argv[0x05], "%d", &num);
                        frm.body[0x03] = num & 0xff;         // assign vehicle number
                        frm.cmd        = ICHP_PC_ASSIGN;
                        frm.len        = 0x04;
                        frm.road       = dyn_info.addr[frm.body[0x00] - 0x01].logic;
                        ServerFrameCmdHandler(frm);
                    }
                    break;
                }
                case 's':
                {
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    tmp = num & 0xff;  //carid
                    
                    if (dyn_info.addr[tmp - 0x01].logic)
                    {
                        sea_printf("\nICHP_SV_ROUTE car id %d",num);
                        ppath_t rut = msg_info.find(num);
                        sea_sendroutetable(rut);
                        
                    }
                    else
                    {
                        sea_printf("\ncan not find car id");
                    }

                    break;
                }
                case 'o':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic) 
                    {
                        sea_printf("\nopen vehicle %d, logic address %d", tmp, dyn_info.addr[tmp - 0x01].logic);
                        frm.cmd        = ICHP_SV_OPEN;
                        frm.len        = 0x01;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        frm.body[0x00] = tmp;
                        ServerFrameCmdHandler(frm);
                    }
                    else
                        sea_printf("\nvehicle no logic address");
                    break;
                case 'c':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic) 
                    {
                        sea_printf("\nclose vehicle %d, logic address %d", tmp, dyn_info.addr[tmp - 0x01].logic);
                        frm.cmd        = ICHP_SV_CLOSE;
                        frm.len        = 0x01;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        frm.body[0x00] = tmp;
                        ServerFrameCmdHandler(frm);
                    }
                    else
                        sea_printf("\nvehicle no logic address");
                    break;
                case 'e':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic) 
                    {
                        sea_printf("\nsend end_line vehicle %d, logic address %d", tmp, dyn_info.addr[tmp - 0x01].logic);
                        frm.cmd        = ICHP_SV_END;
                        frm.len        = 0x01;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        frm.body[0x00] = tmp;
                        ServerFrameCmdHandler(frm);
                    }
                    else
                        sea_printf("\nvehicle no logic address");
                    break;
                case 'b':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic) 
                    {
                        sea_printf("\nsend reboot vehicle %d, logic address %d", tmp, dyn_info.addr[tmp - 0x01].logic);
                        frm.cmd        = ICHP_SV_REBOOT;
                        frm.len        = 0x01;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        frm.body[0x00] = tmp;
                        ServerFrameCmdHandler(frm);
                    }
                    else
                        sea_printf("\nvehicle no logic address");
                    break;
                case 'n':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic)
                    {
                        device_t dev;
                        u16 addr = dyn_info.addr[tmp - 0x01].logic;
                        sea_memset(&dev, 0x00, sizeof(device_t));
                        sscanf(argv[0x03], "%d", &num);
                        dev.num = num & 0xff;
                        sscanf(argv[0x04], "%d", &num);
                        dev.type = num & 0xff;
                        if (isCallDevice(dev.type, dev.num))
                        {
                            sea_printf("\nyou want to change call number %d to %d, type %d", tmp, dev.num, dev.type);
                            w108frm1.put(&w108frm1, ICHP_SV_ADDRESS, sizeof(device_t), addr, (u8 *)&dev);
                        }
                        else if (isCarDevice(dev.type, dev.num))
                        {
                            sea_printf("\nyou want to change vehiclde number %d to %d, type %d", tmp, dev.num, dev.type);
                            w108frm1.put(&w108frm1, ICHP_SV_ADDRESS, sizeof(device_t), addr, (u8 *)&dev);
                        }
                        else
                        {
                            sea_printf("\nyou want to change wrong number or type.");
                        }
#if 0                        
                        if (dev.num > 0x00 && dev.num < sys_info.ctrl.maxdev || 
                            addr != dyn_info.addr[dev.num - 0x01].logic)
                        {
                            sea_printf("\nyou want to change number %d to %d, type %d", tmp, dev.num, dev.type);
                            w108frm1.put(&w108frm1, ICHP_SV_ADDRESS, sizeof(device_t), addr, (u8 *)&dev);
                        }
#endif
                    }
                    break;
                case 'r':
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    tmp = num & 0xff;
                    if (dyn_info.addr[tmp - 0x01].logic && (argc & 0x01) == 0x00)
                    {
                        dyn_info.buffer[0x00] = tmp;                     // number of logic 
                        sscanf(argv[0x03], "%d", &num);           
                        dyn_info.buffer[0x01] = num & 0xff;              // index of route table
                        dyn_info.buffer[0x02] = (argc - 0x04) / 0x02;    // sum of route table
                        dyn_info.buffer[0x03] = (argc - 0x04) / 0x02;    // count of route table
                        for (u8 i = 0x00; i < argc - 0x04; i ++)
                        {
                            sscanf(argv[0x04 + i], "%d", &num);          // id and action
                            dyn_info.buffer[i + 0x04] = num & 0xff;
                        }
                        frm.cmd        = ICHP_PC_ROUTE;
                        frm.len        = argc;
                        frm.road       = dyn_info.addr[tmp - 0x01].logic;
                        sea_memcpy(frm.body, dyn_info.buffer, frm.len);
                        ServerFrameCmdHandler(frm);
                    }
                    break;
                case 'x':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= MAXLAMPS)
                    {
                        ctrl_t ctrl;
                        sea_memcpy(&ctrl, &sys_info.ctrl, sizeof(ctrl_t));
                        ctrl.maxdev = num;
                        sscanf(argv[0x03], "%d", &num);
                        if (num < ctrl.maxdev)
                        {
                            ctrl.base = num;
                            w108frm1.put(&w108frm1, ICHP_SV_CTRL, sizeof(ctrl_t), sys_info.ctrl.road, (u8 *)&ctrl);
                        }
                    }
                    break;
                case 'i':
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    tmp = num & 0xff;  
                    if (dyn_info.addr[tmp - 0x01].logic)
                    {
                        frm.body[0x00] = tmp;                // device number
                        sscanf(argv[0x03], "%d", &num);
                        frm.body[0x01] = num & 0xff;         // radio channel index
                        frm.cmd        = ICHP_SV_ADJUST;
                        frm.len        = 0x02;
                        frm.road       = dyn_info.addr[frm.body[0x00]  - 0x01].logic;
                        ServerFrameCmdHandler(frm);
                    }
                    break;
                default:
                    printCommandHelp();
                    return;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

