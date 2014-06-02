//
// card.h
// define the card function
// include the GPIO configure  
// @Author:王建华
// copyright 2013 by HDU Embeded Lab.
// All rights reserved.
//

#ifndef __CARD_EN
#define __CARD_EN
#include "stm32f10x_type.h"
#include "ember-types.h"
#include "lamp.h"

//extern single_t single_info;         // 单结点路灯

#define MAXCARDTAB 4

/*the card infomation*/
typedef struct
{
    u8   cardID;       // the card ID
    u8   cardInfo[3];  // the card info
} cardInfoTab;

//
// cardGPIOConfig
// configure the card gpio
// @param void
// @return void
//
void cardGPIOConfig( void );

//
// isCardExist
// accrording to the card Infomation table ,
// distinct the swipe card is exist
// @param cardID: the needed distinct cardID 
// @return 1 exist,else 0 not exist
//
u8 isCardExist(u8 cardID);

//
// findCard
// according to the card table,find the exist direction card 
// @param void
// @return if exist the card return the index in the card table
//         else return 0xFF
//
u8 findCard( void);


//
// cardBufTick
// 2 sec,erase the receive buffer from the swipe card 
// @param :void
// @return:void
//
void cardBufTick (void);

/** 
 * 
 * 
 * @param cardID
 * 
 * @return u8
 */
u8 isCardExist(u8 cardID);

#endif //__CARD_EN