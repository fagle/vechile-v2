#include "config.h"


#define NET_MODE  0x01
#define AUTO_MODE 0x02

#define AUTO_CHANGE_ENABLE 0x01

typedef struct
{
    u8 mode;
    u8 modeAutoChange;         // 1 auto change accord to network state, 0 don't change 
} controlMode_t;

typedef struct
{
    u8 netPercent;
    u8 autoPercent;
    u8 lastFinalPercent;
    u8 netLastOpenPercent;
} percent_t;

extern percent_t currentPercent;

void modeInit( void );


void modePercent( void );




extern controlMode_t currentMode;
