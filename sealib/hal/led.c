
#include "config.h"
#include "ember.h"
#include "hal.h"

void halSetLed(HalBoardLed led)
{
  static volatile int32u *const clrRegs[] = 
    { (volatile int32u *)GPIO_PACLR_ADDR,
      (volatile int32u *)GPIO_PBCLR_ADDR,
      (volatile int32u *)GPIO_PCCLR_ADDR };
  if(led/8 < 3) {
    *clrRegs[led/8] = BIT(led&7);
  }
}

void halClearLed(HalBoardLed led)
{
  static volatile int32u *const setRegs[] = 
    { (volatile int32u *)GPIO_PASET_ADDR,
      (volatile int32u *)GPIO_PBSET_ADDR,
      (volatile int32u *)GPIO_PCSET_ADDR };
  if(led/8 < 3) {
    *setRegs[led/8] = BIT(led&7);
  }
}

void halToggleLed(HalBoardLed led)
{
  static volatile int32u *const outRegs[] = 
    { (volatile int32u *)GPIO_PAOUT_ADDR,
      (volatile int32u *)GPIO_PBOUT_ADDR,
      (volatile int32u *)GPIO_PCOUT_ADDR };
  //to avoid contention with other code using the other pins for other
  //purposes, we disable interrupts
  ATOMIC(
    if(led/8 < 3) {
      *outRegs[led/8] ^= BIT(led&7);
    }
  )
}

void halStackIndicateActivity(boolean turnOn)
{
  if(turnOn) {
    halSetLed(BOARD_ACTIVITY_LED);
  } else {
    halClearLed(BOARD_ACTIVITY_LED);
  }
}

// when app is not joined, the LEDs are
// 1: ON
// 2: ON
// 3: OFF
void appSetLEDsToInitialState(void)
{
#ifndef NO_LED
  halSetLed(BOARDLED1);
  halSetLed(BOARDLED3);
  halClearLed(BOARDLED2);
#endif//NO_LED
}

// when app is joined, the LEDs are
// 1: ON
// 2: alternating ON/OFF with 3
// 3: alternating ON/OFF with 2
void appSetLEDsToRunningState(void)
{
#ifndef NO_LED
  halToggleLed(BOARDLED3);
  halToggleLed(BOARDLED2);
#endif//NO_LED
}