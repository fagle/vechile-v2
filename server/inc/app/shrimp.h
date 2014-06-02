#include "stm32f10x.h"


#define BIT(x)  ((u32)0x01<<(x))

//#pragma pack(1)     //按一字节对齐

typedef struct 
{
   u8  id;
   u8  status;
   float data;    
}sensor_t;


typedef struct 
{
    u16 sof;
    u16 length;
    u16 id;
    u8 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8  passwd[5];
    u8  status;
    float temperature;
    float humidity;
    float voltage;
    u8  reserved[12];
    sensor_t sensor[5];
    u16 eof;
}shrimp_frame;
//#pragma pack()      //恢复缺省对齐

//#define MAX_BIG_STAGE      (0x04)
#define SHRIMP_FRM_LENGTH  (74)

extern u16 Ir_data[128];
extern u8 num;
extern u8 IrReceive_complete;


void shrimp_configuration(void);
void framebuffer2date(void);
u8 * get_shrimp_frame(void);
void IrReceive_Config(void);
void watersw_setbits(u32 bits);
void watersw_resetbits(u32 bits);
void watersw_setbits_bygroup(u8 group,u32 bits);
void watersw_resetbits_bygroup(u8 group,u32 bits);